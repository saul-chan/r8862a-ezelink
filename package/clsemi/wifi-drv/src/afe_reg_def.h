#ifndef __AFE_REG_DEF__H__
#define __AFE_REG_DEF__H__

#if (defined(DUBHE2000) && (DUBHE2000))
#include "afe_reg_def_d2k.h"
#elif (defined(MERAK2000) && (MERAK2000))
#include "afe_reg_def_d2k.h"
#else
#include "afe_reg_def_d1k.h"
#endif // DUBHE2000

#endif  // __AFE_REG_DEF__H__
