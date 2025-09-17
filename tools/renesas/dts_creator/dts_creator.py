"""Generate Renesas RZ DTS files from configuration and C parameter sources."""

import argparse
import ast
import operator
import re
import sys
from pathlib import Path

import yaml


MARKER_PREFIX = "/* Add "
MARKER_SUFFIX = " here. */"


def format_marker(name):
    return f"{MARKER_PREFIX}{name}{MARKER_SUFFIX}"


sys.dont_write_bytecode = True


class Parser:
    """Lightweight C-like parser helpers used by DTS creator."""

    # Matches `#define NAME value` while tolerating inline C / C++ comments.
    _DEFINE_RE = re.compile(
        r"^\s*#\s*define\s+(?P<name>[A-Za-z0-9_]+)\s+(?P<value>.+?)\s*"
        r"(?:/\*.*\*/\s*)?(?://.*)?$"
    )
    # Captures decimal / hexadecimal literals with optional unsigned suffixes.
    _NUM_RE = re.compile(
        r"^(?:\(|\s*)?(0x[0-9A-Fa-f]+|\d+)(?:U|UL|L)?(?:\)|\s*)$"
    )
    _C_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
    _CPP_COMMENT_RE = re.compile(r"//.*?(?=\n|$)")
    # Used to tokenize identifiers and numeric literals inside initializer blocks.
    _TOKEN_RE = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*|0x[0-9A-Fa-f]+|\d+)\b")

    def parse_macros_from_headers(self, headers):
        """Parse numeric macros from headers without invoking a C preprocessor.

        Supports lines in the form:
          #define NAME (0x1234)
          #define NAME 0x1234
          #define NAME 1234

        Returns a mapping NAME -> integer value.
        """
        macros = {}
        pending = []
        for header in headers:
            if not header.is_file():
                continue
            lines = header.read_text(encoding="utf-8", errors="ignore").splitlines()
            i = 0
            while i < len(lines):
                line = lines[i]
                m = self._DEFINE_RE.match(line)
                if not m:
                    i += 1
                    continue
                name = m.group("name")
                val_text = m.group("value").rstrip()
                while val_text.endswith("\\") and i + 1 < len(lines):
                    i += 1
                    continuation = lines[i].strip()
                    val_text = val_text[:-1] + " " + continuation
                expr = self._clean_macro_expr(val_text)
                literal = self._parse_numeric_literal(expr) if expr else None
                if literal is not None:
                    macros[name] = literal
                else:
                    pending.append((name, expr))
                i += 1

        # Resolve complex expressions iteratively
        unresolved = pending
        while unresolved:
            next_round = []
            progress = False
            for name, expr in unresolved:
                if not expr:
                    continue
                value = self._try_eval_macro_expr(expr, macros)
                if value is None:
                    next_round.append((name, expr))
                    continue
                macros[name] = value
                progress = True
            if not progress:
                break
            unresolved = next_round
        return macros

    def _clean_macro_expr(self, expr):
        expr = self._strip_comments(expr)
        return expr.replace("\\", " ").strip()

    def _strip_comments(self, text):
        text = self._C_COMMENT_RE.sub(" ", text)
        text = self._CPP_COMMENT_RE.sub(" ", text)
        return text

    def extract_array_initializer(self, source, symbol):
        """Return the raw initializer string (inside braces) for `symbol`.

        Works for arrays declared as either:
          const uint32_t name[N];
          const uint32_t name[N][2];
          const struct { ... } name[];
        We don't implement a full C parser; this is a best-effort regex that
        handles the code style used by ATF parameter files.
        """
        txt = self._strip_comments(
            source.read_text(encoding="utf-8", errors="ignore")
        )
        return self._find_initializer_in_text(txt, str(source), symbol)

    def extract_array_initializer_from_text(
        self, text, symbol, source_name="<memory>"
    ):
        """Variant of :meth:`extract_array_initializer` for cached strings."""
        txt = self._strip_comments(text)
        return self._find_initializer_in_text(txt, source_name, symbol)

    def _find_initializer_in_text(self, txt, source_name, symbol):
        pat = re.compile(
            r"\b" + re.escape(symbol) + r"\s*(?:\[[^\]]*\]\s*)+=\s*\{",
            re.MULTILINE,
        )
        m = pat.search(txt)
        if not m:
            pat2 = re.compile(r"\b" + re.escape(symbol) + r"\s*=\s*\{", re.MULTILINE)
            m = pat2.search(txt)
            if not m:
                raise ValueError(f"symbol {symbol} not found in {source_name}")

        i = m.end()  # position after the opening '{'
        depth = 1
        j = i
        while j < len(txt) and depth > 0:
            if txt[j] == '{':
                depth += 1
            elif txt[j] == '}':
                depth -= 1
            j += 1
        if depth != 0:
            raise ValueError(
                f"unbalanced braces when parsing {symbol} in {source_name}"
            )
        return txt[i : j - 1]

    def _sanitize_numeric_suffixes(self, expr):
        """Remove common integer suffixes to simplify later expression parsing."""
        expr = re.sub(r"\bU\(", "(", expr)
        expr = re.sub(r"(?<=0x[0-9A-Fa-f])([uUlL]+)", "", expr)
        expr = re.sub(r"(?<=\d)([uUlL]+)", "", expr)
        return expr

    def _safe_eval_expression(self, expr):
        """Evaluate a restricted expression consisting of numeric and bitwise ops."""
        tree = ast.parse(expr, mode="eval")

        def eval_node(node):
            if isinstance(node, ast.Expression):
                return eval_node(node.body)
            if isinstance(node, ast.Constant):
                if isinstance(node.value, (int, bool)):
                    return int(node.value)
                raise ValueError("unsupported constant")
            if isinstance(node, ast.UnaryOp) and isinstance(
                node.op, (ast.UAdd, ast.USub, ast.Invert)
            ):
                operand = eval_node(node.operand)
                if isinstance(node.op, ast.UAdd):
                    return +operand
                if isinstance(node.op, ast.USub):
                    return -operand
                if isinstance(node.op, ast.Invert):
                    return ~operand
            if isinstance(node, ast.BinOp) and isinstance(
                node.op,
                (
                    ast.BitOr,
                    ast.BitAnd,
                    ast.BitXor,
                    ast.LShift,
                    ast.RShift,
                    ast.Add,
                    ast.Sub,
                ),
            ):
                left = eval_node(node.left)
                right = eval_node(node.right)
                op_map = {
                    ast.BitOr: operator.or_,
                    ast.BitAnd: operator.and_,
                    ast.BitXor: operator.xor,
                    ast.LShift: operator.lshift,
                    ast.RShift: operator.rshift,
                    ast.Add: operator.add,
                    ast.Sub: operator.sub,
                }
                return op_map[type(node.op)](left, right)
            raise ValueError("unsupported expression node")

        return eval_node(tree)

    def _try_eval_macro_expr(self, expr, macros):
        """Attempt to evaluate a macro expression given known macro values."""
        cleaned = self._sanitize_numeric_suffixes(expr)
        identifiers = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
        unresolved = []

        def repl(match):
            name = match.group(0)
            if name in macros:
                return str(macros[name])
            if name in {"UL", "U", "L"}:
                return ""
            unresolved.append(name)
            return name

        substituted = identifiers.sub(repl, cleaned)
        if unresolved:
            return None
        try:
            return self._safe_eval_expression(substituted)
        except Exception:
            return None

    def _eval_token(self, tok, macros):
        if tok.startswith("0x") or tok.isdigit():
            return int(tok, 0)
        if tok in macros:
            return macros[tok]
        raise KeyError(tok)

    def parse_flat_u32_list(self, initializer, macros):
        """Parse a C array initializer of scalars into a list of integers."""
        items = []
        flat = initializer.replace('{', ' ').replace('}', ' ').replace(',', ' ')
        for m in self._TOKEN_RE.finditer(flat):
            tok = m.group(1)
            try:
                items.append(self._eval_token(tok, macros))
            except KeyError:
                continue
        return items

    def parse_pairs_list(self, initializer, macros):
        """Parse a C array initializer of '{ A, B }' pairs into a list of (A, B)."""
        pairs = []
        chunks = [c.strip() for c in initializer.split('},')]
        for c in chunks:
            if not c:
                continue
            c = c.replace('{', ' ').replace('}', ' ').replace(',', ' ')
            toks = [t for t in self._TOKEN_RE.findall(c)]
            if not toks:
                continue
            try:
                a = self._eval_token(toks[0], macros)
                b = self._eval_token(toks[1], macros) if len(toks) > 1 else None
                if b is None:
                    b = 0
                pairs.append((a, b))
            except KeyError:
                continue
        return pairs

    def _parse_numeric_literal(self, expr):
        text = expr.strip()
        m = self._NUM_RE.match(text)
        if not m:
            return None
        try:
            return int(m.group(1), 0)
        except ValueError:
            return None


