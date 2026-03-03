'use strict';
'require baseclass';
'require view';
'require rpc';
'require network';
'require poll';
'require uci';
'require dom';
'require tools.cls-images as clsimg';

document.querySelector('head').appendChild(E('link', {
	'rel': 'stylesheet',
	'type': 'text/css',
	'href': L.resource('view/overview/css/overview.css')
}));

var callSystemBoard = rpc.declare({
	object: 'system',
	method: 'board',
	expect: { '': {} }
});

var callSystemInfo = rpc.declare({
	object: 'system',
	method: 'info',
	expect: { '': {} }
});

var callNetworkDevice = rpc.declare({
	object: 'network.device',
	method: 'status',
	parmas: [ "device" ],
	expect: { '': {} }
});

var callNetworkInterface = rpc.declare({
	object: 'network.interface',
	method: 'dump',
	expect: { '': {} }
});

var callClmeshTopology = rpc.declare({
	object: 'clmesh.api',
	method: 'topology',
	expect: { '': {} }
});

var callDHCPLeases = rpc.declare({
	object: 'luci-rpc',
	method: 'getDHCPLeases',
	expect: { '': {} }
});

function formatTime(seconds) {
	let days = Math.floor(seconds / (24 * 3600)),
		hours = Math.floor(seconds % (24 * 3600) / 3600),
		minutes = Math.floor(seconds % 3600 / 60);

	if (days == 0 && hours == 0) {
		return minutes + _('minutes');
	} else if (days == 0) {
		return hours + _('hours') + ' ' + minutes + _('minutes');
	} else if (hours == 0) {
		return days + _('days') + ' ' + minutes + _('minutes');
	} else {
		return days + _('days') + ' ' + hours + _('hours') + ' ' + minutes + _('minutes');
	}
}

function getClientNumber(iface) {
	let cunt = 0;
	iface.forEach(function(subObj) {
		if (subObj.peers) {
			subObj.peers.forEach(function(subPeer) {
				cunt++;
				if (subPeer.interfaces)
					cunt = cunt + getClientNumber(subPeer.interfaces);
			});
		} else if (subObj.clients) {
			cunt = cunt + subObj.clients.length;
		}
	});

	return cunt;
}

function parseTopology(obj) {
	let result = {
		name: obj.al_mac,
		symbol: obj.type? null: 'image://'+clsimg.lookupImage('gateway'),
		symbolSize: [60, 60],
		children: []
	};

	if (obj.interfaces) {
		obj.interfaces.forEach((interfaceData) => {
			if (interfaceData.peers) {
				interfaceData.peers.forEach((peer) => {
					let child = {
						parentName: interfaceData.mac,
						name: peer['al_mac'],
						symbol: interfaceData.type == 'ethernet'? 'image://'+clsimg.lookupImage('laptop'): null,
						symbolSize: [60, 60],
						lineStyle: {
							type: interfaceData.type == 'ethernet'? 'solid': null,
							color: 'black'
						}
					};

					if (peer.interfaces)
						child.children = parseTopology({interfaces: peer.interfaces});

					result.children.push(child);
				});
			}

			if (interfaceData.clients) {
				interfaceData.clients.forEach((client) => {
					let child = {
						parentName: interfaceData.mac,
						name: client.mac,
						symbol: interfaceData.type == 'wifi'? 'image://'+clsimg.lookupImage('phone'):null,
						symbolSize: [60, 60],
						lineStyle: {
							type: interfaceData.type == 'wifi'? 'dashed': 'solid',
							color: 'green'
						}
					};

					result.children.push(child);
				});
			}
		})
	}

	var addInternetNode = {
		name:'internet',
		symbol: 'image://'+clsimg.lookupImage('internet'),
		symbolSize:[60, 60],
		children: [result]
	};

	return addInternetNode;
}

let dhcpLeases = {};

