'use strict';
'require view';
'require dom';
'require rpc';
'require uci';
'require form';
'require fs';
'require poll';
'require network';
'require tools.cls-qos as clstool';
'require tools.widgets as widgets';

document.querySelector('head').appendChild(E('link', {
    'rel': 'stylesheet',
    'type': 'text/css',
    'href': L.resource('view/network/css/cls-qos.css')
}));

function rule_proto_show(s) {
    var f = (uci.get('cls-qos', s, 'ip_family') || '').toLowerCase().replace(/^(?:any|\*)$/, '');

    var proto = L.toArray(uci.get('cls-qos', s, 'proto')).filter(function(p) {
        return (p != '*' && p != 'any' && p != 'all');
    }).map(function(p) {
        var pr = clstool.lookupProto(p);
        return {
            num:   pr[0],
            name:  pr[1]
        };
    });

    return clstool.fmt(_('%{(src||dest)?%{null}:IP family} %{ipv6?%{ipv4?<var>IPv4</var> and <var>IPv6</var>:<var>IPv6</var>}:<var>IPv4</var>}%{proto? <br>:}%{proto? Protocol %{proto#%{next?, }%{item.types?<var class="cbi-tooltip-container">%{item.name}<span class="cbi-tooltip">ICMP with types %{item.types#%{next?, }<var>%{item}</var>}</span></var>:<var>%{item.name}</var>}}}'), {
        ipv4: (!f || f == 'ipv4'),
        ipv6: (!f || f == 'ipv6'),
        src:  uci.get('cls-qos', s, 'srcip'),
        dest: uci.get('cls-qos', s, 'dstip'),
        proto: proto
    });
}

function rule_src_show(s, hosts) {
    return clstool.fmt(_('%{srcip? IP %{srcip#%{next?, }<var%{item.inv? data-tooltip="Match IP addresses except %{item.val}."}>%{item.ival}</var>}}%{srcport? <br>:}%{srcport? Port %{srcport#%{next?, }<var%{item.inv? data-tooltip="Match ports except %{item.val}."}>%{item.ival}</var>}}%{srcmac? <br>:}%{srcmac? MAC %{srcmac#%{next?, }<var%{item.inv? data-tooltip="Match MACs except %{item.val}%{item.hint.name? a.k.a. %{item.hint.name}}.":%{item.hint.name? data-tooltip="%{item.hint.name}"}}>%{item.ival}</var>}}'), {
        srcip: clstool.map_invert(uci.get('cls-qos', s, 'srcip'), 'toLowerCase'),
        srcmac: clstool.map_invert(uci.get('cls-qos', s, 'srcmac'), 'toUpperCase').map(function(v) { return Object.assign(v, { hint: hosts[v.val] }) }),
        srcport: clstool.map_invert(uci.get('cls-qos', s, 'srcport'))
    });
}

function rule_dst_show(s, hosts) {
    return clstool.fmt(_('%{dstip? IP %{dstip#%{next?, }<var%{item.inv? data-tooltip="Match IP addresses except %{item.val}."}>%{item.ival}</var>}}%{dstport? <br>:}%{dstport? Port %{dstport#%{next?, }<var%{item.inv? data-tooltip="Match ports except %{item.val}."}>%{item.ival}</var>}}%{dstmac? <br>:}%{dstmac? MAC %{dstmac#%{next?, }<var%{item.inv? data-tooltip="Match MACs except %{item.val}%{item.hint.name? a.k.a. %{item.hint.name}}.":%{item.hint.name? data-tooltip="%{item.hint.name}"}}>%{item.ival}</var>}}'), {
        dstip: clstool.map_invert(uci.get('cls-qos', s, 'dstip'), 'toLowerCase'),
        dstmac: clstool.map_invert(uci.get('cls-qos', s, 'dstmac'), 'toUpperCase').map(function(v) { return Object.assign(v, { hint: hosts[v.val] }) }),
        dstport: clstool.map_invert(uci.get('cls-qos', s, 'dstport'))
    });
}

