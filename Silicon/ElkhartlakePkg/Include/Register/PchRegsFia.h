/** @file
  Register definition for FIA component

  Conventions:

  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values within the bits
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position
  - In general, PCH registers are denoted by "_PCH_" in register names
  - Registers / bits that are different between PCH generations are denoted by
    "_PCH_[generation_name]_" in register/bit names.
  - Registers / bits that are specific to PCH-H denoted by "_H_" in register/bit names.
    Registers / bits that are specific to PCH-LP denoted by "_LP_" in register/bit names.
    e.g., "_PCH_H_", "_PCH_LP_"
    Registers / bits names without _H_ or _LP_ apply for both H and LP.
  - Registers / bits that are different between SKUs are denoted by "_[SKU_name]"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a PCH generation will be just named
    as "_PCH_" without [generation_name] inserted.

  Copyright (c) 2014 - 2017, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#ifndef _PCH_REGS_FIA_H_
#define _PCH_REGS_FIA_H_


//
// PID:FIA
//
#define CNL_H_PCH_MAX_FIA_DRCM                            7
#define PCH_MAX_FIA_DRCRM                                 CNL_H_PCH_MAX_FIA_DRCM


#define R_PCH_LP_PCR_FIA_LOS1_REG_BASE                    0x300
#define R_PCH_LP_PCR_FIA_LOS2_REG_BASE                    0x304
#define R_PCH_LP_PCR_FIA_LOS3_REG_BASE                    0x308
#define R_PCH_LP_PCR_FIA_LOS4_REG_BASE                    0x30C

#endif