function getHostName(mac) {
	var dhcpleases = dhcpLeases.dhcp_leases, parts;
	var errorMsg = _('Unknow');

	mac = mac.toUpperCase();
	parts = mac.split(/[:-]/);

	for (var i = 0; i < parts.length; i++) {
		if (parts[i].length == 1)
			parts[i] = '0' + parts[i];
	}

	mac = parts.join(':');
	dhcpleases.forEach(function(subLease) {
		if (subLease.macaddr == mac && subLease.hostname)
			return subLease.hostname;
	});

	return errorMsg;
}

function renderChartOption(data, chart) {
	let option = {
		tooltip: {
			show: true,
			trigger: 'item',
			triggerOn: 'click',
			formatter: function(params) {
				if (params.name != 'internet') {
					return '<div>'+
						_('Host Name') + ': ' +  getHostName(params.name) + '<br>' +
						_('MAC Address') + ': ' + params.name + '<br>' +
						_('Link Type') + ': ' + (params.data.parentName? params.data.lineStyle.type == 'solid'? 'Ethernet': 'Wi-Fi': 'Ethernet') + '<br>' +
						'</div>';
				}
			},
			extraCssText: 'pointer-events: auto;',
			enterable: true,
		},
		title: {
			text: _('Topology'),
			top: '10px',
			left: '15px',
			textStyle: { fontFamily: 'sans-serif' }
		},
		series: [
			{
				type: 'tree',
				id: 0,
				name: 'tree1',
				data: [data],
				symbolSize: 7,
				edgeForkPosition: '30%',
				expandAndCollapse: false,
				initialTreeDepth: 3,
				levels: [
					{
						siblingDistance: 20,
					}
				],
				lineStyle: {
					width: 2
				},
				label: {
					backgroundColor: '#fff',
					position: 'left',
					verticalAlign: 'middle',
					align: 'right',
					show: false
				},
				orient: 'vertical',
				leaves: {
					label: {
						position: 'right',
						verticalAlign: 'middle',
						align: 'left'
					}
				},
				emphasis: {
					focus: 'none'
				},
				expandAndCollapse: false,
				animationDuration: 550,
				animationDurationUpdate: 750
			}
		]
	};

	chart.setOption(option);
}

function setTrianglePosition(flag) {
	let offset,
		cbiSectionOffset = document.getElementById('cbi-section').offsetWidth,
		triangle = document.getElementById('triangle-up');
	
	if (flag == 'netopo-network')
		offset = cbiSectionOffset*0.16;
	else if (flag == 'netopo-client')
		offset = cbiSectionOffset*0.81;
	else if (flag == 'netopo-router')
		offset = cbiSectionOffset*0.49;

	triangle.style['margin-left'] = offset + 'px';
}

function showNetworkInfo() {
	let net = document.getElementById('network-info'),
		rot = document.getElementById('router-info'),
		cli = document.getElementById('client-info');

	setTrianglePosition('netopo-network');

	net.classList.remove('hidden');
	rot.classList.contains('hidden')? '': rot.classList.add('hidden');
	cli.classList.contains('hidden')? '': cli.classList.add('hidden');
}

function showRouterInfo() {
	let net = document.getElementById('network-info'),
		rot = document.getElementById('router-info'),
		cli = document.getElementById('client-info');

	setTrianglePosition('netopo-router');

	rot.classList.remove('hidden');
	net.classList.contains('hidden')? '': net.classList.add('hidden');
	cli.classList.contains('hidden')? '': cli.classList.add('hidden');
}

function showClientTopology() {
	let net = document.getElementById('network-info'),
		rot = document.getElementById('router-info'),
		cli = document.getElementById('client-info'),
		chart = document.getElementById('cli-topology');

	setTrianglePosition('netopo-client');

	cli.classList.remove('hidden');
	rot.classList.contains('hidden')? '': rot.classList.add('hidden');
	net.classList.contains('hidden')? '': net.classList.add('hidden');

	echarts.getInstanceByDom(chart).resize();
}

