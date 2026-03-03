-- Copyright 2012 Christian Gagneraud <chris@techworks.ie>
-- Licensed to the public under the Apache License 2.0.
local m, section

m = Map("io",translate("GPIO Controller"), "Control wifi enable via gpio.")

s = m:section(TypedSection, "io")
s.anonymous = true
s.addremove = false

enabled=s:option( ListValue, "enable", translate("ON/OFF"), translate("If set to ON then WIFl DOWN by default, when GPIO pull up WIFI up. </br>If set to OFF the WIFI up always."))
enabled:value("0", translate("OFF"))
enabled:value("1", translate("ON"))

function m.on_after_commit(map)
	io.popen("/etc/init.d/io_init restart &")
end

return m
-- return m
