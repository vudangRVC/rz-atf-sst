RZ Device-Tree Creator
======================

Helper script to generate Renesas RZ device-tree files for TF-A from
board/SoC configuration and ATF C sources. It reads a YAML config, picks
the right template, expands C macros from header files, extracts arrays
from the SPI and DDR parameter sources, and writes the final DTS.

Prerequisites
-------------

- Python 3.8 or newer available on the PATH.
- Python packages: install pyyaml (``pip install pyyaml``).
- Optional tooling for schema checks:

  - dtc (Device Tree Compiler). 

    - On Windows, MSYS2 is required to build and run dtc and related tools.
        - Download and install MSYS2 from `MSYS2 <https://www.msys2.org/>`_ (check Installation section and download the .exe installer)
        - Follow the setup steps to complete the installation
        - Once the installation is complete, open the MSYS2 shell and run:

        .. code-block:: shell

            pacman -S base-devel mingw-w64-x86_64-gcc
            git clone https://git.kernel.org/pub/scm/utils/dtc/dtc.git
            cd dtc
            make dtc

        After building, dtc.exe will be located in the dtc directory; you can run it from Windows CMD/PowerShell or add its location to your PATH.

    - On Linux, run:

    ``sudo apt install device-tree-compiler``

  - dtschema for dt-validate.

    - On Windows, requires Microsoft Visual C++ 14.0 or newer. For installation steps:
        - Download the installer from: `Microsoft C++ Build Tools - Visual Studio <https://visualstudio.microsoft.com/visual-cpp-build-tools/>`_
        - Run the installer and select the Desktop development with C++ workload.
        - After installation, verify by running:

        .. code-block:: shell

            cl.exe
        

        - It should print the Microsoft compiler version (14.0 or newer).

        - Install Python packages. From Powershell, run:

        ``pip install dtschema``

    - On Linux, install build dependencies with:
        On Linux, install system packages first. Open the terminal and run the following commands.
        If Python 3.12 is in use: set up a virtual environment first.

        .. code-block:: shell

            sudo apt install python3.12-venv
            python3 -m venv .venv
            source .venv/bin/activate

        Then run the following commands

        ``sudo apt install swig python3-dev``

        ``pip3 install pyyaml dtschema``

Folder Hierarchy
----------------

.. code-block:: text

   tools/renesas
   ├── dts_creator
   │   ├── configs                                       <---- Configuration files
   │   │   ├── rzg2l_evk.yaml
   │   │   ├── rzg2l_sbc.yaml
   │   │   ├── rzv2h_evk.yaml
   │   │   └── rzv2l_evk.yaml
   │   ├── dts_creator.py                                <---- Main script
   │   ├── schemas
   │   │   └── rz-boards.yaml                            <---- ATF device tree schema
   │   └── template                                      <---- Template SoC device tree file
   │       ├── rzg2l_template.dts
   │       ├── rzv2h_template.dts
   │       └── rzv2l_template.dts
   ├── rcar_layout_create
   ├── rz_boot_param
   ├── rzg_layout_create
   └── rzg_security_tools

Adding a New Board
------------------

Board configuration is done through a YAML file under tools/renesas/dts_creator/configs/.
To add a new board, copy an existing config and edit it. The config file have the following structure:

.. code-block:: yaml

   soc: <soc_name [rzg2l, rzv2l, rzv2h]>
   board: <board_type ("evk", "sbc")>

   # Optional output dts path (repo-relative). If not specified, defaults to: fdts/<soc>-<board>.dts
   # output: <fdts/custom_device_tree_name>

   spi:
     protocol: <Device SPI protocol [spi_multi, xspi]>
     flash_chip: <Flash chip model series [AT25QL128A, IS25WP256, MT25QU512ABB]>

   ddr:
     type: <ddr_type [ddr4, lpddr4]>
     ddr sources:
       - <path to the first ddr source .c file>
       - <path to the second ddr source .c file>

Configuration Notes
-------------------

- soc: soc name (rzg2l, rzv2l, or rzv2h).

- board: device type (e,g: evk, sbc).

- spi:
    - protocol: spi module driver (spi_multi or xspi).

    - flash_chip: selects the flash family used to load presets
        (AT25QL128A, IS25WP256, MT25QU512ABB). For new parts, add a
        matching header with spi_multi_reg_values.h under
        plat/renesas/rz/common/include/drivers/spi_multi.

- ddr:

    - type: sets the memory profile (ddr4 or lpddr4).

    - ddr sources: lists the DDR parameter .c files that define the
        board-specific tables.

Usage
-----

1. Edit or create a config under tools/renesas/dts_creator/configs/.
2. Run the tool from the repository root.

Windows
   .. code-block:: shell

      python tools/renesas/dts_creator/dts_creator.py -c tools/renesas/dts_creator/configs/rzg2l_sbc.yaml

Linux
   .. code-block:: shell

      python3 tools/renesas/dts_creator/dts_creator.py -c tools/renesas/dts_creator/configs/rzg2l_sbc.yaml

   Optional: -o <output-path> overrides the output declared in the config.

Validation
----------

- Install the dtschema tooling (for example pip install dtschema).
- After generating a DTS, validate it against the Renesas schema:

  .. code-block:: shell

     dtc -I dts -O dtb -o <temp>.dtb <path-to-generated.dts>
     dt-validate --schema tools/renesas/dts_creator/schemas/rz-boards.yaml <temp>.dtb

  The DTS should pass without warnings or errors before it is committed.

Example:

  .. code-block:: shell

     dtc -I dts -O dtb -o fdts/rzg2l-sbc.dtb fdts/rzg2l-sbc.dts
     dt-validate --schema tools/renesas/dts_creator/schemas/rz-boards.yaml fdts/rzg2l-sbc.dtb