function initializeTableData(elem, data) {
	let td = elem.querySelectorAll('td');
	let tdValue = {};
	
	data[0].interface.forEach(function(subObj) {
		if (subObj.interface == 'wan') {
			tdValue.linktype = subObj.proto.toUpperCase();
			tdValue.ip = subObj['ipv4-address']? subObj['ipv4-address'][0]? subObj['ipv4-address'][0].address: _('Unknow'): _('Unknow');
			tdValue.dns = subObj['dns-server']? subObj['dns-server'][0]? subObj['dns-server'][0]: _('Unknow'): _('Unknow');
			tdValue.gateway = subObj.data.dhcpserver? subObj.data.dhcpserver: _('Unknow');
		} else if (subObj.interface == 'wan6')
			tdValue.linktype_v6 = subObj.proto;			
	})

	tdValue.hwversion = data[2].board_name;
	tdValue.softversion = data[2].release.version;
	tdValue.mac = data[1].eth4.macaddr.toUpperCase();
	tdValue.uptime = data[3].uptime;
	tdValue.uplinkbw = 0;
	tdValue.downlinkbw = 0;
	
	td[0].textContent = tdValue.linktype;
	td[1].textContent = tdValue.ip;
	td[2].textContent = tdValue.dns;
	td[3].textContent = tdValue.gateway;
	td[4].textContent = tdValue.hwversion;
	td[5].textContent = tdValue.softversion;
	td[6].textContent = tdValue.mac;
	td[7].textContent = formatTime(tdValue.uptime);
}

function drawNetworkState(chart, data) {
	let option = {
		tooltip: {
			trigger: 'axis',
			axisPointer: {
				type: 'cross',
				label: {
					backgroundColor: '#6a7985'
				}
			},
			show: false
		},
		grid: {
			left: '3%',
			right: '4%',
			bottom: '3%',
			containLabel: true
		},
		legend: {
			data: [_('Uplink Rate'), _('Downlink Rate')]
		},
		xAxis: [
			{
				type: 'category',
				boundaryGap: false,
				data: []
			}
		],
		yAxis: [
			{
				type: 'value',
				axisLabel: { formatter: '{value} kbps' }
			}
		],
		series: [
			{
				name: _('Uplink Rate'),
				type: 'line',
				areaStyle: {},
				data: []
			},
			{
				name: _('Downlink Rate'),
				type: 'line',
				areaStyle: {},
				data: []
			}
		]
	};

	chart.setOption(option);
	
	setTimeout(function() {
		chart.resize();
	}, 800);
	
	return chart;
}

function drawStorageState(chart, data) {
	let option = {
		legend: {
			orient: 'vertical',
			left: '60%',
			top: 'center',
			data: ['Free', 'Used']
		},
		series: [
			{
				type: 'pie',
				radius: '70%',
				right: '40%',
				labelLine: { show: false },
				label: { show: false },
				data: [],
				emphasis: {
					itemStyle: {
						shadowBlur: 10,
						shadowOffsetX: 0,
						shadowColor: 'rgba(0, 0, 0, 0.5)'
					}
				}
			}
		]
	};

	chart.setOption(option);
	
	setTimeout(function() {
		chart.resize();
	}, 700);
}

function drawCPUState(chart, data) {	
	let option = {
		tooltip: {
			trigger: 'axis',
			axisPointer: {
				type: 'cross',
				label: {
					backgroundColor: '#6a7985'
				}
			}
		},
		grid: {
			left: '3%',
			right: '4%',
			bottom: '3%',
			containLabel: true
		},
		xAxis: [
			{
				type: 'category',
				boundaryGap: false,
				data: []
			}
		],
		yAxis: [
			{
				type: 'value',
				axisLabel: { formatter: '{value} %' }
			}
		],
		legend: {
			data: [_('Average load within 1 minute'), _('Average load within 5 minutes'), _('Average load within 15 minutes')]
		},
		series: [
			{
				name: _('Average load within 1 minute'),
				type: 'line',
				areaStyle: {},
				data: []
			},
			{
				name: _('Average load within 5 minutes'),
				type: 'line',
				areaStyle: {},
				data: []
			},
			{
				name: _('Average load within 15 minutes'),
				type: 'line',
				areaStyle: {},
				data: []
			}
		]
	};

	chart.setOption(option);
	
	setTimeout(function() {
		chart.resize();
	}, 700);
	
	return chart;
}

