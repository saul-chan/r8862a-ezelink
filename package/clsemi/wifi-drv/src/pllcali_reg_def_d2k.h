#ifndef _PLLCALI_CFG_REG_DEF_D2K_H_
#define _PLLCALI_CFG_REG_DEF_D2K_H_

/* clang-format off */
// codegen version: 8
// codegen date   : 2024/03/04 18:03:40
// author         : Acker Liu
// header  version: 1
// changlog       : init version; same as fw

// #pragma region //MAIN_STATE_START
#define MAIN_STATE_START                                    (0x00000000) // 0x90422000 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_21_H000_WIDTH                           (11)
#define RESERVED_31_21_H000_LSB                             (21)
#define RESERVED_31_21_H000_MASK                            (((1 << RESERVED_31_21_H000_WIDTH) - 1) << RESERVED_31_21_H000_LSB)
#define CFG_PLLLOCKD_START_WIDTH                            (1)
#define CFG_PLLLOCKD_START_LSB                              (20)
#define CFG_PLLLOCKD_START_MASK                             (((1 << CFG_PLLLOCKD_START_WIDTH) - 1) << CFG_PLLLOCKD_START_LSB)
#define RESERVED_19_17_H000_WIDTH                           (3)
#define RESERVED_19_17_H000_LSB                             (17)
#define RESERVED_19_17_H000_MASK                            (((1 << RESERVED_19_17_H000_WIDTH) - 1) << RESERVED_19_17_H000_LSB)
#define CFG_LOCK_DET_START_WIDTH                            (1)
#define CFG_LOCK_DET_START_LSB                              (16)
#define CFG_LOCK_DET_START_MASK                             (((1 << CFG_LOCK_DET_START_WIDTH) - 1) << CFG_LOCK_DET_START_LSB)
#define RESERVED_15_13_H000_WIDTH                           (3)
#define RESERVED_15_13_H000_LSB                             (13)
#define RESERVED_15_13_H000_MASK                            (((1 << RESERVED_15_13_H000_WIDTH) - 1) << RESERVED_15_13_H000_LSB)
#define CFG_LOOPWORK_START_WIDTH                            (1)
#define CFG_LOOPWORK_START_LSB                              (12)
#define CFG_LOOPWORK_START_MASK                             (((1 << CFG_LOOPWORK_START_WIDTH) - 1) << CFG_LOOPWORK_START_LSB)
#define RESERVED_11_9_H000_WIDTH                            (3)
#define RESERVED_11_9_H000_LSB                              (9)
#define RESERVED_11_9_H000_MASK                             (((1 << RESERVED_11_9_H000_WIDTH) - 1) << RESERVED_11_9_H000_LSB)
#define CFG_OPENAFC2_START_WIDTH                            (1)
#define CFG_OPENAFC2_START_LSB                              (8)
#define CFG_OPENAFC2_START_MASK                             (((1 << CFG_OPENAFC2_START_WIDTH) - 1) << CFG_OPENAFC2_START_LSB)
#define RESERVED_7_5_H000_WIDTH                             (3)
#define RESERVED_7_5_H000_LSB                               (5)
#define RESERVED_7_5_H000_MASK                              (((1 << RESERVED_7_5_H000_WIDTH) - 1) << RESERVED_7_5_H000_LSB)
#define CFG_OPENKVCO_START_WIDTH                            (1)
#define CFG_OPENKVCO_START_LSB                              (4)
#define CFG_OPENKVCO_START_MASK                             (((1 << CFG_OPENKVCO_START_WIDTH) - 1) << CFG_OPENKVCO_START_LSB)
#define RESERVED_3_1_H000_WIDTH                             (3)
#define RESERVED_3_1_H000_LSB                               (1)
#define RESERVED_3_1_H000_MASK                              (((1 << RESERVED_3_1_H000_WIDTH) - 1) << RESERVED_3_1_H000_LSB)
#define CFG_OPEN_AFC_START_WIDTH                            (1)
#define CFG_OPEN_AFC_START_LSB                              (0)
#define CFG_OPEN_AFC_START_MASK                             (((1 << CFG_OPEN_AFC_START_WIDTH) - 1) << CFG_OPEN_AFC_START_LSB)

// #pragma endregion //MAIN_STATE_START

// #pragma region //MAIN_STATE_NOJUMP
#define MAIN_STATE_NOJUMP                                   (0x00000004) // 0x90422004 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H004_WIDTH                           (15)
#define RESERVED_31_17_H004_LSB                             (17)
#define RESERVED_31_17_H004_MASK                            (((1 << RESERVED_31_17_H004_WIDTH) - 1) << RESERVED_31_17_H004_LSB)
#define CFG_LOCK_DET_NOJUMP_WIDTH                           (1)
#define CFG_LOCK_DET_NOJUMP_LSB                             (16)
#define CFG_LOCK_DET_NOJUMP_MASK                            (((1 << CFG_LOCK_DET_NOJUMP_WIDTH) - 1) << CFG_LOCK_DET_NOJUMP_LSB)
#define RESERVED_15_13_H004_WIDTH                           (3)
#define RESERVED_15_13_H004_LSB                             (13)
#define RESERVED_15_13_H004_MASK                            (((1 << RESERVED_15_13_H004_WIDTH) - 1) << RESERVED_15_13_H004_LSB)
#define CFG_LOOPWORK_NOJUMP_WIDTH                           (1)
#define CFG_LOOPWORK_NOJUMP_LSB                             (12)
#define CFG_LOOPWORK_NOJUMP_MASK                            (((1 << CFG_LOOPWORK_NOJUMP_WIDTH) - 1) << CFG_LOOPWORK_NOJUMP_LSB)
#define RESERVED_11_9_H004_WIDTH                            (3)
#define RESERVED_11_9_H004_LSB                              (9)
#define RESERVED_11_9_H004_MASK                             (((1 << RESERVED_11_9_H004_WIDTH) - 1) << RESERVED_11_9_H004_LSB)
#define CFG_OPENAFC2_NOJUMP_WIDTH                           (1)
#define CFG_OPENAFC2_NOJUMP_LSB                             (8)
#define CFG_OPENAFC2_NOJUMP_MASK                            (((1 << CFG_OPENAFC2_NOJUMP_WIDTH) - 1) << CFG_OPENAFC2_NOJUMP_LSB)
#define RESERVED_7_5_H004_WIDTH                             (3)
#define RESERVED_7_5_H004_LSB                               (5)
#define RESERVED_7_5_H004_MASK                              (((1 << RESERVED_7_5_H004_WIDTH) - 1) << RESERVED_7_5_H004_LSB)
#define CFG_OPENKVCO_NOJUMP_WIDTH                           (1)
#define CFG_OPENKVCO_NOJUMP_LSB                             (4)
#define CFG_OPENKVCO_NOJUMP_MASK                            (((1 << CFG_OPENKVCO_NOJUMP_WIDTH) - 1) << CFG_OPENKVCO_NOJUMP_LSB)
#define RESERVED_3_1_H004_WIDTH                             (3)
#define RESERVED_3_1_H004_LSB                               (1)
#define RESERVED_3_1_H004_MASK                              (((1 << RESERVED_3_1_H004_WIDTH) - 1) << RESERVED_3_1_H004_LSB)
#define CFG_OPEN_AFC_NOJUMP_WIDTH                           (1)
#define CFG_OPEN_AFC_NOJUMP_LSB                             (0)
#define CFG_OPEN_AFC_NOJUMP_MASK                            (((1 << CFG_OPEN_AFC_NOJUMP_WIDTH) - 1) << CFG_OPEN_AFC_NOJUMP_LSB)

// #pragma endregion //MAIN_STATE_NOJUMP

// #pragma region //MAIN_STATE_RD
#define MAIN_STATE_RD                                       (0x00000008) // 0x90422008 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_25_H008_WIDTH                           (7)
#define RESERVED_31_25_H008_LSB                             (25)
#define RESERVED_31_25_H008_MASK                            (((1 << RESERVED_31_25_H008_WIDTH) - 1) << RESERVED_31_25_H008_LSB)
#define OPEN_AFC2_STA_IND_WIDTH                             (1)
#define OPEN_AFC2_STA_IND_LSB                               (24)
#define OPEN_AFC2_STA_IND_MASK                              (((1 << OPEN_AFC2_STA_IND_WIDTH) - 1) << OPEN_AFC2_STA_IND_LSB)
#define RESERVED_23_17_H008_WIDTH                           (7)
#define RESERVED_23_17_H008_LSB                             (17)
#define RESERVED_23_17_H008_MASK                            (((1 << RESERVED_23_17_H008_WIDTH) - 1) << RESERVED_23_17_H008_LSB)
#define OPEN_KVCO_STA_IND_WIDTH                             (1)
#define OPEN_KVCO_STA_IND_LSB                               (16)
#define OPEN_KVCO_STA_IND_MASK                              (((1 << OPEN_KVCO_STA_IND_WIDTH) - 1) << OPEN_KVCO_STA_IND_LSB)
#define RESERVED_15_9_H008_WIDTH                            (7)
#define RESERVED_15_9_H008_LSB                              (9)
#define RESERVED_15_9_H008_MASK                             (((1 << RESERVED_15_9_H008_WIDTH) - 1) << RESERVED_15_9_H008_LSB)
#define OPEN_AFC_STA_IND_WIDTH                              (1)
#define OPEN_AFC_STA_IND_LSB                                (8)
#define OPEN_AFC_STA_IND_MASK                               (((1 << OPEN_AFC_STA_IND_WIDTH) - 1) << OPEN_AFC_STA_IND_LSB)
#define RESERVED_7_3_H008_WIDTH                             (5)
#define RESERVED_7_3_H008_LSB                               (3)
#define RESERVED_7_3_H008_MASK                              (((1 << RESERVED_7_3_H008_WIDTH) - 1) << RESERVED_7_3_H008_LSB)
#define MAIN_CURRENT_STATE_WIDTH                            (3)
#define MAIN_CURRENT_STATE_LSB                              (0)
#define MAIN_CURRENT_STATE_MASK                             (((1 << MAIN_CURRENT_STATE_WIDTH) - 1) << MAIN_CURRENT_STATE_LSB)

// #pragma endregion //MAIN_STATE_RD

// #pragma region //AFC_REF_CLK_NDIV
#define AFC_REF_CLK_NDIV                                    (0x0000000c) // 0x9042200c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_24_H00C_WIDTH                           (8)
#define RESERVED_31_24_H00C_LSB                             (24)
#define RESERVED_31_24_H00C_MASK                            (((1 << RESERVED_31_24_H00C_WIDTH) - 1) << RESERVED_31_24_H00C_LSB)
#define AFC_REF_CLK_NDIV_WIDTH                              (24)
#define AFC_REF_CLK_NDIV_LSB                                (0)
#define AFC_REF_CLK_NDIV_MASK                               (((1 << AFC_REF_CLK_NDIV_WIDTH) - 1) << AFC_REF_CLK_NDIV_LSB)

// #pragma endregion //AFC_REF_CLK_NDIV

// #pragma region //AFC_K_BASE
#define AFC_K_BASE                                          (0x00000010) // 0x90422010 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_28_H010_WIDTH                           (4)
#define RESERVED_31_28_H010_LSB                             (28)
#define RESERVED_31_28_H010_MASK                            (((1 << RESERVED_31_28_H010_WIDTH) - 1) << RESERVED_31_28_H010_LSB)
#define AFC_K_BASE_WIDTH                                    (28)
#define AFC_K_BASE_LSB                                      (0)
#define AFC_K_BASE_MASK                                     (((1 << AFC_K_BASE_WIDTH) - 1) << AFC_K_BASE_LSB)

// #pragma endregion //AFC_K_BASE

// #pragma region //AFC_WT_TIME
#define AFC_WT_TIME                                         (0x00000014) // 0x90422014 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_8_H014_WIDTH                            (24)
#define RESERVED_31_8_H014_LSB                              (8)
#define RESERVED_31_8_H014_MASK                             (((1 << RESERVED_31_8_H014_WIDTH) - 1) << RESERVED_31_8_H014_LSB)
#define AFC_CBANK_WT_TIME_WIDTH                             (8)
#define AFC_CBANK_WT_TIME_LSB                               (0)
#define AFC_CBANK_WT_TIME_MASK                              (((1 << AFC_CBANK_WT_TIME_WIDTH) - 1) << AFC_CBANK_WT_TIME_LSB)

// #pragma endregion //AFC_WT_TIME

// #pragma region //AFC_OVF_CLR
#define AFC_OVF_CLR                                         (0x00000018) // 0x90422018 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H018_WIDTH                            (31)
#define RESERVED_31_1_H018_LSB                              (1)
#define RESERVED_31_1_H018_MASK                             (((1 << RESERVED_31_1_H018_WIDTH) - 1) << RESERVED_31_1_H018_LSB)
#define AFC_OVERFLOW_CLEAR_WIDTH                            (1)
#define AFC_OVERFLOW_CLEAR_LSB                              (0)
#define AFC_OVERFLOW_CLEAR_MASK                             (((1 << AFC_OVERFLOW_CLEAR_WIDTH) - 1) << AFC_OVERFLOW_CLEAR_LSB)

// #pragma endregion //AFC_OVF_CLR

// #pragma region //AFC_OVF_FLAG
#define AFC_OVF_FLAG                                        (0x0000001c) // 0x9042201c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H01C_PLL_WIDTH                        (31)
#define RESERVED_31_1_H01C_PLL_LSB                          (1)
#define RESERVED_31_1_H01C_PLL_MASK                         (((1 << RESERVED_31_1_H01C_PLL_WIDTH) - 1) << RESERVED_31_1_H01C_PLL_LSB)
#define AFC_OVERFLOW_FLAG_WIDTH                             (1)
#define AFC_OVERFLOW_FLAG_LSB                               (0)
#define AFC_OVERFLOW_FLAG_MASK                              (((1 << AFC_OVERFLOW_FLAG_WIDTH) - 1) << AFC_OVERFLOW_FLAG_LSB)

// #pragma endregion //AFC_OVF_FLAG

// #pragma region //AFC_MANUAL_CTRL
#define AFC_MANUAL_CTRL                                     (0x00000020) // 0x90422020 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H020_WIDTH                           (15)
#define RESERVED_31_17_H020_LSB                             (17)
#define RESERVED_31_17_H020_MASK                            (((1 << RESERVED_31_17_H020_WIDTH) - 1) << RESERVED_31_17_H020_LSB)
#define AFC_MANUAL_START_WIDTH                              (1)
#define AFC_MANUAL_START_LSB                                (16)
#define AFC_MANUAL_START_MASK                               (((1 << AFC_MANUAL_START_WIDTH) - 1) << AFC_MANUAL_START_LSB)
#define AFC_MANUAL_CBANK_WIDTH                              (8)
#define AFC_MANUAL_CBANK_LSB                                (8)
#define AFC_MANUAL_CBANK_MASK                               (((1 << AFC_MANUAL_CBANK_WIDTH) - 1) << AFC_MANUAL_CBANK_LSB)
#define RESERVED_7_1_H020_WIDTH                             (7)
#define RESERVED_7_1_H020_LSB                               (1)
#define RESERVED_7_1_H020_MASK                              (((1 << RESERVED_7_1_H020_WIDTH) - 1) << RESERVED_7_1_H020_LSB)
#define AFC_MANUAL_MODE_WIDTH                               (1)
#define AFC_MANUAL_MODE_LSB                                 (0)
#define AFC_MANUAL_MODE_MASK                                (((1 << AFC_MANUAL_MODE_WIDTH) - 1) << AFC_MANUAL_MODE_LSB)

// #pragma endregion //AFC_MANUAL_CTRL

// #pragma region //AFC_MANUAL_RSULT
#define AFC_MANUAL_RSULT                                    (0x00000024) // 0x90422024 # CHA=0x00000000; CHN=0x800;

#define AFC_MANUAL_KDET_VALID_WIDTH                         (1)
#define AFC_MANUAL_KDET_VALID_LSB                           (31)
#define AFC_MANUAL_KDET_VALID_MASK                          (((1 << AFC_MANUAL_KDET_VALID_WIDTH) - 1) << AFC_MANUAL_KDET_VALID_LSB)
#define RESERVED_30_28_H024_WIDTH                           (3)
#define RESERVED_30_28_H024_LSB                             (28)
#define RESERVED_30_28_H024_MASK                            (((1 << RESERVED_30_28_H024_WIDTH) - 1) << RESERVED_30_28_H024_LSB)
#define AFC_MANUAL_KDET_WIDTH                               (28)
#define AFC_MANUAL_KDET_LSB                                 (0)
#define AFC_MANUAL_KDET_MASK                                (((1 << AFC_MANUAL_KDET_WIDTH) - 1) << AFC_MANUAL_KDET_LSB)

// #pragma endregion //AFC_MANUAL_RSULT

