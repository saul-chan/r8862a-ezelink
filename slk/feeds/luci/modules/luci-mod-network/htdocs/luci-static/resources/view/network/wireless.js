'use strict';
'require view';
'require dom';
'require poll';
'require fs';
'require ui';
'require rpc';
'require uci';
'require form';
'require network';
'require firewall';
'require tools.widgets as widgets';

var isReadonlyView = !L.hasViewPermission();
var g_dualband, dualBandFlag = true;

var callUciDelete = rpc.declare({
	object: 'uci',
	method: 'delete',
	params: ['config', 'section']
});

var callUciSet = rpc.declare({
	object: 'uci',
	method: 'set',
	params: ['config', 'section', 'values']
});

var callUciSet_first = rpc.declare({
	object: 'uci',
	method: 'set',
	params: ['config', 'type', 'values']
});

var callUciCommit = rpc.declare({
	object: 'uci',
	method: 'commit',
	params: ['config']
});

var callUciAdd = rpc.declare({
	object: 'uci',
	method: 'add',
	params: ['config', 'type']
});

var g_sync = [ "ssid", "encryption", "key", "macfilter" ];

function sync_2g_configuration(src_array, dst_array) {
	if (dst_array.length > src_array.length) {
		for (let i  = src_array.length; i < dst_array.length; i++)
			callUciDelete('wireless', dst_array[i]['.name']);
	}

	for (let i = 0; i < src_array.length && i < dst_array.length; i++) {
		for (let j = 0; j < g_sync.length; j++) {
			if (src_array[i].hasOwnProperty(g_sync[j]))
				callUciSet('wireless', dst_array[i]['.name'], { [g_sync[j]]: src_array[i][g_sync[j]]});
		}
	}

	for (let i = 0; i < src_array.length && i < dst_array.length; i++) {
		if (dst_array[i].hasOwnProperty('maclist'))
			callUciSet('wireless', dst_array[i]['.name'], {'maclist': ''});

		if (src_array[i].hasOwnProperty('maclist'))
			callUciSet('wireless', dst_array[i]['.name'], {'maclist': src_array[i].maclist});
	}
}

function add_5g_suffix(dst_array) {
	dst_array.forEach(function(obj) {
		callUciSet('wireless', obj['.name'], {'ssid': obj.ssid + '_5G'});
	});
}

function set_clsmesh_bandsteer(flag) {
	var sections = uci.sections('cls-mesh'), hasRoamingPolicy = false;
	var bs = {'band_steer': flag? '1': '0'};

	sections.forEach(function(subArray) {
		if (subArray['.type'] == 'roaming_policy')
			hasRoamingPolicy = true;
	});

	if (hasRoamingPolicy)
		callUciSet_first('cls-mesh', 'roaming_policy', bs);
	else {
		callUciAdd('cls-mesh', 'roaming_policy');
		callUciSet_first('cls-mesh', 'roaming_policy', bs);
	}
};

function set_bt_nbr_for_each_iface(src, dst, flag) {
	var bss = {'bss_transition': flag? '1': '0'},
	    rrm = {'rrm_neighbor_report': flag? '1': '0'};

	for (let i = 0; i < src.length && i < dst.length; i++) {
		callUciSet('wireless', src[i]['.name'], bss);
		callUciSet('wireless', src[i]['.name'], rrm);
		callUciSet('wireless', dst[i]['.name'], bss);
		callUciSet('wireless', dst[i]['.name'], rrm);
	}
};

function set_pwrmode(array) {
	array?.forEach(function(subArray) {
		if (subArray.includes('highpwr')) {
			uci.sections('wireless', 'wifi-device').forEach(subObj => {
				if (subArray[0] == 'set')
					uci.set('wireless', subObj['.name'], 'pwrmode', 'high');
				else if (subArray[0] == 'remove')
					uci.set('wireless', subObj['.name'], 'pwrmode', '');
			});
		}
	});
}

function radio0_mapping_radio1(sid) {
	var wiface = uci.sections('wireless', 'wifi-iface'),
		radio0 = [], radio1 = [];

	wiface.forEach(function(subArray) {
		if (subArray['device'] == 'radio0')
			radio0.push(subArray);
		else
			radio1.push(subArray);
	});

	for (var i = 0; i < radio0.length; i++) {
		if (!radio1[i])
			continue;

		if (radio0[i]['.name'] == sid)
			return radio1[i]['.name'];
	}
}

function count_changes(section_id) {
	var changes = ui.changes.changes, n = 0;

	if (!L.isObject(changes))
		return n;

	if (Array.isArray(changes.wireless))
		for (var i = 0; i < changes.wireless.length; i++)
			n += (changes.wireless[i][1] == section_id);

	return n;
}

function render_radio_badge(radioDev) {
	var isArray = Array.isArray(radioDev),
		isup;

	if (isArray)
		for (var i = 0; i < radioDev.length; i++)
			radioDev[i] == 'undefined'? radioDev[i].isUp()? isup = radioDev[i].isUp(): isup = false: isup = false;

	return E('span', { 'class': 'ifacebadge' }, [
		E('img', { 'src': L.resource('icons/wifi%s.png').format(isArray? isup? '': '_disabled': radioDev.isUp()? '': '_disabled')}),
		' ',
		g_dualband? 'radio': radioDev.getName()
	]);
}

function render_signal_badge(signalPercent, signalValue, noiseValue, wrap, mode) {
	var icon, title, value;

	if (signalPercent < 0)
		icon = L.resource('icons/signal-none.png');
	else if (signalPercent == 0)
		icon = L.resource('icons/signal-0.png');
	else if (signalPercent < 25)
		icon = L.resource('icons/signal-0-25.png');
	else if (signalPercent < 50)
		icon = L.resource('icons/signal-25-50.png');
	else if (signalPercent < 75)
		icon = L.resource('icons/signal-50-75.png');
	else
		icon = L.resource('icons/signal-75-100.png');

	if (signalValue != null && signalValue != 0) {
		if (noiseValue != null && noiseValue != 0) {
			value = '%d/%d\xa0%s'.format(signalValue, noiseValue, _('dBm'));
			title = '%s: %d %s / %s: %d %s / %s %d'.format(
				_('Signal'), signalValue, _('dBm'),
				_('Noise'), noiseValue, _('dBm'),
				_('SNR'), signalValue - noiseValue);
		}
		else {
			value = '%d\xa0%s'.format(signalValue, _('dBm'));
			title = '%s: %d %s'.format(_('Signal'), signalValue, _('dBm'));
		}
	}
	else if (signalPercent > -1) {
		switch (mode) {
			case 'ap':
				title = _('No client associated');
				break;

			case 'sta':
			case 'adhoc':
			case 'mesh':
				title = _('Not associated');
				break;

			default:
				title = _('No RX signal');
		}

		if (noiseValue != null && noiseValue != 0) {
			value = '---/%d\x0a%s'.format(noiseValue, _('dBm'));
			title = '%s / %s: %d %s'.format(title, _('Noise'), noiseValue, _('dBm'));
		}
		else {
			value = '---\xa0%s'.format(_('dBm'));
		}
	}
	else {
		value = E('em', {}, E('small', {}, [ _('disabled') ]));
		title = _('Interface is disabled');
	}

	return E('div', {
		'class': wrap ? 'center' : 'ifacebadge',
		'title': title,
		'data-signal': signalValue,
		'data-noise': noiseValue
	}, [
		E('img', { 'src': icon }),
		E('span', {}, [
			wrap ? E('br') : ' ',
			value
		])
	]);
}

function render_network_badge(radioNet) {
	if (!Array.isArray(radioNet)) {
		return render_signal_badge(
			radioNet.isUp() ? radioNet.getSignalPercent() : -1,
			radioNet.getSignal(), radioNet.getNoise(), false, radioNet.getMode());
	} else {
		var badge_info = [], i;
		
		for (i = 0; i < radioNet.length; i++) {
			if (!radioNet[i])
				break;

			if (radioNet[i].getSignalPercent() != 0 || radioNet[i].getSignal() != 0) {
				badge_info.push(radioNet[i].getSignalPercent());
				badge_info.push(radioNet[i].getSignal());
				badge_info.push(radioNet[i].getNoise());
				badge_info.push(radioNet[i].getMode());
				break;
			}
		}
		
		return render_signal_badge(
			i > 0? radioNet[i-1].isUp() ? badge_info[0] : -1: radioNet[i].isUp() ? badge_info[0] : -1,
			badge_info[1], badge_info[2], false, badge_info[3]);
	}
}

function render_radio_status(radioDev, wifiNets) {
	var isArray = Array.isArray(radioDev),
		name = isArray? '': radioDev.getI18n().replace(/ Wireless Controller .+$/, ''),
	    node = E('div', [ E('big', {}, E('strong', {}, name)), E('div') ]),
	    channel, channel_r1, frequency, frequency_r1, bitrate, bitrate_r1;

	for (var i = 0; i < wifiNets.length; i++) {
		if (isArray){
			if (i > wifiNets.length/2 - 1) {
				channel_r1 = channel_r1 || wifiNets[i].getChannel();
				frequency_r1 = wifiNets[i].getFrequency();
				bitrate_r1 = bitrate_r1 || wifiNets[i].getBitRate();
			} else {
				channel   = channel   || wifiNets[i].getChannel();
				frequency = frequency || wifiNets[i].getFrequency();
				bitrate   = bitrate   || wifiNets[i].getBitRate();
			}
		} else {
			channel   = channel   || wifiNets[i].getChannel();
			frequency = frequency || wifiNets[i].getFrequency();
			bitrate   = bitrate   || wifiNets[i].getBitRate();
		}
	}

	if (!isArray) {
		if (radioDev.isUp())
			L.itemlist(node.lastElementChild, [
				_('Channel'), '%s (%s %s)'.format(channel || '?', frequency || '?', _('GHz')),
				_('Bitrate'), '%s %s'.format(bitrate || '?', _('Mbit/s'))
			], ' | ');
		else
			node.lastElementChild.appendChild(E('em', _('Device is not active')));
	}else{
		if (isArray? radioDev[0].isUp(): radioDev.isUp())
			L.itemlist(node.lastElementChild, [
				_('Radio0 Channel'), '%s (%s %s)'.format(channel || '?', frequency || '?', _('GHz')),
				_('Bitrate'), '%s %s'.format(bitrate || '?', _('Mbit/s')),
				_('Radio1 Channel'), '%s (%s %s)'.format(channel_r1 || '?', frequency_r1 || '?', _('GHz')),
				_('Bitrate'), '%s %s'.format(bitrate_r1 || '?', _('Mbit/s')),
			], [' | ', E('br')]);
		else
			node.lastElementChild.appendChild(E('em', _('Device is not active')))
	}

	return node;
}

function render_network_status(radioNet) {
	var isArray = Array.isArray(radioNet),
		mode = isArray? radioNet[0].getActiveMode(): radioNet.getActiveMode(),
	    bssid = isArray? radioNet[0].getActiveBSSID(): radioNet.getActiveBSSID(),
	    channel = isArray? radioNet[0].getChannel(): radioNet.getChannel(),
	    disabled = isArray? (radioNet[0].get('disabled') == '1' || uci.get('wireless', radioNet[0].getWifiDeviceName(), 'disabled') == '1'): (radioNet.get('disabled') == '1' || uci.get('wireless', radioNet.getWifiDeviceName(), 'disabled') == '1'),    
	    is_assoc = (bssid && bssid != '00:00:00:00:00:00' && channel && mode != 'Unknown' && !disabled),
	    is_mesh = isArray? (radioNet[0].getMode() == 'mesh'): (radioNet.getMode() == 'mesh'),
	    changecount = isArray? count_changes(radioNet[0].getName()): count_changes(radioNet.getName()),
	    status_text = null;

	if (changecount)
		status_text = E('a', {
			href: '#',
			click: L.bind(ui.changes.displayChanges, ui.changes)
		}, _('Interface has %d pending changes').format(changecount));
	else if (!is_assoc)
		status_text = E('em', disabled ? _('Wireless is disabled') : _('Wireless is not associated'));

	if (!isArray) {
		return L.itemlist(E('div'), [
			is_mesh ? _('Mesh ID') : _('SSID'), (is_mesh ? radioNet.getMeshID() : radioNet.getSSID()) || '?',
			_('Mode'),       radioNet.getActiveMode(),
			_('BSSID'),      (!changecount && is_assoc) ? bssid : null,
			_('Encryption'), (!changecount && is_assoc) ? radioNet.getActiveEncryption() || _('None') : null,
			null,            status_text
		], [ ' | ', E('br') ]);
	}else {
		return L.itemlist(E('div'), [
			is_mesh ? _('Mesh ID') : _('SSID'), (is_mesh ? radioNet[0].getMeshID() : radioNet[0].getSSID()) || '?',
			_('Encryption'), (!changecount && is_assoc) ? radioNet[0].getActiveEncryption() || _('None') : null,
			null,            status_text
		], [ ' | ', E('br') ]);
	}
}

function render_modal_status(node, radioNet) {
	var mode = radioNet.getActiveMode(),
	    noise = radioNet.getNoise(),
	    bssid = radioNet.getActiveBSSID(),
	    channel = radioNet.getChannel(),
	    disabled = (radioNet.get('disabled') == '1'),
	    is_assoc = (bssid && bssid != '00:00:00:00:00:00' && channel && mode != 'Unknown' && !disabled);

	if (node == null)
		node = E('span', { 'class': 'ifacebadge large', 'data-network': radioNet.getName() }, [ E('small'), E('span') ]);

	dom.content(node.firstElementChild, render_signal_badge(
		disabled ? -1 : radioNet.getSignalPercent(),
		radioNet.getSignal(), noise, true, radioNet.getMode()));

	L.itemlist(node.lastElementChild, [
		_('Mode'),       mode,
		_('SSID'),       radioNet.getSSID() || '?',
		_('BSSID'),      is_assoc ? bssid : null,
		_('Encryption'), is_assoc ? radioNet.getActiveEncryption() || _('None') : null,
		_('Channel'),    is_assoc ? '%d (%.3f %s)'.format(radioNet.getChannel(), radioNet.getFrequency() || 0, _('GHz')) : null,
		_('Tx-Power'),   is_assoc ? '%d %s'.format(radioNet.getTXPower(), _('dBm')) : null,
		_('Signal'),     is_assoc ? '%d %s'.format(radioNet.getSignal(), _('dBm')) : null,
		_('Noise'),      (is_assoc && noise != null) ? '%d %s'.format(noise, _('dBm')) : null,
		_('Bitrate'),    is_assoc ? '%.1f %s'.format(radioNet.getBitRate() || 0, _('Mbit/s')) : null,
		_('Country'),    is_assoc ? radioNet.getCountryCode() : null
	], [ ' | ', E('br'), E('br'), E('br'), E('br'), E('br'), ' | ', E('br'), ' | ' ]);

	if (!is_assoc)
		dom.append(node.lastElementChild, E('em', disabled ? _('Wireless is disabled') : _('Wireless is not associated')));

	return node;
}

function format_wifirate(rate) {
	var s = '%.1f\xa0%s, %d\xa0%s'.format(rate.rate / 1000, _('Mbit/s'), rate.mhz, _('MHz')),
	    ht = rate.ht, vht = rate.vht,
	    mhz = rate.mhz, nss = rate.nss,
	    mcs = rate.mcs, sgi = rate.short_gi,
	    he = rate.he, he_gi = rate.he_gi,
	    he_dcm = rate.he_dcm;

	if (ht || vht) {
		if (vht) s += ', VHT-MCS\xa0%d'.format(mcs);
		if (nss) s += ', VHT-NSS\xa0%d'.format(nss);
		if (ht)  s += ', MCS\xa0%s'.format(mcs);
		if (sgi) s += ', ' + _('Short GI').replace(/ /g, '\xa0');
	}

	if (he) {
		s += ', HE-MCS\xa0%d'.format(mcs);
		if (nss) s += ', HE-NSS\xa0%d'.format(nss);
		if (he_gi) s += ', HE-GI\xa0%d'.format(he_gi);
		if (he_dcm) s += ', HE-DCM\xa0%d'.format(he_dcm);
	}

	return s;
}