function drawClientTopology(chart, data) {
	let tree = parseTopology(data[4].network);
		
	renderChartOption(tree, chart);

	setTimeout(function() {
		chart.resize();
	}, 700);

	return chart;
}

let uplinkData = [], downlinkData = [], time = [];
let newUp, oldUp, newDown, oldDown;

function updateNetworkState(chart, data) {
	let option = chart.getOption();
	let rate;

	newUp = data.eth4.statistics.tx_bytes;
	newDown = data.eth4.statistics.rx_bytes;

	//update uplink data
	if (oldUp == 'undefined')
		rate = 0;
	else
		rate = Math.floor((newUp - oldUp)/1024);

	if (uplinkData.length == 10) {
		uplinkData.shift();
		uplinkData.push(rate);
	} else
		uplinkData.push(rate);
    
	//update downlink data
	if (oldDown == 'undefined')
		rate = 0;
	else
		rate = Math.floor((newDown - oldDown)/1024);

	if (downlinkData.length == 10) {
		downlinkData.shift();
		downlinkData.push(rate);
	} else
		downlinkData.push(rate);

	oldUp = newUp;
	oldDown = newDown;

	option.series[0].data = uplinkData;
	option.series[1].data = downlinkData; 

	chart.setOption(option, true);
}

function updateStorageState(chart, data) {
	let option = chart.getOption();
	let storageFree = {}, storageUsed = {};

	storageFree = {
		value: data.root.free,
		name: 'Free'
	};

	storageUsed = {
		value: data.root.used,
		name: 'Used'
    };
    
	option.series[0].data = [];
	option.series[0].data.push(storageFree);
	option.series[0].data.push(storageUsed);

	chart.setOption(option, true);
}

let loadavg_1min = [], loadavg_5min = [], loadavg_15min = [];

function updateCPUState(chart, data) {
	let option = chart.getOption();

	if (loadavg_1min.length == 10) {
		loadavg_1min.shift();
		loadavg_5min.shift();
		loadavg_15min.shift();
		loadavg_1min.push(Math.round((data.load[0]/65535)*100));
		loadavg_5min.push(Math.round((data.load[1]/65535)*100));
		loadavg_15min.push(Math.round((data.load[2]/65535)*100));
	} else {
		loadavg_1min.push(Math.round((data.load[0]/65535)*100));
		loadavg_5min.push(Math.round((data.load[1]/65535)*100));
		loadavg_15min.push(Math.round((data.load[2]/65535)*100));
	}
  

	option.series[0].data = loadavg_1min;
	option.series[1].data = loadavg_5min;
	option.series[2].data = loadavg_15min;
	option.xAxis[0].data = time; 

	chart.setOption(option, true);
}

function updateClientInfo(chart, data) {}