// #pragma region //KVCO_DEFAULT_VREF_VCTRL
#define KVCO_DEFAULT_VREF_VCTRL                             (0x00000028) // 0x90422028 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_12_H028_WIDTH                           (20)
#define RESERVED_31_12_H028_LSB                             (12)
#define RESERVED_31_12_H028_MASK                            (((1 << RESERVED_31_12_H028_WIDTH) - 1) << RESERVED_31_12_H028_LSB)
#define KVCO_DEFAULT_VCTRL_SEL_WIDTH                        (4)
#define KVCO_DEFAULT_VCTRL_SEL_LSB                          (8)
#define KVCO_DEFAULT_VCTRL_SEL_MASK                         (((1 << KVCO_DEFAULT_VCTRL_SEL_WIDTH) - 1) << KVCO_DEFAULT_VCTRL_SEL_LSB)
#define RESERVED_7_5_H028_WIDTH                             (3)
#define RESERVED_7_5_H028_LSB                               (5)
#define RESERVED_7_5_H028_MASK                              (((1 << RESERVED_7_5_H028_WIDTH) - 1) << RESERVED_7_5_H028_LSB)
#define KVCO_DEFAULT_VREF_SEL_WIDTH                         (5)
#define KVCO_DEFAULT_VREF_SEL_LSB                           (0)
#define KVCO_DEFAULT_VREF_SEL_MASK                          (((1 << KVCO_DEFAULT_VREF_SEL_WIDTH) - 1) << KVCO_DEFAULT_VREF_SEL_LSB)

// #pragma endregion //KVCO_DEFAULT_VREF_VCTRL

// #pragma region //KVCO_REF_CLK_NDIV
#define KVCO_REF_CLK_NDIV                                   (0x0000002c) // 0x9042202c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_24_H02C_WIDTH                           (8)
#define RESERVED_31_24_H02C_LSB                             (24)
#define RESERVED_31_24_H02C_MASK                            (((1 << RESERVED_31_24_H02C_WIDTH) - 1) << RESERVED_31_24_H02C_LSB)
#define KVCO_REF_CLK_NDIV_WIDTH                             (24)
#define KVCO_REF_CLK_NDIV_LSB                               (0)
#define KVCO_REF_CLK_NDIV_MASK                              (((1 << KVCO_REF_CLK_NDIV_WIDTH) - 1) << KVCO_REF_CLK_NDIV_LSB)

// #pragma endregion //KVCO_REF_CLK_NDIV

// #pragma region //KVCO_VREF_CFG
#define KVCO_VREF_CFG                                       (0x00000030) // 0x90422030 # CHA=0x00000000; CHN=0x800;

#define KVCO_VREF_SEL_WT_TIME_WIDTH                         (16)
#define KVCO_VREF_SEL_WT_TIME_LSB                           (16)
#define KVCO_VREF_SEL_WT_TIME_MASK                          (((1 << KVCO_VREF_SEL_WT_TIME_WIDTH) - 1) << KVCO_VREF_SEL_WT_TIME_LSB)
#define RESERVED_15_13_H030_PLL_WIDTH                       (3)
#define RESERVED_15_13_H030_PLL_LSB                         (13)
#define RESERVED_15_13_H030_PLL_MASK                        (((1 << RESERVED_15_13_H030_PLL_WIDTH) - 1) << RESERVED_15_13_H030_PLL_LSB)
#define KVCO_VREF_SEL2_V_WIDTH                              (5)
#define KVCO_VREF_SEL2_V_LSB                                (8)
#define KVCO_VREF_SEL2_V_MASK                               (((1 << KVCO_VREF_SEL2_V_WIDTH) - 1) << KVCO_VREF_SEL2_V_LSB)
#define RESERVED_7_5_H030_WIDTH                             (3)
#define RESERVED_7_5_H030_LSB                               (5)
#define RESERVED_7_5_H030_MASK                              (((1 << RESERVED_7_5_H030_WIDTH) - 1) << RESERVED_7_5_H030_LSB)
#define KVCO_VREF_SEL1_V_WIDTH                              (5)
#define KVCO_VREF_SEL1_V_LSB                                (0)
#define KVCO_VREF_SEL1_V_MASK                               (((1 << KVCO_VREF_SEL1_V_WIDTH) - 1) << KVCO_VREF_SEL1_V_LSB)

// #pragma endregion //KVCO_VREF_CFG

// #pragma region //KVCO_KREG_LOW
#define KVCO_KREG_LOW                                       (0x00000034) // 0x90422034 # CHA=0x00000000; CHN=0x800;

#define KVCO_KREG_LOW_WIDTH                                 (32)
#define KVCO_KREG_LOW_LSB                                   (0)
#define KVCO_KREG_LOW_MASK                                  (0xffffffff)

// #pragma endregion //KVCO_KREG_LOW

// #pragma region //KVCO_KREG_HIGH
#define KVCO_KREG_HIGH                                      (0x00000038) // 0x90422038 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_6_H038_WIDTH                            (26)
#define RESERVED_31_6_H038_LSB                              (6)
#define RESERVED_31_6_H038_MASK                             (((1 << RESERVED_31_6_H038_WIDTH) - 1) << RESERVED_31_6_H038_LSB)
#define KVCO_KREG_HIGH_WIDTH                                (6)
#define KVCO_KREG_HIGH_LSB                                  (0)
#define KVCO_KREG_HIGH_MASK                                 (((1 << KVCO_KREG_HIGH_WIDTH) - 1) << KVCO_KREG_HIGH_LSB)

// #pragma endregion //KVCO_KREG_HIGH

// #pragma region //KVCO_FORCE_OUT
#define KVCO_FORCE_OUT                                      (0x0000003c) // 0x9042203c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_12_H03C_PLL_WIDTH                       (20)
#define RESERVED_31_12_H03C_PLL_LSB                         (12)
#define RESERVED_31_12_H03C_PLL_MASK                        (((1 << RESERVED_31_12_H03C_PLL_WIDTH) - 1) << RESERVED_31_12_H03C_PLL_LSB)
#define KVCO_VCTRL_SEL_SET_VALUE_WIDTH                      (4)
#define KVCO_VCTRL_SEL_SET_VALUE_LSB                        (8)
#define KVCO_VCTRL_SEL_SET_VALUE_MASK                       (((1 << KVCO_VCTRL_SEL_SET_VALUE_WIDTH) - 1) << KVCO_VCTRL_SEL_SET_VALUE_LSB)
#define RESERVED_7_1_H03C_WIDTH                             (7)
#define RESERVED_7_1_H03C_LSB                               (1)
#define RESERVED_7_1_H03C_MASK                              (((1 << RESERVED_7_1_H03C_WIDTH) - 1) << RESERVED_7_1_H03C_LSB)
#define KVCO_VCTRL_SEL_SET_EN_WIDTH                         (1)
#define KVCO_VCTRL_SEL_SET_EN_LSB                           (0)
#define KVCO_VCTRL_SEL_SET_EN_MASK                          (((1 << KVCO_VCTRL_SEL_SET_EN_WIDTH) - 1) << KVCO_VCTRL_SEL_SET_EN_LSB)

// #pragma endregion //KVCO_FORCE_OUT

// #pragma region //KVCO_DIVCAL_OVF_CLR
#define KVCO_DIVCAL_OVF_CLR                                 (0x00000040) // 0x90422040 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H040_WIDTH                            (31)
#define RESERVED_31_1_H040_LSB                              (1)
#define RESERVED_31_1_H040_MASK                             (((1 << RESERVED_31_1_H040_WIDTH) - 1) << RESERVED_31_1_H040_LSB)
#define KVCO_DIVCALC_OVF_CLEAR_WIDTH                        (1)
#define KVCO_DIVCALC_OVF_CLEAR_LSB                          (0)
#define KVCO_DIVCALC_OVF_CLEAR_MASK                         (((1 << KVCO_DIVCALC_OVF_CLEAR_WIDTH) - 1) << KVCO_DIVCALC_OVF_CLEAR_LSB)

// #pragma endregion //KVCO_DIVCAL_OVF_CLR

// #pragma region //KVCO_DIVCALC_OVF
#define KVCO_DIVCALC_OVF                                    (0x00000044) // 0x90422044 # CHA=0x00000000; CHN=0x800;

#define KVCO_DIVCALC_OVF_WIDTH                              (1)
#define KVCO_DIVCALC_OVF_LSB                                (31)
#define KVCO_DIVCALC_OVF_MASK                               (((1 << KVCO_DIVCALC_OVF_WIDTH) - 1) << KVCO_DIVCALC_OVF_LSB)
#define RESERVED_30_28_H044_WIDTH                           (3)
#define RESERVED_30_28_H044_LSB                             (28)
#define RESERVED_30_28_H044_MASK                            (((1 << RESERVED_30_28_H044_WIDTH) - 1) << RESERVED_30_28_H044_LSB)
#define KVCO_DIVCALC_OVF_KDELTA_WIDTH                       (28)
#define KVCO_DIVCALC_OVF_KDELTA_LSB                         (0)
#define KVCO_DIVCALC_OVF_KDELTA_MASK                        (((1 << KVCO_DIVCALC_OVF_KDELTA_WIDTH) - 1) << KVCO_DIVCALC_OVF_KDELTA_LSB)

// #pragma endregion //KVCO_DIVCALC_OVF

// #pragma region //KVCO_KDET_OVF_CLR
#define KVCO_KDET_OVF_CLR                                   (0x00000048) // 0x90422048 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H048_PLL_WIDTH                        (31)
#define RESERVED_31_1_H048_PLL_LSB                          (1)
#define RESERVED_31_1_H048_PLL_MASK                         (((1 << RESERVED_31_1_H048_PLL_WIDTH) - 1) << RESERVED_31_1_H048_PLL_LSB)
#define KVCO_KDET_OVF_CLEAR_WIDTH                           (1)
#define KVCO_KDET_OVF_CLEAR_LSB                             (0)
#define KVCO_KDET_OVF_CLEAR_MASK                            (((1 << KVCO_KDET_OVF_CLEAR_WIDTH) - 1) << KVCO_KDET_OVF_CLEAR_LSB)

// #pragma endregion //KVCO_KDET_OVF_CLR

// #pragma region //KVCO_KDET_OVF
#define KVCO_KDET_OVF                                       (0x0000004c) // 0x9042204c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H04C_PLL_WIDTH                        (31)
#define RESERVED_31_1_H04C_PLL_LSB                          (1)
#define RESERVED_31_1_H04C_PLL_MASK                         (((1 << RESERVED_31_1_H04C_PLL_WIDTH) - 1) << RESERVED_31_1_H04C_PLL_LSB)
#define KVCO_KDET_OVF_WIDTH                                 (1)
#define KVCO_KDET_OVF_LSB                                   (0)
#define KVCO_KDET_OVF_MASK                                  (((1 << KVCO_KDET_OVF_WIDTH) - 1) << KVCO_KDET_OVF_LSB)

// #pragma endregion //KVCO_KDET_OVF

// #pragma region //KVCO_MANUAL_CTRL
#define KVCO_MANUAL_CTRL                                    (0x00000050) // 0x90422050 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H050_WIDTH                           (15)
#define RESERVED_31_17_H050_LSB                             (17)
#define RESERVED_31_17_H050_MASK                            (((1 << RESERVED_31_17_H050_WIDTH) - 1) << RESERVED_31_17_H050_LSB)
#define KVCO_MANUAL_START_WIDTH                             (1)
#define KVCO_MANUAL_START_LSB                               (16)
#define KVCO_MANUAL_START_MASK                              (((1 << KVCO_MANUAL_START_WIDTH) - 1) << KVCO_MANUAL_START_LSB)
#define RESERVED_15_1_H050_WIDTH                            (15)
#define RESERVED_15_1_H050_LSB                              (1)
#define RESERVED_15_1_H050_MASK                             (((1 << RESERVED_15_1_H050_WIDTH) - 1) << RESERVED_15_1_H050_LSB)
#define KVCO_MANUAL_MODE_WIDTH                              (1)
#define KVCO_MANUAL_MODE_LSB                                (0)
#define KVCO_MANUAL_MODE_MASK                               (((1 << KVCO_MANUAL_MODE_WIDTH) - 1) << KVCO_MANUAL_MODE_LSB)

// #pragma endregion //KVCO_MANUAL_CTRL

// #pragma region //KVCO_MANUAL_KDET1_RESULT
#define KVCO_MANUAL_KDET1_RESULT                            (0x00000054) // 0x90422054 # CHA=0x00000000; CHN=0x800;

#define KVCO_MANUAL_KDET1_VALID_WIDTH                       (1)
#define KVCO_MANUAL_KDET1_VALID_LSB                         (31)
#define KVCO_MANUAL_KDET1_VALID_MASK                        (((1 << KVCO_MANUAL_KDET1_VALID_WIDTH) - 1) << KVCO_MANUAL_KDET1_VALID_LSB)
#define RESERVED_30_28_H054_WIDTH                           (3)
#define RESERVED_30_28_H054_LSB                             (28)
#define RESERVED_30_28_H054_MASK                            (((1 << RESERVED_30_28_H054_WIDTH) - 1) << RESERVED_30_28_H054_LSB)
#define KVCO_MANUAL_KDET1_VALUE_WIDTH                       (28)
#define KVCO_MANUAL_KDET1_VALUE_LSB                         (0)
#define KVCO_MANUAL_KDET1_VALUE_MASK                        (((1 << KVCO_MANUAL_KDET1_VALUE_WIDTH) - 1) << KVCO_MANUAL_KDET1_VALUE_LSB)

// #pragma endregion //KVCO_MANUAL_KDET1_RESULT

// #pragma region //KVCO_MANUAL_KDET2_RESULT
#define KVCO_MANUAL_KDET2_RESULT                            (0x00000058) // 0x90422058 # CHA=0x00000000; CHN=0x800;

#define KVCO_MANUAL_KDET2_VALID_WIDTH                       (1)
#define KVCO_MANUAL_KDET2_VALID_LSB                         (31)
#define KVCO_MANUAL_KDET2_VALID_MASK                        (((1 << KVCO_MANUAL_KDET2_VALID_WIDTH) - 1) << KVCO_MANUAL_KDET2_VALID_LSB)
#define RESERVED_30_28_H058_WIDTH                           (3)
#define RESERVED_30_28_H058_LSB                             (28)
#define RESERVED_30_28_H058_MASK                            (((1 << RESERVED_30_28_H058_WIDTH) - 1) << RESERVED_30_28_H058_LSB)
#define KVCO_MANUAL_KDET2_VALUE_WIDTH                       (28)
#define KVCO_MANUAL_KDET2_VALUE_LSB                         (0)
#define KVCO_MANUAL_KDET2_VALUE_MASK                        (((1 << KVCO_MANUAL_KDET2_VALUE_WIDTH) - 1) << KVCO_MANUAL_KDET2_VALUE_LSB)

// #pragma endregion //KVCO_MANUAL_KDET2_RESULT

// #pragma region //AFC2_REF_CLK_NDIV
#define AFC2_REF_CLK_NDIV                                   (0x0000005c) // 0x9042205c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_24_H05C_PLL_WIDTH                       (8)
#define RESERVED_31_24_H05C_PLL_LSB                         (24)
#define RESERVED_31_24_H05C_PLL_MASK                        (((1 << RESERVED_31_24_H05C_PLL_WIDTH) - 1) << RESERVED_31_24_H05C_PLL_LSB)
#define AFC2_REF_CLK_NDIV_WIDTH                             (24)
#define AFC2_REF_CLK_NDIV_LSB                               (0)
#define AFC2_REF_CLK_NDIV_MASK                              (((1 << AFC2_REF_CLK_NDIV_WIDTH) - 1) << AFC2_REF_CLK_NDIV_LSB)

// #pragma endregion //AFC2_REF_CLK_NDIV

// #pragma region //AFC2_K_BASE
#define AFC2_K_BASE                                         (0x00000060) // 0x90422060 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_28_H060_WIDTH                           (4)
#define RESERVED_31_28_H060_LSB                             (28)
#define RESERVED_31_28_H060_MASK                            (((1 << RESERVED_31_28_H060_WIDTH) - 1) << RESERVED_31_28_H060_LSB)
#define AFC2_K_BASE_WIDTH                                   (28)
#define AFC2_K_BASE_LSB                                     (0)
#define AFC2_K_BASE_MASK                                    (((1 << AFC2_K_BASE_WIDTH) - 1) << AFC2_K_BASE_LSB)

// #pragma endregion //AFC2_K_BASE

// #pragma region //AFC2_WT_TIME
#define AFC2_WT_TIME                                        (0x00000064) // 0x90422064 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_8_H064_WIDTH                            (24)
#define RESERVED_31_8_H064_LSB                              (8)
#define RESERVED_31_8_H064_MASK                             (((1 << RESERVED_31_8_H064_WIDTH) - 1) << RESERVED_31_8_H064_LSB)
#define AFC2_CBANK_WT_TIME_WIDTH                            (8)
#define AFC2_CBANK_WT_TIME_LSB                              (0)
#define AFC2_CBANK_WT_TIME_MASK                             (((1 << AFC2_CBANK_WT_TIME_WIDTH) - 1) << AFC2_CBANK_WT_TIME_LSB)