function radio_restart(id, ev) {
	var row = document.querySelector('.cbi-section-table-row[data-sid="%s"]'.format(id)),
	    dsc = row.querySelector('[data-name="_stat"] > div'),
	    btn = row.querySelector('.cbi-section-actions button');

	btn.blur();
	btn.classList.add('spinning');
	btn.disabled = true;

	dsc.setAttribute('restart', '');
	dom.content(dsc, E('em', _('Device is restarting…')));
}

function network_updown(id, map, ev) {
	var radio = uci.get('wireless', id, 'device'),
	    disabled = (uci.get('wireless', id, 'disabled') == '1') ||
	               (uci.get('wireless', radio, 'disabled') == '1');

	if (disabled) {
		uci.unset('wireless', id, 'disabled');
		uci.unset('wireless', radio, 'disabled');
	}
	else {
		uci.set('wireless', id, 'disabled', '1');

		var all_networks_disabled = true,
		    wifi_ifaces = uci.sections('wireless', 'wifi-iface');

		for (var i = 0; i < wifi_ifaces.length; i++) {
			if (wifi_ifaces[i].device == radio && wifi_ifaces[i].disabled != '1') {
				all_networks_disabled = false;
				break;
			}
		}

		if (all_networks_disabled)
			uci.set('wireless', radio, 'disabled', '1');
	}

	return map.save().then(function() {
		ui.changes.apply()
	});
}

function next_free_sid(offset) {
	var sid = 'wifinet' + String(offset).padStart(2, '0');

	while (uci.get('wireless', sid))
		sid = 'wifinet' + String(++offset).padStart(2, '0');

	return sid;
}

function add_dependency_permutations(o, deps) {
	var res = null;

	for (var key in deps) {
		if (!deps.hasOwnProperty(key) || !Array.isArray(deps[key]))
			continue;

		var list = deps[key],
		    tmp = [];

		for (var j = 0; j < list.length; j++) {
			for (var k = 0; k < (res ? res.length : 1); k++) {
				var item = (res ? Object.assign({}, res[k]) : {});
				item[key] = list[j];
				tmp.push(item);
			}
		}

		res = tmp;
	}

	for (var i = 0; i < (res ? res.length : 0); i++)
		o.depends(res[i]);
}

function add_dependency(o, deps) {
	const result = [];

	Object.entries(deps).forEach(([key1, values1]) => {
		values1.forEach(value1 => {
			Object.entries(deps).forEach(([key2, values2]) => {
				if (key2 !== key1) {
					values2.forEach(value2 => {
						if (!result.some(obj => obj[key1] === value1 && obj[key2] === value2)) {
							const obj = {
								[key1]: value1,
								[key2]: value2
							};

							result.push(obj);
						}
					});
				}
			});
		});
	});

	for (var i = 0; i < result.length; i++)
		o.depends(result[i]);
}

function add_encryption_dependency(o, deps) {
	var result = [];

	deps.encryption.forEach((element) => {
		result.push({"encryption":element});
	});

	for(let i = 0; i < result.length; i++)
		o.depends(result[i]);
}

var CBIWifiSetVlanPolicy = form.Value.extend({
	renderWidget: function(section_id) {
		var elem = E('div', { style: 'margin: 10px;' });
		var brvlan = L.toArray(uci.get('wireless', section_id, 'brvlan'));
		var cfgvalue;

		var style = `
			table {
				width: 100%;
				border-collapse: collapse;
			}
			th, td {
				text-align: left;
			}
		`;

		var table = E('table', {
			id: 'vlan-policy-table',
			class: 'custom-vlan-table'
		}, [
			E('thead', [
				E('tr', [
					E('th', _('VLAN ID')),
					E('th', _('Primary')),
					E('th', _('Options'))
				])
			]),
			E('tbody')
		]);

		dom.content(elem, [
			E('style', {}, style),
			table
		]);

		var tbody = table.querySelector('tbody');
		var field = this;

		uci.sections('network', 'bridge-vlan')?.forEach((subObj) => {
			if (brvlan.length == 0)
				cfgvalue = '-';
			else {
				for(let i = 0; i < brvlan.length; i++) {
					if (brvlan[i].split(':')[0] == subObj.vlan) {
						switch(brvlan[i].split(':')[1]) {
						case 'u':
						case 't':
							cfgvalue = brvlan[i].split(':')[1];
							break;
						case 'u*':
						case 't*':
							cfgvalue = [brvlan[i].split(':')[1][0], '*'];
							break;
						}
						break;
					} else
						cfgvalue = '-';
				}
			}

			var widget = new ui.Dropdown(cfgvalue, {
				'-': E([], [
					E('span', { 'class': 'hide-open', 'style': 'font-family:monospace' }, [ '-' ]),
					E('span', { 'class': 'hide-close' }, [ _('Do not participate', 'VLAN port state') ])
				]),
				'u': E([], [
					E('span', { 'class': 'hide-open', 'style': 'font-family:monospace' }, [ 'u' ]),
					E('span', { 'class': 'hide-close' }, [ _('Egress untagged', 'VLAN port state') ])
				]),
				't': E([], [
					E('span', { 'class': 'hide-open', 'style': 'font-family:monospace' }, [ 't' ]),
					E('span', { 'class': 'hide-close' }, [ _('Egress tagged', 'VLAN port state') ])
				]),
				'*': E([], [
					E('span', { 'class': 'hide-open', 'style': 'font-family:monospace' }, [ '*' ]),
					E('span', { 'class': 'hide-close' }, [ _('Primary VLAN ID', 'VLAN port state') ])
				])
			}, {
				id: `options-select-${subObj.vlan}`,
				sort: ['-', 'u', 't', '*'],
				optional: false,
				multiple: true
			});

			widget.toggleItem = function(sb, li, force_state) {
				var lis = li.parentNode.querySelectorAll('li'),
				    toggle = ui.Dropdown.prototype.toggleItem;

				toggle.apply(this, [sb, li, force_state]);

				if (force_state != null)
					return;

				var dataValue = li.getAttribute('data-value');

				switch (li.getAttribute('data-value'))
				{
				case '-':
					if (li.hasAttribute('selected')) {
						for (var i = 0; i < lis.length; i++) {
							switch (lis[i].getAttribute('data-value'))
							{
							case '-':
								break;

							case '*':
								toggle.apply(this, [sb, lis[i], false]);
								lis[i].setAttribute('unselectable', '');
								break;

							default:
								toggle.apply(this, [sb, lis[i], false]);
							}
						}
					}
					break;

				case 't':
				case 'u':
					if (li.hasAttribute('selected')) {
						for (var i = 0; i < lis.length; i++) {
							switch (lis[i].getAttribute('data-value'))
							{
							case li.getAttribute('data-value'):
								break;

							case '*':
							lis[i].removeAttribute('unselectable');
							break;

							default:
							toggle.apply(this, [sb, lis[i], false]);
							}
						}
					} else
						toggle.apply(this, [sb, li, true]);
					break;
				case '*':
					if (li.hasAttribute('selected')) {
						var table = document.querySelector('#vlan-policy-table'),
						    rows = table.querySelectorAll('tr');

						for (var i = 1; i < rows.length; i++) {
							var vid = rows[i].cells[0].textContent.trim(),
							    dropdown = rows[i].cells[2].querySelector('ul');

							if (dropdown && dropdown !== sb) {
								var selectedItems = Array.from(dropdown.querySelectorAll('li[selected]'));
								var values = selectedItems.map(function(item) {
									return item.getAttribute('data-value');
								});

								if (values.includes('*')) {
									if (vid != subObj.vlan) {
										for (let j = 1; j < selectedItems.length; j++) {
											let newvalue = rows[i].cells[2].querySelectorAll('div')[0].value[0];

											dropdown.querySelectorAll('li')[3].removeAttribute('selected');
											dropdown.querySelectorAll('li')[3].removeAttribute('display');
											rows[i].cells[2].querySelectorAll('div')[0].value = newvalue;
										}
									}
								}
							}
						}
					}
					break;
				}
			};

			var node = widget.render();

			if (cfgvalue == '-')
				node.querySelector('li[data-value="*"]').setAttribute('unselectable', '');

			var row = E('tr', [
				E('td', subObj.vlan),
				E('td', subObj.primary? '1': '0'),
				E('td', [ node ])
			]);

			tbody.appendChild(row);
		});

		return elem;
	},

	formvalue: function(section_id) {
		var values = [], tmp = [];
		var brvlans = uci.get('wireless', section_id, 'brvlan');
		var container = this.map.root;
		var table = container.querySelector('#vlan-policy-table');
		var rows = table.querySelectorAll('tr');

		for (let i = 1; i < rows.length; i++) {
			for (let j = 0; j < rows[i].cells.length; j++) {
				if (j == 2) {
					var vid = rows[i].cells[0].textContent,
					    policy = rows[i].cells[j].firstChild.value.replace(/\s+/g, '');
					switch(policy) {
					case '-':
						break;
					case 't':
					case 'u':
					case 't*':
					case 'u*':
						var str = `${vid}:${policy}`;
						values.push(str);
						break;
					}
				}
			}
		}

		if (values.length == 0)
			uci.unset('wireless', section_id, 'brvlan');
		else
			uci.set('wireless', section_id, 'brvlan', values);
	}
});

var CBIWifiFrequencyValue = form.Value.extend({
	callFrequencyList: rpc.declare({
		object: 'iwinfo',
		method: 'freqlist',
		params: [ 'device' ],
		expect: { results: [] }
	}),

	load: function(section_id) {
		return Promise.all([
			network.getWifiDevice(section_id),
			this.callFrequencyList(section_id)
		]).then(L.bind(function(data) {
			this.channels = {
				'2g': L.hasSystemFeature('hostapd', 'acs') ? [ 'auto', 'auto', true ] : [],
				'5g': L.hasSystemFeature('hostapd', 'acs') ? [ 'auto', 'auto', true ] : [],
				'6g': [],
				'60g': []
			};

			for (var i = 0; i < data[1].length; i++) {
				var band;

				if (data[1][i].mhz >= 2412 && data[1][i].mhz <= 2484)
					band = '2g';
				else if (data[1][i].mhz >= 5160 && data[1][i].mhz <= 5885)
					band = '5g';
				else if (data[1][i].mhz >= 5925 && data[1][i].mhz <= 7125)
					band = '6g';
				else if (data[1][i].mhz >= 58320 && data[1][i].mhz <= 69120)
					band = '60g';
				else
					continue;

				this.channels[band].push(
					data[1][i].channel,
					'%d (%d Mhz)'.format(data[1][i].channel, data[1][i].mhz),
					!data[1][i].restricted
				);
			}

			var hwmodelist = L.toArray(data[0] ? data[0].getHWModes() : null)
				.reduce(function(o, v) { o[v] = true; return o }, {});

			this.modes = [
				'', 'Legacy', true,
				'n', 'N', hwmodelist.n,
				'ac', 'AC', hwmodelist.ac,
				'ax', 'AX', hwmodelist.ax
			];

			var htmodelist = L.toArray(data[0] ? data[0].getHTModes() : null)
				.reduce(function(o, v) { o[v] = true; return o }, {});

			this.htmodes = {
				'': [ '', '-', true ],
				'n': [
					'HT20', '20 MHz', htmodelist.HT20,
					'HT40', '40 MHz', htmodelist.HT40
				],
				'ac': [
					'VHT20', '20 MHz', htmodelist.VHT20,
					'VHT40', '40 MHz', htmodelist.VHT40,
					'VHT80', '80 MHz', htmodelist.VHT80,
					'VHT160', '160 MHz', htmodelist.VHT160
				],
				'ax': [
					'HE20', '20 MHz', htmodelist.HE20,
					'HE40', '40 MHz', htmodelist.HE40,
					'HE80', '80 MHz', htmodelist.HE80,
					'HE160', '160 MHz', htmodelist.HE160
				]
			};

			this.bands = {
				'': [
					'2g', '2.4 GHz', this.channels['2g'].length > 3,
					'5g', '5 GHz', this.channels['5g'].length > 3,
					'60g', '60 GHz', this.channels['60g'].length > 0
				],
				'n': [
					'2g', '2.4 GHz', this.channels['2g'].length > 3,
					'5g', '5 GHz', this.channels['5g'].length > 3
				],
				'ac': [
					'5g', '5 GHz', true
				],
				'ax': [
					'2g', '2.4 GHz', this.channels['2g'].length > 3,
					'5g', '5 GHz', this.channels['5g'].length > 3
				]
			};
		}, this));
	},

	setValues: function(sel, vals) {
		if (sel.vals)
			sel.vals.selected = sel.selectedIndex;

		while (sel.options[0])
			sel.remove(0);

		for (var i = 0; vals && i < vals.length; i += 3)
			if (vals[i+2])
				sel.add(E('option', { value: vals[i+0] }, [ vals[i+1] ]));

		if (vals && !isNaN(vals.selected))
			sel.selectedIndex = vals.selected;

		sel.parentNode.style.display = (sel.options.length <= 1) ? 'none' : '';
		sel.vals = vals;
	},

	toggleWifiMode: function(elem) {
		this.toggleWifiHTMode(elem);
		this.toggleWifiBand(elem);
	},

	toggleWifiHTMode: function(elem) {
		var mode = elem.querySelector('.mode');
		var bwdt = elem.querySelector('.htmode');

		this.setValues(bwdt, this.htmodes[mode.value]);
	},

	toggleWifiBand: function(elem) {
		var mode = elem.querySelector('.mode');
		var band = elem.querySelector('.band');

		this.setValues(band, this.bands[mode.value]);
		this.toggleWifiChannel(elem);

		this.map.checkDepends();
	},

	toggleWifiChannel: function(elem) {
		var band = elem.querySelector('.band');
		var chan = elem.querySelector('.channel');

		this.setValues(chan, this.channels[band.value]);
	},

	setInitialValues: function(section_id, elem) {
		var mode = elem.querySelector('.mode'),
		    band = elem.querySelector('.band'),
		    chan = elem.querySelector('.channel'),
		    bwdt = elem.querySelector('.htmode'),
		    htval = uci.get('wireless', section_id, 'htmode'),
		    hwval = uci.get('wireless', section_id, 'hwmode'),
		    chval = uci.get('wireless', section_id, 'channel'),
		    bandval = uci.get('wireless', section_id, 'band');

		this.setValues(mode, this.modes);

		if (/HE20|HE40|HE80|HE160/.test(htval))
			mode.value = 'ax';
		else if (/VHT20|VHT40|VHT80|VHT160/.test(htval))
			mode.value = 'ac';
		else if (/HT20|HT40/.test(htval))
			mode.value = 'n';
		else
			mode.value = '';

		this.toggleWifiMode(elem);

		if (hwval != null) {
			this.useBandOption = false;

			if (/a/.test(hwval))
				band.value = '5g';
			else
				band.value = '2g';
		}
		else {
			this.useBandOption = true;

			band.value = bandval;
		}

		this.toggleWifiBand(elem);

		bwdt.value = htval;
		chan.value = chval || chan.options[0].value;

		return elem;
	},

	renderWidget: function(section_id, option_index, cfgvalue) {
		var elem = E('div');

		dom.content(elem, [
			E('label', { 'style': 'float:left; margin-right:3px; line-height: 20px;' }, [
				_('Mode'), E('br'),
				E('select', {
					'class': 'mode',
					'style': 'width:auto',
					'change': L.bind(this.toggleWifiMode, this, elem),
					'disabled': (this.disabled != null) ? this.disabled : this.map.readonly
				})
			]),
			E('label', { 'style': 'float:left; margin-right:3px; line-height: 20px;' }, [
				_('Band'), E('br'),
				E('select', {
					'class': 'band',
					'style': 'width:auto',
					'change': L.bind(this.toggleWifiBand, this, elem),
					'disabled': (this.disabled != null) ? this.disabled : this.map.readonly
				})
			]),
			E('label', { 'style': 'float:left; margin-right:3px; line-height: 20px;' }, [
				_('Channel'), E('br'),
				E('select', {
					'class': 'channel',
					'style': 'width:auto',
					'change': L.bind(this.map.checkDepends, this.map),
					'disabled': (this.disabled != null) ? this.disabled : this.map.readonly
				})
			]),
			E('label', { 'style': 'float:left; margin-right:3px; line-height: 20px;' }, [
				_('Width'), E('br'),
				E('select', {
					'class': 'htmode',
					'style': 'width:auto',
					'change': L.bind(this.map.checkDepends, this.map),
					'disabled': (this.disabled != null) ? this.disabled : this.map.readonly
				})
			]),
			E('br', { 'style': 'clear:left' })
		]);

		return this.setInitialValues(section_id, elem);
	},

	cfgvalue: function(section_id) {
		return [
		    uci.get('wireless', section_id, 'htmode'),
		    uci.get('wireless', section_id, 'hwmode') || uci.get('wireless', section_id, 'band'),
		    uci.get('wireless', section_id, 'channel')
		];
	},

	formvalue: function(section_id) {
		var node = this.map.findElement('data-field', this.cbid(section_id));

		return [
		    node.querySelector('.htmode').value,
		    node.querySelector('.band').value,
 			node.querySelector('.channel').value
		];
	},

	write: function(section_id, value) {
		uci.set('wireless', section_id, 'htmode', value[0] || null);

		if (this.useBandOption)
			uci.set('wireless', section_id, 'band', value[1]);
		else
			uci.set('wireless', section_id, 'hwmode', (value[1] == '2g') ? '11g' : '11a');

		uci.set('wireless', section_id, 'channel', value[2]);
	}
});

