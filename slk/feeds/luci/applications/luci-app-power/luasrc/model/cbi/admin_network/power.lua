local sys   = require "luci.sys"
local m, s
local fs = require "nixio.fs"
local util = require "nixio.util"

m = Map("spwr", translate("Parameter Settings"))
m:chain("luci")

s = m:section(TypedSection, "serial_power", translate("POE Power Output"))
s.anonymous = true
s.addremove = false

power_en = s:option(ListValue, "power_en", translate("ON/OFF"), translate("POE Power Output ON/OFF"))
power_en:value("ON", translate("ON"))
power_en:value("OFF", translate("OFF"))

-- local apply = luci.http.formvalue("cbi.apply")
-- if apply then
    -- io.popen("/etc/init.d/spower start &")
-- end
function m.on_after_commit(map)
	io.popen("/etc/init.d/spower start &")
end
return m