// #pragma endregion //AFC2_WT_TIME

// #pragma region //AFC2_OVF_CLR
#define AFC2_OVF_CLR                                        (0x00000068) // 0x90422068 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H068_WIDTH                            (31)
#define RESERVED_31_1_H068_LSB                              (1)
#define RESERVED_31_1_H068_MASK                             (((1 << RESERVED_31_1_H068_WIDTH) - 1) << RESERVED_31_1_H068_LSB)
#define AFC2_OVERFLOW_CLEAR_WIDTH                           (1)
#define AFC2_OVERFLOW_CLEAR_LSB                             (0)
#define AFC2_OVERFLOW_CLEAR_MASK                            (((1 << AFC2_OVERFLOW_CLEAR_WIDTH) - 1) << AFC2_OVERFLOW_CLEAR_LSB)

// #pragma endregion //AFC2_OVF_CLR

// #pragma region //AFC2_OVF_FLAG
#define AFC2_OVF_FLAG                                       (0x0000006c) // 0x9042206c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H06C_WIDTH                            (31)
#define RESERVED_31_1_H06C_LSB                              (1)
#define RESERVED_31_1_H06C_MASK                             (((1 << RESERVED_31_1_H06C_WIDTH) - 1) << RESERVED_31_1_H06C_LSB)
#define AFC2_OVERFLOW_FLAG_WIDTH                            (1)
#define AFC2_OVERFLOW_FLAG_LSB                              (0)
#define AFC2_OVERFLOW_FLAG_MASK                             (((1 << AFC2_OVERFLOW_FLAG_WIDTH) - 1) << AFC2_OVERFLOW_FLAG_LSB)

// #pragma endregion //AFC2_OVF_FLAG

// #pragma region //AFC2_MANUAL_CTRL
#define AFC2_MANUAL_CTRL                                    (0x00000070) // 0x90422070 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H070_PLL_WIDTH                       (15)
#define RESERVED_31_17_H070_PLL_LSB                         (17)
#define RESERVED_31_17_H070_PLL_MASK                        (((1 << RESERVED_31_17_H070_PLL_WIDTH) - 1) << RESERVED_31_17_H070_PLL_LSB)
#define AFC2_MANUAL_START_WIDTH                             (1)
#define AFC2_MANUAL_START_LSB                               (16)
#define AFC2_MANUAL_START_MASK                              (((1 << AFC2_MANUAL_START_WIDTH) - 1) << AFC2_MANUAL_START_LSB)
#define AFC2_MANUAL_CBANK_WIDTH                             (8)
#define AFC2_MANUAL_CBANK_LSB                               (8)
#define AFC2_MANUAL_CBANK_MASK                              (((1 << AFC2_MANUAL_CBANK_WIDTH) - 1) << AFC2_MANUAL_CBANK_LSB)
#define RESERVED_7_1_H070_WIDTH                             (7)
#define RESERVED_7_1_H070_LSB                               (1)
#define RESERVED_7_1_H070_MASK                              (((1 << RESERVED_7_1_H070_WIDTH) - 1) << RESERVED_7_1_H070_LSB)
#define AFC2_MANUAL_MODE_WIDTH                              (1)
#define AFC2_MANUAL_MODE_LSB                                (0)
#define AFC2_MANUAL_MODE_MASK                               (((1 << AFC2_MANUAL_MODE_WIDTH) - 1) << AFC2_MANUAL_MODE_LSB)

// #pragma endregion //AFC2_MANUAL_CTRL

// #pragma region //AFC2_MANUAL_RSULT
#define AFC2_MANUAL_RSULT                                   (0x00000074) // 0x90422074 # CHA=0x00000000; CHN=0x800;

#define AFC2_MANUAL_KDET_VALID_WIDTH                        (1)
#define AFC2_MANUAL_KDET_VALID_LSB                          (31)
#define AFC2_MANUAL_KDET_VALID_MASK                         (((1 << AFC2_MANUAL_KDET_VALID_WIDTH) - 1) << AFC2_MANUAL_KDET_VALID_LSB)
#define RESERVED_30_28_H074_WIDTH                           (3)
#define RESERVED_30_28_H074_LSB                             (28)
#define RESERVED_30_28_H074_MASK                            (((1 << RESERVED_30_28_H074_WIDTH) - 1) << RESERVED_30_28_H074_LSB)
#define AFC2_MANUAL_KDET_WIDTH                              (28)
#define AFC2_MANUAL_KDET_LSB                                (0)
#define AFC2_MANUAL_KDET_MASK                               (((1 << AFC2_MANUAL_KDET_WIDTH) - 1) << AFC2_MANUAL_KDET_LSB)

// #pragma endregion //AFC2_MANUAL_RSULT

// #pragma region //LOOPSTATE_WAIT_TIME
#define LOOPSTATE_WAIT_TIME                                 (0x00000078) // 0x90422078 # CHA=0x00000000; CHN=0x800;

#define LOOP_WORK_WT_TIME_WIDTH                             (32)
#define LOOP_WORK_WT_TIME_LSB                               (0)
#define LOOP_WORK_WT_TIME_MASK                              (0xffffffff)

// #pragma endregion //LOOPSTATE_WAIT_TIME

// #pragma region //LOOPSTATE_MANUAL_CFG
#define LOOPSTATE_MANUAL_CFG                                (0x0000007c) // 0x9042207c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H07C_WIDTH                            (31)
#define RESERVED_31_1_H07C_LSB                              (1)
#define RESERVED_31_1_H07C_MASK                             (((1 << RESERVED_31_1_H07C_WIDTH) - 1) << RESERVED_31_1_H07C_LSB)
#define LOOP_MANUAL_EN_WIDTH                                (1)
#define LOOP_MANUAL_EN_LSB                                  (0)
#define LOOP_MANUAL_EN_MASK                                 (((1 << LOOP_MANUAL_EN_WIDTH) - 1) << LOOP_MANUAL_EN_LSB)

// #pragma endregion //LOOPSTATE_MANUAL_CFG

// #pragma region //LOOPSTATE_LOOPFUNC_CFG
#define LOOPSTATE_LOOPFUNC_CFG                              (0x00000080) // 0x90422080 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H080_PLL_WIDTH                       (15)
#define RESERVED_31_17_H080_PLL_LSB                         (17)
#define RESERVED_31_17_H080_PLL_MASK                        (((1 << RESERVED_31_17_H080_PLL_WIDTH) - 1) << RESERVED_31_17_H080_PLL_LSB)
#define LOOP_CFG_UP_WIDTH                                   (1)
#define LOOP_CFG_UP_LSB                                     (16)
#define LOOP_CFG_UP_MASK                                    (((1 << LOOP_CFG_UP_WIDTH) - 1) << LOOP_CFG_UP_LSB)
#define RESERVED_15_5_H080_WIDTH                            (11)
#define RESERVED_15_5_H080_LSB                              (5)
#define RESERVED_15_5_H080_MASK                             (((1 << RESERVED_15_5_H080_WIDTH) - 1) << RESERVED_15_5_H080_LSB)
#define LOOP_CFG_PSGEN_EN_WIDTH                             (1)
#define LOOP_CFG_PSGEN_EN_LSB                               (4)
#define LOOP_CFG_PSGEN_EN_MASK                              (((1 << LOOP_CFG_PSGEN_EN_WIDTH) - 1) << LOOP_CFG_PSGEN_EN_LSB)
#define LOOP_CFG_DTC_LMS_EN_WIDTH                           (1)
#define LOOP_CFG_DTC_LMS_EN_LSB                             (3)
#define LOOP_CFG_DTC_LMS_EN_MASK                            (((1 << LOOP_CFG_DTC_LMS_EN_WIDTH) - 1) << LOOP_CFG_DTC_LMS_EN_LSB)
#define LOOP_CFG_DCC_EN_WIDTH                               (1)
#define LOOP_CFG_DCC_EN_LSB                                 (2)
#define LOOP_CFG_DCC_EN_MASK                                (((1 << LOOP_CFG_DCC_EN_WIDTH) - 1) << LOOP_CFG_DCC_EN_LSB)
#define LOOP_CFG_DIDTC_EN_WIDTH                             (1)
#define LOOP_CFG_DIDTC_EN_LSB                               (1)
#define LOOP_CFG_DIDTC_EN_MASK                              (((1 << LOOP_CFG_DIDTC_EN_WIDTH) - 1) << LOOP_CFG_DIDTC_EN_LSB)
#define LOOP_CFG_DSM_EN_WIDTH                               (1)
#define LOOP_CFG_DSM_EN_LSB                                 (0)
#define LOOP_CFG_DSM_EN_MASK                                (((1 << LOOP_CFG_DSM_EN_WIDTH) - 1) << LOOP_CFG_DSM_EN_LSB)

// #pragma endregion //LOOPSTATE_LOOPFUNC_CFG

// #pragma region //DCC_CFG
#define DCC_CFG                                             (0x00000084) // 0x90422084 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H084_PLL_WIDTH                       (15)
#define RESERVED_31_17_H084_PLL_LSB                         (17)
#define RESERVED_31_17_H084_PLL_MASK                        (((1 << RESERVED_31_17_H084_PLL_WIDTH) - 1) << RESERVED_31_17_H084_PLL_LSB)
#define DCC_CFG_UP_WIDTH                                    (1)
#define DCC_CFG_UP_LSB                                      (16)
#define DCC_CFG_UP_MASK                                     (((1 << DCC_CFG_UP_WIDTH) - 1) << DCC_CFG_UP_LSB)
#define RESERVED_15_9_H084_WIDTH                            (7)
#define RESERVED_15_9_H084_LSB                              (9)
#define RESERVED_15_9_H084_MASK                             (((1 << RESERVED_15_9_H084_WIDTH) - 1) << RESERVED_15_9_H084_LSB)
#define DCC_INV_WIDTH                                       (1)
#define DCC_INV_LSB                                         (8)
#define DCC_INV_MASK                                        (((1 << DCC_INV_WIDTH) - 1) << DCC_INV_LSB)
#define RESERVED_7_5_H084_WIDTH                             (3)
#define RESERVED_7_5_H084_LSB                               (5)
#define RESERVED_7_5_H084_MASK                              (((1 << RESERVED_7_5_H084_WIDTH) - 1) << RESERVED_7_5_H084_LSB)
#define DCC_U_WIDTH                                         (5)
#define DCC_U_LSB                                           (0)
#define DCC_U_MASK                                          (((1 << DCC_U_WIDTH) - 1) << DCC_U_LSB)

// #pragma endregion //DCC_CFG

// #pragma region //DCC_BUF_INI_WREN_RDEN
#define DCC_BUF_INI_WREN_RDEN                               (0x00000088) // 0x90422088 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H088_PLL_WIDTH                       (15)
#define RESERVED_31_17_H088_PLL_LSB                         (17)
#define RESERVED_31_17_H088_PLL_MASK                        (((1 << RESERVED_31_17_H088_PLL_WIDTH) - 1) << RESERVED_31_17_H088_PLL_LSB)
#define DCC_BUF_RDEN_WIDTH                                  (1)
#define DCC_BUF_RDEN_LSB                                    (16)
#define DCC_BUF_RDEN_MASK                                   (((1 << DCC_BUF_RDEN_WIDTH) - 1) << DCC_BUF_RDEN_LSB)
#define RESERVED_15_1_H088_WIDTH                            (15)
#define RESERVED_15_1_H088_LSB                              (1)
#define RESERVED_15_1_H088_MASK                             (((1 << RESERVED_15_1_H088_WIDTH) - 1) << RESERVED_15_1_H088_LSB)
#define DCC_BUF_INI_WREN_WIDTH                              (1)
#define DCC_BUF_INI_WREN_LSB                                (0)
#define DCC_BUF_INI_WREN_MASK                               (((1 << DCC_BUF_INI_WREN_WIDTH) - 1) << DCC_BUF_INI_WREN_LSB)

// #pragma endregion //DCC_BUF_INI_WREN_RDEN

// #pragma region //DCC_BUF_INI_LOW
#define DCC_BUF_INI_LOW                                     (0x0000008c) // 0x9042208c # CHA=0x00000000; CHN=0x800;

#define DCC_BUF_INI_LOW_WIDTH                               (32)
#define DCC_BUF_INI_LOW_LSB                                 (0)
#define DCC_BUF_INI_LOW_MASK                                (0xffffffff)

// #pragma endregion //DCC_BUF_INI_LOW

// #pragma region //DCC_BUF_INI_HIGH
#define DCC_BUF_INI_HIGH                                    (0x00000090) // 0x90422090 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_4_H090_WIDTH                            (28)
#define RESERVED_31_4_H090_LSB                              (4)
#define RESERVED_31_4_H090_MASK                             (((1 << RESERVED_31_4_H090_WIDTH) - 1) << RESERVED_31_4_H090_LSB)
#define DCC_BUF_INI_HIGH_WIDTH                              (4)
#define DCC_BUF_INI_HIGH_LSB                                (0)
#define DCC_BUF_INI_HIGH_MASK                               (((1 << DCC_BUF_INI_HIGH_WIDTH) - 1) << DCC_BUF_INI_HIGH_LSB)

// #pragma endregion //DCC_BUF_INI_HIGH

// #pragma region //DCC_BUF_INI_RD_LOW
#define DCC_BUF_INI_RD_LOW                                  (0x00000094) // 0x90422094 # CHA=0x00000000; CHN=0x800;

#define DCC_BUF_RD_LOW_WIDTH                                (32)
#define DCC_BUF_RD_LOW_LSB                                  (0)
#define DCC_BUF_RD_LOW_MASK                                 (0xffffffff)

// #pragma endregion //DCC_BUF_INI_RD_LOW

// #pragma region //DCC_BUF_INI_RD_HIGH
#define DCC_BUF_INI_RD_HIGH                                 (0x00000098) // 0x90422098 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_4_H098_WIDTH                            (28)
#define RESERVED_31_4_H098_LSB                              (4)
#define RESERVED_31_4_H098_MASK                             (((1 << RESERVED_31_4_H098_WIDTH) - 1) << RESERVED_31_4_H098_LSB)
#define DCC_BUF_RD_HIGH_WIDTH                               (4)
#define DCC_BUF_RD_HIGH_LSB                                 (0)
#define DCC_BUF_RD_HIGH_MASK                                (((1 << DCC_BUF_RD_HIGH_WIDTH) - 1) << DCC_BUF_RD_HIGH_LSB)

// #pragma endregion //DCC_BUF_INI_RD_HIGH

// #pragma region //DCC_BUF_OVF_CLR
#define DCC_BUF_OVF_CLR                                     (0x0000009c) // 0x9042209c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H09C_WIDTH                            (31)
#define RESERVED_31_1_H09C_LSB                              (1)
#define RESERVED_31_1_H09C_MASK                             (((1 << RESERVED_31_1_H09C_WIDTH) - 1) << RESERVED_31_1_H09C_LSB)
#define DCC_BUF_OVF_CLR_WIDTH                               (1)
#define DCC_BUF_OVF_CLR_LSB                                 (0)
#define DCC_BUF_OVF_CLR_MASK                                (((1 << DCC_BUF_OVF_CLR_WIDTH) - 1) << DCC_BUF_OVF_CLR_LSB)

// #pragma endregion //DCC_BUF_OVF_CLR

// #pragma region //DCC_BUF_OVF
#define DCC_BUF_OVF                                         (0x000000a0) // 0x904220a0 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H0A0_WIDTH                            (31)
#define RESERVED_31_1_H0A0_LSB                              (1)
#define RESERVED_31_1_H0A0_MASK                             (((1 << RESERVED_31_1_H0A0_WIDTH) - 1) << RESERVED_31_1_H0A0_LSB)
#define DCC_BUF_OVF_WIDTH                                   (1)
#define DCC_BUF_OVF_LSB                                     (0)
#define DCC_BUF_OVF_MASK                                    (((1 << DCC_BUF_OVF_WIDTH) - 1) << DCC_BUF_OVF_LSB)

// #pragma endregion //DCC_BUF_OVF

// #pragma region //DCC_FORCE_COMP_EN
#define DCC_FORCE_COMP_EN                                   (0x000000a4) // 0x904220a4 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H0A4_WIDTH                            (31)
#define RESERVED_31_1_H0A4_LSB                              (1)
#define RESERVED_31_1_H0A4_MASK                             (((1 << RESERVED_31_1_H0A4_WIDTH) - 1) << RESERVED_31_1_H0A4_LSB)
#define DCC_USE_CFG_COMP_WIDTH                              (1)
#define DCC_USE_CFG_COMP_LSB                                (0)
#define DCC_USE_CFG_COMP_MASK                               (((1 << DCC_USE_CFG_COMP_WIDTH) - 1) << DCC_USE_CFG_COMP_LSB)

