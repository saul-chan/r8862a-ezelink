/*
 * Copyright (c) 2021-2024, Clourney Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 * Reproduction and redistribution in binary or source form, with or without modification,
 * for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 * must retain the above copyright notice.
 *
 * By using this software and/or documentation, you agree to the limited terms and conditions.
 */

/* dubhe2000_switch_pause.c
 * Shared functions for accessing and configuring the Switch Pause
 */

#include "dubhe2000_switch.h"

struct dubhe2000_switch_pause_config {
	char *desc;
	/* ERM config*/
	t_ResourceLimiterSet d_res_limiter[ResourceLimiterSet_nr_entries];
	t_ERMRedConfiguration d_erm_red;
	t_ERMYellowConfiguration d_erm_yellow;
	t_EgressResourceManagerPointer d_e_res_mg_pointer[EgressResourceManagerPointer_nr_entries];

	/* FlowCrtl config*/
	t_TailDropFFAThreshold d_tdrop_ffa_thres;
	t_PortTailDropFFAThreshold d_port_tdrop_ffa_thres[PortTailDropFFAThreshold_nr_entries];

	t_XoffFFAThreshold d_xoff_ffa_thres;
	t_PortXoffFFAThreshold d_port_xoff_ffa_thres[PortXoffFFAThreshold_nr_entries];

	t_XonFFAThreshold d_xon_ffa_thres;
	t_PortXonFFAThreshold d_port_xon_ffa_thres[PortXonFFAThreshold_nr_entries];

	t_PortReserved d_port_reserved[PortReserved_nr_entries];
	t_PortPauseSettings d_port_pause[PortPauseSettings_nr_entries];
	t_PortTailDropSettings d_port_tdrop[PortTailDropSettings_nr_entries];
};

struct dubhe2000_switch_pause_config dubhe2000_pause_tbl[] = {
	{//index0
		.desc = "InitValue",
		.d_res_limiter = {
			{.yellowAccumulated = 0x2d, .yellowLimit = 0x28, .redLimit = 0x15, .maxCells = 0xff},
			{.yellowAccumulated = 0x2d, .yellowLimit = 0x28, .redLimit = 0x15, .maxCells = 0xff},
			{.yellowAccumulated = 0x2d, .yellowLimit = 0x28, .redLimit = 0x15, .maxCells = 0xff},
			{.yellowAccumulated = 0x2d, .yellowLimit = 0x28, .redLimit = 0x15, .maxCells = 0xff},
		},
		.d_erm_red = {.redXoff = 0x50, .redXon = 0xc8, .redMaxCells = 0x9},
		.d_erm_yellow = {.yellowXoff = 0x164, .yellowXon = 0x1fe, .redPortEn = 0x3f, .redPortXoff = 0x10b},
		.d_e_res_mg_pointer = {
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
		},

		.d_port_reserved = {
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
		},
		.d_port_pause = {
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
		},
		.d_port_tdrop = {//Port Tail-Drop Setting
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
		},

		.d_tdrop_ffa_thres = {.cells = 0, .enable = 0, .trip = 0},
		.d_port_tdrop_ffa_thres = {
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
		},

		.d_xoff_ffa_thres = {.cells = 0, .enable = 0, .trip = 0},
		.d_port_xoff_ffa_thres = {
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
		},

		.d_xon_ffa_thres = {.cells = 0},
		.d_port_xon_ffa_thres = {
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
		},
	},
	{//index1
		.desc = "2*2.5Gbps + EDMA",
		.d_res_limiter = {
			{.yellowAccumulated = 350, .yellowLimit = 350, .redLimit = 350, .maxCells = 255},
			{.yellowAccumulated = 300, .yellowLimit = 300, .redLimit = 300, .maxCells = 255},
			{.yellowAccumulated = 150, .yellowLimit = 150, .redLimit = 150, .maxCells = 255},
			{.yellowAccumulated = 50,  .yellowLimit = 50,  .redLimit = 9,   .maxCells = 255},
		},
		.d_erm_red = {.redXoff = 63, .redXon = 100, .redMaxCells = 9},
		.d_erm_yellow = {.yellowXoff = 152, .yellowXon = 400, .redPortEn = 0, .redPortXoff = 115},
		.d_e_res_mg_pointer = {
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
		},

		.d_port_reserved = {
			{.cells = 50},
			{.cells = 50},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 50},
		},
		.d_port_pause = {
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
		},
		.d_port_tdrop = {//Port Tail-Drop Setting
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
		},

		.d_tdrop_ffa_thres = {.cells = 648, .enable = 1, .trip = 0},
		.d_port_tdrop_ffa_thres = {
			{.cells = 290, .enable = 1, .trip = 0},
			{.cells = 270, .enable = 1, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 290, .enable = 1, .trip = 0},
		},

