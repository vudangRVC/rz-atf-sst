/*
 * Copyright (c) 2023, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PWRC_H
#define PWRC_H

void pwrc_setup(void);
void plat_secondary_reset(void);
void pwrc_suspend_to_ram(void);
void rzv2h_pwrc_setup(void *fdt);
#endif /* PWRC_H */
