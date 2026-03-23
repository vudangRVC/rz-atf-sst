/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Weak stubs for functions referenced by common code but only
 * implemented in the CMN/V2H platform.  For G2L these code paths
 * are never reached at runtime (guarded by FCONF / DDR-type checks).
 */

void __attribute__((weak)) pwrc_setup(void) {}
void __attribute__((weak)) lpddr4_setup(void) {}