		.d_xoff_ffa_thres = {.cells = 585, .enable = 1, .trip = 0},
		.d_port_xoff_ffa_thres = {
			{.cells = 250, .enable = 1, .trip = 0},
			{.cells = 230, .enable = 1, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 250, .enable = 1, .trip = 0},
		},

		.d_xon_ffa_thres = {.cells = 250},
		.d_port_xon_ffa_thres = {
			{.cells = 150},
			{.cells = 130},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 150},
		},
	},
	{//index2
		.desc = "1*2.5/5/10Gbps + 1*1G + EDMA",
		.d_res_limiter = {
			{.yellowAccumulated = 350, .yellowLimit = 350, .redLimit = 350, .maxCells = 255},
			{.yellowAccumulated = 300, .yellowLimit = 300, .redLimit = 300, .maxCells = 255},
			{.yellowAccumulated = 150, .yellowLimit = 150, .redLimit = 150, .maxCells = 255},
			{.yellowAccumulated = 50,  .yellowLimit = 50,  .redLimit = 9,   .maxCells = 255},
		},
		.d_erm_red = {.redXoff = 63, .redXon = 100, .redMaxCells = 9},
		.d_erm_yellow = {.yellowXoff = 152, .yellowXon = 400, .redPortEn = 0, .redPortXoff = 225},
		.d_e_res_mg_pointer = {
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
		},

		.d_port_reserved = {
			{.cells = 50},
			{.cells = 50},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 50},
		},
		.d_port_pause = {
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 0, .reserved = 0, .force = 0, .pattern = 0},
		},
		.d_port_tdrop = {//Port Tail-Drop Setting
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
			{.enable = 0, .mode = 0},
		},

		.d_tdrop_ffa_thres = {.cells = 648, .enable = 1, .trip = 0},
		.d_port_tdrop_ffa_thres = {
			{.cells = 310, .enable = 1, .trip = 0},
			{.cells = 220, .enable = 1, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 310, .enable = 1, .trip = 0},
		},

		.d_xoff_ffa_thres = {.cells = 585, .enable = 1, .trip = 0},
		.d_port_xoff_ffa_thres = {
			{.cells = 270, .enable = 1, .trip = 0},
			{.cells = 180, .enable = 1, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 0, .enable = 0, .trip = 0},
			{.cells = 270, .enable = 1, .trip = 0},
		},

		.d_xon_ffa_thres = {.cells = 250},
		.d_port_xon_ffa_thres = {
			{.cells = 150},
			{.cells = 100},
			{.cells = 0},
			{.cells = 0},
			{.cells = 0},
			{.cells = 150},
		},
	},
	{//index3
		.desc = "1*2.5G/5G/10G + 4*1G + EDMA",
		.d_res_limiter = {
			{.yellowAccumulated = 550, .yellowLimit = 550, .redLimit = 550, .maxCells = 255},
			{.yellowAccumulated = 250, .yellowLimit = 200, .redLimit = 150, .maxCells = 255},
			{.yellowAccumulated = 250, .yellowLimit = 50,  .redLimit = 50,  .maxCells = 255},
			{.yellowAccumulated = 9,   .yellowLimit = 9,   .redLimit = 9,   .maxCells = 255},
		},
		.d_erm_red = {.redXoff = 100, .redXon = 150, .redMaxCells = 9},
		.d_erm_yellow = {.yellowXoff = 250, .yellowXon = 350, .redPortEn = 0, .redPortXoff = 115},
		.d_e_res_mg_pointer = {
			{.q0 = 0, .q1 = 0, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 0, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 0, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 0, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 0, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 0, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
		},

		.d_port_reserved = {
			{.cells = 50},
			{.cells = 30},
			{.cells = 30},
			{.cells = 30},
			{.cells = 30},
			{.cells = 50},
		},
		.d_port_pause = {
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
		},
		.d_port_tdrop = {//Port Tail-Drop Setting
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
		},

		.d_tdrop_ffa_thres = {.cells = 578, .enable = 1, .trip = 0},
		.d_port_tdrop_ffa_thres = {
			{.cells = 300, .enable = 1, .trip = 0},
			{.cells = 250, .enable = 1, .trip = 0},
			{.cells = 250, .enable = 1, .trip = 0},
			{.cells = 250, .enable = 1, .trip = 0},
			{.cells = 250, .enable = 1, .trip = 0},
			{.cells = 300, .enable = 1, .trip = 0},
		},

		.d_xoff_ffa_thres = {.cells = 470, .enable = 1, .trip = 0},
		.d_port_xoff_ffa_thres = {
			{.cells = 250, .enable = 1, .trip = 0},
			{.cells = 200, .enable = 1, .trip = 0},
			{.cells = 200, .enable = 1, .trip = 0},
			{.cells = 200, .enable = 1, .trip = 0},
			{.cells = 200, .enable = 1, .trip = 0},
			{.cells = 250, .enable = 1, .trip = 0},
		},

		.d_xon_ffa_thres = {.cells = 350},
		.d_port_xon_ffa_thres = {
			{.cells = 150},
			{.cells = 100},
			{.cells = 100},
			{.cells = 100},
			{.cells = 100},
			{.cells = 150},
		},
	},
	{//index4
		.desc = "10G + 2.5G + 1G + EDMA",
		.d_res_limiter = {
			{.yellowAccumulated = 700, .yellowLimit = 700, .redLimit = 700, .maxCells = 255},
			{.yellowAccumulated = 250, .yellowLimit = 200, .redLimit = 150, .maxCells = 255},
			{.yellowAccumulated = 250, .yellowLimit = 50,  .redLimit = 50,  .maxCells = 255},
			{.yellowAccumulated = 9,   .yellowLimit = 9,   .redLimit = 9,   .maxCells = 255},
		},
		.d_erm_red = {.redXoff = 100, .redXon = 150, .redMaxCells = 9},
		.d_erm_yellow = {.yellowXoff = 100, .yellowXon = 200, .redPortEn = 0, .redPortXoff = 115},
		.d_e_res_mg_pointer = {
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
			{.q0 = 0, .q1 = 0, .q2 = 0, .q3 = 0, .q4 = 0, .q5 = 0, .q6 = 0, .q7 = 0},
		},

		.d_port_reserved = {
			{.cells = 50},
			{.cells = 30},
			{.cells = 0},
			{.cells = 0},
			{.cells = 10},
			{.cells = 50},
		},
		.d_port_pause = {
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
		},
		.d_port_tdrop = {//Port Tail-Drop Setting
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
		},

		.d_tdrop_ffa_thres = {.cells = 700, .enable = 1, .trip = 0},
		.d_port_tdrop_ffa_thres = {
			{.cells = 450, .enable = 1, .trip = 0},
			{.cells = 300, .enable = 1, .trip = 0},
			{.cells = 300, .enable = 1, .trip = 0},
			{.cells = 300, .enable = 1, .trip = 0},
			{.cells = 300, .enable = 1, .trip = 0},
			{.cells = 250, .enable = 1, .trip = 0},
		},

		.d_xoff_ffa_thres = {.cells = 400, .enable = 1, .trip = 0},
		.d_port_xoff_ffa_thres = {
			{.cells = 350, .enable = 1, .trip = 0},
			{.cells = 200, .enable = 1, .trip = 0},
			{.cells = 200, .enable = 1, .trip = 0},
			{.cells = 200, .enable = 1, .trip = 0},
			{.cells = 200, .enable = 1, .trip = 0},
			{.cells = 200, .enable = 1, .trip = 0},
		},

		.d_xon_ffa_thres = {.cells = 350},
		.d_port_xon_ffa_thres = {
			{.cells = 250},
			{.cells = 150},
			{.cells = 150},
			{.cells = 150},
			{.cells = 150},
			{.cells = 100},
		},
	},
	{//index5
		.desc = "10G + 2.5G + 1G + EDMA + Support Egress Queue Scheduling",
		.d_res_limiter = {
			{.yellowAccumulated = 700, .yellowLimit = 700, .redLimit = 700,	.maxCells = 255},
			{.yellowAccumulated = 130, .yellowLimit = 100, .redLimit = 50,	.maxCells = 255},
			{.yellowAccumulated = 160, .yellowLimit = 100, .redLimit = 50,	.maxCells = 255},
			{.yellowAccumulated = 190, .yellowLimit = 100, .redLimit = 50,	.maxCells = 255},
		},
		.d_erm_red = {.redXoff = 100, .redXon = 150, .redMaxCells = 9},
		.d_erm_yellow = {.yellowXoff = 400, .yellowXon = 550, .redPortEn = 63, .redPortXoff = 500},
		.d_e_res_mg_pointer = {
			{.q0 = 0, .q1 = 1, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 1, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 1, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 1, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 1, .q2 = 1, .q3 = 1, .q4 = 2, .q5 = 2, .q6 = 3, .q7 = 3},
			{.q0 = 0, .q1 = 1, .q2 = 1, .q3 = 1, .q4 = 1, .q5 = 1, .q6 = 1, .q7 = 1},
		},
		.d_port_reserved = {
			{.cells = 50},
			{.cells = 20},
			{.cells = 20},
			{.cells = 20},
			{.cells = 20},
			{.cells = 50},
		},
		.d_port_pause = {
			{.enable = 0, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 0, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
			{.enable = 1, .mode = 1, .reserved = 0, .force = 0, .pattern = 0},
		},
		.d_port_tdrop = {
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
			{.enable = 1, .mode = 1},
		},

		.d_tdrop_ffa_thres = {.cells = 800, .enable = 1, .trip = 0},
		.d_port_tdrop_ffa_thres = {
			{.cells = 500, .enable = 1, .trip = 0},
			{.cells = 500, .enable = 1, .trip = 0},
			{.cells = 500, .enable = 1, .trip = 0},
			{.cells = 500, .enable = 1, .trip = 0},
			{.cells = 500, .enable = 1, .trip = 0},
			{.cells = 500, .enable = 1, .trip = 0},
		},

		.d_xoff_ffa_thres = {.cells = 600, .enable = 1, .trip = 0},
		.d_port_xoff_ffa_thres = {
			{.cells = 400, .enable = 1, .trip = 0},
			{.cells = 300, .enable = 1, .trip = 0},
			{.cells = 300, .enable = 1, .trip = 0},
			{.cells = 300, .enable = 1, .trip = 0},
			{.cells = 300, .enable = 1, .trip = 0},
			{.cells = 400, .enable = 1, .trip = 0},
		},

		.d_xon_ffa_thres = {.cells = 350},
		.d_port_xon_ffa_thres = {
			{.cells = 250},
			{.cells = 150},
			{.cells = 150},
			{.cells = 150},
			{.cells = 150},
			{.cells = 250},
		},
	},
};

int dubhe2000_max_pause_mod(void)
{
	return ARRAY_SIZE(dubhe2000_pause_tbl) - 1;
}

void dubhe2000_switch_pause_config(struct dubhe1000_adapter *adapter)
{
	struct dubhe2000_switch_pause_config *config;
	int i;

	if (adapter->switch_pause_mod >= ARRAY_SIZE(dubhe2000_pause_tbl)) {
		pr_info("[%s] invalid mode! keep initvalue\n", __func__);

		adapter->switch_pause_mod = 0;
	}

	switch_pause_mod = adapter->switch_pause_mod;

	config = &dubhe2000_pause_tbl[adapter->switch_pause_mod];

	pr_info("[%s] mode=%d (%s)\n", __func__, adapter->switch_pause_mod, config->desc);

	if (adapter->switch_pause_mod == 0) // InitValue, ignore it
		return;

	/* ERM config*/
	//Resource Limiter Set 0/1/2/3
	for (i = 0; i < ARRAY_SIZE(config->d_res_limiter); i++)
		wr_ResourceLimiterSet(adapter, i, &config->d_res_limiter[i]);

	//ERM Red Configuration
	rd_ERMRedConfiguration(adapter, &config->d_erm_red);

	//ERM Yellow Configuration
	rd_ERMYellowConfiguration(adapter, &config->d_erm_yellow);

	//Egress Resource Manager Pointer 0/1/2/3/4/5/
	for (i = 0; i < ARRAY_SIZE(config->d_e_res_mg_pointer); i++)
		wr_EgressResourceManagerPointer(adapter, i, &config->d_e_res_mg_pointer[i]);

	/* FlowCrtl config*/
	//Tail-Drop FFA Threshold
	wr_TailDropFFAThreshold(adapter, &config->d_tdrop_ffa_thres);

	//Port Tail-drop FFA Threshold 0/1/2/3/4/5
	for (i = 0; i < ARRAY_SIZE(config->d_port_tdrop_ffa_thres); i++)
		wr_PortTailDropFFAThreshold(adapter, i, &config->d_port_tdrop_ffa_thres[i]);

	//Xoff FFA Threshold
	wr_XoffFFAThreshold(adapter, &config->d_xoff_ffa_thres);
	//Port Xoff FFA Threshold 0/1/2/3/4/5
	for (i = 0; i < ARRAY_SIZE(config->d_port_xoff_ffa_thres); i++)
		wr_PortXoffFFAThreshold(adapter, i, &config->d_port_xoff_ffa_thres[i]);

	//Xon FFA Threshold
	wr_XonFFAThreshold(adapter, &config->d_xon_ffa_thres);
	//Port Xon FFA Threshold 0/1/2/3/4/5
	for (i = 0; i < ARRAY_SIZE(config->d_port_xon_ffa_thres); i++)
		wr_PortXonFFAThreshold(adapter, i, &config->d_port_xon_ffa_thres[i]);

	//Port Reserved 5
	for (i = 0; i < ARRAY_SIZE(config->d_port_reserved); i++)
		wr_PortReserved(adapter, i, &config->d_port_reserved[i]);

	//Port Pause Settings 0/1/2/3/4/5
	for (i = 0; i < ARRAY_SIZE(config->d_port_pause); i++)
		wr_PortPauseSettings(adapter, i, &config->d_port_pause[i]);

	//Port Tail-drop Setting 0/1/2/3/4/5
	for (i = 0; i < ARRAY_SIZE(config->d_port_tdrop); i++)
		wr_PortTailDropSettings(adapter, i, &config->d_port_tdrop[i]);
}
