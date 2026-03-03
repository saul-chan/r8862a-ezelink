'use strict';
'require view';
'require dom';
'require form';
'require uci';
'require rpc';
'require network';
'require tools.cls-qos as clstool';
'require tools.widgets as widgets';
'require tools.cls-widgets as clswidgets';

function rule_proto_show(s) {
	var f = (uci.get('cls-qos', s, 'family') || '').toLowerCase().replace(/^(?:any|\*)$/, '');

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
		src:  uci.get('cls-qos', s, 'src_ip'),
		dest: uci.get('cls-qos', s, 'dest_ip'),
		proto: proto
	});
}

function rule_src_show(s, hosts) {
	return clstool.fmt(_('%{src_ip? IP %{src_ip#%{next?, }<var%{item.inv? data-tooltip="Match IP addresses except %{item.val}."}>%{item.ival}</var>}}%{src_port? <br>:}%{src_port? Port %{src_port#%{next?, }<var%{item.inv? data-tooltip="Match ports except %{item.val}."}>%{item.ival}</var>}}%{src_mac? <br>:}%{src_mac? MAC %{src_mac#%{next?, }<var%{item.inv? data-tooltip="Match MACs except %{item.val}%{item.hint.name? a.k.a. %{item.hint.name}}.":%{item.hint.name? data-tooltip="%{item.hint.name}"}}>%{item.ival}</var>}}'), {
		src_ip: clstool.map_invert(uci.get('cls-qos', s, 'src_ip'), 'toLowerCase'),
		src_mac: clstool.map_invert(uci.get('cls-qos', s, 'src_mac'), 'toUpperCase').map(function(v) { return Object.assign(v, { hint: hosts[v.val] }) }),
		src_port: clstool.map_invert(uci.get('cls-qos', s, 'src_port'))
	});
}

function rule_dst_show(s, hosts) {
	return clstool.fmt(_('%{dest_ip? IP %{dest_ip#%{next?, }<var%{item.inv? data-tooltip="Match IP addresses except %{item.val}."}>%{item.ival}</var>}}%{dest_port? <br>:}%{dest_port? Port %{dest_port#%{next?, }<var%{item.inv? data-tooltip="Match ports except %{item.val}."}>%{item.ival}</var>}}%{dest_mac? <br>:}%{dest_mac? MAC %{dest_mac#%{next?, }<var%{item.inv? data-tooltip="Match MACs except %{item.val}%{item.hint.name? a.k.a. %{item.hint.name}}.":%{item.hint.name? data-tooltip="%{item.hint.name}"}}>%{item.ival}</var>}}'), {
		dest_ip: clstool.map_invert(uci.get('cls-qos', s, 'dest_ip'), 'toLowerCase'),
		dest_mac: clstool.map_invert(uci.get('cls-qos', s, 'dest_mac'), 'toUpperCase').map(function(v) { return Object.assign(v, { hint: hosts[v.val] }) }),
		dest_port: clstool.map_invert(uci.get('cls-qos', s, 'dest_port'))
	});
}

function rule_action_show(s) {
	return clstool.fmt(_('%{dscp? Set DSCP %{dscp#%{next?, }<var%{item.inv? data-tooltip="Match DSCP except %{item.val}."}>%{item.ival}</var>}}%{priority? <br>:}%{priority? Set Priority %{priority#%{next?, }<var%{item.inv? data-tooltip="Match Priority except %{item.val}."}>%{item.ival}</var>}}%{vlan8021p? <br>:}%{vlan8021p? Set Vlan8021p %{vlan8021p#%{next?, }<var%{item.inv? data-tooltip="Match vlan8021p except %{item.val}."}>%{item.ival}</var>}}%{rate_limit? <br>:}%{rate_limit? Bandwidth Limit %{rate_limit#%{next?, }<var%{item.inv? data-tooltip="Match Rate_Limit except %{item.val}."}>%{item.ival}</var>}}'), {
		dscp: clstool.map_invert(uci.get('cls-qos', s, 'set_dscp')),
		priority: clstool.map_invert(uci.get('cls-qos', s, 'set_priority')),
		vlan8021p: clstool.map_invert(uci.get('cls-qos', s, 'set_vlan8021p')),
		rate_limit: clstool.map_invert(uci.get('cls-qos', s, 'rate_limit_mbps')),
	});
}