function updateNetopo(data, elem) {
	let connect;
	let point = elem.querySelector('.icon-dian'),
		spanNet = elem.querySelector('.span-net'),
		line = elem.querySelector('.line'),
		ssidSpan = elem.querySelector('.span-2g'),
		ssidSpan_5g = elem.querySelector('.span-5g'),
		cliNumSpan = elem.querySelector('.span-cli-num');

	data[1].interface.forEach(function(subObj) {
		if (subObj.interface == 'wan')
			connect = subObj.up;
	});

	if (connect) {
		point.classList.remove('dian-off');
		point.classList.add('dian-on');
		line.classList.remove('line-err');
		line.classList.add('line-ok');

		spanNet.innerText = _('Connected');
	} else {
		point.classList.remove('dian-on');
		point.classList.add('dian-off');
		line.classList.remove('line-ok');
		line.classList.add('line-err');

		spanNet.innerText = _('Disconnect');
	}

	if (data[2].wlan0.up) {
		elem.querySelector('.dian-2g').classList.remove('dian-off');
		elem.querySelector('.dian-2g').classList.add('dian-on');
	} else {
		elem.querySelector('.dian-2g').classList.add('dian-off');
		elem.querySelector('.dian-2g').classList.remove('dian-on');
	}

	if (data[2].wlan1.up) {
		elem.querySelector('.dian-5g').classList.remove('dian-off');
		elem.querySelector('.dian-5g').classList.add('dian-on');
	} else {
		elem.querySelector('.dian-5g').classList.add('dian-off');
		elem.querySelector('.dian-5g').classList.remove('dian-on');
	}

	uci.sections('wireless').forEach(function(subObj) {
		if (subObj['.name'] == 'default_radio0')
			ssidSpan.innerText = subObj.ssid;
		else if (subObj['.name'] == 'default_radio1')
			ssidSpan_5g.innerText = subObj.ssid;
	});

	cliNumSpan.innerText = getClientNumber(data[3].network.interfaces);
}