var CBIWifiTxPowerValue = form.ListValue.extend({
	callTxPowerList: rpc.declare({
		object: 'iwinfo',
		method: 'txpowerlist',
		params: [ 'device' ],
		expect: { results: [] }
	}),

	load: function(section_id) {
		return this.callTxPowerList(section_id).then(L.bind(function(pwrlist) {
			this.powerval = this.wifiNetwork ? this.wifiNetwork.getTXPower() : null;
			this.poweroff = this.wifiNetwork ? this.wifiNetwork.getTXPowerOffset() : null;

			this.value('', _('driver default'));

			for (var i = 0; i < pwrlist.length; i++)
				this.value(pwrlist[i].dbm, '%d dBm (%d mW)'.format(pwrlist[i].dbm, pwrlist[i].mw));

			return form.ListValue.prototype.load.apply(this, [section_id]);
		}, this));
	},

	renderWidget: function(section_id, option_index, cfgvalue) {
		var widget = form.ListValue.prototype.renderWidget.apply(this, [section_id, option_index, cfgvalue]);
		    widget.firstElementChild.style.width = 'auto';

		dom.append(widget, E('span', [
			' - ', _('Current power'), ': ',
			E('span', [ this.powerval != null ? '%d dBm'.format(this.powerval) : E('em', _('unknown')) ]),
			this.poweroff ? ' + %d dB offset = %s dBm'.format(this.poweroff, this.powerval != null ? this.powerval + this.poweroff : '?') : ''
		]));

		return widget;
	}
});

var CBIWifiCountryValue = form.Value.extend({
	callCountryList: rpc.declare({
		object: 'iwinfo',
		method: 'countrylist',
		params: [ 'device' ],
		expect: { results: [] }
	}),

	load: function(section_id) {
		return this.callCountryList(section_id).then(L.bind(function(countrylist) {
			if (Array.isArray(countrylist) && countrylist.length > 0) {
				this.value('', _('driver default'));

				for (var i = 0; i < countrylist.length; i++)
					this.value(countrylist[i].iso3166, '%s - %s'.format(countrylist[i].iso3166, countrylist[i].country));
			}

			return form.Value.prototype.load.apply(this, [section_id]);
		}, this));
	},

	validate: function(section_id, formvalue) {
		if (formvalue != null && formvalue != '' && !/^[A-Z0-9][A-Z0-9]$/.test(formvalue))
			return _('Use ISO/IEC 3166 alpha2 country codes.');

		return true;
	},

	renderWidget: function(section_id, option_index, cfgvalue) {
		var typeClass = (this.keylist && this.keylist.length) ? form.ListValue : form.Value;
		return typeClass.prototype.renderWidget.apply(this, [section_id, option_index, cfgvalue]);
	}
});