PARSER = Parser()


class DtsCreator:
    """Generate DTS output from YAML configuration and C source files."""

    DDR_TYPE_VALUES = {
        "ddr4": 0,
        "lpddr4": 1,
    }

    DDR_PAIR_PROPS = (
        "mc_init_tbl",
        "swizzle_mc_tbl",
        "mc_phy_settings_tbl",
        "swizzle_phy_tbl",
        "param_setup_mc",
        "param_phyinit_c",
        "param_phyinit_swizzle",
        "param_phyinit_f_1d_0",
        "param_phyinit_f_2d_0",
        "param_phyinit_i",
    )

    DDR_FLAT_PROPS = (
        "mc_odt_pins_tbl",
        "mc_mr1_tbl",
        "mc_mr2_tbl",
        "mc_mr5_tbl",
        "mc_mr6_tbl",
        "phyinit_1d",
        "phyinit_2d",
        "retention_phyreglist_1d",
        "retention_phyreglist_2d",
        "retention_mcreglist",
    )

    def __init__(self, cfg, repo_root):
        self.cfg = cfg
        self.repo_root = repo_root
        self._source_cache = {}

    def fmt_hex(self, val):
        return f"0x{val:08x}"

    def indent_of(self, line):
        return line[: len(line) - len(line.lstrip())]

    def replace_marker_block(self, template, marker, block):
        lines = template.splitlines()
        for i, ln in enumerate(lines):
            if marker in ln:
                ind = self.indent_of(ln)
                adj_block = "\n".join((ind + b if b else b) for b in block.splitlines())
                lines[i] = adj_block
                return "\n".join(lines)
        return template

    def gen_property_block(self, name, pairs):
        body = "\n".join(
            f"\t{self.fmt_hex(a)} {self.fmt_hex(b)}" for a, b in pairs
        )
        return f"{name} = <\n{body}\n>;"

    def gen_scalar_property(self, name, value):
        return f"{name} = <{self.fmt_hex(value)}>;"

    def gen_flat_list_property(self, name, values):
        if not values:
            return f"{name} = <>;"
        body = "\n".join(f"\t{self.fmt_hex(v)}" for v in values)
        return f"{name} = <\n{body}\n>;"

    def _dedupe_preserve_order(self, items):
        """Return items without duplicates while preserving their original order."""
        seen = set()
        result = []
        for item in items:
            if item in seen:
                continue
            seen.add(item)
            result.append(item)
        return result

    def _resolve_template_path(self):
        cfg_template = self.cfg.get("template")
        if cfg_template:
            return self.repo_root / cfg_template

        soc = self.cfg.get("soc")
        if not soc:
            raise ValueError("config missing 'soc'")

        candidate = (
            self.repo_root
            / "tools"
            / "renesas"
            / "dts_creator"
            / "template"
            / f"{soc}_template.dts"
        )
        if candidate.is_file():
            return candidate
        raise FileNotFoundError(f"template for SoC '{soc}' not found at {candidate}")

    def _resolve_output_path(self):
        out_rel = self.cfg.get("output")
        if not out_rel:
            soc = self.cfg.get("soc", "unknown")
            board = self.cfg.get("board", "board")
            out_rel = f"fdts/{soc}-{board}.dts"
        return self.repo_root / out_rel

    def _soc_subdir(self, soc):
        return soc[2:] if soc.startswith("rz") and len(soc) > 2 else soc

    def _auto_header_paths(
        self, ddr_cfg, spi_cfg=None
    ):
        """Return headers to scan for macros, including optional SPI sources."""

        headers = []
        user_headers = ddr_cfg.get("include_headers")
        if user_headers:
            return [self.repo_root / h for h in user_headers]

        soc = self.cfg.get("soc", "")
        soc_dir = self._soc_subdir(soc)
        candidate_dirs = [
            self.repo_root / "plat" / "renesas" / "rz" / "soc" / soc_dir / "include",
            self.repo_root / "plat" / "renesas" / "rz" / "soc" / "cmn" / "include",
            self.repo_root / "plat" / "renesas" / "rz" / "common" / "include",
        ]
        for directory in candidate_dirs:
            if not directory.is_dir():
                continue
            for header in sorted(directory.glob("*.h")):
                headers.append(header)
        extra_headers = []
        spi_cfg = spi_cfg or {}
        protocol = str(spi_cfg.get("protocol", "")).lower()
        if protocol == "spi_multi":
            spi_base = (
                self.repo_root
                / "plat"
                / "renesas"
                / "rz"
                / "common"
                / "include"
                / "drivers"
                / "spi_multi"
            )
            for rel in [
                "spi_multi.h",
                "spi_multi_regs.h",
                "spi_multi_regs_offset.h",
            ]:
                extra_headers.append(spi_base / rel)
            flash_chip = spi_cfg.get("flash_chip")
            if flash_chip:
                extra_headers.append(
                    spi_base / str(flash_chip) / "spi_multi_reg_values.h"
                )
        headers.extend(extra_headers)
        return self._dedupe_preserve_order(headers)

    def _gather_source_paths(self, ddr_cfg):
        """Collect relevant DDR source files referenced by the configuration."""

        paths = []
        for key in ("mc_param_file", "swizzle_file"):
            rel = ddr_cfg.get(key)
            if rel:
                paths.append(self.repo_root / rel)

        for key in ("ddr sources", "ddr_sources", "sources"):
            rel_list = ddr_cfg.get(key) or []
            for rel in rel_list:
                paths.append(self.repo_root / rel)

        existing = (path for path in paths if path.is_file())
        return self._dedupe_preserve_order(existing)

    def _load_source_text(self, path):
        if path not in self._source_cache:
            self._source_cache[path] = path.read_text(encoding="utf-8", errors="ignore")
        return self._source_cache[path]

    def _find_symbol_initializer(self, symbol, sources):
        for source in sources:
            try:
                text = self._load_source_text(source)
            except OSError:
                continue
            try:
                return PARSER.extract_array_initializer_from_text(text, symbol, str(source))
            except ValueError:
                continue
        return None

    def _extract_pairs(
        self, symbol, macros, sources
    ):
        initializer = self._find_symbol_initializer(symbol, sources)
        if initializer is None:
            return []
        return PARSER.parse_pairs_list(initializer, macros)

    def _extract_flat(
        self, symbol, macros, sources
    ):
        initializer = self._find_symbol_initializer(symbol, sources)
        if initializer is None:
            return []
        return PARSER.parse_flat_u32_list(initializer, macros)

    def _has_marker(self, template, name):
        return format_marker(name) in template

    def _apply_ddr_type(self, template, ddr_type):
        """Update an existing `ddr_type` assignment in the template, if present."""

        value = self.DDR_TYPE_VALUES.get(ddr_type)
        if value is None:
            return template, None

        def repl(match):
            return f"{match.group(1)}{value}{match.group(3)}"

        pattern = re.compile(r"(ddr_type\s*=\s*<)(0x[0-9a-fA-F]+|\d+)(>\s*;)")
        updated, count = pattern.subn(repl, template, count=1)
        return (updated if count else template), value

    def _apply_ddr_settings(
        self,
        template,
        macros,
        ddr_cfg,
        source_paths,
    ):
        """Populate DDR markers (type, tables, registers) from parsed sources."""
        raw_type = ddr_cfg.get("type")
        ddr_type = str(raw_type).lower() if raw_type else ""
        template, type_value = self._apply_ddr_type(template, ddr_type)

        if type_value is None:
            if self._has_marker(template, "ddr_type"):
                raise ValueError(
                    "Unsupported DDR type '{raw}' (expected 'ddr4' or 'lpddr4')".format(
                        raw=raw_type or "<unset>"
                    )
                )
        else:
            if self._has_marker(template, "ddr_type"):
                block = self.gen_scalar_property("ddr_type", type_value)
                template = self.replace_marker_block(
                    template, format_marker("ddr_type"), block
                )

        ddrmc_to_val = {
            name.lower(): val for name, val in macros.items() if name.startswith("DDRMC_R")
        }

        ddr_marker_pattern = (
            re.escape(MARKER_PREFIX) + r"(ddrmc_r\d{3})" + re.escape(MARKER_SUFFIX)
        )
        for register in re.findall(ddr_marker_pattern, template):
            addr = ddrmc_to_val.get(register)
            if addr is None:
                continue
            block = self.gen_scalar_property(register, addr)
            template = self.replace_marker_block(
                template, format_marker(register), block
            )

        if self._has_marker(template, "mc_init_num"):
            mc_init_num = macros.get("MC_INIT_NUM")
            if mc_init_num is not None:
                block = self.gen_scalar_property("mc_init_num", mc_init_num)
                template = self.replace_marker_block(
                    template, format_marker("mc_init_num"), block
                )

        for prop in self.DDR_PAIR_PROPS:
            marker = format_marker(prop)
            if marker not in template:
                continue
            pairs = self._extract_pairs(prop, macros, source_paths)
            if not pairs:
                continue
            block = self.gen_property_block(prop, pairs)
            template = self.replace_marker_block(template, marker, block)

        for prop in self.DDR_FLAT_PROPS:
            marker = format_marker(prop)
            if marker not in template:
                continue
            values = self._extract_flat(prop, macros, source_paths)
            if not values:
                continue
            block = self.gen_flat_list_property(prop, values)
            template = self.replace_marker_block(template, marker, block)

        return template

    def _get_supported_spi_multi_flash_chips(self):
        spi_base = (
            self.repo_root
            / "plat"
            / "renesas"
            / "rz"
            / "common"
            / "include"
            / "drivers"
            / "spi_multi"
        )
        if not spi_base.is_dir():
            return []
        chips = []
        for entry in sorted(spi_base.iterdir()):
            if entry.is_dir():
                chips.append(entry.name)
        return chips

    def _apply_spi_settings(
        self, template, macros, spi_cfg
    ):
        """Populate SPI-related markers when the board uses the SPI-multi IP."""
        protocol = str(spi_cfg.get("protocol", "")).lower()
        if protocol != "spi_multi":
            return template

        flash_chip = spi_cfg.get("flash_chip")
        if not flash_chip:
            raise ValueError("spi.flash_chip is required for protocol 'spi_multi'")

        supported = self._get_supported_spi_multi_flash_chips()
        if flash_chip not in supported:
            opts = ", ".join(supported) if supported else "<none>"
            raise ValueError(
                f"Unsupported spi flash_chip '{flash_chip}'. Supported options: {opts}"
            )

        mapping = {
            "spim_phycnt": "SPIM_PHYCNT_SET_VALUE",
            "spim_phyoffset1": "SPIM_PHYOFFSET1_SET_VALUE",
            "spim_phyoffset2": "SPIM_PHYOFFSET2_SET_VALUE",
            "spim_cmncr": "SPIM_CMNCR_EXTREAD_SET_VALUE",
            "spim_ssldr": "SPIM_SSLDR_SET_VALUE",
            "spim_drcr": "SPIM_DRCR_SET_VALUE",
            "spim_drcmr": "SPIM_DRCMR_SET_VALUE",
            "spim_drear": "SPIM_DREAR_SET_VALUE",
            "spim_drenr": "SPIM_DRENR_SET_VALUE",
            "spim_drdmcr": "SPIM_DRDMCR_SET_VALUE",
            "spim_drdrenr": "SPIM_DRDRENR_SET_VALUE",
        }

        missing = []
        for prop, macro_name in mapping.items():
            marker = format_marker(prop)
            if marker not in template:
                continue
            value = macros.get(macro_name)
            if value is None:
                missing.append(macro_name)
                continue
            block = self.gen_scalar_property(prop, value)
            template = self.replace_marker_block(template, marker, block)

        if missing:
            raise ValueError(
                "Missing SPI macro definitions: " + ", ".join(sorted(set(missing)))
            )

        return template

    def generate(self):
        """Render the DTS template based on the current configuration."""
        cfg = self.cfg
        ddr_cfg = cfg.get("ddr", {})
        spi_cfg = cfg.get("spi", {})

        template_path = self._resolve_template_path()
        template = template_path.read_text(encoding="utf-8")
        out_path = self._resolve_output_path()

        header_paths = self._auto_header_paths(ddr_cfg, spi_cfg)
        macros = PARSER.parse_macros_from_headers(header_paths)

        self._source_cache.clear()
        source_paths = self._gather_source_paths(ddr_cfg)

        template = self._apply_ddr_settings(template, macros, ddr_cfg, source_paths)
        template = self._apply_spi_settings(template, macros, spi_cfg)

        return template, out_path

    def write(self, content, out_path):
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(content, encoding="utf-8")

    def run(self, override_output=None):
        content, default_out = self.generate()
        out_path = override_output if override_output else default_out
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(content, encoding="utf-8")
        return out_path


def load_cfg(path):
    with path.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f)


def main(argv):
    ap = argparse.ArgumentParser(description="Renesas RZ DTS creator")
    ap.add_argument("-c", "--config", required=True, help="YAML config path")
    ap.add_argument("-o", "--output", help="Override output DTS path")
    args = ap.parse_args(argv)

    cfg = load_cfg(Path(args.config))
    repo_root = Path(__file__).resolve().parents[3]

    creator = DtsCreator(cfg, repo_root)
    override = Path(args.output) if args.output else None
    out_path = creator.run(override)

    try:
        rel = out_path.relative_to(repo_root)
    except Exception:
        rel = out_path
    print(f"[dts-creator] Wrote {rel}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
