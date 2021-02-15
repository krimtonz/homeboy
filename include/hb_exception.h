#ifndef _HB_EXCEPTION_H
#define _HB_EXCEPTION_H

#include "vc.h"

void init_hb_exceptions(void);
void __handle_exception(enum ppc_exception exception);

#endif