// #pragma endregion //DCC_FORCE_COMP_EN

// #pragma region //DCC_FORCE_COMP_LOW
#define DCC_FORCE_COMP_LOW                                  (0x000000a8) // 0x904220a8 # CHA=0x00000000; CHN=0x800;

#define DCC_CFG_COMP_LOW_WIDTH                              (32)
#define DCC_CFG_COMP_LOW_LSB                                (0)
#define DCC_CFG_COMP_LOW_MASK                               (0xffffffff)

// #pragma endregion //DCC_FORCE_COMP_LOW

// #pragma region //DCC_FORCE_COMP_HIGH
#define DCC_FORCE_COMP_HIGH                                 (0x000000ac) // 0x904220ac # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_4_H0AC_WIDTH                            (28)
#define RESERVED_31_4_H0AC_LSB                              (4)
#define RESERVED_31_4_H0AC_MASK                             (((1 << RESERVED_31_4_H0AC_WIDTH) - 1) << RESERVED_31_4_H0AC_LSB)
#define DCC_CFG_COMP_HIGH_WIDTH                             (4)
#define DCC_CFG_COMP_HIGH_LSB                               (0)
#define DCC_CFG_COMP_HIGH_MASK                              (((1 << DCC_CFG_COMP_HIGH_WIDTH) - 1) << DCC_CFG_COMP_HIGH_LSB)

// #pragma endregion //DCC_FORCE_COMP_HIGH

// #pragma region //DIDTC_CFG
#define DIDTC_CFG                                           (0x000000b0) // 0x904220b0 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H0B0_WIDTH                           (15)
#define RESERVED_31_17_H0B0_LSB                             (17)
#define RESERVED_31_17_H0B0_MASK                            (((1 << RESERVED_31_17_H0B0_WIDTH) - 1) << RESERVED_31_17_H0B0_LSB)
#define DIDTC_CFG_UP_WIDTH                                  (1)
#define DIDTC_CFG_UP_LSB                                    (16)
#define DIDTC_CFG_UP_MASK                                   (((1 << DIDTC_CFG_UP_WIDTH) - 1) << DIDTC_CFG_UP_LSB)
#define DIDTC_OS_WIDTH                                      (8)
#define DIDTC_OS_LSB                                        (8)
#define DIDTC_OS_MASK                                       (((1 << DIDTC_OS_WIDTH) - 1) << DIDTC_OS_LSB)
#define RESERVED_7_1_H0B0_WIDTH                             (7)
#define RESERVED_7_1_H0B0_LSB                               (1)
#define RESERVED_7_1_H0B0_MASK                              (((1 << RESERVED_7_1_H0B0_WIDTH) - 1) << RESERVED_7_1_H0B0_LSB)
#define DIDTC_INV_WIDTH                                     (1)
#define DIDTC_INV_LSB                                       (0)
#define DIDTC_INV_MASK                                      (((1 << DIDTC_INV_WIDTH) - 1) << DIDTC_INV_LSB)

// #pragma endregion //DIDTC_CFG

// #pragma region //DIDTC_BUF_CFG
#define DIDTC_BUF_CFG                                       (0x000000b4) // 0x904220b4 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_21_H0B4_WIDTH                           (11)
#define RESERVED_31_21_H0B4_LSB                             (21)
#define RESERVED_31_21_H0B4_MASK                            (((1 << RESERVED_31_21_H0B4_WIDTH) - 1) << RESERVED_31_21_H0B4_LSB)
#define DIDTC_BUF_RDEN_WIDTH                                (1)
#define DIDTC_BUF_RDEN_LSB                                  (20)
#define DIDTC_BUF_RDEN_MASK                                 (((1 << DIDTC_BUF_RDEN_WIDTH) - 1) << DIDTC_BUF_RDEN_LSB)
#define RESERVED_19_17_H0B4_WIDTH                           (3)
#define RESERVED_19_17_H0B4_LSB                             (17)
#define RESERVED_19_17_H0B4_MASK                            (((1 << RESERVED_19_17_H0B4_WIDTH) - 1) << RESERVED_19_17_H0B4_LSB)
#define DIDTC_BUF_WREN_WIDTH                                (1)
#define DIDTC_BUF_WREN_LSB                                  (16)
#define DIDTC_BUF_WREN_MASK                                 (((1 << DIDTC_BUF_WREN_WIDTH) - 1) << DIDTC_BUF_WREN_LSB)
#define RESERVED_15_9_H0B4_WIDTH                            (7)
#define RESERVED_15_9_H0B4_LSB                              (9)
#define RESERVED_15_9_H0B4_MASK                             (((1 << RESERVED_15_9_H0B4_WIDTH) - 1) << RESERVED_15_9_H0B4_LSB)
#define DIDTC_BUF_INI_WIDTH                                 (9)
#define DIDTC_BUF_INI_LSB                                   (0)
#define DIDTC_BUF_INI_MASK                                  (((1 << DIDTC_BUF_INI_WIDTH) - 1) << DIDTC_BUF_INI_LSB)

// #pragma endregion //DIDTC_BUF_CFG

// #pragma region //DIDTC_BUF_RD
#define DIDTC_BUF_RD                                        (0x000000b8) // 0x904220b8 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_9_H0B8_WIDTH                            (23)
#define RESERVED_31_9_H0B8_LSB                              (9)
#define RESERVED_31_9_H0B8_MASK                             (((1 << RESERVED_31_9_H0B8_WIDTH) - 1) << RESERVED_31_9_H0B8_LSB)
#define DIDTC_BUF_RD_WIDTH                                  (9)
#define DIDTC_BUF_RD_LSB                                    (0)
#define DIDTC_BUF_RD_MASK                                   (((1 << DIDTC_BUF_RD_WIDTH) - 1) << DIDTC_BUF_RD_LSB)

// #pragma endregion //DIDTC_BUF_RD

// #pragma region //DIDTC_OVF_CLR
#define DIDTC_OVF_CLR                                       (0x000000bc) // 0x904220bc # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H0BC_WIDTH                           (15)
#define RESERVED_31_17_H0BC_LSB                             (17)
#define RESERVED_31_17_H0BC_MASK                            (((1 << RESERVED_31_17_H0BC_WIDTH) - 1) << RESERVED_31_17_H0BC_LSB)
#define DIDTC_OUT_OVF_CLR_WIDTH                             (1)
#define DIDTC_OUT_OVF_CLR_LSB                               (16)
#define DIDTC_OUT_OVF_CLR_MASK                              (((1 << DIDTC_OUT_OVF_CLR_WIDTH) - 1) << DIDTC_OUT_OVF_CLR_LSB)
#define RESERVED_15_1_H0BC_WIDTH                            (15)
#define RESERVED_15_1_H0BC_LSB                              (1)
#define RESERVED_15_1_H0BC_MASK                             (((1 << RESERVED_15_1_H0BC_WIDTH) - 1) << RESERVED_15_1_H0BC_LSB)
#define DIDTC_BUF_OVF_CLR_WIDTH                             (1)
#define DIDTC_BUF_OVF_CLR_LSB                               (0)
#define DIDTC_BUF_OVF_CLR_MASK                              (((1 << DIDTC_BUF_OVF_CLR_WIDTH) - 1) << DIDTC_BUF_OVF_CLR_LSB)

// #pragma endregion //DIDTC_OVF_CLR

// #pragma region //DIDTC_OVF
#define DIDTC_OVF                                           (0x000000c0) // 0x904220c0 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H0C0_PLL_WIDTH                       (15)
#define RESERVED_31_17_H0C0_PLL_LSB                         (17)
#define RESERVED_31_17_H0C0_PLL_MASK                        (((1 << RESERVED_31_17_H0C0_PLL_WIDTH) - 1) << RESERVED_31_17_H0C0_PLL_LSB)
#define DIDTC_OUT_OVF_WIDTH                                 (1)
#define DIDTC_OUT_OVF_LSB                                   (16)
#define DIDTC_OUT_OVF_MASK                                  (((1 << DIDTC_OUT_OVF_WIDTH) - 1) << DIDTC_OUT_OVF_LSB)
#define RESERVED_15_1_H0C0_WIDTH                            (15)
#define RESERVED_15_1_H0C0_LSB                              (1)
#define RESERVED_15_1_H0C0_MASK                             (((1 << RESERVED_15_1_H0C0_WIDTH) - 1) << RESERVED_15_1_H0C0_LSB)
#define DIDTC_BUF_OVF_WIDTH                                 (1)
#define DIDTC_BUF_OVF_LSB                                   (0)
#define DIDTC_BUF_OVF_MASK                                  (((1 << DIDTC_BUF_OVF_WIDTH) - 1) << DIDTC_BUF_OVF_LSB)

// #pragma endregion //DIDTC_OVF

// #pragma region //DSM_FCW_FR
#define DSM_FCW_FR                                          (0x000000c4) // 0x904220c4 # CHA=0x00000000; CHN=0x800;

#define DSM_FCW_FR_WIDTH                                    (32)
#define DSM_FCW_FR_LSB                                      (0)
#define DSM_FCW_FR_MASK                                     (0xffffffff)

// #pragma endregion //DSM_FCW_FR

// #pragma region //DSM_FCW_INT
#define DSM_FCW_INT                                         (0x000000c8) // 0x904220c8 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_8_H0C8_WIDTH                            (24)
#define RESERVED_31_8_H0C8_LSB                              (8)
#define RESERVED_31_8_H0C8_MASK                             (((1 << RESERVED_31_8_H0C8_WIDTH) - 1) << RESERVED_31_8_H0C8_LSB)
#define DSM_FCW_INT_WIDTH                                   (8)
#define DSM_FCW_INT_LSB                                     (0)
#define DSM_FCW_INT_MASK                                    (((1 << DSM_FCW_INT_WIDTH) - 1) << DSM_FCW_INT_LSB)

// #pragma endregion //DSM_FCW_INT

// #pragma region //DSM_CFG
#define DSM_CFG                                             (0x000000cc) // 0x904220cc # CHA=0x00000000; CHN=0x800;

#define DSM_CFG_UP_WIDTH                                    (1)
#define DSM_CFG_UP_LSB                                      (31)
#define DSM_CFG_UP_MASK                                     (((1 << DSM_CFG_UP_WIDTH) - 1) << DSM_CFG_UP_LSB)
#define DSM_PRBS_INITIAL_WIDTH                              (23)
#define DSM_PRBS_INITIAL_LSB                                (8)
#define DSM_PRBS_INITIAL_MASK                               (((1 << DSM_PRBS_INITIAL_WIDTH) - 1) << DSM_PRBS_INITIAL_LSB)
#define RESERVED_7_5_H0CC_WIDTH                             (3)
#define RESERVED_7_5_H0CC_LSB                               (5)
#define RESERVED_7_5_H0CC_MASK                              (((1 << RESERVED_7_5_H0CC_WIDTH) - 1) << RESERVED_7_5_H0CC_LSB)
#define DSM_ORDER3_EN_WIDTH                                 (1)
#define DSM_ORDER3_EN_LSB                                   (4)
#define DSM_ORDER3_EN_MASK                                  (((1 << DSM_ORDER3_EN_WIDTH) - 1) << DSM_ORDER3_EN_LSB)
#define RESERVED_3_2_H0CC_WIDTH                             (2)
#define RESERVED_3_2_H0CC_LSB                               (2)
#define RESERVED_3_2_H0CC_MASK                              (((1 << RESERVED_3_2_H0CC_WIDTH) - 1) << RESERVED_3_2_H0CC_LSB)
#define DSM_MODE_SEL_WIDTH                                  (2)
#define DSM_MODE_SEL_LSB                                    (0)
#define DSM_MODE_SEL_MASK                                   (((1 << DSM_MODE_SEL_WIDTH) - 1) << DSM_MODE_SEL_LSB)

// #pragma endregion //DSM_CFG

// #pragma region //DSM_OVF_CLR
#define DSM_OVF_CLR                                         (0x000000d0) // 0x904220d0 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H0D0_WIDTH                            (31)
#define RESERVED_31_1_H0D0_LSB                              (1)
#define RESERVED_31_1_H0D0_MASK                             (((1 << RESERVED_31_1_H0D0_WIDTH) - 1) << RESERVED_31_1_H0D0_LSB)
#define DSM_OVF_CLR_WIDTH                                   (1)
#define DSM_OVF_CLR_LSB                                     (0)
#define DSM_OVF_CLR_MASK                                    (((1 << DSM_OVF_CLR_WIDTH) - 1) << DSM_OVF_CLR_LSB)

// #pragma endregion //DSM_OVF_CLR

// #pragma region //DSM_OVF
#define DSM_OVF                                             (0x000000d4) // 0x904220d4 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H0D4_WIDTH                            (31)
#define RESERVED_31_1_H0D4_LSB                              (1)
#define RESERVED_31_1_H0D4_MASK                             (((1 << RESERVED_31_1_H0D4_WIDTH) - 1) << RESERVED_31_1_H0D4_LSB)
#define DSM_OVF_WIDTH                                       (1)
#define DSM_OVF_LSB                                         (0)
#define DSM_OVF_MASK                                        (((1 << DSM_OVF_WIDTH) - 1) << DSM_OVF_LSB)

// #pragma endregion //DSM_OVF

// #pragma region //LMS_1CFG
#define LMS_1CFG                                            (0x000000d8) // 0x904220d8 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_27_H0D8_WIDTH                           (5)
#define RESERVED_31_27_H0D8_LSB                             (27)
#define RESERVED_31_27_H0D8_MASK                            (((1 << RESERVED_31_27_H0D8_WIDTH) - 1) << RESERVED_31_27_H0D8_LSB)
#define LMS_ACC_LEN_WIDTH                                   (11)
#define LMS_ACC_LEN_LSB                                     (16)
#define LMS_ACC_LEN_MASK                                    (((1 << LMS_ACC_LEN_WIDTH) - 1) << LMS_ACC_LEN_LSB)
#define RESERVED_15_9_H0D8_WIDTH                            (7)
#define RESERVED_15_9_H0D8_LSB                              (9)
#define RESERVED_15_9_H0D8_MASK                             (((1 << RESERVED_15_9_H0D8_WIDTH) - 1) << RESERVED_15_9_H0D8_LSB)
#define LMS_DTC_CODE_SEL_WIDTH                              (1)
#define LMS_DTC_CODE_SEL_LSB                                (8)
#define LMS_DTC_CODE_SEL_MASK                               (((1 << LMS_DTC_CODE_SEL_WIDTH) - 1) << LMS_DTC_CODE_SEL_LSB)
#define RESERVED_7_3_H0D8_WIDTH                             (5)
#define RESERVED_7_3_H0D8_LSB                               (3)
#define RESERVED_7_3_H0D8_MASK                              (((1 << RESERVED_7_3_H0D8_WIDTH) - 1) << RESERVED_7_3_H0D8_LSB)
#define LMS_DLY_SEL_WIDTH                                   (3)
#define LMS_DLY_SEL_LSB                                     (0)
#define LMS_DLY_SEL_MASK                                    (((1 << LMS_DLY_SEL_WIDTH) - 1) << LMS_DLY_SEL_LSB)

// #pragma endregion //LMS_1CFG

// #pragma region //LMS_2CFG
#define LMS_2CFG                                            (0x000000dc) // 0x904220dc # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_25_H0DC_WIDTH                           (7)
#define RESERVED_31_25_H0DC_LSB                             (25)
#define RESERVED_31_25_H0DC_MASK                            (((1 << RESERVED_31_25_H0DC_WIDTH) - 1) << RESERVED_31_25_H0DC_LSB)
#define LMS_CFGOK_WIDTH                                     (1)
#define LMS_CFGOK_LSB                                       (24)
#define LMS_CFGOK_MASK                                      (((1 << LMS_CFGOK_WIDTH) - 1) << LMS_CFGOK_LSB)
#define RESERVED_23_18_H0DC_WIDTH                           (6)
#define RESERVED_23_18_H0DC_LSB                             (18)
#define RESERVED_23_18_H0DC_MASK                            (((1 << RESERVED_23_18_H0DC_WIDTH) - 1) << RESERVED_23_18_H0DC_LSB)
#define LMS_OS_WIDTH                                        (10)
#define LMS_OS_LSB                                          (8)
#define LMS_OS_MASK                                         (((1 << LMS_OS_WIDTH) - 1) << LMS_OS_LSB)
#define RESERVED_7_5_H0DC_WIDTH                             (3)
#define RESERVED_7_5_H0DC_LSB                               (5)
#define RESERVED_7_5_H0DC_MASK                              (((1 << RESERVED_7_5_H0DC_WIDTH) - 1) << RESERVED_7_5_H0DC_LSB)
#define LMS_U_WIDTH                                         (5)
#define LMS_U_LSB                                           (0)
#define LMS_U_MASK                                          (((1 << LMS_U_WIDTH) - 1) << LMS_U_LSB)

