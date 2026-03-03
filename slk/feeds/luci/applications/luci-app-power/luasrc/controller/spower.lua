--[[
LuCI - Lua Configuration Interface

Copyright (c) 2013 Qualcomm Atheros, Inc.

All Rights Reserved.
Qualcomm Atheros Confidential and Proprietary.

]]--

module("luci.controller.spower", package.seeall)

function index()
	local page

	if nixio.fs.access("/etc/config/spwr") then
		page = entry({"admin", "network","power"}, cbi("admin_network/power"), _("POE Power"), 5)
	end
	

end
