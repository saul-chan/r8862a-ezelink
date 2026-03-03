module("luci.controller.serial", package.seeall)

function index()
	
		entry({"admin", "serial"},alias("serial","gnss"), _("Serial Utility"), 55).index = true
			entry({"admin", "serial","gnss"}, cbi("serial/GNSS"), _("GNSS"), 2)
			entry({"admin", "serial","switch"}, cbi("serial/switch"), _("IO switch quantity"), 3)	
			entry({"admin", "serial","io_control"}, cbi("serial/io_control"), _("IO Control"), 4)
			entry({"admin", "serial","port"}, cbi("serial/port"), _("PORT"), 5)

end