// #pragma endregion //LMS_2CFG

// #pragma region //LMS_GAIN_RDWREN
#define LMS_GAIN_RDWREN                                     (0x000000e0) // 0x904220e0 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_2_H0E0_WIDTH                            (30)
#define RESERVED_31_2_H0E0_LSB                              (2)
#define RESERVED_31_2_H0E0_MASK                             (((1 << RESERVED_31_2_H0E0_WIDTH) - 1) << RESERVED_31_2_H0E0_LSB)
#define LMS_GAIN_RDEN_WIDTH                                 (1)
#define LMS_GAIN_RDEN_LSB                                   (1)
#define LMS_GAIN_RDEN_MASK                                  (((1 << LMS_GAIN_RDEN_WIDTH) - 1) << LMS_GAIN_RDEN_LSB)
#define LMS_GAIN_WREN_WIDTH                                 (1)
#define LMS_GAIN_WREN_LSB                                   (0)
#define LMS_GAIN_WREN_MASK                                  (((1 << LMS_GAIN_WREN_WIDTH) - 1) << LMS_GAIN_WREN_LSB)

// #pragma endregion //LMS_GAIN_RDWREN

// #pragma region //LMS_GAIN_LOW
#define LMS_GAIN_LOW                                        (0x000000e4) // 0x904220e4 # CHA=0x00000000; CHN=0x800;

#define LMS_GAIN_INI_LOW_WIDTH                              (32)
#define LMS_GAIN_INI_LOW_LSB                                (0)
#define LMS_GAIN_INI_LOW_MASK                               (0xffffffff)

// #pragma endregion //LMS_GAIN_LOW

// #pragma region //LMS_GAIN_HIGH
#define LMS_GAIN_HIGH                                       (0x000000e8) // 0x904220e8 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_3_H0E8_WIDTH                            (29)
#define RESERVED_31_3_H0E8_LSB                              (3)
#define RESERVED_31_3_H0E8_MASK                             (((1 << RESERVED_31_3_H0E8_WIDTH) - 1) << RESERVED_31_3_H0E8_LSB)
#define LMS_GAIN_INI_HIGH_WIDTH                             (3)
#define LMS_GAIN_INI_HIGH_LSB                               (0)
#define LMS_GAIN_INI_HIGH_MASK                              (((1 << LMS_GAIN_INI_HIGH_WIDTH) - 1) << LMS_GAIN_INI_HIGH_LSB)

// #pragma endregion //LMS_GAIN_HIGH

// #pragma region //LMS_GAIN_RD_LOW
#define LMS_GAIN_RD_LOW                                     (0x000000ec) // 0x904220ec # CHA=0x00000000; CHN=0x800;

#define LMS_GAIN_RD_LOW_WIDTH                               (32)
#define LMS_GAIN_RD_LOW_LSB                                 (0)
#define LMS_GAIN_RD_LOW_MASK                                (0xffffffff)

// #pragma endregion //LMS_GAIN_RD_LOW

// #pragma region //LMS_GAIN_RD_HIGH
#define LMS_GAIN_RD_HIGH                                    (0x000000f0) // 0x904220f0 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_3_H0F0_WIDTH                            (29)
#define RESERVED_31_3_H0F0_LSB                              (3)
#define RESERVED_31_3_H0F0_MASK                             (((1 << RESERVED_31_3_H0F0_WIDTH) - 1) << RESERVED_31_3_H0F0_LSB)
#define LMS_GAIN_RD_HIGH_WIDTH                              (3)
#define LMS_GAIN_RD_HIGH_LSB                                (0)
#define LMS_GAIN_RD_HIGH_MASK                               (((1 << LMS_GAIN_RD_HIGH_WIDTH) - 1) << LMS_GAIN_RD_HIGH_LSB)

// #pragma endregion //LMS_GAIN_RD_HIGH

// #pragma region //LMS_OVF_CLR
#define LMS_OVF_CLR                                         (0x000000f4) // 0x904220f4 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H0F4_WIDTH                           (15)
#define RESERVED_31_17_H0F4_LSB                             (17)
#define RESERVED_31_17_H0F4_MASK                            (((1 << RESERVED_31_17_H0F4_WIDTH) - 1) << RESERVED_31_17_H0F4_LSB)
#define LMS_OUT_OVF_CLR_WIDTH                               (1)
#define LMS_OUT_OVF_CLR_LSB                                 (16)
#define LMS_OUT_OVF_CLR_MASK                                (((1 << LMS_OUT_OVF_CLR_WIDTH) - 1) << LMS_OUT_OVF_CLR_LSB)
#define RESERVED_15_1_H0F4_WIDTH                            (15)
#define RESERVED_15_1_H0F4_LSB                              (1)
#define RESERVED_15_1_H0F4_MASK                             (((1 << RESERVED_15_1_H0F4_WIDTH) - 1) << RESERVED_15_1_H0F4_LSB)
#define LMS_OVF_CLR_WIDTH                                   (1)
#define LMS_OVF_CLR_LSB                                     (0)
#define LMS_OVF_CLR_MASK                                    (((1 << LMS_OVF_CLR_WIDTH) - 1) << LMS_OVF_CLR_LSB)

// #pragma endregion //LMS_OVF_CLR

// #pragma region //LMS_OVF
#define LMS_OVF                                             (0x000000f8) // 0x904220f8 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_17_H0F8_WIDTH                           (15)
#define RESERVED_31_17_H0F8_LSB                             (17)
#define RESERVED_31_17_H0F8_MASK                            (((1 << RESERVED_31_17_H0F8_WIDTH) - 1) << RESERVED_31_17_H0F8_LSB)
#define LMS_OUT_OVF_WIDTH                                   (1)
#define LMS_OUT_OVF_LSB                                     (16)
#define LMS_OUT_OVF_MASK                                    (((1 << LMS_OUT_OVF_WIDTH) - 1) << LMS_OUT_OVF_LSB)
#define RESERVED_15_1_H0F8_WIDTH                            (15)
#define RESERVED_15_1_H0F8_LSB                              (1)
#define RESERVED_15_1_H0F8_MASK                             (((1 << RESERVED_15_1_H0F8_WIDTH) - 1) << RESERVED_15_1_H0F8_LSB)
#define LMS_OVF_WIDTH                                       (1)
#define LMS_OVF_LSB                                         (0)
#define LMS_OVF_MASK                                        (((1 << LMS_OVF_WIDTH) - 1) << LMS_OVF_LSB)

// #pragma endregion //LMS_OVF

// #pragma region //LMS_MANUAL_CTRL
#define LMS_MANUAL_CTRL                                     (0x000000fc) // 0x904220fc # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_26_H0FC_WIDTH                           (6)
#define RESERVED_31_26_H0FC_LSB                             (26)
#define RESERVED_31_26_H0FC_MASK                            (((1 << RESERVED_31_26_H0FC_WIDTH) - 1) << RESERVED_31_26_H0FC_LSB)
#define LMS_MANUAL_DTC_STATIC_CODE_WIDTH                    (10)
#define LMS_MANUAL_DTC_STATIC_CODE_LSB                      (16)
#define LMS_MANUAL_DTC_STATIC_CODE_MASK                     (((1 << LMS_MANUAL_DTC_STATIC_CODE_WIDTH) - 1) << LMS_MANUAL_DTC_STATIC_CODE_LSB)
#define RESERVED_15_9_H0FC_WIDTH                            (7)
#define RESERVED_15_9_H0FC_LSB                              (9)
#define RESERVED_15_9_H0FC_MASK                             (((1 << RESERVED_15_9_H0FC_WIDTH) - 1) << RESERVED_15_9_H0FC_LSB)
#define LMS_MANUAL_TX_MODE_WIDTH                            (1)
#define LMS_MANUAL_TX_MODE_LSB                              (8)
#define LMS_MANUAL_TX_MODE_MASK                             (((1 << LMS_MANUAL_TX_MODE_WIDTH) - 1) << LMS_MANUAL_TX_MODE_LSB)
#define RESERVED_7_1_H0FC_WIDTH                             (7)
#define RESERVED_7_1_H0FC_LSB                               (1)
#define RESERVED_7_1_H0FC_MASK                              (((1 << RESERVED_7_1_H0FC_WIDTH) - 1) << RESERVED_7_1_H0FC_LSB)
#define LMS_MANUAL_TX_DTC_CODE_EN_WIDTH                     (1)
#define LMS_MANUAL_TX_DTC_CODE_EN_LSB                       (0)
#define LMS_MANUAL_TX_DTC_CODE_EN_MASK                      (((1 << LMS_MANUAL_TX_DTC_CODE_EN_WIDTH) - 1) << LMS_MANUAL_TX_DTC_CODE_EN_LSB)

// #pragma endregion //LMS_MANUAL_CTRL

// #pragma region //LMS_MANUAL_PERIOD_CTRL
#define LMS_MANUAL_PERIOD_CTRL                              (0x00000100) // 0x90422100 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_10_H100_WIDTH                           (22)
#define RESERVED_31_10_H100_LSB                             (10)
#define RESERVED_31_10_H100_MASK                            (((1 << RESERVED_31_10_H100_WIDTH) - 1) << RESERVED_31_10_H100_LSB)
#define LMS_MANUAL_TX_PERIOD_N_WIDTH                        (10)
#define LMS_MANUAL_TX_PERIOD_N_LSB                          (0)
#define LMS_MANUAL_TX_PERIOD_N_MASK                         (((1 << LMS_MANUAL_TX_PERIOD_N_WIDTH) - 1) << LMS_MANUAL_TX_PERIOD_N_LSB)

// #pragma endregion //LMS_MANUAL_PERIOD_CTRL

// #pragma region //LMS_MANUAL_PERIOD_CODE_CTRL
#define LMS_MANUAL_PERIOD_CODE_CTRL                         (0x00000104) // 0x90422104 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_26_H104_WIDTH                           (6)
#define RESERVED_31_26_H104_LSB                             (26)
#define RESERVED_31_26_H104_MASK                            (((1 << RESERVED_31_26_H104_WIDTH) - 1) << RESERVED_31_26_H104_LSB)
#define LMS_MANUAL_TX_PERIOD_CODE_B_WIDTH                   (10)
#define LMS_MANUAL_TX_PERIOD_CODE_B_LSB                     (16)
#define LMS_MANUAL_TX_PERIOD_CODE_B_MASK                    (((1 << LMS_MANUAL_TX_PERIOD_CODE_B_WIDTH) - 1) << LMS_MANUAL_TX_PERIOD_CODE_B_LSB)
#define RESERVED_15_10_H104_WIDTH                           (6)
#define RESERVED_15_10_H104_LSB                             (10)
#define RESERVED_15_10_H104_MASK                            (((1 << RESERVED_15_10_H104_WIDTH) - 1) << RESERVED_15_10_H104_LSB)
#define LMS_MANUAL_TX_PERIOD_CODE_A_WIDTH                   (10)
#define LMS_MANUAL_TX_PERIOD_CODE_A_LSB                     (0)
#define LMS_MANUAL_TX_PERIOD_CODE_A_MASK                    (((1 << LMS_MANUAL_TX_PERIOD_CODE_A_WIDTH) - 1) << LMS_MANUAL_TX_PERIOD_CODE_A_LSB)

// #pragma endregion //LMS_MANUAL_PERIOD_CODE_CTRL

// #pragma region //INL_1CFG
#define INL_1CFG                                            (0x00000108) // 0x90422108 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_7_H108_WIDTH                            (25)
#define RESERVED_31_7_H108_LSB                              (7)
#define RESERVED_31_7_H108_MASK                             (((1 << RESERVED_31_7_H108_WIDTH) - 1) << RESERVED_31_7_H108_LSB)
#define DTC_INL_U_A_WIDTH                                   (3)
#define DTC_INL_U_A_LSB                                     (4)
#define DTC_INL_U_A_MASK                                    (((1 << DTC_INL_U_A_WIDTH) - 1) << DTC_INL_U_A_LSB)
#define RESERVED_3_1_H108_WIDTH                             (3)
#define RESERVED_3_1_H108_LSB                               (1)
#define RESERVED_3_1_H108_MASK                              (((1 << RESERVED_3_1_H108_WIDTH) - 1) << RESERVED_3_1_H108_LSB)
#define DTC_INL_EN_WIDTH                                    (1)
#define DTC_INL_EN_LSB                                      (0)
#define DTC_INL_EN_MASK                                     (((1 << DTC_INL_EN_WIDTH) - 1) << DTC_INL_EN_LSB)

// #pragma endregion //INL_1CFG

// #pragma region //INL_2CFG
#define INL_2CFG                                            (0x0000010c) // 0x9042210c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_2_H10C_WIDTH                            (30)
#define RESERVED_31_2_H10C_LSB                              (2)
#define RESERVED_31_2_H10C_MASK                             (((1 << RESERVED_31_2_H10C_WIDTH) - 1) << RESERVED_31_2_H10C_LSB)
#define DTC_INL_COE2_RDEN_WIDTH                             (1)
#define DTC_INL_COE2_RDEN_LSB                               (1)
#define DTC_INL_COE2_RDEN_MASK                              (((1 << DTC_INL_COE2_RDEN_WIDTH) - 1) << DTC_INL_COE2_RDEN_LSB)
#define DTC_INL_COE2_WREN_WIDTH                             (1)
#define DTC_INL_COE2_WREN_LSB                               (0)
#define DTC_INL_COE2_WREN_MASK                              (((1 << DTC_INL_COE2_WREN_WIDTH) - 1) << DTC_INL_COE2_WREN_LSB)

// #pragma endregion //INL_2CFG

// #pragma region //INL_3CFG
#define INL_3CFG                                            (0x00000110) // 0x90422110 # CHA=0x00000000; CHN=0x800;

#define DTC_INL_COE2_INI_LOW_WIDTH                          (32)
#define DTC_INL_COE2_INI_LOW_LSB                            (0)
#define DTC_INL_COE2_INI_LOW_MASK                           (0xffffffff)

// #pragma endregion //INL_3CFG

// #pragma region //INL_4CFG
#define INL_4CFG                                            (0x00000114) // 0x90422114 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_7_H114_WIDTH                            (25)
#define RESERVED_31_7_H114_LSB                              (7)
#define RESERVED_31_7_H114_MASK                             (((1 << RESERVED_31_7_H114_WIDTH) - 1) << RESERVED_31_7_H114_LSB)
#define DTC_INL_COE2_INI_HIGH_WIDTH                         (7)
#define DTC_INL_COE2_INI_HIGH_LSB                           (0)
#define DTC_INL_COE2_INI_HIGH_MASK                          (((1 << DTC_INL_COE2_INI_HIGH_WIDTH) - 1) << DTC_INL_COE2_INI_HIGH_LSB)

// #pragma endregion //INL_4CFG

// #pragma region //INL_5CFG
#define INL_5CFG                                            (0x00000118) // 0x90422118 # CHA=0x00000000; CHN=0x800;

#define DTC_INL_COE2_RD_LOW_WIDTH                           (32)
#define DTC_INL_COE2_RD_LOW_LSB                             (0)
#define DTC_INL_COE2_RD_LOW_MASK                            (0xffffffff)

// #pragma endregion //INL_5CFG

// #pragma region //INL_6CFG
#define INL_6CFG                                            (0x0000011c) // 0x9042211c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_7_H11C_WIDTH                            (25)
#define RESERVED_31_7_H11C_LSB                              (7)
#define RESERVED_31_7_H11C_MASK                             (((1 << RESERVED_31_7_H11C_WIDTH) - 1) << RESERVED_31_7_H11C_LSB)
#define DTC_INL_COE2_RD_HIGH_WIDTH                          (7)
#define DTC_INL_COE2_RD_HIGH_LSB                            (0)
#define DTC_INL_COE2_RD_HIGH_MASK                           (((1 << DTC_INL_COE2_RD_HIGH_WIDTH) - 1) << DTC_INL_COE2_RD_HIGH_LSB)

// #pragma endregion //INL_6CFG

// #pragma region //INL_7CFG
#define INL_7CFG                                            (0x00000120) // 0x90422120 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H120_WIDTH                            (31)
#define RESERVED_31_1_H120_LSB                              (1)
#define RESERVED_31_1_H120_MASK                             (((1 << RESERVED_31_1_H120_WIDTH) - 1) << RESERVED_31_1_H120_LSB)
#define DTC_INL_OVF_CLR_WIDTH                               (1)
#define DTC_INL_OVF_CLR_LSB                                 (0)
#define DTC_INL_OVF_CLR_MASK                                (((1 << DTC_INL_OVF_CLR_WIDTH) - 1) << DTC_INL_OVF_CLR_LSB)

// #pragma endregion //INL_7CFG