function rule_action_show(s) {
    return clstool.fmt(_('%{dscp? DSCP %{dscp#%{next?, }<var%{item.inv? data-tooltip="Match DSCP except %{item.val}."}>%{item.ival}</var>}}%{priority? <br>:}%{priority? Priority %{priority#%{next?, }<var%{item.inv? data-tooltip="Match Priority except %{item.val}."}>%{item.ival}</var>}}%{vlan8021p? <br>:}%{vlan8021p? Vlan8021p %{vlan8021p#%{next?, }<var%{item.inv? data-tooltip="Match vlan8021p except %{item.val}."}>%{item.ival}</var>}}%{min_ul? <br>:}%{min_ul? Uplink Min Rate %{min_ul#%{next?, }<var%{item.inv? data-tooltip="Match Min_Uplink except %{item.val}."}>%{item.ival}</var>}}%{min_dl? <br>:}%{min_dl? Downlink Min Rate %{min_dl#%{next?, }<var%{item.inv? data-tooltip="Match Min_Downlink except %{item.val}."}>%{item.ival}</var>}}%{max_ul? <br>:}%{max_ul? Uplink Max Rate %{max_ul#%{next?, }<var%{item.inv? data-tooltip="Match Max_Uplink except %{item.val}."}>%{item.ival}</var>}}%{max_dl? <br>:}%{max_dl? Downlink Max Rate %{max_dl#%{next?, }<var%{item.inv? data-tooltip="Match Min_Uplink except %{item.val}."}>%{item.ival}</var>}}'), {
        dscp: clstool.map_invert(uci.get('cls-qos', s, 'set_dscp')),
        priority: clstool.map_invert(uci.get('cls-qos', s, 'set_priority')),
        vlan8021p: clstool.map_invert(uci.get('cls-qos', s, 'set_vlan8021p')),
        min_ul: clstool.map_invert(uci.get('cls-qos', s, 'min_ul_traffic_rate')),
        min_dl: clstool.map_invert(uci.get('cls-qos', s, 'min_dl_traffic_rate')),
        max_ul: clstool.map_invert(uci.get('cls-qos', s, 'max_ul_traffic_rate')),
        max_dl: clstool.map_invert(uci.get('cls-qos', s, 'max_dl_traffic_rate'))
    });
}

function startPolling() {
    var updateDeviceList = function() {
        return fs.exec('/sbin/ip', ['neigh', 'show', 'dev', 'br-lan']).then(function(results) {
            var deviceInfo = results.stdout;
            var ipRegex = /(\d+\.\d+\.\d+\.\d+)/,
                macRegex = /lladdr ([\w:]+)/;

            var ip = deviceInfo.match(ipRegex)[0],
                mac = deviceInfo.match(macRegex)[1],
                name = 'unknow';

            var rows = [];

            for (let i = 0; i < 1; i++) {
                rows.push([
                    E('input', {
                        'style': s,
                    }, name),
                    E('span', {'style': s}, ip),
                    E('span', {'style': s}, mac),
                    E('button', {
                        'style': s
                    }, [
                        E('span', {'style':s}, 'apply')
                    ])
                ]);
            }

            var table = document.querySelector('#advance_device_list');

            cbi_update_table(table, rows);
        });
    };

    return updateDeviceList().then(function() {
        poll.add(updateDeviceList);
    });
}