function set_dscp_value(o) {
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
}

return  view.extend({
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

	render: function(data) {
		var hosts = data[0], lanlist = data[1], m, s, o;

		m = new form.Map('cls-qos', _('CLS-QoS setting'));

		s = m.section(form.GridSection, 'rule');
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
		clstool.addMACOption(s, 'general', 'src_mac', _('Source MAC'), null, hosts);

		/* Destination MAC */
		clstool.addMACOption(s, 'general', 'dest_mac', _('Destination MAC'), null, hosts);

		/* IP family */
		o = s.taboption('general', form.ListValue, 'family', _('IP family'));
		o.modalonly = true;
		o.rmempty = true;
		o.value('', _('IPv4 and IPv6'));
		o.value('ipv4', _('IPv4 only'));
		o.value('ipv6', _('IPv6 only'));
		o.validate = function(section_id, value) {
			clstool.updateHostHints(this.map, section_id, 'src_ip', value, hosts);
			clstool.updateHostHints(this.map, section_id, 'dest_ip', value, hosts);
			return true;
		};

		/* Protocol */
		o = s.taboption('general', clstool.CBIProtocolSelect, 'proto', _('Protocol'));
		o.default = 'tcp udp';
		o.modalonly = true;
		
		/* Source IP which is depends on IP family */
		clstool.addIPOption(s, 'general', 'src_ip', _('Source IP'), null, '', hosts, true);

		/* Port which is depends on Protocol */
		o = s.taboption('general', form.Value, 'src_port', _('Source port'));
		o.modalonly = true;
		o.datatype = 'list(neg(portrange))';
		o.placeholder = _('any');
		o.depends({ proto: 'tcp', '!contains': true });
		o.depends({ proto: 'udp', '!contains': true });
		
		/* Destination IP which is depends on IP family */
		clstool.addIPOption(s, 'general', 'dest_ip', _('Destination IP'), null, '', hosts, true);

		o = s.taboption('general', form.Value, 'dest_port', _('Destination port'));
		o.modalonly = true;
		o.datatype = 'list(neg(portrange))';
		o.placeholder = _('any');
		o.depends({ proto: 'tcp', '!contains': true });
		o.depends({ proto: 'udp', '!contains': true });

		o = s.taboption('general', form.Value, 'dscp', _('Match DSCP'));
		o.placeholder = _('any');
		o.modalonly = true;
		set_dscp_value(o);

		/* Action */
		o = s.taboption('action', form.Value, 'set_priority', _('Priority value'), _('The range of Priority value is 0~7'));
		o.modalonly = true;
		o.validate = function(section_id, value) {
			var optval = Number(value);

			if (optval < 0 || optval > 7 || !Number.isInteger(optval))
				return _('Invalid data! Please enter an integer of 0~7');
			return true;
		};

		o = s.taboption('action', form.Value, 'set_dscp', _('DSCP Value'));
		o.placeholder = _('Disable');
		o.modalonly = true;
		set_dscp_value(o);

		o = s.taboption('action', form.Value, 'set_vlan8021p', _('Vlan priority'), _('The range of Vlan priority is 0~7'));
		o.modalonly = true;
		o.validate = function(section_id, value) {
			var optval = Number(value);

			if (optval < 0 || optval > 7 || !Number.isInteger(optval))
				return _('Invalid data! Please enter an integer of 0~7');
			return true;
		};

		/* Bandwidth Setting */
		o = s.taboption('action', form.Value, 'rate_limit_mbps', _('Bandwidth Limit'), _('Mbps'));
		o.modalonly = true;
		o.validate = function(section_id, value) {
			if (value === '')
				return true;

			var optval = Number(value);
			
			if (optval < 0.5 || optval > 1000)
				return _('Invalid data! Please enter an value from 0.5 to 1000');

			return true;
		};

		return m.render();
	}
});