return view.extend({
	load: function() {
		return Promise.all([
			callNetworkInterface(),
			callNetworkDevice(),
			callSystemBoard(),
			callSystemInfo(),
			callClmeshTopology()
		]);
	},

	startPolling: function(myChart, elem) {
		var updateOverview = function() {
			return network.flushCache().then(function() {
				return Promise.all([
					callSystemInfo(),
					callNetworkInterface(),
					callNetworkDevice(),
					callClmeshTopology(),
					callDHCPLeases()
				]);
			}).then(function(results) {
				dhcpLeases = results[4];
				updateNetworkState(myChart[0], results[2]);
				updateStorageState(myChart[1], results[0]);
				updateCPUState(myChart[2], results[0]);
				updateClientInfo(myChart[3], results[3]);
				updateNetopo(results, elem);
			});
		};

		return updateOverview().then(function() {
			poll.add(updateOverview);
		});
	},

	renderHtml: function(data) {
		let container = E('div');

		dom.content(container, [
			E('div', { 'class': 'cbi-section', 'id': 'cbi-section', 'style': 'display: flex; flex-direction: row;align-items: center;' }, [
				E('div', { 'class': 'netopo-network', 'id': 'netopo-network', 'style': 'border-radius: 5px;' }, [
					E('span', { 
						'class': 'icon-internet-1 cls-75px cls-green',
						'click': showNetworkInfo
					}),
					E('p', _('Internet')),
					E('div', { 'class': 'item-stat' }, [
						E('div', { 'class': 'stat-info stat-net-info' }, [
							E('span', { 'class': 'icon-dian dian-on' }),
							E('span', { 'class': 'span-net' }, _('Connected'))
						])
					])
				]),
				E('div', { 'id': 'net-to-dev', 'class': 'line line-ok'}),
				E('div', { 'class': 'netopo-router', 'id': 'netopo-router', 'style': 'border-radius: 5px;' }, [
					E('span', { 
						'class': 'icon-router cls-70px focus-on',
						'click': showRouterInfo
					}),
					E('p', _('Router')),
					E('div', { 'class': 'item-stat' }, [
						E('div', { 'class': 'stat-info stat-2g-info' }, [
							E('span', { 'class': 'icon-dian dian-2g dian-on' }),
							E('span', {}, _('2.4G: ')),
							E('span', { 'class': 'span-2g' }, _('Clourney'))
						]),
						E('div', { 'class': 'stat-info stat-5g-info' }, [
							E('span', { 'class': 'icon-dian dian-5g dian-off' }),
							E('span', {}, _('5G: ')),
							E('span', { 'class': 'span-5g' }, _('Clourney-5G'))
						]),
					]),
				]),
				E('div', { 'id': 'dev-to-cli', 'class': 'line line-ok'}),
				E('div', { 'class': 'netopo-client', 'id': 'netopo-client', 'style': 'border-radius: 5px;' }, [
					E('span', {
						'class': 'icon-phone cls-70px',
						'click': showClientTopology
					}),
					E('p', _('Clients')),
					E('div', { 'class': 'item-stat' }, [
						E('div', { 'class': 'stat-info stat-cli-info' }, [
							E('span', _('Number: ')),
							E('span', { 'class': 'span-cli-num' }, _('1'))
						])
					])
				])
			]),
			E('div', { 'class': 'triangle-up', 'id': 'triangle-up', 'style': 'margin-left: 49%;' }),
			E('div', { 'id': 'network-info', 'class': 'cbi-section hidden' }, [
				E('div', { 'class': 'net-container' }, [
					E('div', { 'class': 'item net-state' }, [
						E('h3', _('Network State')),
						E('table', [
							E('tr', [ E('th', _('Link Type')), E('td', _('Unknow')) ]),
							E('tr', [ E('th', _('IP Address')), E('td', _('Unknow')) ])
						])
					]),
					E('div', { 'class': 'item net-state part-of-netstat' }, [
						E('h3', _('  ')),
						E('table', [
							E('tr', [ E('th', _('DNS')), E('td', _('Unknow')) ]),
							E('tr', [ E('th', _('Gateway')), E('td', _('Unknow')) ])
						])
					]),
				])
			]),
			E('div', { 'id': 'router-info', 'class': 'cbi-section' }, [
				E('div', { 'class': 'router-container' }, [
					E('div', { 'class': 'item rot-info' }, [
						E('h3', _('Device Info')),
						E('table', [
							E('tr', [ E('th', _('Hardware Version')), E('td', _('Unknow')) ]),
							E('tr', [ E('th', _('Software Version')), E('td', _('Unknow')) ]),
							E('tr', [ E('th', _('MAC Address')), E('td', _('Unknow')) ]),
							E('tr', [ E('th', _('Up Time')), E('td', _('Unknow')) ])
						])
					]),
					E('div', { 'class': 'item rot-rate' }, [
						E('h3', _('Realtime Network State')),
						E('div', { 'id': 'realtime-net-state', 'style': 'width: 100%; height: 100%;' })
					]),
					E('div', { 'class': 'item rot-client' }, [
						E('h3', _('Realtime Storage State')),
						E('div', { 'id': 'realtime-storage-state', 'style': 'width: 100%; height: 100%;' })
					]),
					E('div', { 'class': 'item rot-cpu' }, [
						E('h3', _('Realtime CPU State')),
						E('div', { 'id': 'realtime-cpu-state', 'style': 'width: 100%; height: 100%;' })
					]),
				])
			]),
			E('div', { 'id': 'client-info', 'class': 'cbi-section hidden' }, [
				E('div', { 'id': 'cli-topology', 'style': 'width: 100%; height: 600%;' })
			])
		]);

		initializeTableData(container, data);
		
		let netChart = echarts.init(container.querySelector('#realtime-net-state'), 'light'),
			storageChart = echarts.init(container.querySelector('#realtime-storage-state'), 'light'),
			cpuChart = echarts.init(container.querySelector('#realtime-cpu-state'), 'light'),
			topoChart = echarts.init(container.querySelector('#cli-topology'), 'light');

		let chart = [netChart, storageChart, cpuChart, topoChart];

		drawNetworkState(netChart, data);
		drawStorageState(storageChart, data);
		drawCPUState(cpuChart, data);
		drawClientTopology(topoChart, data);

		return this.startPolling(chart, container).then(function() {
			return container;
		});
	},

	render: function(data) {
		return Promise.all([
			this.renderHtml(data)
		]);
	},

	handleSave: null,
	handleSaveApply: null,
	handleReset: null
})
