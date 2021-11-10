/**@file

  Copyright (c) 2018 - 2019, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/


//
// Definition for GPIO groups and pads
//
#ifndef _GPIO_ACPI_DEFINES_CNL_
#define _GPIO_ACPI_DEFINES_CNL_

#define GPIO_CNL_LP_ACPI_HID  "INT34BB"
#define GPIO_CNL_H_ACPI_HID   "INT3450"

#include "GpioPinsCmlLp.h"
#include "GpioPinsCmlH.h"

#define GPIO_CNL_LP_DRIVER_GPP_A_00 0
#define GPIO_CNL_LP_DRIVER_GPP_A_01 1
#define GPIO_CNL_LP_DRIVER_GPP_A_02 2
#define GPIO_CNL_LP_DRIVER_GPP_A_03 3
#define GPIO_CNL_LP_DRIVER_GPP_A_04 4
#define GPIO_CNL_LP_DRIVER_GPP_A_05 5
#define GPIO_CNL_LP_DRIVER_GPP_A_06 6
#define GPIO_CNL_LP_DRIVER_GPP_A_07 7
#define GPIO_CNL_LP_DRIVER_GPP_A_08 8
#define GPIO_CNL_LP_DRIVER_GPP_A_09 9
#define GPIO_CNL_LP_DRIVER_GPP_A_10 10
#define GPIO_CNL_LP_DRIVER_GPP_A_11 11
#define GPIO_CNL_LP_DRIVER_GPP_A_12 12
#define GPIO_CNL_LP_DRIVER_GPP_A_13 13
#define GPIO_CNL_LP_DRIVER_GPP_A_14 14
#define GPIO_CNL_LP_DRIVER_GPP_A_15 15
#define GPIO_CNL_LP_DRIVER_GPP_A_16 16
#define GPIO_CNL_LP_DRIVER_GPP_A_17 17
#define GPIO_CNL_LP_DRIVER_GPP_A_18 18
#define GPIO_CNL_LP_DRIVER_GPP_A_19 19
#define GPIO_CNL_LP_DRIVER_GPP_A_20 20
#define GPIO_CNL_LP_DRIVER_GPP_A_21 21
#define GPIO_CNL_LP_DRIVER_GPP_A_22 22
#define GPIO_CNL_LP_DRIVER_GPP_A_23 23

#define GPIO_CNL_LP_DRIVER_GPP_B_00 32
#define GPIO_CNL_LP_DRIVER_GPP_B_01 33
#define GPIO_CNL_LP_DRIVER_GPP_B_02 34
#define GPIO_CNL_LP_DRIVER_GPP_B_03 35
#define GPIO_CNL_LP_DRIVER_GPP_B_04 36
#define GPIO_CNL_LP_DRIVER_GPP_B_05 37
#define GPIO_CNL_LP_DRIVER_GPP_B_06 38
#define GPIO_CNL_LP_DRIVER_GPP_B_07 39
#define GPIO_CNL_LP_DRIVER_GPP_B_08 40
#define GPIO_CNL_LP_DRIVER_GPP_B_09 41
#define GPIO_CNL_LP_DRIVER_GPP_B_10 42
#define GPIO_CNL_LP_DRIVER_GPP_B_11 43
#define GPIO_CNL_LP_DRIVER_GPP_B_12 44
#define GPIO_CNL_LP_DRIVER_GPP_B_13 45
#define GPIO_CNL_LP_DRIVER_GPP_B_14 46
#define GPIO_CNL_LP_DRIVER_GPP_B_15 47
#define GPIO_CNL_LP_DRIVER_GPP_B_16 48
#define GPIO_CNL_LP_DRIVER_GPP_B_17 49
#define GPIO_CNL_LP_DRIVER_GPP_B_18 50
#define GPIO_CNL_LP_DRIVER_GPP_B_19 51
#define GPIO_CNL_LP_DRIVER_GPP_B_20 52
#define GPIO_CNL_LP_DRIVER_GPP_B_21 53
#define GPIO_CNL_LP_DRIVER_GPP_B_22 54
#define GPIO_CNL_LP_DRIVER_GPP_B_23 55

#define GPIO_CNL_LP_DRIVER_GPP_G_00 64
#define GPIO_CNL_LP_DRIVER_GPP_G_01 65
#define GPIO_CNL_LP_DRIVER_GPP_G_02 66
#define GPIO_CNL_LP_DRIVER_GPP_G_03 67
#define GPIO_CNL_LP_DRIVER_GPP_G_04 68
#define GPIO_CNL_LP_DRIVER_GPP_G_05 69
#define GPIO_CNL_LP_DRIVER_GPP_G_06 70
#define GPIO_CNL_LP_DRIVER_GPP_G_07 71

#define GPIO_CNL_LP_DRIVER_GPP_D_00 96
#define GPIO_CNL_LP_DRIVER_GPP_D_01 97
#define GPIO_CNL_LP_DRIVER_GPP_D_02 98
#define GPIO_CNL_LP_DRIVER_GPP_D_03 99


#define GPIO_CNL_H_DRIVER_GPP_A_00 0
#define GPIO_CNL_H_DRIVER_GPP_A_01 1
#define GPIO_CNL_H_DRIVER_GPP_A_02 2
#define GPIO_CNL_H_DRIVER_GPP_A_03 3
#define GPIO_CNL_H_DRIVER_GPP_A_04 4
#define GPIO_CNL_H_DRIVER_GPP_A_05 5
#define GPIO_CNL_H_DRIVER_GPP_A_06 6
#define GPIO_CNL_H_DRIVER_GPP_A_07 7
#define GPIO_CNL_H_DRIVER_GPP_A_08 8
#define GPIO_CNL_H_DRIVER_GPP_A_09 9
#define GPIO_CNL_H_DRIVER_GPP_A_10 10
#define GPIO_CNL_H_DRIVER_GPP_A_11 11
#define GPIO_CNL_H_DRIVER_GPP_A_12 12
#define GPIO_CNL_H_DRIVER_GPP_A_13 13
#define GPIO_CNL_H_DRIVER_GPP_A_14 14
#define GPIO_CNL_H_DRIVER_GPP_A_15 15
#define GPIO_CNL_H_DRIVER_GPP_A_16 16
#define GPIO_CNL_H_DRIVER_GPP_A_17 17
#define GPIO_CNL_H_DRIVER_GPP_A_18 18
#define GPIO_CNL_H_DRIVER_GPP_A_19 19
#define GPIO_CNL_H_DRIVER_GPP_A_20 20
#define GPIO_CNL_H_DRIVER_GPP_A_21 21
#define GPIO_CNL_H_DRIVER_GPP_A_22 22
#define GPIO_CNL_H_DRIVER_GPP_A_23 23

#define GPIO_CNL_H_DRIVER_GPP_B_00 32
#define GPIO_CNL_H_DRIVER_GPP_B_01 33
#define GPIO_CNL_H_DRIVER_GPP_B_02 34
#define GPIO_CNL_H_DRIVER_GPP_B_03 35
#define GPIO_CNL_H_DRIVER_GPP_B_04 36
#define GPIO_CNL_H_DRIVER_GPP_B_05 37
#define GPIO_CNL_H_DRIVER_GPP_B_06 38
#define GPIO_CNL_H_DRIVER_GPP_B_07 39
#define GPIO_CNL_H_DRIVER_GPP_B_08 40
#define GPIO_CNL_H_DRIVER_GPP_B_09 41
#define GPIO_CNL_H_DRIVER_GPP_B_10 42
#define GPIO_CNL_H_DRIVER_GPP_B_11 43
#define GPIO_CNL_H_DRIVER_GPP_B_12 44
#define GPIO_CNL_H_DRIVER_GPP_B_13 45
#define GPIO_CNL_H_DRIVER_GPP_B_14 46
#define GPIO_CNL_H_DRIVER_GPP_B_15 47
#define GPIO_CNL_H_DRIVER_GPP_B_16 48
#define GPIO_CNL_H_DRIVER_GPP_B_17 49
#define GPIO_CNL_H_DRIVER_GPP_B_18 50
#define GPIO_CNL_H_DRIVER_GPP_B_19 51
#define GPIO_CNL_H_DRIVER_GPP_B_20 52
#define GPIO_CNL_H_DRIVER_GPP_B_21 53
#define GPIO_CNL_H_DRIVER_GPP_B_22 54
#define GPIO_CNL_H_DRIVER_GPP_B_23 55

#define GPIO_CNL_H_DRIVER_GPP_C_00 64
#define GPIO_CNL_H_DRIVER_GPP_C_01 65
#define GPIO_CNL_H_DRIVER_GPP_C_02 66
#define GPIO_CNL_H_DRIVER_GPP_C_03 67
#define GPIO_CNL_H_DRIVER_GPP_C_04 68
#define GPIO_CNL_H_DRIVER_GPP_C_05 69
#define GPIO_CNL_H_DRIVER_GPP_C_06 70
#define GPIO_CNL_H_DRIVER_GPP_C_07 71
#define GPIO_CNL_H_DRIVER_GPP_C_08 72
#define GPIO_CNL_H_DRIVER_GPP_C_09 73
#define GPIO_CNL_H_DRIVER_GPP_C_10 74
#define GPIO_CNL_H_DRIVER_GPP_C_11 75
#define GPIO_CNL_H_DRIVER_GPP_C_12 76
#define GPIO_CNL_H_DRIVER_GPP_C_13 77
#define GPIO_CNL_H_DRIVER_GPP_C_14 78
#define GPIO_CNL_H_DRIVER_GPP_C_15 79
#define GPIO_CNL_H_DRIVER_GPP_C_16 80
#define GPIO_CNL_H_DRIVER_GPP_C_17 81
#define GPIO_CNL_H_DRIVER_GPP_C_18 82
#define GPIO_CNL_H_DRIVER_GPP_C_19 83
#define GPIO_CNL_H_DRIVER_GPP_C_20 84
#define GPIO_CNL_H_DRIVER_GPP_C_21 85
#define GPIO_CNL_H_DRIVER_GPP_C_22 86
#define GPIO_CNL_H_DRIVER_GPP_C_23 87

#define GPIO_CNL_H_DRIVER_GPP_D_00 96
#define GPIO_CNL_H_DRIVER_GPP_D_01 97
#define GPIO_CNL_H_DRIVER_GPP_D_02 98
#define GPIO_CNL_H_DRIVER_GPP_D_03 99


#endif // _GPIO_ACPI_DEFINES_CNL_