return view.extend({
	poll_status: function(map, data) {
		var rows = map.querySelectorAll('.cbi-section-table-row[data-sid]');

		for (var i = 0; i < rows.length; i++) {
			var section_id = rows[i].getAttribute('data-sid'),
			    radioDev = data[1].filter(function(d) { return d.getName() == section_id })[0],
			    radioNet = data[2].filter(function(n) { return n.getName() == section_id })[0],
			    badge = rows[i].querySelector('[data-name="_badge"] > div'),
			    stat = rows[i].querySelector('[data-name="_stat"]'),
			    btns = rows[i].querySelectorAll('.cbi-section-actions button'),
			    busy = btns[0].classList.contains('spinning') || btns[1].classList.contains('spinning') || btns[2].classList.contains('spinning');

			if (radioDev) {
				if (g_dualband) {
					var mapping_sid = radioDev.getName().replace('0', '1'),
						r1_radioDev = data[2].filter(function(n) { return n.getName() == mapping_sid })[0];

					radioDev = [radioDev, r1_radioDev];
				}
				dom.content(badge, render_radio_badge(radioDev));
				dom.content(stat, render_radio_status(radioDev, g_dualband? data[2]: data[2].filter(function(n) { return n.getWifiDeviceName() == radioDev.getName() })));
			}
			else {
				if (g_dualband) {
					var mapping_sid = radioNet.getName().charAt(0) == 'd'? radioNet.getName().replace('0', '1'): radio0_mapping_radio1(radioNet.getName()),
						r1_radioNet = data[2].filter(function(n) { return n.getName() == mapping_sid })[0];

					radioNet = [radioNet, r1_radioNet];
				}
				dom.content(badge, render_network_badge(radioNet));
				dom.content(stat, render_network_status(radioNet));
			}

			if (stat.hasAttribute('restart'))
				dom.content(stat, E('em', _('Device is restarting…')));

			btns[0].disabled = isReadonlyView || busy;
			btns[1].disabled = (isReadonlyView && radioDev) || busy;
			btns[2].disabled = isReadonlyView || busy;
		}

		var table = document.querySelector('#wifi_assoclist_table'),
		    hosts = data[0],
		    trows = [];

		for (var i = 0; i < data[3].length; i++) {
			var bss = data[3][i],
			    name = hosts.getHostnameByMACAddr(bss.mac),
			    ipv4 = hosts.getIPAddrByMACAddr(bss.mac),
			    ipv6 = hosts.getIP6AddrByMACAddr(bss.mac);

			var hint;

			if (name && ipv4 && ipv6)
				hint = '%s <span class="hide-xs">(%s, %s)</span>'.format(name, ipv4, ipv6);
			else if (name && (ipv4 || ipv6))
				hint = '%s <span class="hide-xs">(%s)</span>'.format(name, ipv4 || ipv6);
			else
				hint = name || ipv4 || ipv6 || '?';

			var row = [
				E('span', {
					'class': 'ifacebadge',
					'data-ifname': bss.network.getIfname(),
					'data-ssid': bss.network.getSSID()
				}, [
					E('img', {
						'src': L.resource('icons/wifi%s.png').format(bss.network.isUp() ? '' : '_disabled'),
						'title': bss.radio.getI18n()
					}),
					E('span', [
						' %s '.format(bss.network.getShortName()),
						E('small', '(%s)'.format(bss.network.getIfname()))
					])
				]),
				bss.mac,
				hint,
				render_signal_badge(Math.min((bss.signal + 110) / 70 * 100, 100), bss.signal, bss.noise),
				E('span', {}, [
					E('span', format_wifirate(bss.rx)),
					E('br'),
					E('span', format_wifirate(bss.tx))
				])
			];

			if (bss.network.isClientDisconnectSupported()) {
				if (table.firstElementChild.childNodes.length < 6)
					table.firstElementChild.appendChild(E('th', { 'class': 'th cbi-section-actions'}));

				row.push(E('button', {
					'class': 'cbi-button cbi-button-remove',
					'click': L.bind(function(net, mac, ev) {
						dom.parent(ev.currentTarget, '.tr').style.opacity = 0.5;
						ev.currentTarget.classList.add('spinning');
						ev.currentTarget.disabled = true;
						ev.currentTarget.blur();

						net.disconnectClient(mac, true, 5, 60000);
					}, this, bss.network, bss.mac),
					'disabled': isReadonlyView || null
				}, [ _('Disconnect') ]));
			}
			else {
				row.push('-');
			}

			trows.push(row);
		}

		cbi_update_table(table, trows, E('em', _('No information available')));

		var stat = document.querySelector('.cbi-modal [data-name="_wifistat_modal"] .ifacebadge.large');

		if (stat)
			render_modal_status(stat, data[2].filter(function(n) { return n.getName() == stat.getAttribute('data-network') })[0]);

		return network.flushCache();
	},

	load: function() {
		return Promise.all([
			uci.changes(),
			uci.load('wireless'),
			uci.load('cls-opmode')
		]);
	},

	checkAnonymousSections: function() {
		var wifiIfaces = uci.sections('wireless', 'wifi-iface');

		for (var i = 0; i < wifiIfaces.length; i++)
			if (wifiIfaces[i]['.anonymous'])
				return true;

		return false;
	},

	callUciRename: rpc.declare({
		object: 'uci',
		method: 'rename',
		params: [ 'config', 'section', 'name' ]
	}),

	handleDualbandIntegration: function(ev) {
		ev.preventDefault();
		ev.stopImmediatePropagation();

		let pending = ui.changes.changes
			&& ui.changes.changes.wireless
			&& ui.changes.changes.wireless.length > 0;

		if (pending) {
			ui.changes.displayStatus('warning', [
				E('p', _('You have unsaved wireless configuration changes. Discard and continue?')),
				E('div', { 'class': 'button-row' }, [
					E('button', {
						'class': 'btn',
						'click': L.bind(ui.changes.displayStatus, ui.changes, false)
					}, [ _('Cancel') ]),
					' ',
					E('button', {
						'class': 'btn cbi-button cbi-button-positive important',
						'click': L.bind(function() {
							ui.changes.clear();
							ui.changes.displayStatus(false);
							this._doDualbandIntegration();
						}, this)
					}, [ _('Confirm') ])
				])
			]);

			return;
		}

		this._doDualbandIntegration();
	},

	_doDualbandIntegration: function() {
		var wiface = uci.sections('wireless', 'wifi-iface');
		let values = { 'dualband': g_dualband ? '' : '1' };
		var src_ifaces = [], dst_ifaces = [];

		for (let i = 0; i < wiface.length; i++) {
			if (wiface[i].mode == 'sta' || (wiface[i].hasOwnProperty('multi_ap') && wiface[i].multi_ap == '1'))
				continue;

			if (wiface[i].device == 'radio0')
				src_ifaces.push(wiface[i]);
			else
				dst_ifaces.push(wiface[i]);
		}

		if (g_dualband) {
			add_5g_suffix(dst_ifaces);
			set_clsmesh_bandsteer(false);
			set_bt_nbr_for_each_iface(src_ifaces, dst_ifaces, false);
		} else {
			sync_2g_configuration(src_ifaces, dst_ifaces);
			set_clsmesh_bandsteer(true);
			set_bt_nbr_for_each_iface(src_ifaces, dst_ifaces, true);
		}

		callUciSet('wireless', 'globals', values)
		.then(function() {
			callUciCommit('wireless');
			callUciCommit('cls-mesh');
		})
		.then(function() {
			ui.changes.displayStatus('notice spinning',
				E('p', _('Starting Dualband Integration...')));

			window.setTimeout(function() {
				ui.changes.displayStatus(false);
				window.location = window.location.href.split('#')[0];
			}, L.env.apply_display * 4000);
		});
	},

	render: function() {
		return Promise.all([
			this.checkAnonymousSections()?this.renderMigration():this.renderOverview()
		]);
	},

	handleMigration: function(ev) {
		var wifiIfaces = uci.sections('wireless', 'wifi-iface'),
		    id_offset = 0,
		    tasks = [];

		for (var i = 0; i < wifiIfaces.length; i++) {
			if (!wifiIfaces[i]['.anonymous'])
				continue;

			var new_name = next_free_sid(id_offset);

			tasks.push(this.callUciRename('wireless', wifiIfaces[i]['.name'], new_name));
			id_offset = +new_name.substring(7) + 1;
		}

		return Promise.all(tasks)
			.then(L.bind(ui.changes.init, ui.changes))
			.then(L.bind(ui.changes.apply, ui.changes));
	},

	renderMigration: function() {
		ui.showModal(_('Wireless configuration migration'), [
			E('p', _('The existing wireless configuration needs to be changed for LuCI to function properly.')),
			E('p', _('Upon pressing "Continue", anonymous "wifi-iface" sections will be assigned with a name in the form <em>wifinet#</em> and the network will be restarted to apply the updated configuration.')),
			E('div', { 'class': 'right' },
				E('button', {
					'class': 'btn cbi-button-action important',
					'click': ui.createHandlerFn(this, 'handleMigration')
				}, _('Continue')))
		]);
	},

	renderOverview: function() {
		var m, s, o, opmode;

		m = new form.Map('wireless');
		m.chain('network');
		m.chain('firewall');
		m.chain('cls-mesh');

		g_dualband = uci.get('wireless', 'globals', 'dualband');
		opmode = uci.get('cls-opmode', 'globals', 'mode');

		s = m.section(form.NamedSection, 'globals', 'globals', _('Wireless Feature'));
		s.anonymous = false;
		s.addremove = false;

		o = s.option(form.Flag, 'highpwr', _('High Power Mode'));

		o = s.option(form.Button, 'dualband', _('Enable Wi-Fi Dualband integration'), _('2.4G and 5G signals are displayed together, and the faster 5G is preferred for the same signal strength. Turn off this switch to set it separately.'));
		o.inputstyle = 'action important';
		if (g_dualband)
			o.inputtitle = _('Enabled');
		else
			o.inputtitle = _('Disabled');
		o.onclick = L.bind(this.handleDualbandIntegration, this);

		s = m.section(form.GridSection, 'wifi-device', _('Wireless Overview'));
		s.anonymous = true;
		s.addremove = false;

		s.load = function() {
			return network.getWifiDevices().then(L.bind(function(radios) {
				this.radios = radios.sort(function(a, b) {
					return a.getName() > b.getName();
				});

				var tasks = [];

				for (var i = 0; i < radios.length; i++)
					tasks.push(radios[i].getWifiNetworks());

				return Promise.all(tasks);
			}, this)).then(L.bind(function(data) {
				this.wifis = [];
				if (opmode == 'router' || opmode == 'bridge') {
					for (var i = 0; i < data.length; i++) {
						const idx = data[i].findIndex(item => item.sid == 'station_iface' && !item['_ubusdata'].dev);
						if (idx != -1)
							data[i].splice(idx, 1);
					}
				}

				for (var i = 0; i < data.length; i++)
					this.wifis.push.apply(this.wifis, data[i]);
			}, this));
		};

		s.cfgsections = function() {
			var rv = [];

			if (g_dualband) {
			 	for (var i = 0; i < 1; i++) {
					rv.push(this.radios[i].getName());

					for (var j = 0; j < this.wifis.length; j++)
						if (this.wifis[j].getWifiDeviceName() == this.radios[i].getName())
							rv.push(this.wifis[j].getName());
				}
			}else{
				for (var i = 0; i < this.radios.length; i++) {
					rv.push(this.radios[i].getName());

					for (var j = 0; j < this.wifis.length; j++)
						if (this.wifis[j].getWifiDeviceName() == this.radios[i].getName())
							rv.push(this.wifis[j].getName());
				}
			}

			return rv;
		};

		s.modaltitle = function(section_id) {
			var radioNet = this.wifis.filter(function(w) { return w.getName() == section_id})[0];
			return radioNet ? radioNet.getI18n() : _('Edit wireless network');
		};

		s.lookupRadioOrNetwork = function(section_id) {
			var radioDev = this.radios.filter(function(r) { return r.getName() == section_id })[0];
			if (radioDev)
				return radioDev;

			var radioNet = this.wifis.filter(function(w) { return w.getName() == section_id })[0];
			if (radioNet)
				return radioNet;

			return null;
		};

		s.renderRowActions = function(section_id) {
			var inst = this.lookupRadioOrNetwork(section_id), btns;

			if (inst.getWifiNetworks) {
				btns = [
					E('button', {
						'class': 'cbi-button cbi-button-neutral',
						'title': _('Restart radio interface'),
						'click': ui.createHandlerFn(this, radio_restart, section_id)
					}, _('Restart')),
					E('button', {
						'class': 'cbi-button cbi-button-action important',
						'title': _('Find and join network'),
						'click': ui.createHandlerFn(this, 'handleScan', inst)
					}, _('Scan')),
					E('button', {
						'class': 'cbi-button cbi-button-add',
						'title': _('Provide new network'),
						'click': ui.createHandlerFn(this, 'handleAdd', inst)
					}, _('Add'))
				];
			}
			else {
				var isDisabled = (inst.get('disabled') == '1' ||
					uci.get('wireless', inst.getWifiDeviceName(), 'disabled') == '1');

				btns = [
					E('button', {
						'class': 'cbi-button cbi-button-neutral enable-disable',
						'title': isDisabled ? _('Enable this network') : _('Disable this network'),
						'click': ui.createHandlerFn(this, network_updown, section_id, this.map)
					}, isDisabled ? _('Enable') : _('Disable')),
					E('button', {
						'class': 'cbi-button cbi-button-action important',
						'title': _('Edit this network'),
						'click': ui.createHandlerFn(this, 'handleEdit', section_id)
					}, _('Edit')),
					E('button', {
						'class': 'cbi-button cbi-button-negative remove',
						'title': _('Delete this network'),
						'click': ui.createHandlerFn(this, 'handleRemove', section_id)
					}, _('Remove'))
				];
			}

			return E('td', { 'class': 'td middle cbi-section-actions' }, E('div', btns));
		};

		s.addModalOptions = function(s) {
			var radioNet, r1_radioNet, sid;

			if (s.section.charAt(0) == 'd')
				sid = s.section.replace('0', '1');
			else
				sid = radio0_mapping_radio1(s.section);

			return network.getWifiNetwork(s.section)
			.then(function(res) {
				radioNet = res;

				if (sid)
					return network.getWifiNetwork(sid).then(function(res) {
						r1_radioNet = res;
					});
			})
			.then(function() {
				var hwtype = uci.get('wireless', radioNet.getWifiDeviceName(), 'type');
				var o, ss, sss;

				if (g_dualband) {
					if (!dualBandFlag) {
						o = s.option(form.SectionValue, '_device', form.NamedSection, radioNet.getWifiDeviceName(), 'wifi-device', _('Device Configuration'));
						o.modalonly = true;

						ss = o.subsection;
						ss.tab('radio0', _('Radio0'));
						ss.tab('radio1', _('Radio1'));

						o = ss.taboption('radio0', form.SectionValue, '_device', form.NamedSection, 'radio0', 'wifi-device');

						sss = o.subsection;
						sss.ucisection = 'radio0';
						sss.addremove = false;
						sss.anonymous = true;

						sss.tab('general', _('General Setup'));
						sss.tab('advanced', _('Advanced Settings'));

						var isDisabled = (radioNet.get('disabled') == '1' ||
							uci.get('wireless', radioNet.getWifiDeviceName(), 'disabled') == 1);

						o = sss.taboption('general', form.DummyValue, '_wifistat_modal', _('Status'));
						o.cfgvalue = L.bind(function(radioNet) {
							return render_modal_status(null, radioNet);
						}, this, radioNet);
						o.write = function() {};

						o = sss.taboption('general', form.Button, '_toggle', isDisabled ? _('Wireless network is disabled') : _('Wireless network is enabled'));
						o.inputstyle = isDisabled ? 'apply' : 'reset';
						o.inputtitle = isDisabled ? _('Enable') : _('Disable');
						o.onclick = ui.createHandlerFn(s, network_updown, s.section, s.map);

						o = sss.taboption('general', CBIWifiFrequencyValue, '_freq', '<br />' + _('Operating frequency'));
						o.ucisection = s.section;

						if (hwtype == 'mac80211') {
							o = sss.taboption('general', form.Flag, 'legacy_rates', _('Allow legacy 802.11b rates'), _('Legacy or badly behaving devices may require legacy 802.11b rates to interoperate. Airtime efficiency may be significantly reduced where these are used. It is recommended to not allow 802.11b rates where possible.'));
							o.depends({'_freq': '2g', '!contains': true});

							o = sss.taboption('general', CBIWifiTxPowerValue, 'txpower', _('Maximum transmit power'), _('Specifies the maximum transmit power the wireless radio may use. Depending on regulatory requirements and wireless usage, the actual transmit power may be reduced by the driver.'));
							o.wifiNetwork = radioNet;

							o = sss.taboption('advanced', CBIWifiCountryValue, 'country', _('Country Code'));
							o.wifiNetwork = radioNet;

							o = sss.taboption('advanced', form.ListValue, 'cell_density', _('Coverage cell density'), _('Configures data rates based on the coverage cell density. Normal configures basic rates to 6, 12, 24 Mbps if legacy 802.11b rates are not used else to 5.5, 11 Mbps. High configures basic rates to 12, 24 Mbps if legacy 802.11b rates are not used else to the 11 Mbps rate. Very High configures 24 Mbps as the basic rate. Supported rates lower than the minimum basic rate are not offered.'));
							o.value('0', _('Disabled'));
							o.value('1', _('Normal'));
							o.value('2', _('High'));
							o.value('3', _('Very High'));

							o = sss.taboption('advanced', form.Value, 'distance', _('Distance Optimization'), _('Distance to farthest network member in meters.'));
							o.datatype = 'or(range(0,114750),"auto")';
							o.placeholder = 'auto';

							o = sss.taboption('advanced', form.Value, 'frag', _('Fragmentation Threshold'));
							o.datatype = 'min(256)';
							o.placeholder = _('off');

							o = sss.taboption('advanced', form.Value, 'rts', _('RTS/CTS Threshold'));
							o.datatype = 'uinteger';
							o.placeholder = _('off');

							o = sss.taboption('advanced', form.Flag, 'noscan', _('Force 40MHz mode'), _('Always use 40MHz channels even if the secondary channel overlaps. Using this option does not comply with IEEE 802.11n-2009!'));
							o.rmempty = true;

							o = sss.taboption('advanced', form.Value, 'beacon_int', _('Beacon Interval'));
							o.datatype = 'range(15,65535)';
							o.placeholder = 100;
							o.rmempty = true;
						}

						o = ss.taboption('radio1', form.SectionValue, '_device', form.NamedSection, 'radio1', 'wifi-device');

						sss = o.subsection;
						sss.ucisection = 'radio1';
						sss.addremove = false;
						sss.anonymous = true;

						sss.tab('general', _('General Setup'));
						sss.tab('advanced', _('Advanced Settings'));

						if (sid) {
							var isDisabled = (r1_radioNet.get('disabled') == '1' ||
							uci.get('wireless', r1_radioNet.getWifiDeviceName(), 'disabled') == 1);

							o = sss.taboption('general', form.DummyValue, '_wifistat_modal', _('Status'));
							o.cfgvalue = L.bind(function(r1_radioNet) {
								return render_modal_status(null, r1_radioNet);
							}, this, r1_radioNet);
						} else {
							var isDisabled = (radioNet.get('disabled') == '1' ||
							uci.get('wireless', radioNet.getWifiDeviceName(), 'disabled') == 1);

							o = sss.taboption('general', form.DummyValue, '_wifistat_modal', _('Status'));
							o.cfgvalue = L.bind(function(radioNet) {
								return render_modal_status(null, radioNet);
							}, this, radioNet);
						}
						o.write = function() {};

						o = sss.taboption('general', form.Button, '_toggle', isDisabled ? _('Wireless network is disabled') : _('Wireless network is enabled'));
						o.inputstyle = isDisabled ? 'apply' : 'reset';
						o.inputtitle = isDisabled ? _('Enable') : _('Disable');
						o.onclick = ui.createHandlerFn(s, network_updown, s.section, s.map);

						o = sss.taboption('general', CBIWifiFrequencyValue, '_freq', '<br />' + _('Operating frequency'));
						o.ucisection = s.section;

						if (hwtype == 'mac80211') {
							o = sss.taboption('general', form.Flag, 'legacy_rates', _('Allow legacy 802.11b rates'), _('Legacy or badly behaving devices may require legacy 802.11b rates to interoperate. Airtime efficiency may be significantly reduced where these are used. It is recommended to not allow 802.11b rates where possible.'));
							o.depends({'_freq': '2g', '!contains': true});

							o = sss.taboption('general', CBIWifiTxPowerValue, 'txpower', _('Maximum transmit power'), _('Specifies the maximum transmit power the wireless radio may use. Depending on regulatory requirements and wireless usage, the actual transmit power may be reduced by the driver.'));
							o.wifiNetwork = sid? r1_radioNet: radioNet;

							o = sss.taboption('advanced', CBIWifiCountryValue, 'country', _('Country Code'));
							o.wifiNetwork = sid? r1_radioNet: radioNet;

							o = sss.taboption('advanced', form.ListValue, 'cell_density', _('Coverage cell density'), _('Configures data rates based on the coverage cell density. Normal configures basic rates to 6, 12, 24 Mbps if legacy 802.11b rates are not used else to 5.5, 11 Mbps. High configures basic rates to 12, 24 Mbps if legacy 802.11b rates are not used else to the 11 Mbps rate. Very High configures 24 Mbps as the basic rate. Supported rates lower than the minimum basic rate are not offered.'));
							o.value('0', _('Disabled'));
							o.value('1', _('Normal'));
							o.value('2', _('High'));
							o.value('3', _('Very High'));

							o = sss.taboption('advanced', form.Value, 'distance', _('Distance Optimization'), _('Distance to farthest network member in meters.'));
							o.datatype = 'or(range(0,114750),"auto")';
							o.placeholder = 'auto';

							o = sss.taboption('advanced', form.Value, 'frag', _('Fragmentation Threshold'));
							o.datatype = 'min(256)';
							o.placeholder = _('off');

							o = sss.taboption('advanced', form.Value, 'rts', _('RTS/CTS Threshold'));
							o.datatype = 'uinteger';
							o.placeholder = _('off');

							o = sss.taboption('advanced', form.Flag, 'noscan', _('Force 40MHz mode'), _('Always use 40MHz channels even if the secondary channel overlaps. Using this option does not comply with IEEE 802.11n-2009!'));
							o.rmempty = true;

							o = sss.taboption('advanced', form.Value, 'beacon_int', _('Beacon Interval'));
							o.datatype = 'range(15,65535)';
							o.placeholder = 100;
							o.rmempty = true;
						}
					}
				}else{
					o = s.option(form.SectionValue, '_device', form.NamedSection, radioNet.getWifiDeviceName(), 'wifi-device', _('Device Configuration'));
					o.modalonly = true;

					ss = o.subsection;
					ss.tab('general', _('General Setup'));
					ss.tab('advanced', _('Advanced Settings'));

					var isDisabled = (radioNet.get('disabled') == '1' ||
						uci.get('wireless', radioNet.getWifiDeviceName(), 'disabled') == 1);

					o = ss.taboption('general', form.DummyValue, '_wifistat_modal', _('Status'));
					o.cfgvalue = L.bind(function(radioNet) {
						return render_modal_status(null, radioNet);
					}, this, radioNet);
					o.write = function() {};

					o = ss.taboption('general', form.Button, '_toggle', isDisabled ? _('Wireless network is disabled') : _('Wireless network is enabled'));
					o.inputstyle = isDisabled ? 'apply' : 'reset';
					o.inputtitle = isDisabled ? _('Enable') : _('Disable');
					o.onclick = ui.createHandlerFn(s, network_updown, s.section, s.map);

					o = ss.taboption('general', CBIWifiFrequencyValue, '_freq', '<br />' + _('Operating frequency'));
					o.ucisection = s.section;

					if (hwtype == 'mac80211') {
						o = ss.taboption('general', form.Flag, 'legacy_rates', _('Allow legacy 802.11b rates'), _('Legacy or badly behaving devices may require legacy 802.11b rates to interoperate. Airtime efficiency may be significantly reduced where these are used. It is recommended to not allow 802.11b rates where possible.'));
						o.depends({'_freq': '2g', '!contains': true});

						o = ss.taboption('general', CBIWifiTxPowerValue, 'txpower', _('Maximum transmit power'), _('Specifies the maximum transmit power the wireless radio may use. Depending on regulatory requirements and wireless usage, the actual transmit power may be reduced by the driver.'));
						o.wifiNetwork = radioNet;

						o = ss.taboption('advanced', CBIWifiCountryValue, 'country', _('Country Code'));
						o.wifiNetwork = radioNet;

						o = ss.taboption('advanced', form.ListValue, 'cell_density', _('Coverage cell density'), _('Configures data rates based on the coverage cell density. Normal configures basic rates to 6, 12, 24 Mbps if legacy 802.11b rates are not used else to 5.5, 11 Mbps. High configures basic rates to 12, 24 Mbps if legacy 802.11b rates are not used else to the 11 Mbps rate. Very High configures 24 Mbps as the basic rate. Supported rates lower than the minimum basic rate are not offered.'));
						o.value('0', _('Disabled'));
						o.value('1', _('Normal'));
						o.value('2', _('High'));
						o.value('3', _('Very High'));

						o = ss.taboption('advanced', form.Value, 'distance', _('Distance Optimization'), _('Distance to farthest network member in meters.'));
						o.datatype = 'or(range(0,114750),"auto")';
						o.placeholder = 'auto';

						o = ss.taboption('advanced', form.Value, 'frag', _('Fragmentation Threshold'));
						o.datatype = 'min(256)';
						o.placeholder = _('off');

						o = ss.taboption('advanced', form.Value, 'rts', _('RTS/CTS Threshold'));
						o.datatype = 'uinteger';
						o.placeholder = _('off');

						o = ss.taboption('advanced', form.Flag, 'noscan', _('Force 40MHz mode'), _('Always use 40MHz channels even if the secondary channel overlaps. Using this option does not comply with IEEE 802.11n-2009!'));
						o.rmempty = true;

						o = ss.taboption('advanced', form.Value, 'beacon_int', _('Beacon Interval'));
						o.datatype = 'range(15,65535)';
						o.placeholder = 100;
						o.rmempty = true;
					}
				}

				o = s.option(form.SectionValue, '_device', form.NamedSection, radioNet.getName(), 'wifi-iface', _('Interface Configuration'));
				o.modalonly = true;

				ss = o.subsection;
				ss.tab('general', _('General Setup'));
				ss.tab('encryption', _('Wireless Security'));
				ss.tab('macfilter', _('MAC-Filter'));
				ss.tab('advanced', _('Advanced Settings'));
				ss.tab('vlan', _('VLAN Settings'));

				o = ss.taboption('vlan', CBIWifiSetVlanPolicy, '_vlan');

				o = ss.taboption('general', form.ListValue, 'mode', _('Mode'));
				o.value('ap', _('Access Point'));
				o.value('sta', _('Client'));
				o.value('adhoc', _('Ad-Hoc'));

				o = ss.taboption('general', form.Value, 'mesh_id', _('Mesh Id'));
				o.depends('mode', 'mesh');

				o = ss.taboption('advanced', form.Flag, 'mesh_fwding', _('Forward mesh peer traffic'));
				o.rmempty = false;
				o.default = '1';
				o.depends('mode', 'mesh');

				o = ss.taboption('advanced', form.Value, 'mesh_rssi_threshold', _('RSSI threshold for joining'), _('0 = not using RSSI threshold, 1 = do not change driver default'));
				o.rmempty = false;
				o.default = '0';
				o.datatype = 'range(-255,1)';
				o.depends('mode', 'mesh');

				o = ss.taboption('general', form.Value, 'ssid', _('<abbr title="Extended Service Set Identifier">ESSID</abbr>'));
				o.datatype = 'maxlength(32)';
				o.depends('mode', 'ap');
				o.depends('mode', 'sta');
				o.depends('mode', 'adhoc');
				o.depends('mode', 'ahdemo');
				o.depends('mode', 'monitor');
				o.depends('mode', 'ap-wds');
				o.depends('mode', 'sta-wds');
				o.depends('mode', 'wds');

				o = ss.taboption('general', form.Value, 'bssid', _('<abbr title="Basic Service Set Identifier">BSSID</abbr>'));
				o.datatype = 'macaddr';

				o = ss.taboption('general', form.ListValue, 'multi_ap', _('Multiple AP'));
				o.value('', _('Disabled'));
				o.value('1', _('Backhaul BSS'));
				o.value('2', _('Fronthaul BSS'));

				o = ss.taboption('general', form.Value, 'multi_ap_backhaul_ssid', _('Backhaul SSID'));
				o.depends('multi_ap', '2')

				o = ss.taboption('general', form.Value, 'multi_ap_backhaul_key', _('Backhaul Key'));
				o.depends('multi_ap', '2')

				o = ss.taboption('general', form.Value, 'multi_ap_profile', _('Profile'));
				o.default = 2;
				o.validate = function(section_id, value) {
					const num = parseInt(value, 10);

					if (Number.isInteger(num) && num > 1)
						return true;

					return false;
				};

				o = ss.taboption('general', form.Flag, 'wds', _('Enable WDS'));
				o.depends('mode', 'sta');
				o.depends('multi_ap', '1');

				o = ss.taboption('general', widgets.NetworkSelect, 'network', _('Network'), _('Choose the network(s) you want to attach to this wireless interface or fill out the <em>custom</em> field to define a new network.'));
				o.rmempty = true;
				o.multiple = true;
				o.novirtual = true;
				o.write = function(section_id, value) {
					return network.getDevice(section_id).then(L.bind(function(dev) {
						var old_networks = dev.getNetworks().reduce(function(o, v) { o[v.getName()] = v; return o }, {}),
						    new_networks = {},
						    values = L.toArray(value),
						    tasks = [];

						for (var i = 0; i < values.length; i++) {
							new_networks[values[i]] = true;

							if (old_networks[values[i]])
								continue;

							tasks.push(network.getNetwork(values[i]).then(L.bind(function(name, net) {
								return net || network.addNetwork(name, { proto: 'none' });
							}, this, values[i])).then(L.bind(function(dev, net) {
								if (net) {
									if (!net.isEmpty()) {
										var target_dev = net.getDevice();

										/* Resolve parent interface of vlan */
										while (target_dev && target_dev.getType() == 'vlan')
											target_dev = target_dev.getParent();

										if (!target_dev || target_dev.getType() != 'bridge')
											net.set('type', 'bridge');
									}

									net.addDevice(dev);
								}
							}, this, dev)));
						}

						for (var name in old_networks)
							if (!new_networks[name])
								tasks.push(network.getNetwork(name).then(L.bind(function(dev, net) {
									if (net)
										net.deleteDevice(dev);
								}, this, dev)));

						return Promise.all(tasks);
					}, this));
				};

				encr = o = ss.taboption('encryption', form.ListValue, 'encryption', _('Encryption'));
				o.depends('mode', 'ap');
				o.depends('mode', 'sta');
				o.depends('mode', 'adhoc');
				o.depends('mode', 'ahdemo');
				o.depends('mode', 'ap-wds');
				o.depends('mode', 'sta-wds');
				o.depends('mode', 'mesh');

				o.cfgvalue = function(section_id) {
					var v = String(uci.get('wireless', section_id, 'encryption'));
					if (v == 'wep')
						return 'wep-mixed';
					else if (v.match(/\+/))
						return v.replace(/\+.+$/, '');
					return v;
				};

				o.write = function(section_id, value) {
					var e = this.section.children.filter(function(o) { return o.option == 'encryption' })[0].formvalue(section_id),
					    co = this.section.children.filter(function(o) { return o.option == 'cipher' })[0], c = co.formvalue(section_id);

					if (value == 'wpa' || value == 'wpa2' || value == 'wpa3' || value == 'wpa3-mixed')
						uci.unset('wireless', section_id, 'key');

					if (co.isActive(section_id) && e && (c == 'tkip' || c == 'ccmp' || c == 'tkip+ccmp'))
						e += '+' + c;

					uci.set('wireless', section_id, 'encryption', e);
				};

				if (hwtype == 'mac80211') {
					var mode = ss.children[0],
					    bssid = ss.children[5],
					    encr;

					mode.value('mesh', '802.11s');
					mode.value('ahdemo', _('Pseudo Ad-Hoc (ahdemo)'));
					mode.value('monitor', _('Monitor'));

					bssid.depends('mode', 'adhoc');
					bssid.depends('mode', 'sta');
					bssid.depends('mode', 'sta-wds');

					o = ss.taboption('macfilter', form.ListValue, 'macfilter', _('MAC Address Filter'));
					o.depends('mode', 'ap');
					o.depends('mode', 'ap-wds');
					o.value('', _('disable'));
					o.value('allow', _('Allow listed only'));
					o.value('deny', _('Allow all except listed'));

					o = ss.taboption('macfilter', form.DynamicList, 'maclist', _('MAC-List'));
					o.datatype = 'macaddr';
					o.retain = true;
					o.depends('macfilter', 'allow');
					o.depends('macfilter', 'deny');
					o.load = function(section_id) {
						return network.getHostHints().then(L.bind(function(hints) {
							hints.getMACHints().map(L.bind(function(hint) {
								this.value(hint[0], hint[1] ? '%s (%s)'.format(hint[0], hint[1]) : hint[0]);
							}, this));

							return form.DynamicList.prototype.load.apply(this, [section_id]);
						}, this));
					};

					mode.value('ap-wds', '%s (%s)'.format(_('Access Point'), _('WDS')));
					mode.value('sta-wds', '%s (%s)'.format(_('Client'), _('WDS')));

					mode.write = function(section_id, value) {
						switch (value) {
						case 'ap-wds':
							uci.set('wireless', section_id, 'mode', 'ap');
							uci.set('wireless', section_id, 'wds', '1');
							break;

						case 'sta-wds':
							uci.set('wireless', section_id, 'mode', 'sta');
							uci.set('wireless', section_id, 'wds', '1');
							break;

						default:
							uci.set('wireless', section_id, 'mode', value);
							uci.unset('wireless', section_id, 'wds');
							break;
						}
					};

					mode.cfgvalue = function(section_id) {
						var mode = uci.get('wireless', section_id, 'mode'),
						    wds = uci.get('wireless', section_id, 'wds');

						if (mode == 'ap' && wds)
							return 'ap-wds';
						else if (mode == 'sta' && wds)
							return 'sta-wds';

						return mode;
					};

					o = ss.taboption('general', form.Flag, 'hidden', _('Hide <abbr title="Extended Service Set Identifier">ESSID</abbr>'), _('Where the ESSID is hidden, clients may fail to roam and airtime efficiency may be significantly reduced.'));
					o.depends('mode', 'ap');
					o.depends('mode', 'ap-wds');

					o = ss.taboption('general', form.Flag, 'wmm', _('WMM Mode'), _('Where Wi-Fi Multimedia (WMM) Mode QoS is disabled, clients may be limited to 802.11a/802.11g rates.'));
					o.depends('mode', 'ap');
					o.depends('mode', 'ap-wds');
					o.default = o.enabled;

					o = ss.taboption('advanced', form.Flag, 'isolate', _('Isolate Clients'), _('Prevents client-to-client communication'));
					o.depends('mode', 'ap');
					o.depends('mode', 'ap-wds');

					o = ss.taboption('advanced', form.Value, 'ifname', _('Interface name'), _('Override default interface name'));
					o.optional = true;
					o.placeholder = radioNet.getIfname();
					if (/^radio\d+\.network/.test(o.placeholder))
						o.placeholder = '';

					var mbssid = uci.get('wireless', radioNet.getWifiDeviceName(), 'mbssid'),
					    radio_iface = s.section.slice(0, 7);

					if (radio_iface == 'default' || (radio_iface == 'wifinet' && mbssid == undefined)) {
						o = ss.taboption('advanced', form.Value, 'macaddr', _('MAC address'), _('Override default MAC address - the range of usable addresses might be limited by the driver'));
						o.optional = true;
						o.placeholder = radioNet.getActiveBSSID();
						o.datatype = 'macaddr';
					}

					o = ss.taboption('advanced', form.Flag, 'short_preamble', _('Short Preamble'));
					o.default = o.enabled;

					o = ss.taboption('advanced', form.Value, 'dtim_period', _('DTIM Interval'), _('Delivery Traffic Indication Message Interval'));
					o.optional = true;
					o.placeholder = 2;
					o.datatype = 'range(1,255)';

					o = ss.taboption('advanced', form.Value, 'wpa_group_rekey', _('Time interval for rekeying GTK'), _('sec'));
					add_encryption_dependency(o, { encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128', 'psk', 'psk2', 'wpa-mixed', 'psk-mixed', 'sae', 'sae-mixed', 'owe']});
					o.optional    = true;
					o.placeholder = 600;
					o.datatype    = 'uinteger';

					o = ss.taboption('advanced', form.Flag , 'skip_inactivity_poll', _('Disable Inactivity Polling'));
					o.optional    = true;
					o.datatype    = 'uinteger';

					o = ss.taboption('advanced', form.Value, 'max_inactivity', _('Station inactivity limit'), _('sec'));
					o.optional    = true;
					o.placeholder = 300;
					o.datatype    = 'uinteger';

					o = ss.taboption('advanced', form.Value, 'max_listen_interval', _('Maximum allowed Listen Interval'));
					o.optional    = true;
					o.placeholder = 65535;
					o.datatype    = 'uinteger';

					o = ss.taboption('advanced', form.Flag, 'disassoc_low_ack', _('Disassociate On Low Acknowledgement'), _('Allow AP mode to disconnect STAs based on low ACK condition'));
					o.default = o.enabled;
				}

				o = ss.taboption('encryption', form.ListValue, 'cipher', _('Cipher'));
				o.depends('encryption', 'wpa');
				o.depends('encryption', 'wpa2');
				o.depends('encryption', 'wpa3-mixed');
				o.depends('encryption', 'wpa3-ccmp-128');
				o.depends('encryption', 'psk');
				o.depends('encryption', 'psk2');
				o.depends('encryption', 'wpa-mixed');
				o.depends('encryption', 'psk-mixed');
				o.value('auto', _('auto'));
				o.value('ccmp', _('Force CCMP (AES)'));
				if(s.section.slice(0,7) != 'default') {
					o.value('tkip', _('Force TKIP'));
					o.value('tkip+ccmp', _('Force TKIP and CCMP (AES)'));
				}
				o.write = ss.children.filter(function(o) { return o.option == 'encryption' })[0].write;

				o.cfgvalue = function(section_id) {
					var v = String(uci.get('wireless', section_id, 'encryption'));
					if (v.match(/\+/)) {
						v = v.replace(/^[^+]+\+/, '');
						if (v == 'aes')
							v = 'ccmp';
						else if (v == 'tkip+aes' || v == 'aes+tkip' || v == 'ccmp+tkip')
							v = 'tkip+ccmp';
					}
					return v;
				};


				var crypto_modes = [];

				if (hwtype == 'mac80211') {
					var has_supplicant = L.hasSystemFeature('wpasupplicant'),
					    has_hostapd = L.hasSystemFeature('hostapd');

					// Probe EAP support
					var has_ap_eap = L.hasSystemFeature('hostapd', 'eap'),
					    has_sta_eap = L.hasSystemFeature('wpasupplicant', 'eap');

					// Probe SAE support
					var has_ap_sae = L.hasSystemFeature('hostapd', 'sae'),
					    has_sta_sae = L.hasSystemFeature('wpasupplicant', 'sae');

					// Probe OWE support
					var has_ap_owe = L.hasSystemFeature('hostapd', 'owe'),
					    has_sta_owe = L.hasSystemFeature('wpasupplicant', 'owe');

					// Probe Suite-B support
					var has_ap_eap192 = L.hasSystemFeature('hostapd', 'suiteb192'),
					    has_sta_eap192 = L.hasSystemFeature('wpasupplicant', 'suiteb192');

					// Probe WEP support
					var has_ap_wep = L.hasSystemFeature('hostapd', 'wep'),
					    has_sta_wep = L.hasSystemFeature('wpasupplicant', 'wep');

					if (has_hostapd || has_supplicant) {
						crypto_modes.push(['psk2',      'WPA2-PSK',                    35]);
						crypto_modes.push(['psk-mixed', 'WPA-PSK/WPA2-PSK Mixed Mode', 22]);
						crypto_modes.push(['psk',       'WPA-PSK',                     12]);
					}
					else {
						encr.description = _('WPA-Encryption requires wpa_supplicant (for client mode) or hostapd (for AP and ad-hoc mode) to be installed.');
					}

					if (has_ap_sae || has_sta_sae) {
						crypto_modes.push(['sae',       'WPA3-SAE',                     31]);
						crypto_modes.push(['sae-mixed', 'WPA2-PSK/WPA3-SAE Mixed Mode', 30]);
					}

					if (has_ap_wep || has_sta_wep)
						crypto_modes.push(['wep-mixed', _('WEP Open System/WEP Shared Key Mixed Mode'),  10]);

					if (has_ap_eap || has_sta_eap) {
						if (has_ap_eap192 || has_sta_eap192) {
							crypto_modes.push(['wpa3-ccmp-128', 'WPA3-Enterprise-CCMP128', 33]);
							crypto_modes.push(['wpa3', 'WPA3-Enterprise-GCMP256', 33]);
							crypto_modes.push(['wpa3-mixed', 'WPA2-Enterprise/WPA3-Enterprise-GCMP256 Mixed Mode', 32]);
						}

						crypto_modes.push(['wpa2', 'WPA2-Enterprise', 34]);
						crypto_modes.push(['wpa',  'WPA-Enterprise',  20]);
					}

					if (has_ap_owe || has_sta_owe) {
						crypto_modes.push(['owe', 'OWE', 1]);
					}

					encr.crypto_support = {
						'ap': {
							'wep-mixed': has_ap_wep || _('Requires hostapd with WEP support'),
							'psk': has_hostapd || _('Requires hostapd'),
							'psk2': has_hostapd || _('Requires hostapd'),
							'psk-mixed': has_hostapd || _('Requires hostapd'),
							'sae': has_ap_sae || _('Requires hostapd with SAE support'),
							'sae-mixed': has_ap_sae || _('Requires hostapd with SAE support'),
							'wpa': has_ap_eap || _('Requires hostapd with EAP support'),
							'wpa2': has_ap_eap || _('Requires hostapd with EAP support'),
							'wpa3-ccmp-128': has_ap_eap192 || _('Requires hostapd with EAP Suite-B support'),
							'wpa3': has_ap_eap192 || _('Requires hostapd with EAP Suite-B support'),
							'wpa3-mixed': has_ap_eap192 || _('Requires hostapd with EAP Suite-B support'),
							'owe': has_ap_owe || _('Requires hostapd with OWE support')
						},
						'sta': {
							'wep-mixed': has_sta_wep || _('Requires wpa-supplicant with WEP support'),
							'psk': has_supplicant || _('Requires wpa-supplicant'),
							'psk2': has_supplicant || _('Requires wpa-supplicant'),
							'psk-mixed': has_supplicant || _('Requires wpa-supplicant'),
							'sae': has_sta_sae || _('Requires wpa-supplicant with SAE support'),
							'sae-mixed': has_sta_sae || _('Requires wpa-supplicant with SAE support'),
							'wpa': has_sta_eap || _('Requires wpa-supplicant with EAP support'),
							'wpa2': has_sta_eap || _('Requires wpa-supplicant with EAP support'),
							'wpa3-ccmp-128': has_ap_eap192 || _('Requires hostapd with EAP Suite-B support'),
							'wpa3': has_sta_eap192 || _('Requires wpa-supplicant with EAP Suite-B support'),
							'wpa3-mixed': has_sta_eap192 || _('Requires wpa-supplicant with EAP Suite-B support'),
							'owe': has_sta_owe || _('Requires wpa-supplicant with OWE support')
						},
						'adhoc': {
							'wep-mixed': true,
							'psk': has_supplicant || _('Requires wpa-supplicant'),
							'psk2': has_supplicant || _('Requires wpa-supplicant'),
							'psk-mixed': has_supplicant || _('Requires wpa-supplicant'),
						},
						'mesh': {
							'sae': has_sta_sae || _('Requires wpa-supplicant with SAE support')
						},
						'ahdemo': {
							'wep-mixed': true
						},
						'wds': {
							'wep-mixed': true
						}
					};

					encr.crypto_support['ap-wds'] = encr.crypto_support['ap'];
					encr.crypto_support['sta-wds'] = encr.crypto_support['sta'];

					encr.validate = function(section_id, value) {
						var modeopt = this.section.children.filter(function(o) { return o.option == 'mode' })[0],
						    modeval = modeopt.formvalue(section_id),
						    modetitle = modeopt.vallist[modeopt.keylist.indexOf(modeval)],
						    enctitle = this.vallist[this.keylist.indexOf(value)];

						if (value == 'none')
							return true;

						if (!L.isObject(this.crypto_support[modeval]) || !this.crypto_support[modeval].hasOwnProperty(value))
							return _('The selected %s mode is incompatible with %s encryption').format(modetitle, enctitle);

						return this.crypto_support[modeval][value];
					};
				}
				else if (hwtype == 'broadcom') {
					crypto_modes.push(['psk2',     'WPA2-PSK',                    33]);
					crypto_modes.push(['psk+psk2', 'WPA-PSK/WPA2-PSK Mixed Mode', 22]);
					crypto_modes.push(['psk',      'WPA-PSK',                     12]);
					crypto_modes.push(['wep-mixed', _('WEP Open System/WEP Shared Key Mixed Mode'),  10]);
                }

				crypto_modes.push(['none',       _('No Encryption'),   0]);

				crypto_modes.sort(function(a, b) { return b[2] - a[2] });

				for (var i = 0; i < crypto_modes.length; i++) {
					var security_level = (crypto_modes[i][2] >= 30) ? _('strong security')
						: (crypto_modes[i][2] >= 20) ? _('medium security')
							: (crypto_modes[i][2] >= 10) ? _('weak security') : _('open network');

					if (s.section.slice(0,7) == 'default') {
						if (crypto_modes[i][0].slice(0,3) == 'wep')
							continue;
						encr.value(crypto_modes[i][0], '%s (%s)'.format(crypto_modes[i][1], security_level));
					}else{
						encr.value(crypto_modes[i][0], '%s (%s)'.format(crypto_modes[i][1], security_level));
					}
				}


				o = ss.taboption('encryption', form.Value, 'auth_server', _('RADIUS Authentication Server'));
				add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
				o.rmempty = true;
				o.datatype = 'host(0)';

				o = ss.taboption('encryption', form.Value, 'auth_port', _('RADIUS Authentication Port'));
				add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
				o.rmempty = true;
				o.datatype = 'port';
				o.placeholder = '1812';

				o = ss.taboption('encryption', form.Value, 'auth_secret', _('RADIUS Authentication Secret'));
				add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
				o.rmempty = true;
				o.password = true;

				o = ss.taboption('encryption', form.Value, 'acct_server', _('RADIUS Accounting Server'));
				add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
				o.rmempty = true;
				o.datatype = 'host(0)';

				o = ss.taboption('encryption', form.Value, 'acct_port', _('RADIUS Accounting Port'));
				add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
				o.rmempty = true;
				o.datatype = 'port';
				o.placeholder = '1813';

				o = ss.taboption('encryption', form.Value, 'acct_secret', _('RADIUS Accounting Secret'));
				add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
				o.rmempty = true;
				o.password = true;

				o = ss.taboption('encryption', form.Value, 'dae_client', _('DAE-Client'), _('Dynamic Authorization Extension client.'));
				add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
				o.rmempty = true;
				o.datatype = 'host(0)';

				o = ss.taboption('encryption', form.Value, 'dae_port', _('DAE-Port'), _('Dynamic Authorization Extension port.'));
				add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
				o.rmempty = true;
				o.datatype = 'port';
				o.placeholder = '3799';

				o = ss.taboption('encryption', form.Value, 'dae_secret', _('DAE-Secret'), _('Dynamic Authorization Extension secret.'));
				add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
				o.rmempty = true;
				o.password = true;


				o = ss.taboption('encryption', form.Value, '_wpa_key', _('Key'));
				o.depends('encryption', 'psk');
				o.depends('encryption', 'psk2');
				o.depends('encryption', 'psk+psk2');
				o.depends('encryption', 'psk-mixed');
				o.depends('encryption', 'sae');
				o.depends('encryption', 'sae-mixed');
				o.datatype = 'wpakey';
				o.rmempty = true;
				o.password = true;

				o.cfgvalue = function(section_id) {
					var key = uci.get('wireless', section_id, 'key');
					return /^[1234]$/.test(key) ? null : key;
				};

				o.validate = function(section_id, value) {
					if (value === '')
						return "You need set password first!";
					return true;
				};

				o.write = function(section_id, value) {
					uci.set('wireless', section_id, 'key', value);
					uci.unset('wireless', section_id, 'key1');
					uci.unset('wireless', section_id, 'key2');
					uci.unset('wireless', section_id, 'key3');
					uci.unset('wireless', section_id, 'key4');
				};


				o = ss.taboption('encryption', form.ListValue, '_wep_key', _('Used Key Slot'));
				o.depends('encryption', 'wep-mixed');
				o.value('1', _('Key #%d').format(1));
				o.value('2', _('Key #%d').format(2));
				o.value('3', _('Key #%d').format(3));
				o.value('4', _('Key #%d').format(4));

				o.cfgvalue = function(section_id) {
					var slot = +uci.get('wireless', section_id, 'key');
					return (slot >= 1 && slot <= 4) ? String(slot) : '';
				};

				o.write = function(section_id, value) {
					uci.set('wireless', section_id, 'key', value);
				};

				for (var slot = 1; slot <= 4; slot++) {
					o = ss.taboption('encryption', form.Value, 'key%d'.format(slot), _('Key #%d').format(slot));
					o.depends('encryption', 'wep-mixed');
					o.datatype = 'wepkey';
					o.rmempty = true;
					o.password = true;

					o.write = function(section_id, value) {
						if (value != null && (value.length == 5 || value.length == 13))
							value = 's:%s'.format(value);
						uci.set('wireless', section_id, this.option, value);
					};
				}

				if (hwtype == 'mac80211') {
					// Probe 802.11r support (and EAP support as a proxy for Openwrt)
					var has_80211r = L.hasSystemFeature('hostapd', '11r') || L.hasSystemFeature('hostapd', 'eap');

					o = ss.taboption('encryption', form.Flag, 'ieee80211r', _('802.11r Fast Transition'), _('Enables fast roaming among access points that belong to the same Mobility Domain'));
					add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
					if (has_80211r)
						add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['psk', 'psk2', 'psk-mixed', 'sae', 'sae-mixed'] });
					o.rmempty = true;

					o = ss.taboption('encryption', form.Value, 'nasid', _('NAS ID'), _('Used for two different purposes: RADIUS NAS ID and 802.11r R0KH-ID. Not needed with normal WPA(2)-PSK.'));
					add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
					o.depends({ ieee80211r: '1' });
					o.rmempty = true;

					o = ss.taboption('encryption', form.Value, 'mobility_domain', _('Mobility Domain'), _('4-character hexadecimal ID'));
					o.depends({ ieee80211r: '1' });
					o.placeholder = '4f57';
					o.datatype = 'and(hexstring,length(4))';
					o.rmempty = true;

					o = ss.taboption('encryption', form.Value, 'reassociation_deadline', _('Reassociation Deadline'), _('time units (TUs / 1.024 ms) [1000-65535]'));
					o.depends({ ieee80211r: '1' });
					o.placeholder = '1000';
					o.datatype = 'range(1000,65535)';
					o.rmempty = true;

					o = ss.taboption('encryption', form.ListValue, 'ft_over_ds', _('FT protocol'));
					o.depends({ ieee80211r: '1' });
					o.value('0', _('FT over the Air'));
					o.value('1', _('FT over DS'));
					o.rmempty = true;

					o = ss.taboption('encryption', form.Flag, 'ft_psk_generate_local', _('Generate PMK locally'), _('When using a PSK, the PMK can be automatically generated. When enabled, the R0/R1 key options below are not applied. Disable this to use the R0 and R1 key options.'));
					o.depends({ ieee80211r: '1' });
					o.default = o.enabled;
					o.rmempty = false;

					o = ss.taboption('encryption', form.Value, 'r0_key_lifetime', _('R0 Key Lifetime'), _('minutes'));
					o.depends({ ieee80211r: '1' });
					o.placeholder = '10000';
					o.datatype = 'uinteger';
					o.rmempty = true;

					o = ss.taboption('encryption', form.Value, 'r1_key_holder', _('R1 Key Holder'), _('6-octet identifier as a hex string - no colons'));
					o.depends({ ieee80211r: '1' });
					o.placeholder = '00004f577274';
					o.datatype = 'and(hexstring,length(12))';
					o.rmempty = true;

					o = ss.taboption('encryption', form.Flag, 'pmk_r1_push', _('PMK R1 Push'));
					o.depends({ ieee80211r: '1' });
					o.placeholder = '0';
					o.rmempty = true;

					o = ss.taboption('encryption', form.DynamicList, 'r0kh', _('External R0 Key Holder List'), _('List of R0KHs in the same Mobility Domain. <br />Format: MAC-address,NAS-Identifier,128-bit key as hex string. <br />This list is used to map R0KH-ID (NAS Identifier) to a destination MAC address when requesting PMK-R1 key from the R0KH that the STA used during the Initial Mobility Domain Association.'));
					o.depends({ ieee80211r: '1' });
					o.rmempty = true;

					o = ss.taboption('encryption', form.DynamicList, 'r1kh', _('External R1 Key Holder List'), _ ('List of R1KHs in the same Mobility Domain. <br />Format: MAC-address,R1KH-ID as 6 octets with colons,128-bit key as hex string. <br />This list is used to map R1KH-ID to a destination MAC address when sending PMK-R1 key from the R0KH. This is also the list of authorized R1KHs in the MD that can request PMK-R1 keys.'));
					o.depends({ ieee80211r: '1' });
					o.rmempty = true;
					// End of 802.11r options

					o = ss.taboption('encryption', form.ListValue, 'eap_type', _('EAP-Method'));
					o.value('tls',  'TLS');
					o.value('ttls', 'TTLS');
					o.value('peap', 'PEAP');
					o.value('fast', 'FAST');
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });

					o = ss.taboption('encryption', form.Flag, 'ca_cert_usesystem', _('Use system certificates'), _("Validate server certificate using built-in system CA bundle,<br />requires the \"ca-bundle\" package"));
					o.enabled = '1';
					o.disabled = '0';
					o.default = o.disabled;
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });
					o.validate = function(section_id, value) {
						if (value == '1' && !L.hasSystemFeature('cabundle')) {
							return _("This option cannot be used because the ca-bundle package is not installed.");
						}
						return true;
					};

					o = ss.taboption('encryption', form.FileUpload, 'ca_cert', _('Path to CA-Certificate'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], ca_cert_usesystem: ['0'] });

					o = ss.taboption('encryption', form.Value, 'subject_match', _('Certificate constraint (Subject)'), _("Certificate constraint substring - e.g. /CN=wifi.mycompany.com<br />See `logread -f` during handshake for actual values"));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });

					o = ss.taboption('encryption', form.DynamicList, 'altsubject_match', _('Certificate constraint (SAN)'), _("Certificate constraint(s) via Subject Alternate Name values<br />(supported attributes: EMAIL, DNS, URI) - e.g. DNS:wifi.mycompany.com"));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });

					o = ss.taboption('encryption', form.DynamicList, 'domain_match', _('Certificate constraint (Domain)'), _("Certificate constraint(s) against DNS SAN values (if available)<br />or Subject CN (exact match)"));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });

					o = ss.taboption('encryption', form.DynamicList, 'domain_suffix_match', _('Certificate constraint (Wildcard)'), _("Certificate constraint(s) against DNS SAN values (if available)<br />or Subject CN (suffix match)"));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });

					o = ss.taboption('encryption', form.FileUpload, 'client_cert', _('Path to Client-Certificate'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], eap_type: ['tls'] });

					o = ss.taboption('encryption', form.FileUpload, 'priv_key', _('Path to Private Key'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], eap_type: ['tls'] });

					o = ss.taboption('encryption', form.Value, 'priv_key_pwd', _('Password of Private Key'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], eap_type: ['tls'] });
					o.password = true;

					o = ss.taboption('encryption', form.ListValue, 'auth', _('Authentication'));
					o.value('PAP', 'PAP');
					o.value('CHAP', 'CHAP');
					o.value('MSCHAP', 'MSCHAP');
					o.value('MSCHAPV2', 'MSCHAPv2');
					o.value('EAP-GTC', 'EAP-GTC');
					o.value('EAP-MD5', 'EAP-MD5');
					o.value('EAP-MSCHAPV2', 'EAP-MSCHAPv2');
					o.value('EAP-TLS', 'EAP-TLS');
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], eap_type: ['fast', 'peap', 'ttls'] });

					o.validate = function(section_id, value) {
						var eo = this.section.children.filter(function(o) { return o.option == 'eap_type' })[0],
						    ev = eo.formvalue(section_id);

						if (ev != 'ttls' && (value == 'PAP' || value == 'CHAP' || value == 'MSCHAP' || value == 'MSCHAPV2'))
							return _('This authentication type is not applicable to the selected EAP method.');

						return true;
					};

					o = ss.taboption('encryption', form.Flag, 'ca_cert2_usesystem', _('Use system certificates for inner-tunnel'), _("Validate server certificate using built-in system CA bundle,<br />requires the \"ca-bundle\" package"));
					o.enabled = '1';
					o.disabled = '0';
					o.default = o.disabled;
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], auth: ['EAP-TLS'] });
					o.validate = function(section_id, value) {
						if (value == '1' && !L.hasSystemFeature('cabundle')) {
							return _("This option cannot be used because the ca-bundle package is not installed.");
						}
						return true;
					};

					o = ss.taboption('encryption', form.FileUpload, 'ca_cert2', _('Path to inner CA-Certificate'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], auth: ['EAP-TLS'], ca_cert2_usesystem: ['0'] });

					o = ss.taboption('encryption', form.Value, 'subject_match2', _('Inner certificate constraint (Subject)'), _("Certificate constraint substring - e.g. /CN=wifi.mycompany.com<br />See `logread -f` during handshake for actual values"));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], auth: ['EAP-TLS'] });

					o = ss.taboption('encryption', form.DynamicList, 'altsubject_match2', _('Inner certificate constraint (SAN)'), _("Certificate constraint(s) via Subject Alternate Name values<br />(supported attributes: EMAIL, DNS, URI) - e.g. DNS:wifi.mycompany.com"));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], auth: ['EAP-TLS'] });

					o = ss.taboption('encryption', form.DynamicList, 'domain_match2', _('Inner certificate constraint (Domain)'), _("Certificate constraint(s) against DNS SAN values (if available)<br />or Subject CN (exact match)"));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], auth: ['EAP-TLS'] });

					o = ss.taboption('encryption', form.DynamicList, 'domain_suffix_match2', _('Inner certificate constraint (Wildcard)'), _("Certificate constraint(s) against DNS SAN values (if available)<br />or Subject CN (suffix match)"));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], auth: ['EAP-TLS'] });

					o = ss.taboption('encryption', form.FileUpload, 'client_cert2', _('Path to inner Client-Certificate'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], auth: ['EAP-TLS'] });

					o = ss.taboption('encryption', form.FileUpload, 'priv_key2', _('Path to inner Private Key'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], auth: ['EAP-TLS'] });

					o = ss.taboption('encryption', form.Value, 'priv_key2_pwd', _('Password of inner Private Key'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], auth: ['EAP-TLS'] });
					o.password = true;

					o = ss.taboption('encryption', form.Value, 'identity', _('Identity'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], eap_type: ['fast', 'peap', 'tls', 'ttls'] });

					o = ss.taboption('encryption', form.Value, 'anonymous_identity', _('Anonymous Identity'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], eap_type: ['fast', 'peap', 'tls', 'ttls'] });

					o = ss.taboption('encryption', form.Value, 'password', _('Password'));
					add_dependency_permutations(o, { mode: ['sta', 'sta-wds'], encryption: ['wpa', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'], eap_type: ['fast', 'peap', 'ttls'] });
					o.password = true;


					if (hwtype == 'mac80211') {
						// ieee802.11w options
						o = ss.taboption('encryption', form.ListValue, 'ieee80211w', _('802.11w Management Frame Protection'), _("Note: Some wireless drivers do not fully support 802.11w. E.g. mwlwifi may have problems"));
						o.value('0', _('Disabled'));
						o.value('1', _('Optional'));
						o.value('2', _('Required'));
						add_dependency_permutations(o, { mode: ['ap', 'ap-wds', 'sta', 'sta-wds'], encryption: ['owe', 'psk2', 'psk-mixed', 'sae', 'sae-mixed', 'wpa2', 'wpa3', 'wpa3-mixed'] });

						o.defaults = {
							'2': [{ encryption: 'sae' }, { encryption: 'owe' }, { encryption: 'wpa3' }, { encryption: 'wpa3-mixed' }, { encryption: 'wpa3-ccmp-128' }],
							'1': [{ encryption: 'sae-mixed'}],
							'0': []
						};

						o.write = function(section_id, value) {
							if (value != this.default)
								return form.ListValue.prototype.write.call(this, section_id, value);
							else
								return form.ListValue.prototype.remove.call(this, section_id);
						};

						o = ss.taboption('encryption', form.Value, 'ieee80211w_max_timeout', _('802.11w maximum timeout'), _('802.11w Association SA Query maximum timeout'));
						add_dependency(o, { ieee80211w: ['1', '2'], mode: ['ap', 'ap-wds', 'sta-wds']});
						o.datatype = 'uinteger';
						o.placeholder = '1000';
						o.rmempty = true;

						o = ss.taboption('encryption', form.Value, 'ieee80211w_retry_timeout', _('802.11w retry timeout'), _('802.11w Association SA Query retry timeout'));
						add_dependency(o, { ieee80211w: ['1', '2'], mode: ['ap', 'ap-wds', 'sta-wds']});
						o.datatype = 'uinteger';
						o.placeholder = '201';
						o.rmempty = true;

						o = ss.taboption('encryption', form.Flag, 'wpa_disable_eapol_key_retries', _('Enable key reinstallation (KRACK) countermeasures'), _('Complicates key reinstallation attacks on the client side by disabling retransmission of EAPOL-Key frames that are used to install keys. This workaround might cause interoperability issues and reduced robustness of key negotiation especially in environments with heavy traffic load.'));
						add_dependency_permutations(o, { mode: ['ap', 'ap-wds'], encryption: ['psk2', 'psk-mixed', 'sae', 'sae-mixed', 'wpa2', 'wpa3', 'wpa3-mixed', 'wpa3-ccmp-128'] });

						if (L.hasSystemFeature('hostapd', 'wps') && L.hasSystemFeature('wpasupplicant')) {
							o = ss.taboption('encryption', form.Flag, 'wps_pushbutton', _('Enable WPS pushbutton, requires WPA(2)-PSK/WPA3-SAE'))
							o.enabled = '1';
							o.disabled = '0';
							o.default = o.disabled;
							o.depends('encryption', 'none');
							o.depends('encryption', 'psk');
							o.depends('encryption', 'psk2');
							o.depends('encryption', 'psk-mixed');
							o.depends('encryption', 'sae-mixed');

							o = ss.taboption('encryption', form.Flag, 'wps_label', _('Enable WPS PIN, requires WPA(2)-PSK/WPA3-SAE'))
							o.depends('encryption', 'none');
							o.depends('encryption', 'psk');
							o.depends('encryption', 'psk2');
							o.depends('encryption', 'psk-mixed');
							o.depends('encryption', 'sae-mixed');

							o = ss.taboption('encryption', form.Flag, 'wps_keypad', _('Enable WPS PIN connect, requires WPA(2)-PSK/WPA3-SAE'))
							o.depends('encryption', 'none');
							o.depends('encryption', 'psk');
							o.depends('encryption', 'psk2');
							o.depends('encryption', 'psk-mixed');
							o.depends('encryption', 'sae-mixed');
						}
					}
				}
			});
		};

		s.handleRemove = function(section_id, ev) {
			if (section_id.slice(0,7) == 'default') {
				alert(_('This is default interface, you can not remove this section!'));
				return ;
			}

			if (g_dualband) {
				var wiface = uci.sections('wireless', 'wifi-iface');
				var src_ifaces = [], dst_ifaces = [];

				for (let i = 0; i < wiface.length; i++) {
					if (wiface[i].mode == 'sta' || (wiface[i].hasOwnProperty('multi_ap') && wiface[i].multi_ap == '1'))
						continue;

					if (wiface[i].device == 'radio0')
						src_ifaces.push(wiface[i]);
					else
						dst_ifaces.push(wiface[i]);
				}
				
				for (let i = 0; i < dst_ifaces.length && i < dst_ifaces.length; i++) {
					if (src_ifaces[i]['.name'] == section_id)
						callUciDelete('wireless', dst_ifaces[i]['.name']);
				}

				document.querySelector('.cbi-section-table-row[data-sid="%s"]'.format(section_id)).style.opacity = 0.5;
				return form.TypedSection.prototype.handleRemove.apply(this, [section_id, ev]);
			}else{
				document.querySelector('.cbi-section-table-row[data-sid="%s"]'.format(section_id)).style.opacity = 0.5;
				return form.TypedSection.prototype.handleRemove.apply(this, [section_id, ev]);
			}
		};

		s.handleScan = function(radioDev, ev) {
			var table = E('table', { 'class': 'table' }, [
				E('tr', { 'class': 'tr table-titles' }, [
					E('th', { 'class': 'th col-2 middle center' }, _('Signal')),
					E('th', { 'class': 'th col-4 middle left' }, _('SSID')),
					E('th', { 'class': 'th col-2 middle center hide-xs' }, _('Channel')),
					E('th', { 'class': 'th col-2 middle left hide-xs' }, _('Mode')),
					E('th', { 'class': 'th col-3 middle left hide-xs' }, _('BSSID')),
					E('th', { 'class': 'th col-3 middle left' }, _('Encryption')),
					E('th', { 'class': 'th cbi-section-actions right' }, ' '),
				])
			]);

			var stop = E('button', {
				'class': 'btn',
				'click': L.bind(this.handleScanStartStop, this),
				'style': 'display:none',
				'data-state': 'stop'
			}, _('Stop refresh'));

			cbi_update_table(table, [], E('em', { class: 'spinning' }, _('Starting wireless scan...')));

			var md = ui.showModal(_('Join Network: Wireless Scan'), [
				table,
				E('div', { 'class': 'right' }, [
					stop,
					' ',
					E('button', {
						'class': 'btn',
						'click': L.bind(this.handleScanAbort, this)
					}, _('Dismiss'))
				])
			]);

			md.style.maxWidth = '90%';
			md.style.maxHeight = 'none';

			this.pollFn = L.bind(this.handleScanRefresh, this, radioDev, {}, table, stop);

			poll.add(this.pollFn);
			poll.start();
		};

		s.handleScanRefresh = function(radioDev, scanCache, table, stop) {
			return radioDev.getScanList().then(L.bind(function(results) {
				var rows = [];

				for (var i = 0; i < results.length; i++)
					scanCache[results[i].bssid] = results[i];

				for (var k in scanCache)
					if (scanCache[k].stale)
						results.push(scanCache[k]);

				results.sort(function(a, b) {
					var diff = (b.quality - a.quality) || (a.channel - b.channel);

					if (diff)
						return diff;

					if (a.ssid < b.ssid)
						return -1;
					else if (a.ssid > b.ssid)
						return 1;

					if (a.bssid < b.bssid)
						return -1;
					else if (a.bssid > b.bssid)
						return 1;
				});

				for (var i = 0; i < results.length; i++) {
					var res = results[i],
					    qv = res.quality || 0,
					    qm = res.quality_max || 0,
					    q = (qv > 0 && qm > 0) ? Math.floor((100 / qm) * qv) : 0,
					    s = res.stale ? 'opacity:0.5' : '';

					rows.push([
						E('span', { 'style': s }, render_signal_badge(q, res.signal, res.noise)),
						E('span', { 'style': s }, (res.ssid != null) ? '%h'.format(res.ssid) : E('em', _('hidden'))),
						E('span', { 'style': s }, '%d'.format(res.channel)),
						E('span', { 'style': s }, '%h'.format(res.mode)),
						E('span', { 'style': s }, '%h'.format(res.bssid)),
						E('span', { 'style': s }, '%h'.format(network.formatWifiEncryption(res.encryption))),
						E('div', { 'class': 'right' }, E('button', {
							'class': 'cbi-button cbi-button-action important',
							'click': ui.createHandlerFn(this, 'handleJoin', radioDev, res)
						}, _('Join Network')))
					]);

					res.stale = true;
				}

				cbi_update_table(table, rows);

				stop.disabled = false;
				stop.style.display = '';
				stop.classList.remove('spinning');
			}, this));
		};

		s.handleScanStartStop = function(ev) {
			var btn = ev.currentTarget;

			if (btn.getAttribute('data-state') == 'stop') {
				poll.remove(this.pollFn);
				btn.firstChild.data = _('Start refresh');
				btn.setAttribute('data-state', 'start');
			}
			else {
				poll.add(this.pollFn);
				btn.firstChild.data = _('Stop refresh');
				btn.setAttribute('data-state', 'stop');
				btn.classList.add('spinning');
				btn.disabled = true;
			}
		};

		s.handleScanAbort = function(ev) {
			var md = dom.parent(ev.target, 'div[aria-modal="true"]');
			if (md) {
				md.style.maxWidth = '';
				md.style.maxHeight = '';
			}

			ui.hideModal();
			poll.remove(this.pollFn);

			this.pollFn = null;
		};

		s.handleJoinConfirm = function(radioDev, bss, form, ev) {
			var nameopt = L.toArray(form.lookupOption('name', '_new_'))[0],
			    passopt = L.toArray(form.lookupOption('password', '_new_'))[0],
			    ssidopt = L.toArray(form.lookupOption('ssid', '_new_'))[0],
			    bssidopt = L.toArray(form.lookupOption('bssid', '_new_'))[0],
			    zoneopt = L.toArray(form.lookupOption('zone', '_new_'))[0],
			    replopt = L.toArray(form.lookupOption('replace', '_new_'))[0],
			    nameval = (nameopt && nameopt.isValid('_new_')) ? nameopt.formvalue('_new_') : null,
			    passval = (passopt && passopt.isValid('_new_')) ? passopt.formvalue('_new_') : null,
			    ssidval = (ssidopt && ssidopt.isValid('_new_')) ? ssidopt.formvalue('_new_') : null,
			    bssidval = (bssidopt && bssidopt.isValid('_new_')) ? bssidopt.formvalue('_new_') : null,
			    zoneval = zoneopt ? zoneopt.formvalue('_new_') : null,
			    enc = L.isObject(bss.encryption) ? bss.encryption : null,
			    is_wep = (enc && Array.isArray(enc.wep)),
			    is_psk = (enc && Array.isArray(enc.wpa) && L.toArray(enc.authentication).filter(function(a) { return a == 'psk' }).length > 0),
			    is_sae = (enc && Array.isArray(enc.wpa) && L.toArray(enc.authentication).filter(function(a) { return a == 'sae' }).length > 0);

			if (nameval == null || (passopt && passval == null))
				return;

			var section_id = null;

			return this.map.save(function() {
				var wifi_sections = uci.sections('wireless', 'wifi-iface');

				if (replopt.formvalue('_new_') == '1') {
					for (var i = 0; i < wifi_sections.length; i++)
						if (wifi_sections[i].device == radioDev.getName())
							uci.remove('wireless', wifi_sections[i]['.name']);
				}

				if (uci.get('wireless', radioDev.getName(), 'disabled') == '1') {
					for (var i = 0; i < wifi_sections.length; i++)
						if (wifi_sections[i].device == radioDev.getName())
							uci.set('wireless', wifi_sections[i]['.name'], 'disabled', '1');

					uci.unset('wireless', radioDev.getName(), 'disabled');
				}

				section_id = next_free_sid(wifi_sections.length);

				uci.add('wireless', 'wifi-iface', section_id);
				uci.set('wireless', section_id, 'device', radioDev.getName());
				uci.set('wireless', section_id, 'mode', (bss.mode == 'Ad-Hoc') ? 'adhoc' : 'sta');
				uci.set('wireless', section_id, 'network', nameval);

				if (bss.ssid != null) {
					uci.set('wireless', section_id, 'ssid', bss.ssid);

					if (bssidval == '1')
						uci.set('wireless', section_id, 'bssid', bss.bssid);
				}
				else if (bss.bssid != null) {
					uci.set('wireless', section_id, 'bssid', bss.bssid);
				}

				if (ssidval != null)
					uci.set('wireless', section_id, 'ssid', ssidval);

				if (is_sae) {
					uci.set('wireless', section_id, 'encryption', 'sae');
					uci.set('wireless', section_id, 'key', passval);
				}
				else if (is_psk) {
					for (var i = enc.wpa.length - 1; i >= 0; i--) {
						if (enc.wpa[i] == 2) {
							uci.set('wireless', section_id, 'encryption', 'psk2');
							break;
						}
						else if (enc.wpa[i] == 1) {
							uci.set('wireless', section_id, 'encryption', 'psk');
							break;
						}
					}

					uci.set('wireless', section_id, 'key', passval);
				}
				else if (is_wep) {
					uci.set('wireless', section_id, 'encryption', 'wep-mixed');
					uci.set('wireless', section_id, 'key', '1');
					uci.set('wireless', section_id, 'key1', passval);
				}
				else {
					uci.set('wireless', section_id, 'encryption', 'none');
				}

				return network.addNetwork(nameval, { proto: 'dhcp' }).then(function(net) {
					firewall.deleteNetwork(net.getName());

					var zonePromise = zoneval
						? firewall.getZone(zoneval).then(function(zone) { return zone || firewall.addZone(zoneval) })
						: Promise.resolve();

					return zonePromise.then(function(zone) {
						if (zone)
							zone.addNetwork(net.getName());
					});
				});
			}).then(L.bind(function() {
				ui.showModal(null, E('p', { 'class': 'spinning' }, [ _('Loading data…') ]));

				return this.renderMoreOptionsModal(section_id);
			}, this));
		};

		s.handleJoin = function(radioDev, bss, ev) {
			poll.remove(this.pollFn);

			var m2 = new form.Map('wireless'),
			    s2 = m2.section(form.NamedSection, '_new_'),
			    enc = L.isObject(bss.encryption) ? bss.encryption : null,
			    is_wep = (enc && Array.isArray(enc.wep)),
			    is_psk = (enc && Array.isArray(enc.wpa) && L.toArray(enc.authentication).filter(function(a) { return a == 'psk' || a == 'sae' })),
			    replace, passphrase, name, bssid, zone;

			var nameUsed = function(name) {
				var s = uci.get('network', name);
				if (s != null && s['.type'] != 'interface')
					return true;

				var net = (s != null) ? network.instantiateNetwork(name) : null;
				return (net != null && !net.isEmpty());
			};

			s2.render = function() {
				return Promise.all([
					{},
					this.renderUCISection('_new_')
				]).then(this.renderContents.bind(this));
			};

			if (bss.ssid == null) {
				name = s2.option(form.Value, 'ssid', _('Network SSID'), _('The correct SSID must be manually specified when joining a hidden wireless network'));
				name.rmempty = false;
			};

			replace = s2.option(form.Flag, 'replace', _('Replace wireless configuration'), _('Check this option to delete the existing networks from this radio.'));

			name = s2.option(form.Value, 'name', _('Name of the new network'), _('The allowed characters are: <code>A-Z</code>, <code>a-z</code>, <code>0-9</code> and <code>_</code>'));
			name.datatype = 'uciname';
			name.default = 'wwan';
			name.rmempty = false;
			name.validate = function(section_id, value) {
				if (nameUsed(value))
					return _('The network name is already used');

				return true;
			};

			for (var i = 2; nameUsed(name.default); i++)
				name.default = 'wwan%d'.format(i);

			if (is_wep || is_psk) {
				passphrase = s2.option(form.Value, 'password', is_wep ? _('WEP passphrase') : _('WPA passphrase'), _('Specify the secret encryption key here.'));
				passphrase.datatype = is_wep ? 'wepkey' : 'wpakey';
				passphrase.password = true;
				passphrase.rmempty = false;
			}

			if (bss.ssid != null) {
				bssid = s2.option(form.Flag, 'bssid', _('Lock to BSSID'), _('Instead of joining any network with a matching SSID, only connect to the BSSID <code>%h</code>.').format(bss.bssid));
				bssid.default = '0';
			}

			zone = s2.option(widgets.ZoneSelect, 'zone', _('Create / Assign firewall-zone'), _('Choose the firewall zone you want to assign to this interface. Select <em>unspecified</em> to remove the interface from the associated zone or fill out the <em>custom</em> field to define a new zone and attach the interface to it.'));
			zone.default = 'wan';

			return m2.render().then(L.bind(function(nodes) {
				ui.showModal(_('Joining Network: %q').replace(/%q/, '"%h"'.format(bss.ssid)), [
					nodes,
					E('div', { 'class': 'right' }, [
						E('button', {
							'class': 'btn',
							'click': ui.hideModal
						}, _('Cancel')), ' ',
						E('button', {
							'class': 'cbi-button cbi-button-positive important',
							'click': ui.createHandlerFn(this, 'handleJoinConfirm', radioDev, bss, m2)
						}, _('Submit'))
					])
				], 'cbi-modal').querySelector('[id="%s"] input[class][type]'.format((passphrase || name).cbid('_new_'))).focus();
			}, this));
		};

		s.handleAdd = function(radioDev, ev) {
			dualBandFlag = true;

			function addIfaceFor(devName) {
				var sid = next_free_sid(uci.sections('wireless','wifi-iface').length);
					uci.unset('wireless', devName, 'disabled');
					uci.add('wireless', 'wifi-iface', sid);
					uci.set('wireless', sid, 'device',     devName);
					uci.set('wireless', sid, 'mode',       'ap');
					uci.set('wireless', sid, 'ssid',       'OpenWrt');
					uci.set('wireless', sid, 'encryption', 'none');
				if (g_dualband) {
					uci.set('wireless', sid, 'bss_transition',     '1');
					uci.set('wireless', sid, 'rrm_neighbor_report','1');
				}

				return sid;
			}

			var primarySid = addIfaceFor(radioDev.getName());
			var secondarySid = null;

			if (g_dualband) {
				var otherName = radioDev.getName() === 'radio0' ? 'radio1' : 'radio0';
				secondarySid = addIfaceFor(otherName);

				(function(primary, secondary) {
					var _origSet   = uci.set.bind(uci);
					var _origUnset = uci.unset.bind(uci);

					uci.set = function(config, section, option, value) {
						_origSet(config, section, option, value);
						if (section === primary)
							_origSet(config, secondary, option, value);

						return this;
					};

					uci.unset = function(config, section, option) {
						_origUnset(config, section, option);
						if (section === primary)
							_origUnset(config, secondary, option);

						return this;
					};
				})(primarySid, secondarySid);
			}

			if (secondarySid)
				return this.renderMoreOptionsModal(primarySid, secondarySid);
			else
				return this.renderMoreOptionsModal(primarySid);
		};


		s.handleEdit = function(section_id) {
			dualBandFlag = false;
			if (!g_dualband) {
				return this.renderMoreOptionsModal(section_id);
			}

			var primarySid = section_id;
			var primaryDev = uci.get('wireless', primarySid, 'device');
			var otherDev   = primaryDev === 'radio0' ? 'radio1' : 'radio0';

			var allIfaces = uci.sections('wireless', 'wifi-iface');
			var srcList = [], dstList = [];
			allIfaces.forEach(function(s) {
				var sid   = s['.name'];
				var dev   = uci.get('wireless', sid, 'device');
				var mode  = uci.get('wireless', sid, 'mode');
				var multi = uci.get('wireless', sid, 'multi_ap');
				if (mode === 'sta' || multi === '1') return;
				if (dev === primaryDev) srcList.push(sid);
				else if (dev === otherDev) dstList.push(sid);
			});

			var idx = srcList.indexOf(primarySid);
			var secondarySid = (idx >= 0 && idx < dstList.length) ? dstList[idx] : null;
			if (!secondarySid) {
				return this.renderMoreOptionsModal(primarySid);
			}

			(function(primary, secondary) {
				var _origSet   = uci.set.bind(uci);
				var _origUnset = uci.unset.bind(uci);

				var protect = ['multi_ap',
                       			'multi_ap_backhaul_ssid',
                       			'multi_ap_backhaul_key'
				];

				uci.set = function(config, section, option, value) {
					if (protect.indexOf(option) < 0)
						_origSet(config, section, option, value);
					
					if (section === primary)
						_origSet(config, secondary, option, value);
				
					return this;
				};

				uci.unset = function(config, section, option) {
					_origUnset(config, section, option);
					if (section === primary)
						_origUnset(config, secondary, option);
		
					return this;
				};

			})(primarySid, secondarySid);

			return this.renderMoreOptionsModal(primarySid, secondarySid);
		};

		o = s.option(form.DummyValue, '_badge');
		o.modalonly = false;
		o.textvalue = function(section_id) {
			var inst,
			    node = E('div', { 'class': 'center' });

			if (!g_dualband) {
				inst = this.section.lookupRadioOrNetwork(section_id);

				if (inst.getWifiNetworks)
					node.appendChild(render_radio_badge(inst));
				else
					node.appendChild(render_network_badge(inst));

				return node;
			}else{
				switch(section_id.charAt(0)) {
					case 'r':
						var inst_r0 = this.section.lookupRadioOrNetwork(section_id),
						    inst_r1 = this.section.lookupRadioOrNetwork(section_id.replace('0', '1'));
						
						inst = [inst_r0, inst_r1];

						node.appendChild(render_radio_badge(inst));

						return node;
					case 'd':
						var default_r0 = this.section.lookupRadioOrNetwork(section_id),
						    default_r1 = this.section.lookupRadioOrNetwork(section_id.replace('0', '1'));

						inst = [default_r0, default_r1];

						node.appendChild(render_network_badge(inst));

						return node;
					case 'w':
						var wifinet_r0 = this.section.lookupRadioOrNetwork(section_id),
						    wifinet_r1 = this.section.lookupRadioOrNetwork(radio0_mapping_radio1(section_id));
							
						inst = [wifinet_r0, wifinet_r1];
						
						node.appendChild(render_network_badge(inst));
						
						return node;
					default:
						break;
				}
			}
		};

		o = s.option(form.DummyValue, '_stat');
		o.modalonly = false;
		o.textvalue = function(section_id) {
			var inst;

			if (!g_dualband) {
				inst = this.section.lookupRadioOrNetwork(section_id);

				if (inst.getWifiNetworks)
					return render_radio_status(inst, this.section.wifis.filter(function(e) {
						return (e.getWifiDeviceName() == inst.getName());
					}));
				else
					return render_network_status(inst);
			}else{
				switch(section_id.charAt(0)) {
					case 'r':
						var inst_r0 = this.section.lookupRadioOrNetwork(section_id),
							inst_r1 = this.section.lookupRadioOrNetwork(section_id.replace('0', '1'));
							
						inst = [inst_r0, inst_r1];
						
						return render_radio_status(inst, this.section.wifis);
					case 'd':
						var default_r0 = this.section.lookupRadioOrNetwork(section_id),
							default_r1 = this.section.lookupRadioOrNetwork(section_id.replace('0', '1'));
							
						inst = [default_r0, default_r1];
						
						return render_network_status(inst);
					case 'w':
						var wifinet_r0 = this.section.lookupRadioOrNetwork(section_id),
							wifinet_r1 = this.section.lookupRadioOrNetwork(radio0_mapping_radio1(section_id));
							
						inst = [wifinet_r0, wifinet_r1];
						
						return render_network_status(inst);
					default:
						break;
				}
			}
		};

		return m.render().then(L.bind(function(m, nodes) {
			poll.add(L.bind(function() {
				var section_ids = m.children[0].map.children[1].cfgsections(),
				    tasks = [ network.getHostHints(), network.getWifiDevices() ];

				for (var i = 0; i < section_ids.length; i++) {
					var row = nodes.querySelector('.cbi-section-table-row[data-sid="%s"]'.format(section_ids[i])),
						dsc = row.querySelector('[data-name="_stat"] > div'),
					    btns = row.querySelectorAll('.cbi-section-actions button');

					if (dsc.getAttribute('restart') == '') {
						dsc.setAttribute('restart', '1');
						tasks.push(fs.exec('/sbin/wifi', ['up', section_ids[i]]).catch(function(e) {
							ui.addNotification(null, E('p', e.message));
						}));
					}
					else if (dsc.getAttribute('restart') == '1') {
						dsc.removeAttribute('restart');
						btns[0].classList.remove('spinning');
						btns[0].disabled = false;
					}
				}

				return Promise.all(tasks)
					.then(L.bind(function(hosts_radios) {
						var tasks = [];

						for (var i = 0; i < hosts_radios[1].length; i++)
							tasks.push(hosts_radios[1][i].getWifiNetworks());

						return Promise.all(tasks).then(function(data) {
							hosts_radios[2] = [];

							for (var i = 0; i < data.length; i++)
								hosts_radios[2].push.apply(hosts_radios[2], data[i]);

							return hosts_radios;
						});
					}, network))
					.then(L.bind(function(hosts_radios_wifis) {
						var tasks = [];

						for (var i = 0; i < hosts_radios_wifis[2].length; i++)
							tasks.push(hosts_radios_wifis[2][i].getAssocList());

						return Promise.all(tasks).then(function(data) {
							hosts_radios_wifis[3] = [];

							for (var i = 0; i < data.length; i++) {
								var wifiNetwork = hosts_radios_wifis[2][i],
								    radioDev = hosts_radios_wifis[1].filter(function(d) { return d.getName() == wifiNetwork.getWifiDeviceName() })[0];

								for (var j = 0; j < data[i].length; j++)
									hosts_radios_wifis[3].push(Object.assign({ radio: radioDev, network: wifiNetwork }, data[i][j]));
							}

							return hosts_radios_wifis;
						});
					}, network))
					.then(L.bind(this.poll_status, this, nodes));
			}, this), 5);

			var table = E('table', { 'class': 'table assoclist', 'id': 'wifi_assoclist_table' }, [
				E('tr', { 'class': 'tr table-titles' }, [
					E('th', { 'class': 'th nowrap' }, _('Network')),
					E('th', { 'class': 'th hide-xs' }, _('MAC address')),
					E('th', { 'class': 'th' }, _('Host')),
					E('th', { 'class': 'th' }, _('Signal / Noise')),
					E('th', { 'class': 'th' }, _('RX Rate / TX Rate'))
				])
			]);

			cbi_update_table(table, [], E('em', { 'class': 'spinning' }, _('Collecting data...')))

			return E([ nodes, E('h3', _('Associated Stations')), table ]);
		}, this, m));

	},

	handleSaveApply: function(ev, mode) {
		return this.handleSave(ev).then(() =>{
			var changes = classes.ui.changes.changes.wireless;

			if (changes.flat().includes('highpwr'))
				set_pwrmode(changes);

			this.handleSave(ev).then(function() {
				classes.ui.changes.apply(mode == '0');
			});
		});
	}
});
