#ifndef RZ_DT_H
#define RZ_DT_H

#include <stdbool.h>
#include <stdint.h>

#define DT_DISABLED		U(0)
#define DT_NON_SECURE		U(1)
#define DT_SECURE		U(2)
#define DT_SHARED		(DT_NON_SECURE | DT_SECURE)

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
int dt_validation(uintptr_t dt_addr);

#endif /* RZ_DT_H */
