'use strict';
'require view';
'require dom';
'require rpc';
'require uci';
'require form';
'require ui';

document.querySelector('head').appendChild(E('link', {
    'rel': 'stylesheet',
    'type': 'text/css',
    'href': L.resource('view/network/css/custom.css')
}));

var callUciSet = rpc.declare({
    object: 'uci',
    method: 'set',
    params: [ 'config', 'section', 'values' ]
});

var callUciDel = rpc.declare({
    object: 'uci',
    method: 'delete',
    params: [ 'config', 'section', 'option' ]
});

var callUciCommit = rpc.declare({
    object: 'uci',
    method: 'commit',
    params: [ 'config' ]
});


function toggleInputs() {
    var checkbox = document.getElementById("enable-dhcp"),
        dhcpSettings = document.getElementById("dhcp-settings");

    if (checkbox.checked)
		dhcpSettings.classList.remove("hidden");
    else
		dhcpSettings.classList.add("hidden");
}

function applyLANConfiguration(){
    // lan settings
    var oldIp = uci.get('network', 'lan', 'ipaddr'),
        oldNetmask = uci.get('network', 'lan', 'netmask'),
        ipaddr = document.getElementById("lan-ipaddress"),
        netmask = document.getElementById("lan-netmask"),
        lan_conf = [];
    // dhcp settings
    var dhcpState = document.getElementById("enable-dhcp"),
        dhcpStart = document.getElementById("dhcp-start"),
        dhcpStop = document.getElementById("dhcp-stop"),
        leaseTime = document.getElementById("dhcp-lease-time"),
        oldState = uci.get('dhcp', 'lan', 'ignore') == 1? 'on': 'off',
        oldStart = uci.get('dhcp', 'lan', 'start'),
        oldLimit = uci.get('dhcp', 'lan', 'limit'),
        oldLease = uci.get('dhcp', 'lan', 'leasetime'),
        dhcp_conf = [];

    if (ipaddr.value != oldIp)
        lan_conf.push({ "ipaddr": ipaddr.value });

    if (netmask.value != oldNetmask)
        lan_conf.push({ "netmask": netmask.value });

    if (dhcpState.checked) {
        oldState == dhcpState.value? '': dhcp_conf.push({ "ignore": '0'});
        dhcpStart.value == oldStart? '': dhcp_conf.push({ "start": dhcpStart.value });
        dhcpStop.value == oldLimit? '': dhcp_conf.push({ "limit": (dhcpStop.value - dhcpStart.value) });
        leaseTime.value == parseInt(oldLease, 10)? '': dhcp_conf.push({ "leasetime": leaseTime.value + 'h' });
    } else
        oldState == 'on'? dhcp_conf.push({ "ignore": '1'}): '';
    
    if (lan_conf.length == 0 && dhcp_conf.length == 0) {
        ui.changes.displayStatus('notice',
			E('p', _('There are no changes to apply')));

        window.setTimeout(function() {
			ui.changes.displayStatus(false);
        }, L.env.apply_display*1000);
    } else {
		if (lan_conf.length != 0) {
			for (var i = 0; i < lan_conf.length; i++) {
				for (var key in lan_conf[i]) {
					if (key == 'ipaddr')
						callUciSet('network', 'lan', { 'ipaddr': lan_conf[i][key] });
					else if (key == 'netmask')
						callUciSet('network', 'lan', { 'netmask': lan_conf[i][key] });
				}
			}

			callUciCommit('network');
		}

		if (dhcp_conf.length != 0) {
			for (var i = 0; i < dhcp_conf.length; i++) {
				for (var key in dhcp_conf[i]) {
					if (key == 'ignore' && dhcp_conf[key] == '0')
						callUciDel('dhcp', 'lan', key);
					else if (key == 'start')
						callUciSet('dhcp', 'lan', { 'start': dhcp_conf[i][key] });
					else if (key == 'limit')
						callUciSet('dhcp', 'lan', { 'limit': dhcp_conf[i][key] });
					else if (key ==  'leasetime')
						callUciSet('dhcp', 'lan', { 'leasetime': dhcp_conf[i][key] });
				}
			}

			callUciCommit('dhcp');
		}

        ui.changes.displayStatus('notice spinning',
            E('p', _('Starting configuration apply...')));

        window.setTimeout(function() {
			ui.changes.displayStatus(false);
        }, L.env.apply_display*1000);
    }
}

return view.extend({
    load: function() {
        return Promise.all([
            uci.load('dhcp'),
            uci.load('network')
        ]);
    },

    renderLanInfo: function(data) {
        var ip = uci.get('network', 'lan', 'ipaddr'),
            netmask = uci.get('network', 'lan', 'netmask'),
            ipStart = uci.get('dhcp', 'lan', 'start'),
            ipEnd = parseInt(uci.get('dhcp', 'lan', 'limit')) + parseInt(ipStart),
            leaseTime = uci.get('dhcp', 'lan', 'leasetime'),
            enable = uci.get('dhcp', 'lan', 'ignore'),
            elem = E('div', { "class": "network-lan" });

        dom.content(elem, [
            E('div', { 'class': 'cbi-section' }, [
                E('div', { "class": "column lan-info" }, [
                    E('h3', _('LAN Configuration')),
                    E('div', { "class": "form-group" }, [
                        E('label', { "for": "ipAddress" }, _('IP Address')),
                        E('input', {
                            "id": "lan-ipaddress",
                            "type": "text",
                            "value": ip
                        })
                    ]),
                    E('div', { "class": "form-group" }, [
                        E('label', { "for": "netmask" }, _('Subnet Mask')),
                        E('input', {
                            "id": "lan-netmask",
                            "type": "text",
                            "value": netmask
                        })
                    ])
                ]),
                E('div', { "class": "column dhcp-info" }, [
                    E('div', { "class": "form-group" }, [
                        E('label', { "for": "checkbox" }, _('Enable DHCP')),
                        E('input', {
                            "id": "enable-dhcp",
                            "type": "checkbox",
                            "change": L.bind(toggleInputs),
                        })
                    ]),
                    E('div', { "id": "dhcp-settings", "class": "hidden" }, [
                        E('div', { "class": "form-group inline-group" }, [
                            E('label', { "for": "" }, _("Address Pool")),
                            E('span', { "class": "ip-prefix" }, ip.substring(0, ip.lastIndexOf(".")) + '.'),
                            E('input', {
                                "id": "dhcp-start",
                                "type": "text",
                                "value": ipStart
                            }),
                            E('span', _('-')),
                            E('input', {
                                "id": "dhcp-stop",
                                "type": "text",
                                "value": ipEnd
                            }),
                        ]),
                        E('div', { "class": "form-group dhcp-lease" }, [
                            E('label', { "for": "lease-time" }, _('Lease Time')),
                            E('input', {
                                "id": "dhcp-lease-time",
                                "type": "text",
                                "value": parseInt(leaseTime, 10)
                            }),
                            E('span', _('hour'))
                        ])
                    ])
                ])
            ]),
            E('div', { "class": "cbi-page-actions" }, [
                E('button', {
					"class": "dhcp-apply",
					"click": L.bind(applyLANConfiguration)
                }, _('Save & Apply'))
            ])
        ]);

        if (enable != 1) {
			elem.querySelector("#enable-dhcp").checked = true;
			elem.querySelector("#dhcp-settings").classList.remove("hidden");
        }

        return elem;
    },

    render: function(data) {
        return Promise.all([
            this.renderLanInfo(data)
        ]);
    },

    handleSave: null,
    handleSaveApply: null,
    handleReset: null
})