return view.extend({
    callHostHints: rpc.declare({
                object: 'luci-rpc',
                        method: 'getHostHints',
                        expect: { '': {} }
        }),

        load: function() {
                return Promise.all([
                        this.callHostHints(),
                        network.getDevices(),
                        uci.load('cls-qos')
                ]);
        },

    renderTitle: function() {
        var m;
        m = new form.Map('network', _('Advance Settings'));

        return m.render();
    },

    renderDeviceList: function() {
        var table = E('table', {'class':'advance_device_list', 'id':'advance_device_list'}, [
            E('tr', {'class':'tr table-titles'}, [
                E('th', {'class': 'th '}, _('Name')),
                E('th', {'class': 'th '}, _('IP address')),
                E('th', {'class': 'th '}, _('MAC address')),
                E('th', {'class': 'th '}, _('Action')),
            ])
        ]);

        return startPolling().then(function() {
            return E([E('br'), E('h3', _('Device List')), table]);
        });
    },

    renderParentCtrol: function() {
        /* To be design */
    },

    renderUltraQoS: function(data) {
        var hosts = data[0], lanlist = data[1], m, s, o;

        m = new form.Map('cls-qos');

        s = m.section(form.GridSection, 'rule', _('Ultra QoS'), _('Ultra QoS is an advanced network service quality management technology primarily used to optimize and improve network performance.'));
        s.addremove = true;
        s.anonymous = true;
        s.sortable = true;

        s.tab('general', _('General settings'));
        s.tab('action', _('Action settings'));

        /* Table head */
        o = s.option(form.Value, 'name', _('Name'));
        o.modalonly = false;

        o = s.option(form.DummyValue, '_source', _('Source'));
        o.modalonly = false;
        o.textvalue = function(s) {
            return E('small', [
                rule_proto_show(s), E('br'),
                rule_src_show(s, hosts)
            ]);
        };

        o = s.option(form.DummyValue, '_destination', _('Destination'));
        o.modalonly = false;
        o.textvalue = function(s) {
            return E('small', [
                rule_proto_show(s), E('br'),
                rule_dst_show(s, hosts)
            ]);
        };

        o = s.option(form.DummyValue, '_target', _('Action'));
        o.modalonly = false;
        o.textvalue = function(s) {
            return E('small', [
                rule_action_show(s)
            ]);
        };

        o = s.option(form.Flag, 'enabled', _('Enable'));
        o.modalonly = false;
        o.default = o.enabled;
        o.editable = true;

        /* Configuration page */

        /* usr defined string */
        o = s.taboption('general', form.Value, 'name', _('Name'));
        o.placeholder = _('Unnamed rule');
        o.modalonly = true;

        /* Source Lan Device*/
        clstool.addLanDeviceOption(s, 'general', 'src_device', _('Source Lan Device'), null, lanlist)

        /* Destination Lan Device */
        clstool.addLanDeviceOption(s, 'general', 'dest_device', _('Destination Lan Device'), null, lanlist);

        /* Source MAC */
        clstool.addMACOption(s, 'general', 'srcmac', _('Source MAC'), null, hosts);

        /* Destination MAC */
        clstool.addMACOption(s, 'general', 'dstmac', _('Destination MAC'), null, hosts);

        /* IP family */
        o = s.taboption('general', form.ListValue, 'ip_family', _('IP family'));
        o.modalonly = true;
        o.rmempty = true;
        o.value('', _('IPv4 and IPv6'));
        o.value('ipv4', _('IPv4 only'));
        o.value('ipv6', _('IPv6 only'));
        o.validate = function(section_id, value) {
            clstool.updateHostHints(this.map, section_id, 'srcip', value, hosts);
            clstool.updateHostHints(this.map, section_id, 'dstip', value, hosts);
            return true;
        };

        /* Protocol */
        o = s.taboption('general', clstool.CBIProtocolSelect, 'proto', _('Protocol'));
        o.default = 'tcp udp';
        o.modalonly = true;

        /* Source IP which is depends on IP family */
        clstool.addIPOption(s, 'general', 'srcip', _('Source IP'), null, '', hosts, true);

        /* Port which is depends on Protocol */
        o = s.taboption('general', form.Value, 'srcport', _('Source port'));
        o.modalonly = true;
        o.datatype = 'list(neg(portrange))';
        o.placeholder = _('any');
        o.depends({ proto: 'tcp', '!contains': true });
        o.depends({ proto: 'udp', '!contains': true });

        /* Destination IP which is depends on IP family */
        clstool.addIPOption(s, 'general', 'dstip', _('Destination IP'), null, '', hosts, true);

        o = s.taboption('general', form.Value, 'dstport', _('Destination port'));
        o.modalonly = true;
        o.datatype = 'list(neg(portrange))';
        o.placeholder = _('any');
        o.depends({ proto: 'tcp', '!contains': true });
        o.depends({ proto: 'udp', '!contains': true });

        /* Action */
        o = s.taboption('action', form.Value, 'set_priority', _('Priority value'), _('The range of Priority value is 0~7'));
        o.modalonly = true;
        o.validate = function(section_id, value) {
            var optval = Number(value);

            if (optval < 0 || optval > 7 || !Number.isInteger(optval))
                return "Invalid data! Please enter an integer of 0~7";
            return true;
        };

        o = s.taboption('action', form.Value, 'set_dscp', _('DSCP Value'));
        o.placeholder = _('Disable');
        o.modalonly = true;
        o.value('CS0');
        o.value('CS1');
        o.value('CS2');
        o.value('CS3');
        o.value('CS4');
        o.value('CS5');
        o.value('CS6');
        o.value('CS7');
        o.value('BE');
        o.value('AF11');
        o.value('AF12');
        o.value('AF13');
        o.value('AF21');
        o.value('AF22');
        o.value('AF23');
        o.value('AF31');
        o.value('AF32');
        o.value('AF33');
        o.value('AF41');
        o.value('AF42');
        o.value('AF43');
        o.value('EF');

        o = s.taboption('action', form.Value, 'set_vlan8021p', _('Vlan priority'), _('The range of Vlan priority is 0~7'));
        o.modalonly = true;
        o.validate = function(section_id, value) {
            var optval = Number(value);
        
            if (optval < 0 || optval > 7 || !Number.isInteger(optval))
                return "Invalid data! Please enter an integer of 0~7";
            return true;
        };

        /* Bandwidth Setting */
        /* Bandwidth guarantee */
        o = s.taboption('action', form.Value, 'min_ul_traffic_rate', _('Uplink Bandwidth guarantee'), _('kbytes'));
        o.modalonly = true;
        o.validate = function(section_id, value) {
            if (value === '')
                return true;

            var optval = Number(value);

            if (!(/(^[1-9]\d*$)/.test(optval)))
                return "Invalid data! Please input positive integer";

            return true;
        };

        o = s.taboption('action', form.Value, 'min_dl_traffic_rate', _('Downlink Bandwidth guarantee'), _('kbytes'));
        o.modalonly = true;
        o.validate = function(section_id, value) {
            if (value === '')
                return true;

            var optval = Number(value);

            if (!(/(^[1-9]\d*$)/.test(optval)))
                return "Invalid data! Please input positive integer";

            return true;
        };

        /* Bandwidth limit */
        o = s.taboption('action', form.Value, 'max_ul_traffic_rate', _('Uplink Bandwidth limit'), _('kbytes'));
        o.modalonly = true;
        o.validate = function(section_id, value) {
            if (value === '')
                return true;

            var optval = Number(value);

            if (!(/(^[1-9]\d*$)/.test(optval)))
                return "Invalid data! Please input positive integer";

            return true;
        };

        o = s.taboption('action', form.Value, 'max_ul_traffic_rate', _('Downlink Bandwidth limit'), _('kbytes'));
        o.modalonly = true;
        o.validate = function(section_id, value) {
            if (value === '')
                return true;

            var optval = Number(value);

            if (!(/(^[1-9]\d*$)/.test(optval)))
                return "Invalid data! Please input positive integer";

            return true;
        };

        return m.render();
    },

    render: function(data) {
        return Promise.all([
            this.renderUltraQoS(data)
        ]);
    }
})