// #pragma region //INL_8CFG
#define INL_8CFG                                            (0x00000124) // 0x90422124 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H124_PLL_WIDTH                        (31)
#define RESERVED_31_1_H124_PLL_LSB                          (1)
#define RESERVED_31_1_H124_PLL_MASK                         (((1 << RESERVED_31_1_H124_PLL_WIDTH) - 1) << RESERVED_31_1_H124_PLL_LSB)
#define DTC_INL_OVF_WIDTH                                   (1)
#define DTC_INL_OVF_LSB                                     (0)
#define DTC_INL_OVF_MASK                                    (((1 << DTC_INL_OVF_WIDTH) - 1) << DTC_INL_OVF_LSB)

// #pragma endregion //INL_8CFG

// #pragma region //PSGEN_OUT_SEL
#define PSGEN_OUT_SEL                                       (0x00000128) // 0x90422128 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H128_WIDTH                            (31)
#define RESERVED_31_1_H128_LSB                              (1)
#define RESERVED_31_1_H128_MASK                             (((1 << RESERVED_31_1_H128_WIDTH) - 1) << RESERVED_31_1_H128_LSB)
#define PSGEN_PS_SEL_WIDTH                                  (1)
#define PSGEN_PS_SEL_LSB                                    (0)
#define PSGEN_PS_SEL_MASK                                   (((1 << PSGEN_PS_SEL_WIDTH) - 1) << PSGEN_PS_SEL_LSB)

// #pragma endregion //PSGEN_OUT_SEL

// #pragma region //PSGEN_ERR_RECORD_CLR
#define PSGEN_ERR_RECORD_CLR                                (0x0000012c) // 0x9042212c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H12C_PLL_WIDTH                        (31)
#define RESERVED_31_1_H12C_PLL_LSB                          (1)
#define RESERVED_31_1_H12C_PLL_MASK                         (((1 << RESERVED_31_1_H12C_PLL_WIDTH) - 1) << RESERVED_31_1_H12C_PLL_LSB)
#define PSGEN_NDIV_ERR_CLR_WIDTH                            (1)
#define PSGEN_NDIV_ERR_CLR_LSB                              (0)
#define PSGEN_NDIV_ERR_CLR_MASK                             (((1 << PSGEN_NDIV_ERR_CLR_WIDTH) - 1) << PSGEN_NDIV_ERR_CLR_LSB)

// #pragma endregion //PSGEN_ERR_RECORD_CLR

// #pragma region //PSGEN_NDIV_LOWERR
#define PSGEN_NDIV_LOWERR                                   (0x00000130) // 0x90422130 # CHA=0x00000000; CHN=0x800;

#define PSGEN_NDIV_LOWNUM_WIDTH                             (16)
#define PSGEN_NDIV_LOWNUM_LSB                               (16)
#define PSGEN_NDIV_LOWNUM_MASK                              (((1 << PSGEN_NDIV_LOWNUM_WIDTH) - 1) << PSGEN_NDIV_LOWNUM_LSB)
#define RESERVED_15_1_H130_WIDTH                            (15)
#define RESERVED_15_1_H130_LSB                              (1)
#define RESERVED_15_1_H130_MASK                             (((1 << RESERVED_15_1_H130_WIDTH) - 1) << RESERVED_15_1_H130_LSB)
#define PSGEN_NDIV_LOWERR_WIDTH                             (1)
#define PSGEN_NDIV_LOWERR_LSB                               (0)
#define PSGEN_NDIV_LOWERR_MASK                              (((1 << PSGEN_NDIV_LOWERR_WIDTH) - 1) << PSGEN_NDIV_LOWERR_LSB)

// #pragma endregion //PSGEN_NDIV_LOWERR

// #pragma region //PSGEN_NDIV_HIGHERR
#define PSGEN_NDIV_HIGHERR                                  (0x00000134) // 0x90422134 # CHA=0x00000000; CHN=0x800;

#define PSGEN_NDIV_HIGHNUM_WIDTH                            (16)
#define PSGEN_NDIV_HIGHNUM_LSB                              (16)
#define PSGEN_NDIV_HIGHNUM_MASK                             (((1 << PSGEN_NDIV_HIGHNUM_WIDTH) - 1) << PSGEN_NDIV_HIGHNUM_LSB)
#define RESERVED_15_1_H134_WIDTH                            (15)
#define RESERVED_15_1_H134_LSB                              (1)
#define RESERVED_15_1_H134_MASK                             (((1 << RESERVED_15_1_H134_WIDTH) - 1) << RESERVED_15_1_H134_LSB)
#define PSGEN_NDIV_HIGHERR_WIDTH                            (1)
#define PSGEN_NDIV_HIGHERR_LSB                              (0)
#define PSGEN_NDIV_HIGHERR_MASK                             (((1 << PSGEN_NDIV_HIGHERR_WIDTH) - 1) << PSGEN_NDIV_HIGHERR_LSB)

// #pragma endregion //PSGEN_NDIV_HIGHERR

// #pragma region //PSGEN_MANUAL_CTRL
#define PSGEN_MANUAL_CTRL                                   (0x00000138) // 0x90422138 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_21_H138_WIDTH                           (11)
#define RESERVED_31_21_H138_LSB                             (21)
#define RESERVED_31_21_H138_MASK                            (((1 << RESERVED_31_21_H138_WIDTH) - 1) << RESERVED_31_21_H138_LSB)
#define PSGEN_MANUAL_S_WIDTH                                (5)
#define PSGEN_MANUAL_S_LSB                                  (16)
#define PSGEN_MANUAL_S_MASK                                 (((1 << PSGEN_MANUAL_S_WIDTH) - 1) << PSGEN_MANUAL_S_LSB)
#define RESERVED_15_14_H138_WIDTH                           (2)
#define RESERVED_15_14_H138_LSB                             (14)
#define RESERVED_15_14_H138_MASK                            (((1 << RESERVED_15_14_H138_WIDTH) - 1) << RESERVED_15_14_H138_LSB)
#define PSGEN_MANUAL_P_WIDTH                                (6)
#define PSGEN_MANUAL_P_LSB                                  (8)
#define PSGEN_MANUAL_P_MASK                                 (((1 << PSGEN_MANUAL_P_WIDTH) - 1) << PSGEN_MANUAL_P_LSB)
#define RESERVED_7_1_H138_WIDTH                             (7)
#define RESERVED_7_1_H138_LSB                               (1)
#define RESERVED_7_1_H138_MASK                              (((1 << RESERVED_7_1_H138_WIDTH) - 1) << RESERVED_7_1_H138_LSB)
#define PSGEN_MANUAL_CFG_PS_EN_WIDTH                        (1)
#define PSGEN_MANUAL_CFG_PS_EN_LSB                          (0)
#define PSGEN_MANUAL_CFG_PS_EN_MASK                         (((1 << PSGEN_MANUAL_CFG_PS_EN_WIDTH) - 1) << PSGEN_MANUAL_CFG_PS_EN_LSB)

// #pragma endregion //PSGEN_MANUAL_CTRL

// #pragma region //LOCKDET_REF_CLK_NDIV
#define LOCKDET_REF_CLK_NDIV                                (0x0000013c) // 0x9042213c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_24_H13C_WIDTH                           (8)
#define RESERVED_31_24_H13C_LSB                             (24)
#define RESERVED_31_24_H13C_MASK                            (((1 << RESERVED_31_24_H13C_WIDTH) - 1) << RESERVED_31_24_H13C_LSB)
#define LOCKDET_REF_CLK_NDIV_WIDTH                          (24)
#define LOCKDET_REF_CLK_NDIV_LSB                            (0)
#define LOCKDET_REF_CLK_NDIV_MASK                           (((1 << LOCKDET_REF_CLK_NDIV_WIDTH) - 1) << LOCKDET_REF_CLK_NDIV_LSB)

// #pragma endregion //LOCKDET_REF_CLK_NDIV

// #pragma region //LOCKDET_KBASE
#define LOCKDET_KBASE                                       (0x00000140) // 0x90422140 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_28_H140_WIDTH                           (4)
#define RESERVED_31_28_H140_LSB                             (28)
#define RESERVED_31_28_H140_MASK                            (((1 << RESERVED_31_28_H140_WIDTH) - 1) << RESERVED_31_28_H140_LSB)
#define LOCKDET_KBASE_WIDTH                                 (28)
#define LOCKDET_KBASE_LSB                                   (0)
#define LOCKDET_KBASE_MASK                                  (((1 << LOCKDET_KBASE_WIDTH) - 1) << LOCKDET_KBASE_LSB)

// #pragma endregion //LOCKDET_KBASE

// #pragma region //LOCKDET_LOCKED_KDELTA_TH
#define LOCKDET_LOCKED_KDELTA_TH                            (0x00000144) // 0x90422144 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_28_H144_WIDTH                           (4)
#define RESERVED_31_28_H144_LSB                             (28)
#define RESERVED_31_28_H144_MASK                            (((1 << RESERVED_31_28_H144_WIDTH) - 1) << RESERVED_31_28_H144_LSB)
#define LOCKDET_LOCKED_KDELTA_TH_WIDTH                      (28)
#define LOCKDET_LOCKED_KDELTA_TH_LSB                        (0)
#define LOCKDET_LOCKED_KDELTA_TH_MASK                       (((1 << LOCKDET_LOCKED_KDELTA_TH_WIDTH) - 1) << LOCKDET_LOCKED_KDELTA_TH_LSB)

// #pragma endregion //LOCKDET_LOCKED_KDELTA_TH

// #pragma region //LOCKDET_UNLOCK_KDELTA_TH
#define LOCKDET_UNLOCK_KDELTA_TH                            (0x00000148) // 0x90422148 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_28_H148_WIDTH                           (4)
#define RESERVED_31_28_H148_LSB                             (28)
#define RESERVED_31_28_H148_MASK                            (((1 << RESERVED_31_28_H148_WIDTH) - 1) << RESERVED_31_28_H148_LSB)
#define LOCKDET_UNLOCK_KDELTA_TH_WIDTH                      (28)
#define LOCKDET_UNLOCK_KDELTA_TH_LSB                        (0)
#define LOCKDET_UNLOCK_KDELTA_TH_MASK                       (((1 << LOCKDET_UNLOCK_KDELTA_TH_WIDTH) - 1) << LOCKDET_UNLOCK_KDELTA_TH_LSB)

// #pragma endregion //LOCKDET_UNLOCK_KDELTA_TH

// #pragma region //LOCKDET_KDET_OVF_CLR
#define LOCKDET_KDET_OVF_CLR                                (0x0000014c) // 0x9042214c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H14C_WIDTH                            (31)
#define RESERVED_31_1_H14C_LSB                              (1)
#define RESERVED_31_1_H14C_MASK                             (((1 << RESERVED_31_1_H14C_WIDTH) - 1) << RESERVED_31_1_H14C_LSB)
#define LOCKDET_KDET_OVF_CLR_WIDTH                          (1)
#define LOCKDET_KDET_OVF_CLR_LSB                            (0)
#define LOCKDET_KDET_OVF_CLR_MASK                           (((1 << LOCKDET_KDET_OVF_CLR_WIDTH) - 1) << LOCKDET_KDET_OVF_CLR_LSB)

// #pragma endregion //LOCKDET_KDET_OVF_CLR

// #pragma region //LOCKDET_KDET_OVF
#define LOCKDET_KDET_OVF                                    (0x00000150) // 0x90422150 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H150_PLL_WIDTH                        (31)
#define RESERVED_31_1_H150_PLL_LSB                          (1)
#define RESERVED_31_1_H150_PLL_MASK                         (((1 << RESERVED_31_1_H150_PLL_WIDTH) - 1) << RESERVED_31_1_H150_PLL_LSB)
#define LOCKDET_KDET_OVF_WIDTH                              (1)
#define LOCKDET_KDET_OVF_LSB                                (0)
#define LOCKDET_KDET_OVF_MASK                               (((1 << LOCKDET_KDET_OVF_WIDTH) - 1) << LOCKDET_KDET_OVF_LSB)

// #pragma endregion //LOCKDET_KDET_OVF

// #pragma region //LOCKDET_LOCKED_WIN
#define LOCKDET_LOCKED_WIN                                  (0x00000154) // 0x90422154 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_10_H154_WIDTH                           (22)
#define RESERVED_31_10_H154_LSB                             (10)
#define RESERVED_31_10_H154_MASK                            (((1 << RESERVED_31_10_H154_WIDTH) - 1) << RESERVED_31_10_H154_LSB)
#define LOCKDET_LOCKED_WIN_WIDTH                            (10)
#define LOCKDET_LOCKED_WIN_LSB                              (0)
#define LOCKDET_LOCKED_WIN_MASK                             (((1 << LOCKDET_LOCKED_WIN_WIDTH) - 1) << LOCKDET_LOCKED_WIN_LSB)

// #pragma endregion //LOCKDET_LOCKED_WIN

// #pragma region //LOCKDET_UNLOCK_WIN
#define LOCKDET_UNLOCK_WIN                                  (0x00000158) // 0x90422158 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_10_H158_WIDTH                           (22)
#define RESERVED_31_10_H158_LSB                             (10)
#define RESERVED_31_10_H158_MASK                            (((1 << RESERVED_31_10_H158_WIDTH) - 1) << RESERVED_31_10_H158_LSB)
#define LOCKDET_UNLOCK_WIN_WIDTH                            (10)
#define LOCKDET_UNLOCK_WIN_LSB                              (0)
#define LOCKDET_UNLOCK_WIN_MASK                             (((1 << LOCKDET_UNLOCK_WIN_WIDTH) - 1) << LOCKDET_UNLOCK_WIN_LSB)

// #pragma endregion //LOCKDET_UNLOCK_WIN

// #pragma region //LOCKDET_LOCKED_WIN_TH
#define LOCKDET_LOCKED_WIN_TH                               (0x0000015c) // 0x9042215c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_10_H15C_WIDTH                           (22)
#define RESERVED_31_10_H15C_LSB                             (10)
#define RESERVED_31_10_H15C_MASK                            (((1 << RESERVED_31_10_H15C_WIDTH) - 1) << RESERVED_31_10_H15C_LSB)
#define LOCKDET_LOCKED_WIN_TH_WIDTH                         (10)
#define LOCKDET_LOCKED_WIN_TH_LSB                           (0)
#define LOCKDET_LOCKED_WIN_TH_MASK                          (((1 << LOCKDET_LOCKED_WIN_TH_WIDTH) - 1) << LOCKDET_LOCKED_WIN_TH_LSB)

// #pragma endregion //LOCKDET_LOCKED_WIN_TH

// #pragma region //LOCKDET_UNLOCK_WIN_TH
#define LOCKDET_UNLOCK_WIN_TH                               (0x00000160) // 0x90422160 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_10_H160_WIDTH                           (22)
#define RESERVED_31_10_H160_LSB                             (10)
#define RESERVED_31_10_H160_MASK                            (((1 << RESERVED_31_10_H160_WIDTH) - 1) << RESERVED_31_10_H160_LSB)
#define LOCKDET_UNLOCK_WIN_TH_WIDTH                         (10)
#define LOCKDET_UNLOCK_WIN_TH_LSB                           (0)
#define LOCKDET_UNLOCK_WIN_TH_MASK                          (((1 << LOCKDET_UNLOCK_WIN_TH_WIDTH) - 1) << LOCKDET_UNLOCK_WIN_TH_LSB)

// #pragma endregion //LOCKDET_UNLOCK_WIN_TH

// #pragma region //LOCKDET_LOCKUNLOCKWINTIMESRDEN
#define LOCKDET_LOCKUNLOCKWINTIMESRDEN                      (0x00000164) // 0x90422164 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H164_WIDTH                            (31)
#define RESERVED_31_1_H164_LSB                              (1)
#define RESERVED_31_1_H164_MASK                             (((1 << RESERVED_31_1_H164_WIDTH) - 1) << RESERVED_31_1_H164_LSB)
#define LOCKDET_LOCKUNLOCK_TIMES_RDEN_WIDTH                 (1)
#define LOCKDET_LOCKUNLOCK_TIMES_RDEN_LSB                   (0)
#define LOCKDET_LOCKUNLOCK_TIMES_RDEN_MASK                  (((1 << LOCKDET_LOCKUNLOCK_TIMES_RDEN_WIDTH) - 1) << LOCKDET_LOCKUNLOCK_TIMES_RDEN_LSB)

// #pragma endregion //LOCKDET_LOCKUNLOCKWINTIMESRDEN

