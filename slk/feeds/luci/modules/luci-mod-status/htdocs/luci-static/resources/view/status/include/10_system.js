'use strict';
'require baseclass';
'require fs';
'require rpc';
'require uci';

var callSystemBoard = rpc.declare({
	object: 'system',
	method: 'board'
});

var callSystemInfo = rpc.declare({
	object: 'system',
	method: 'info'
});

return baseclass.extend({
	title: _('System'),

	_getModel: function() {
		try {
			uci.load('system');
			var model = uci.get('system', '@system[0]', 'model');
			return model || '';
		} catch (e) {
			console.error('Failed to get model:', e);
			return '';
		}
	},
	
	_getF_ver: function() {
		try {
			uci.load('system');
			var f_ver = uci.get('system', '@system[0]', 'F_ver');
			return f_ver || '';
		} catch (e) {
			console.error('Failed to get F_ver:', e);
			return '';
		}
	},
	
	_getH_ver: function() {
		try {
			uci.load('system');
			var h_ver = uci.get('system', '@system[0]', 'H_ver');
			return h_ver || '';
		} catch (e) {
			console.error('Failed to get H_ver:', e);
			return '';
		}
	},
	
	_getCellModel: function() {
		try {
			uci.load('system');
			var simModel = uci.get('system', '@system[0]', 'module_name');
			var sim1Model = uci.get('system', '@system[0]', 'SIM1_name');
			var sim2Model = uci.get('system', '@system[0]', 'SIM2_name');
			var sim3Model = uci.get('system', '@system[0]', 'SIM3_name');
			var sim4Model = uci.get('system', '@system[0]', 'SIM4_name');
			
			uci.load('luci');
			var theme = uci.get('luci', 'main', 'mediaurlbase');
			if (theme) {
				if (simModel && theme.includes('slk')) {
					simModel = 'SLK' + simModel;
				}
				if (sim1Model && theme.includes('slk')) {
					sim1Model = 'SLK' + sim1Model;
				}
				if (sim2Model && theme.includes('slk')) {
					sim2Model = 'SLK' + sim2Model;
				}
				if (sim3Model && theme.includes('slk')) {
					sim3Model = 'SLK' + sim3Model;
				}
				if (sim4Model && theme.includes('slk')) {
					sim4Model = 'SLK' + sim4Model;
				}
			}
			
			var result = '';
			if (simModel) result += simModel;
			if (sim1Model) result += (result ? ' / ' : '') + 'CELL1: ' + sim1Model;
			if (sim2Model) result += (result ? ' / ' : '') + 'CELL2: ' + sim2Model;
			if (sim3Model) result += (result ? ' / ' : '') + 'CELL3: ' + sim3Model;
			if (sim4Model) result += (result ? ' / ' : '') + 'CELL4: ' + sim4Model;
			
			return result || '';
		} catch (e) {
			console.error('Failed to get model:', e);
			return '';
		}
	},
	
	_getCell_ver: function() {
		try {
			uci.load('system');
			var simVersion = uci.get('system', '@system[0]', 'module_version');
			var sim1Version = uci.get('system', '@system[0]', 'SIM1_version');
			var sim2Version = uci.get('system', '@system[0]', 'SIM2_version');
			var sim3Version = uci.get('system', '@system[0]', 'SIM3_version');
			var sim4Version = uci.get('system', '@system[0]', 'SIM4_version');
			
			var result = '';
			if (simVersion) result += simVersion;
			if (sim1Version) result += (result ? ' / ' : '') + 'CELL1: ' + sim1Version;
			if (sim2Version) result += (result ? ' / ' : '') + 'CELL2: ' + sim2Version;
			if (sim3Version) result += (result ? ' / ' : '') + 'CELL3: ' + sim3Version;
			if (sim4Version) result += (result ? ' / ' : '') + 'CELL4: ' + sim4Version;
			
			return result || '';
		} catch (e) {
			console.error('Failed to get version:', e);
			return '';
		}
	},
	
	_getAllSimTemps: function() {
		return Promise.all([
			fs.exec_direct('/sbin/uci', ['-P', '/var/state', '-q', 'get', 'cellular.SIM.TEMP']),
			fs.exec_direct('/sbin/uci', ['-P', '/var/state', '-q', 'get', 'cellular.SIM1.TEMP']),
			fs.exec_direct('/sbin/uci', ['-P', '/var/state', '-q', 'get', 'cellular.SIM2.TEMP']),
			fs.exec_direct('/sbin/uci', ['-P', '/var/state', '-q', 'get', 'cellular.SIM3.TEMP']),
			fs.exec_direct('/sbin/uci', ['-P', '/var/state', '-q', 'get', 'cellular.SIM4.TEMP'])
		]).then(function(temps) {
			var simTemps = [];
			
			temps.forEach(function(temp, index) {
				if (temp && temp.trim()) {
					var match = temp.trim().match(/^\+TEMP:\s*(\d+)\s*$/);
					if (match && match[1]) {
						simTemps.push({
							name: index === 0 ? 'CELL' : 'CELL' + index,
							value: match[1] + '°C'
						});
					}
				}
			});
			
			return simTemps;
		}).catch(function(error) {
			console.error('读取SIM卡温度失败:', error);
			return [];
		});
	},
	
	_preprocessCpuTemperature: function(content) {
		if (!content) return 'N/A';
		
		var temp_str = String(content).trim();
		if (!temp_str.match(/^\d+$/)) return 'N/A';
		
		var temp_num = parseInt(temp_str, 10);
		
		if (temp_num > 1000) {
			return (temp_num / 1000).toFixed(1);
		} else if (temp_num >= 20 && temp_num <= 120) {
			return temp_num.toString();
		} else {
			return 'N/A';
		}
	},
	
	_buildTemperatureDisplay: function(cpu, wifi0, wifi1, simTemps) {
		var parts = [];
		
		if (cpu && cpu !== 'N/A') {
			parts.push('CPU: (' + cpu + '°C)');
		}
		
		var wifiParts = [];
		if (wifi0 && wifi0 !== 'N/A') {
			wifiParts.push('WIFI-2.4G: (' + wifi0 + '°C)');
		}
		if (wifi1 && wifi1 !== 'N/A') {
			wifiParts.push('WIFI-5.8G: (' + wifi1 + '°C)');
		}
		
		if (wifiParts.length > 0) {
			parts.push(wifiParts.join(' / '));
		}
		
		if (simTemps && simTemps.length > 0) {
			var cellParts = [];
			simTemps.forEach(function(simTemp) {
				cellParts.push(simTemp.name + ': (' + simTemp.value + ')');
			});
			parts.push(cellParts.join(' / '));
		}
		
		return parts.length > 0 ? parts.join('   ') : 'N/A';
	},

	load: function() {
		return Promise.all([
			callSystemBoard().catch(function() { return {}; }),
			callSystemInfo().catch(function() { return {}; }),
			Promise.resolve(this._getF_ver()),
			Promise.resolve(this._getH_ver()),
			fs.exec_direct('/usr/sbin/slktool', ['/dev/mmcblk0p25', 'sn']).catch(function() { return ''; }),
			fs.trimmed('/sys/class/net/wifi0/thermal/temp').catch(function() { return ''; }),
			fs.trimmed('/sys/class/net/wifi1/thermal/temp').catch(function() { return ''; }),
			fs.trimmed('/sys/class/thermal/thermal_zone0/temp').catch(function() { return ''; }),
			this._getAllSimTemps()
		]);
	},

	render: function(data) {
		var boardinfo = data[0] || {};
		var systeminfo = data[1] || {};
		var f_ver = data[2];
		var h_ver = data[3];
		var sn = data[4] || '';
		var wifi0_temp = data[5] || '';
		var wifi1_temp = data[6] || '';
		var cpu_temp_raw = data[7] || '';
		var simTemps = data[8] || [];
		
		// 处理SN
		if (sn && sn.includes(':')) {
			sn = sn.substring(sn.indexOf(':') + 1).trim();
		}
		
		// 处理CPU温度
		var cpu_temp = this._preprocessCpuTemperature(cpu_temp_raw);
		
		var datestr = null;
		if (systeminfo.localtime) {
			var date = new Date(systeminfo.localtime * 1000);
			datestr = '%04d-%02d-%02d %02d:%02d:%02d'.format(
				date.getUTCFullYear(),
				date.getUTCMonth() + 1,
				date.getUTCDate(),
				date.getUTCHours(),
				date.getUTCMinutes(),
				date.getUTCSeconds()
			);
		}
		
		var theme = '';
		try {
			uci.load('luci');
			theme = uci.get('luci', 'main', 'mediaurlbase') || '';
		} catch (e) {
			console.error('Failed to get theme:', e);
		}
		
		var fields = [
			_('Hostname'), boardinfo.hostname || 'N/A',
			_('Model'), (!theme.includes('zx') ? this._getModel() : 'N/A'),
			_('Firmware Version'), f_ver,
			_('Hardware Version'), h_ver,
			_('Serial Number'), sn,
			_('Cellular Model'), this._getCellModel(),
			_('Cellular Version'), this._getCell_ver(),
			_('Local Time'), datestr || 'N/A',
			_('Uptime'), systeminfo.uptime ? '%t'.format(systeminfo.uptime) : 'N/A',
			_('Load Average'), Array.isArray(systeminfo.load) ? '%.2f, %.2f, %.2f'.format(
				systeminfo.load[0] / 65535.0,
				systeminfo.load[1] / 65535.0,
				systeminfo.load[2] / 65535.0
			) : 'N/A',
			_('Temperature'), this._buildTemperatureDisplay(cpu_temp, wifi0_temp, wifi1_temp, simTemps)
		];

		var table = E('table', { 'class': 'table' });

		for (var i = 0; i < fields.length; i += 2) {
			if (fields[i + 1] !== 'N/A' && fields[i + 1] !== '') {
				table.appendChild(E('tr', { 'class': 'tr' }, [
					E('td', { 'class': 'td left', 'width': '45%' }, [ fields[i] ]),
					E('td', { 'class': 'td left', 'width': '50%' }, [ fields[i + 1] ])
				]));
			}
		}

		return table;
	}
});