// #pragma region //LOCKDET_LOCKED_UNLOCK_WIN_TIMES
#define LOCKDET_LOCKED_UNLOCK_WIN_TIMES                     (0x00000168) // 0x90422168 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_26_H168_WIDTH                           (6)
#define RESERVED_31_26_H168_LSB                             (26)
#define RESERVED_31_26_H168_MASK                            (((1 << RESERVED_31_26_H168_WIDTH) - 1) << RESERVED_31_26_H168_LSB)
#define LOCKDET_UNLOCK_TIMES_WIDTH                          (10)
#define LOCKDET_UNLOCK_TIMES_LSB                            (16)
#define LOCKDET_UNLOCK_TIMES_MASK                           (((1 << LOCKDET_UNLOCK_TIMES_WIDTH) - 1) << LOCKDET_UNLOCK_TIMES_LSB)
#define RESERVED_15_10_H168_WIDTH                           (6)
#define RESERVED_15_10_H168_LSB                             (10)
#define RESERVED_15_10_H168_MASK                            (((1 << RESERVED_15_10_H168_WIDTH) - 1) << RESERVED_15_10_H168_LSB)
#define LOCKDET_LOCKED_TIMES_WIDTH                          (10)
#define LOCKDET_LOCKED_TIMES_LSB                            (0)
#define LOCKDET_LOCKED_TIMES_MASK                           (((1 << LOCKDET_LOCKED_TIMES_WIDTH) - 1) << LOCKDET_LOCKED_TIMES_LSB)

// #pragma endregion //LOCKDET_LOCKED_UNLOCK_WIN_TIMES

// #pragma region //LOCKDET_LOCKED_SEL
#define LOCKDET_LOCKED_SEL                                  (0x0000016c) // 0x9042216c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_2_H16C_WIDTH                            (30)
#define RESERVED_31_2_H16C_LSB                              (2)
#define RESERVED_31_2_H16C_MASK                             (((1 << RESERVED_31_2_H16C_WIDTH) - 1) << RESERVED_31_2_H16C_LSB)
#define LOCKDET_LOCKED_SEL_WIDTH                            (2)
#define LOCKDET_LOCKED_SEL_LSB                              (0)
#define LOCKDET_LOCKED_SEL_MASK                             (((1 << LOCKDET_LOCKED_SEL_WIDTH) - 1) << LOCKDET_LOCKED_SEL_LSB)

// #pragma endregion //LOCKDET_LOCKED_SEL

// #pragma region //LOCKDET_LOCKED_IND
#define LOCKDET_LOCKED_IND                                  (0x00000170) // 0x90422170 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_3_H170_WIDTH                            (29)
#define RESERVED_31_3_H170_LSB                              (3)
#define RESERVED_31_3_H170_MASK                             (((1 << RESERVED_31_3_H170_WIDTH) - 1) << RESERVED_31_3_H170_LSB)
#define LOCKDET_WIN_UNLOCK_FLAG_WIDTH                       (1)
#define LOCKDET_WIN_UNLOCK_FLAG_LSB                         (2)
#define LOCKDET_WIN_UNLOCK_FLAG_MASK                        (((1 << LOCKDET_WIN_UNLOCK_FLAG_WIDTH) - 1) << LOCKDET_WIN_UNLOCK_FLAG_LSB)
#define LOCKDET_WIN_LOCKED_FLAG_WIDTH                       (1)
#define LOCKDET_WIN_LOCKED_FLAG_LSB                         (1)
#define LOCKDET_WIN_LOCKED_FLAG_MASK                        (((1 << LOCKDET_WIN_LOCKED_FLAG_WIDTH) - 1) << LOCKDET_WIN_LOCKED_FLAG_LSB)
#define LOCKDET_LOCKED_WIDTH                                (1)
#define LOCKDET_LOCKED_LSB                                  (0)
#define LOCKDET_LOCKED_MASK                                 (((1 << LOCKDET_LOCKED_WIDTH) - 1) << LOCKDET_LOCKED_LSB)

// #pragma endregion //LOCKDET_LOCKED_IND

// #pragma region //LOCKDET_MANUAL_CFG
#define LOCKDET_MANUAL_CFG                                  (0x00000174) // 0x90422174 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_9_H174_WIDTH                            (23)
#define RESERVED_31_9_H174_LSB                              (9)
#define RESERVED_31_9_H174_MASK                             (((1 << RESERVED_31_9_H174_WIDTH) - 1) << RESERVED_31_9_H174_LSB)
#define LOCKDET_MANUAL_START_WIDTH                          (1)
#define LOCKDET_MANUAL_START_LSB                            (8)
#define LOCKDET_MANUAL_START_MASK                           (((1 << LOCKDET_MANUAL_START_WIDTH) - 1) << LOCKDET_MANUAL_START_LSB)
#define RESERVED_7_1_H174_WIDTH                             (7)
#define RESERVED_7_1_H174_LSB                               (1)
#define RESERVED_7_1_H174_MASK                              (((1 << RESERVED_7_1_H174_WIDTH) - 1) << RESERVED_7_1_H174_LSB)
#define LOCKDET_MANUAL_MODE_WIDTH                           (1)
#define LOCKDET_MANUAL_MODE_LSB                             (0)
#define LOCKDET_MANUAL_MODE_MASK                            (((1 << LOCKDET_MANUAL_MODE_WIDTH) - 1) << LOCKDET_MANUAL_MODE_LSB)

// #pragma endregion //LOCKDET_MANUAL_CFG

// #pragma region //LOCKDET_MANUAL_KDET
#define LOCKDET_MANUAL_KDET                                 (0x00000178) // 0x90422178 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_30_H178_WIDTH                           (2)
#define RESERVED_31_30_H178_LSB                             (30)
#define RESERVED_31_30_H178_MASK                            (((1 << RESERVED_31_30_H178_WIDTH) - 1) << RESERVED_31_30_H178_LSB)
#define LOCKDET_MANUAL_DET_VALID_WIDTH                      (1)
#define LOCKDET_MANUAL_DET_VALID_LSB                        (29)
#define LOCKDET_MANUAL_DET_VALID_MASK                       (((1 << LOCKDET_MANUAL_DET_VALID_WIDTH) - 1) << LOCKDET_MANUAL_DET_VALID_LSB)
#define RESERVED_28_28_H178_WIDTH                           (1)
#define RESERVED_28_28_H178_LSB                             (28)
#define RESERVED_28_28_H178_MASK                            (((1 << RESERVED_28_28_H178_WIDTH) - 1) << RESERVED_28_28_H178_LSB)
#define LOCKDET_MANUAL_DET_WIDTH                            (28)
#define LOCKDET_MANUAL_DET_LSB                              (0)
#define LOCKDET_MANUAL_DET_MASK                             (((1 << LOCKDET_MANUAL_DET_WIDTH) - 1) << LOCKDET_MANUAL_DET_LSB)

// #pragma endregion //LOCKDET_MANUAL_KDET

// #pragma region //LOCKDET_MANUAL_LOCK_TIMES
#define LOCKDET_MANUAL_LOCK_TIMES                           (0x0000017c) // 0x9042217c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_26_H17C_WIDTH                           (6)
#define RESERVED_31_26_H17C_LSB                             (26)
#define RESERVED_31_26_H17C_MASK                            (((1 << RESERVED_31_26_H17C_WIDTH) - 1) << RESERVED_31_26_H17C_LSB)
#define LOCKDET_MANUAL_UNLOCK_TIMES_WIDTH                   (10)
#define LOCKDET_MANUAL_UNLOCK_TIMES_LSB                     (16)
#define LOCKDET_MANUAL_UNLOCK_TIMES_MASK                    (((1 << LOCKDET_MANUAL_UNLOCK_TIMES_WIDTH) - 1) << LOCKDET_MANUAL_UNLOCK_TIMES_LSB)
#define RESERVED_15_10_H17C_WIDTH                           (6)
#define RESERVED_15_10_H17C_LSB                             (10)
#define RESERVED_15_10_H17C_MASK                            (((1 << RESERVED_15_10_H17C_WIDTH) - 1) << RESERVED_15_10_H17C_LSB)
#define LOCKDET_MANUAL_LOCKED_TIMES_WIDTH                   (10)
#define LOCKDET_MANUAL_LOCKED_TIMES_LSB                     (0)
#define LOCKDET_MANUAL_LOCKED_TIMES_MASK                    (((1 << LOCKDET_MANUAL_LOCKED_TIMES_WIDTH) - 1) << LOCKDET_MANUAL_LOCKED_TIMES_LSB)

// #pragma endregion //LOCKDET_MANUAL_LOCK_TIMES

// #pragma region //OUT_IDLE_DFLT_CBANK_VCO_VCTRL_DIDTC
#define OUT_IDLE_DFLT_CBANK_VCO_VCTRL_DIDTC                 (0x00000180) // 0x90422180 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_30_H180_WIDTH                           (2)
#define RESERVED_31_30_H180_LSB                             (30)
#define RESERVED_31_30_H180_MASK                            (((1 << RESERVED_31_30_H180_WIDTH) - 1) << RESERVED_31_30_H180_LSB)
#define SET_VCTRL_SEL_IDLE_WIDTH                            (4)
#define SET_VCTRL_SEL_IDLE_LSB                              (26)
#define SET_VCTRL_SEL_IDLE_MASK                             (((1 << SET_VCTRL_SEL_IDLE_WIDTH) - 1) << SET_VCTRL_SEL_IDLE_LSB)
#define RESERVED_25_23_H180_WIDTH                           (3)
#define RESERVED_25_23_H180_LSB                             (23)
#define RESERVED_25_23_H180_MASK                            (((1 << RESERVED_25_23_H180_WIDTH) - 1) << RESERVED_25_23_H180_LSB)
#define SET_VCO_VREF_SEL_IDLE_WIDTH                         (5)
#define SET_VCO_VREF_SEL_IDLE_LSB                           (18)
#define SET_VCO_VREF_SEL_IDLE_MASK                          (((1 << SET_VCO_VREF_SEL_IDLE_WIDTH) - 1) << SET_VCO_VREF_SEL_IDLE_LSB)
#define SET_CBANK_IDLE_WIDTH                                (8)
#define SET_CBANK_IDLE_LSB                                  (10)
#define SET_CBANK_IDLE_MASK                                 (((1 << SET_CBANK_IDLE_WIDTH) - 1) << SET_CBANK_IDLE_LSB)
#define RESERVED_9_9_H180_WIDTH                             (1)
#define RESERVED_9_9_H180_LSB                               (9)
#define RESERVED_9_9_H180_MASK                              (((1 << RESERVED_9_9_H180_WIDTH) - 1) << RESERVED_9_9_H180_LSB)
#define SET_DIDTC_CODE_IDLE_WIDTH                           (9)
#define SET_DIDTC_CODE_IDLE_LSB                             (0)
#define SET_DIDTC_CODE_IDLE_MASK                            (((1 << SET_DIDTC_CODE_IDLE_WIDTH) - 1) << SET_DIDTC_CODE_IDLE_LSB)

// #pragma endregion //OUT_IDLE_DFLT_CBANK_VCO_VCTRL_DIDTC

// #pragma region //OUT_IDLE_DFLT_P_S_DTC
#define OUT_IDLE_DFLT_P_S_DTC                               (0x00000184) // 0x90422184 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_26_H184_WIDTH                           (6)
#define RESERVED_31_26_H184_LSB                             (26)
#define RESERVED_31_26_H184_MASK                            (((1 << RESERVED_31_26_H184_WIDTH) - 1) << RESERVED_31_26_H184_LSB)
#define SET_DTC_CODE_IDLE_WIDTH                             (10)
#define SET_DTC_CODE_IDLE_LSB                               (16)
#define SET_DTC_CODE_IDLE_MASK                              (((1 << SET_DTC_CODE_IDLE_WIDTH) - 1) << SET_DTC_CODE_IDLE_LSB)
#define RESERVED_15_13_H184_WIDTH                           (3)
#define RESERVED_15_13_H184_LSB                             (13)
#define RESERVED_15_13_H184_MASK                            (((1 << RESERVED_15_13_H184_WIDTH) - 1) << RESERVED_15_13_H184_LSB)
#define SET_DIV_S_IDLE_WIDTH                                (5)
#define SET_DIV_S_IDLE_LSB                                  (8)
#define SET_DIV_S_IDLE_MASK                                 (((1 << SET_DIV_S_IDLE_WIDTH) - 1) << SET_DIV_S_IDLE_LSB)
#define RESERVED_7_6_H184_WIDTH                             (2)
#define RESERVED_7_6_H184_LSB                               (6)
#define RESERVED_7_6_H184_MASK                              (((1 << RESERVED_7_6_H184_WIDTH) - 1) << RESERVED_7_6_H184_LSB)
#define SET_DIV_P_IDLE_WIDTH                                (6)
#define SET_DIV_P_IDLE_LSB                                  (0)
#define SET_DIV_P_IDLE_MASK                                 (((1 << SET_DIV_P_IDLE_WIDTH) - 1) << SET_DIV_P_IDLE_LSB)

// #pragma endregion //OUT_IDLE_DFLT_P_S_DTC

// #pragma region //OUT_AFC_DFLT_CBANK_VCO_VCTRL_DIDTC
#define OUT_AFC_DFLT_CBANK_VCO_VCTRL_DIDTC                  (0x00000188) // 0x90422188 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_28_H188_WIDTH                           (4)
#define RESERVED_31_28_H188_LSB                             (28)
#define RESERVED_31_28_H188_MASK                            (((1 << RESERVED_31_28_H188_WIDTH) - 1) << RESERVED_31_28_H188_LSB)
#define SET_VCTRL_SEL_AFC_WIDTH                             (4)
#define SET_VCTRL_SEL_AFC_LSB                               (24)
#define SET_VCTRL_SEL_AFC_MASK                              (((1 << SET_VCTRL_SEL_AFC_WIDTH) - 1) << SET_VCTRL_SEL_AFC_LSB)
#define RESERVED_23_21_H188_WIDTH                           (3)
#define RESERVED_23_21_H188_LSB                             (21)
#define RESERVED_23_21_H188_MASK                            (((1 << RESERVED_23_21_H188_WIDTH) - 1) << RESERVED_23_21_H188_LSB)
#define SET_VCO_VREF_SEL_AFC_WIDTH                          (5)
#define SET_VCO_VREF_SEL_AFC_LSB                            (16)
#define SET_VCO_VREF_SEL_AFC_MASK                           (((1 << SET_VCO_VREF_SEL_AFC_WIDTH) - 1) << SET_VCO_VREF_SEL_AFC_LSB)
#define RESERVED_15_9_H188_WIDTH                            (7)
#define RESERVED_15_9_H188_LSB                              (9)
#define RESERVED_15_9_H188_MASK                             (((1 << RESERVED_15_9_H188_WIDTH) - 1) << RESERVED_15_9_H188_LSB)
#define SET_DIDTC_CODE_AFC_WIDTH                            (9)
#define SET_DIDTC_CODE_AFC_LSB                              (0)
#define SET_DIDTC_CODE_AFC_MASK                             (((1 << SET_DIDTC_CODE_AFC_WIDTH) - 1) << SET_DIDTC_CODE_AFC_LSB)

// #pragma endregion //OUT_AFC_DFLT_CBANK_VCO_VCTRL_DIDTC

// #pragma region //OUT_AFC_DFLT_P_S_DTC
#define OUT_AFC_DFLT_P_S_DTC                                (0x0000018c) // 0x9042218c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_26_H18C_WIDTH                           (6)
#define RESERVED_31_26_H18C_LSB                             (26)
#define RESERVED_31_26_H18C_MASK                            (((1 << RESERVED_31_26_H18C_WIDTH) - 1) << RESERVED_31_26_H18C_LSB)
#define SET_DTC_CODE_AFC_WIDTH                              (10)
#define SET_DTC_CODE_AFC_LSB                                (16)
#define SET_DTC_CODE_AFC_MASK                               (((1 << SET_DTC_CODE_AFC_WIDTH) - 1) << SET_DTC_CODE_AFC_LSB)
#define RESERVED_15_13_H18C_WIDTH                           (3)
#define RESERVED_15_13_H18C_LSB                             (13)
#define RESERVED_15_13_H18C_MASK                            (((1 << RESERVED_15_13_H18C_WIDTH) - 1) << RESERVED_15_13_H18C_LSB)
#define SET_DIV_S_AFC_WIDTH                                 (5)
#define SET_DIV_S_AFC_LSB                                   (8)
#define SET_DIV_S_AFC_MASK                                  (((1 << SET_DIV_S_AFC_WIDTH) - 1) << SET_DIV_S_AFC_LSB)
#define RESERVED_7_6_H18C_WIDTH                             (2)
#define RESERVED_7_6_H18C_LSB                               (6)
#define RESERVED_7_6_H18C_MASK                              (((1 << RESERVED_7_6_H18C_WIDTH) - 1) << RESERVED_7_6_H18C_LSB)
#define SET_DIV_P_AFC_WIDTH                                 (6)
#define SET_DIV_P_AFC_LSB                                   (0)
#define SET_DIV_P_AFC_MASK                                  (((1 << SET_DIV_P_AFC_WIDTH) - 1) << SET_DIV_P_AFC_LSB)

// #pragma endregion //OUT_AFC_DFLT_P_S_DTC

// #pragma region //OUT_KVCO_DFLT_CBANK_VCO_VCTRL_DIDTC
#define OUT_KVCO_DFLT_CBANK_VCO_VCTRL_DIDTC                 (0x00000190) // 0x90422190 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_9_H190_WIDTH                            (23)
#define RESERVED_31_9_H190_LSB                              (9)
#define RESERVED_31_9_H190_MASK                             (((1 << RESERVED_31_9_H190_WIDTH) - 1) << RESERVED_31_9_H190_LSB)
#define SET_DIDTC_CODE_KVCO_WIDTH                           (9)
#define SET_DIDTC_CODE_KVCO_LSB                             (0)
#define SET_DIDTC_CODE_KVCO_MASK                            (((1 << SET_DIDTC_CODE_KVCO_WIDTH) - 1) << SET_DIDTC_CODE_KVCO_LSB)

// #pragma endregion //OUT_KVCO_DFLT_CBANK_VCO_VCTRL_DIDTC

// #pragma region //OUT_KVCO_DFLT_P_S_DTC
#define OUT_KVCO_DFLT_P_S_DTC                               (0x00000194) // 0x90422194 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_26_H194_WIDTH                           (6)
#define RESERVED_31_26_H194_LSB                             (26)
#define RESERVED_31_26_H194_MASK                            (((1 << RESERVED_31_26_H194_WIDTH) - 1) << RESERVED_31_26_H194_LSB)
#define SET_DTC_CODE_KVCO_WIDTH                             (10)
#define SET_DTC_CODE_KVCO_LSB                               (16)
#define SET_DTC_CODE_KVCO_MASK                              (((1 << SET_DTC_CODE_KVCO_WIDTH) - 1) << SET_DTC_CODE_KVCO_LSB)
#define RESERVED_15_13_H194_WIDTH                           (3)
#define RESERVED_15_13_H194_LSB                             (13)
#define RESERVED_15_13_H194_MASK                            (((1 << RESERVED_15_13_H194_WIDTH) - 1) << RESERVED_15_13_H194_LSB)
#define SET_DIV_S_KVCO_WIDTH                                (5)
#define SET_DIV_S_KVCO_LSB                                  (8)
#define SET_DIV_S_KVCO_MASK                                 (((1 << SET_DIV_S_KVCO_WIDTH) - 1) << SET_DIV_S_KVCO_LSB)
#define RESERVED_7_6_H194_WIDTH                             (2)
#define RESERVED_7_6_H194_LSB                               (6)
#define RESERVED_7_6_H194_MASK                              (((1 << RESERVED_7_6_H194_WIDTH) - 1) << RESERVED_7_6_H194_LSB)
#define SET_DIV_P_KVCO_WIDTH                                (6)
#define SET_DIV_P_KVCO_LSB                                  (0)
#define SET_DIV_P_KVCO_MASK                                 (((1 << SET_DIV_P_KVCO_WIDTH) - 1) << SET_DIV_P_KVCO_LSB)

// #pragma endregion //OUT_KVCO_DFLT_P_S_DTC

// #pragma region //ANALOG_CBANK_VCO_VCTRL_DIDTC
#define ANALOG_CBANK_VCO_VCTRL_DIDTC                        (0x00000198) // 0x90422198 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_30_H198_WIDTH                           (2)
#define RESERVED_31_30_H198_LSB                             (30)
#define RESERVED_31_30_H198_MASK                            (((1 << RESERVED_31_30_H198_WIDTH) - 1) << RESERVED_31_30_H198_LSB)
#define ANALOG_VCTRL_SEL_WIDTH                              (4)
#define ANALOG_VCTRL_SEL_LSB                                (26)
#define ANALOG_VCTRL_SEL_MASK                               (((1 << ANALOG_VCTRL_SEL_WIDTH) - 1) << ANALOG_VCTRL_SEL_LSB)
#define RESERVED_25_23_H198_WIDTH                           (3)
#define RESERVED_25_23_H198_LSB                             (23)
#define RESERVED_25_23_H198_MASK                            (((1 << RESERVED_25_23_H198_WIDTH) - 1) << RESERVED_25_23_H198_LSB)
#define ANALOG_VCO_VREF_SEL_WIDTH                           (5)
#define ANALOG_VCO_VREF_SEL_LSB                             (18)
#define ANALOG_VCO_VREF_SEL_MASK                            (((1 << ANALOG_VCO_VREF_SEL_WIDTH) - 1) << ANALOG_VCO_VREF_SEL_LSB)
#define ANALOG_CBANK_WIDTH                                  (8)
#define ANALOG_CBANK_LSB                                    (10)
#define ANALOG_CBANK_MASK                                   (((1 << ANALOG_CBANK_WIDTH) - 1) << ANALOG_CBANK_LSB)
#define RESERVED_9_9_H198_WIDTH                             (1)
#define RESERVED_9_9_H198_LSB                               (9)
#define RESERVED_9_9_H198_MASK                              (((1 << RESERVED_9_9_H198_WIDTH) - 1) << RESERVED_9_9_H198_LSB)
#define ANALOG_DIDTC_CODE_WIDTH                             (9)
#define ANALOG_DIDTC_CODE_LSB                               (0)
#define ANALOG_DIDTC_CODE_MASK                              (((1 << ANALOG_DIDTC_CODE_WIDTH) - 1) << ANALOG_DIDTC_CODE_LSB)

// #pragma endregion //ANALOG_CBANK_VCO_VCTRL_DIDTC

// #pragma region //ANALOG_DFLT_P_S_DTC
#define ANALOG_DFLT_P_S_DTC                                 (0x0000019c) // 0x9042219c # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_26_H19C_WIDTH                           (6)
#define RESERVED_31_26_H19C_LSB                             (26)
#define RESERVED_31_26_H19C_MASK                            (((1 << RESERVED_31_26_H19C_WIDTH) - 1) << RESERVED_31_26_H19C_LSB)
#define ANALOG_DTC_CODE_WIDTH                               (10)
#define ANALOG_DTC_CODE_LSB                                 (16)
#define ANALOG_DTC_CODE_MASK                                (((1 << ANALOG_DTC_CODE_WIDTH) - 1) << ANALOG_DTC_CODE_LSB)
#define RESERVED_15_13_H19C_WIDTH                           (3)
#define RESERVED_15_13_H19C_LSB                             (13)
#define RESERVED_15_13_H19C_MASK                            (((1 << RESERVED_15_13_H19C_WIDTH) - 1) << RESERVED_15_13_H19C_LSB)
#define ANALOG_DIV_S_WIDTH                                  (5)
#define ANALOG_DIV_S_LSB                                    (8)
#define ANALOG_DIV_S_MASK                                   (((1 << ANALOG_DIV_S_WIDTH) - 1) << ANALOG_DIV_S_LSB)
#define RESERVED_7_6_H19C_WIDTH                             (2)
#define RESERVED_7_6_H19C_LSB                               (6)
#define RESERVED_7_6_H19C_MASK                              (((1 << RESERVED_7_6_H19C_WIDTH) - 1) << RESERVED_7_6_H19C_LSB)
#define ANALOG_DIV_P_WIDTH                                  (6)
#define ANALOG_DIV_P_LSB                                    (0)
#define ANALOG_DIV_P_MASK                                   (((1 << ANALOG_DIV_P_WIDTH) - 1) << ANALOG_DIV_P_LSB)

// #pragma endregion //ANALOG_DFLT_P_S_DTC

// #pragma region //PD_LOOP_START
#define PD_LOOP_START                                       (0x000001a0) // 0x904221a0 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_9_H1A0_WIDTH                            (23)
#define RESERVED_31_9_H1A0_LSB                              (9)
#define RESERVED_31_9_H1A0_MASK                             (((1 << RESERVED_31_9_H1A0_WIDTH) - 1) << RESERVED_31_9_H1A0_LSB)
#define PD_WIDTH                                            (1)
#define PD_LSB                                              (8)
#define PD_MASK                                             (((1 << PD_WIDTH) - 1) << PD_LSB)
#define RESERVED_7_2_H1A0_WIDTH                             (6)
#define RESERVED_7_2_H1A0_LSB                               (2)
#define RESERVED_7_2_H1A0_MASK                              (((1 << RESERVED_7_2_H1A0_WIDTH) - 1) << RESERVED_7_2_H1A0_LSB)
#define MANUAL_LOOPSTART_EN_WIDTH                           (1)
#define MANUAL_LOOPSTART_EN_LSB                             (1)
#define MANUAL_LOOPSTART_EN_MASK                            (((1 << MANUAL_LOOPSTART_EN_WIDTH) - 1) << MANUAL_LOOPSTART_EN_LSB)
#define MANUAL_LOOPSTART_WIDTH                              (1)
#define MANUAL_LOOPSTART_LSB                                (0)
#define MANUAL_LOOPSTART_MASK                               (((1 << MANUAL_LOOPSTART_WIDTH) - 1) << MANUAL_LOOPSTART_LSB)

// #pragma endregion //PD_LOOP_START

// #pragma region //PD_LOOP_START_VLUE
#define PD_LOOP_START_VLUE                                  (0x000001a4) // 0x904221a4 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H1A4_WIDTH                            (31)
#define RESERVED_31_1_H1A4_LSB                              (1)
#define RESERVED_31_1_H1A4_MASK                             (((1 << RESERVED_31_1_H1A4_WIDTH) - 1) << RESERVED_31_1_H1A4_LSB)
#define ANALOG_LOOPSTART_WIDTH                              (1)
#define ANALOG_LOOPSTART_LSB                                (0)
#define ANALOG_LOOPSTART_MASK                               (((1 << ANALOG_LOOPSTART_WIDTH) - 1) << ANALOG_LOOPSTART_LSB)

// #pragma endregion //PD_LOOP_START_VLUE

// #pragma region //UNLOCKED_INTERRUPT_EN
#define UNLOCKED_INTERRUPT_EN                               (0x000001a8) // 0x904221a8 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_2_H1A8_WIDTH                            (30)
#define RESERVED_31_2_H1A8_LSB                              (2)
#define RESERVED_31_2_H1A8_MASK                             (((1 << RESERVED_31_2_H1A8_WIDTH) - 1) << RESERVED_31_2_H1A8_LSB)
#define OVF_INTR_EN_WIDTH                                   (1)
#define OVF_INTR_EN_LSB                                     (1)
#define OVF_INTR_EN_MASK                                    (((1 << OVF_INTR_EN_WIDTH) - 1) << OVF_INTR_EN_LSB)
#define UNLOCK_INTR_EN_WIDTH                                (1)
#define UNLOCK_INTR_EN_LSB                                  (0)
#define UNLOCK_INTR_EN_MASK                                 (((1 << UNLOCK_INTR_EN_WIDTH) - 1) << UNLOCK_INTR_EN_LSB)

// #pragma endregion //UNLOCKED_INTERRUPT_EN

// #pragma region //UNLOCKED_INTERRUPT_RAW_RPT
#define UNLOCKED_INTERRUPT_RAW_RPT                          (0x000001ac) // 0x904221ac # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_2_H1AC_WIDTH                            (30)
#define RESERVED_31_2_H1AC_LSB                              (2)
#define RESERVED_31_2_H1AC_MASK                             (((1 << RESERVED_31_2_H1AC_WIDTH) - 1) << RESERVED_31_2_H1AC_LSB)
#define OVF_INTR_RAW_RPT_WIDTH                              (1)
#define OVF_INTR_RAW_RPT_LSB                                (1)
#define OVF_INTR_RAW_RPT_MASK                               (((1 << OVF_INTR_RAW_RPT_WIDTH) - 1) << OVF_INTR_RAW_RPT_LSB)
#define UNLOCK_INTR_RAW_RPT_WIDTH                           (1)
#define UNLOCK_INTR_RAW_RPT_LSB                             (0)
#define UNLOCK_INTR_RAW_RPT_MASK                            (((1 << UNLOCK_INTR_RAW_RPT_WIDTH) - 1) << UNLOCK_INTR_RAW_RPT_LSB)

// #pragma endregion //UNLOCKED_INTERRUPT_RAW_RPT

// #pragma region //UNLOCKED_INTERRUPT_RPT
#define UNLOCKED_INTERRUPT_RPT                              (0x000001b0) // 0x904221b0 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_2_H1B0_WIDTH                            (30)
#define RESERVED_31_2_H1B0_LSB                              (2)
#define RESERVED_31_2_H1B0_MASK                             (((1 << RESERVED_31_2_H1B0_WIDTH) - 1) << RESERVED_31_2_H1B0_LSB)
#define OVF_INTR_RPT_WIDTH                                  (1)
#define OVF_INTR_RPT_LSB                                    (1)
#define OVF_INTR_RPT_MASK                                   (((1 << OVF_INTR_RPT_WIDTH) - 1) << OVF_INTR_RPT_LSB)
#define UNLOCK_INTR_RPT_WIDTH                               (1)
#define UNLOCK_INTR_RPT_LSB                                 (0)
#define UNLOCK_INTR_RPT_MASK                                (((1 << UNLOCK_INTR_RPT_WIDTH) - 1) << UNLOCK_INTR_RPT_LSB)

// #pragma endregion //UNLOCKED_INTERRUPT_RPT

// #pragma region //UNLOCKED_INTERRUPT_OVLP_RPT
#define UNLOCKED_INTERRUPT_OVLP_RPT                         (0x000001b4) // 0x904221b4 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_2_H1B4_WIDTH                            (30)
#define RESERVED_31_2_H1B4_LSB                              (2)
#define RESERVED_31_2_H1B4_MASK                             (((1 << RESERVED_31_2_H1B4_WIDTH) - 1) << RESERVED_31_2_H1B4_LSB)
#define OVF_INTR_OVLP_RPT_WIDTH                             (1)
#define OVF_INTR_OVLP_RPT_LSB                               (1)
#define OVF_INTR_OVLP_RPT_MASK                              (((1 << OVF_INTR_OVLP_RPT_WIDTH) - 1) << OVF_INTR_OVLP_RPT_LSB)
#define UNLOCK_INTR_OVLP_RPT_WIDTH                          (1)
#define UNLOCK_INTR_OVLP_RPT_LSB                            (0)
#define UNLOCK_INTR_OVLP_RPT_MASK                           (((1 << UNLOCK_INTR_OVLP_RPT_WIDTH) - 1) << UNLOCK_INTR_OVLP_RPT_LSB)

// #pragma endregion //UNLOCKED_INTERRUPT_OVLP_RPT

// #pragma region //MT_FIFO_CLEAR
#define MT_FIFO_CLEAR                                       (0x000001b8) // 0x904221b8 # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_18_H1B8_WIDTH                           (14)
#define RESERVED_31_18_H1B8_LSB                             (18)
#define RESERVED_31_18_H1B8_MASK                            (((1 << RESERVED_31_18_H1B8_WIDTH) - 1) << RESERVED_31_18_H1B8_LSB)
#define MT_AFC_AFC2_SEL_WIDTH                               (1)
#define MT_AFC_AFC2_SEL_LSB                                 (17)
#define MT_AFC_AFC2_SEL_MASK                                (((1 << MT_AFC_AFC2_SEL_WIDTH) - 1) << MT_AFC_AFC2_SEL_LSB)
#define MT_CFG_UPDATE_WIDTH                                 (1)
#define MT_CFG_UPDATE_LSB                                   (16)
#define MT_CFG_UPDATE_MASK                                  (((1 << MT_CFG_UPDATE_WIDTH) - 1) << MT_CFG_UPDATE_LSB)
#define RESERVED_15_1_H1B8_WIDTH                            (15)
#define RESERVED_15_1_H1B8_LSB                              (1)
#define RESERVED_15_1_H1B8_MASK                             (((1 << RESERVED_15_1_H1B8_WIDTH) - 1) << RESERVED_15_1_H1B8_LSB)
#define MT_FIFO_CLEAR_WIDTH                                 (1)
#define MT_FIFO_CLEAR_LSB                                   (0)
#define MT_FIFO_CLEAR_MASK                                  (((1 << MT_FIFO_CLEAR_WIDTH) - 1) << MT_FIFO_CLEAR_LSB)

// #pragma endregion //MT_FIFO_CLEAR

// #pragma region //MT_ANALOG_RD_PULSE
#define MT_ANALOG_RD_PULSE                                  (0x000001bc) // 0x904221bc # CHA=0x00000000; CHN=0x800;

#define RESERVED_31_1_H1BC_WIDTH                            (31)
#define RESERVED_31_1_H1BC_LSB                              (1)
#define RESERVED_31_1_H1BC_MASK                             (((1 << RESERVED_31_1_H1BC_WIDTH) - 1) << RESERVED_31_1_H1BC_LSB)
#define MT_TO_SOFT_RD_PULSE_WIDTH                           (1)
#define MT_TO_SOFT_RD_PULSE_LSB                             (0)
#define MT_TO_SOFT_RD_PULSE_MASK                            (((1 << MT_TO_SOFT_RD_PULSE_WIDTH) - 1) << MT_TO_SOFT_RD_PULSE_LSB)

// #pragma endregion //MT_ANALOG_RD_PULSE
/* clang-format on */

#endif // _PLLCALI_CFG_REG_DEF_H_
