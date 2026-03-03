'use strict';
'require poll';
'require view';
'require dom';
'require form';
'require uci';
'require rpc';
'require network';
'require tools.cls-images as clsimg';

var callAppList = rpc.declare({
    object: 'clsqos',
    method: 'get_app_list',
    expect: { '': {} }
});

var callClmeshTopology = rpc.declare({
    object: 'clmesh.api',
    method: 'topology',
    expect: { '': {} }
});

var setAppToVIP = rpc.declare({
    object: 'clsqos',
    method: 'add_vip_app',
    params: ['app']
});

var setStaToVIP = rpc.declare({
    object: 'clsqos',
    method: 'add_vip_sta',
    params: ['mac'],
    expect: { result: 0 }
});

var delAppVIP = rpc.declare({
    object: 'clsqos',
    method: 'del_vip_app',
    params: ['app']
});

var delStaVip = rpc.declare({
    object: 'clsqos',
    method: 'del_vip_sta',
    params: ['mac'],
    expect: { result: 0 }
});

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
						type: interfaceData.type == 'ethernet'? 'Ethernet': 'Wi-Fi',
						lineStyle: {
							type: interfaceData.type == 'ethernet'? 'solid': null,
							color: 'black'
						},
						apps:[peer.apps[0], peer.apps[1]],
						vip: peer.vip
					};
					if (peer.interfaces) {
						child.children = parseTopology({interfaces: peer.interfaces});
					}
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
						apps:[client.apps[0], client.apps[1]],
						vip: client.vip,
						type: 'Wi-Fi',
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

function renderChartOption(data, chart) {
	let option = {
        tooltip: {
            show: true,
            trigger: 'item',
            triggerOn: 'click',
            renderMode: 'html',
            appendToBody: true,
			formatter: function(params) {
                if (params.data.name === 'internet') {
                    return;
                }else if (!params.data.parentName) {
                    var agentInfo = '<div>' +
                        'Node name: ' + 'TBD' + '<br>' +
                        'MAC address : ' + params.data.name.toUpperCase() + '<br>' +
                        'Link type: ' + 'Ethernet' + '<br>' +
                        '</div>';

                    return agentInfo;
                }else{
                    if (params.data.vip) {
                        var nodeInfo = '<div>' +
                            'Node name: ' + 'TBD' + '<br>' +
                            'MAC address : ' + params.data.name.toUpperCase() + '<br>' +
                            'Link type: ' + params.data.type + '<br>' +
                            'Mode: ' + '<select id="cls-tooltip"><option value="vip" selected=true>VIP</option><option value="normal">Normal</option></select>' + '<br>' +
                            'Status: ' + (params.data.apps[0] || params.data.apps[1]? params.data.apps[0] && params.data.apps[1]? params.data.apps[0] + params.data.apps[1]: params.data.apps[0]? params.data.apps[0]: params.data.apps[1]: 'Idle') + '<br>' +
                            '</div>';
                    } else {
                        var nodeInfo = '<div>' +
                            'Node name: ' + 'TBD' + '<br>' +
                            'MAC address : ' + params.data.name.toUpperCase() + '<br>' +
                            'Link type: ' + params.data.type + '<br>' +
                            'Mode: ' + '<select id="cls-tooltip"><option value="vip">VIP</option><option value="normal" selected=true>Normal</option></select>' + '<br>' +
                            'Status: ' + (params.data.apps[0] || params.data.apps[1]? params.data.apps[0] && params.data.apps[1]? params.data.apps[0] + params.data.apps[1]: params.data.apps[0]? params.data.apps[0]: params.data.apps[1]: 'Idle') + '<br>' +
                            '</div>';
                    }

                    return nodeInfo;
                }
            },
			extraCssText: 'pointer-events: auto;',
			enterable: true,
        },
        series: [
            {
                type: 'tree',
                id: 0,
                name: 'tree1',
                data: [data],
                top: '10%',
                left: '8%',
                bottom: '22%',
                right: '20%',
                symbolSize: 7,
                edgeForkPosition: '63%',
                expandAndCollapse: false,
                initialTreeDepth: 3,
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


function updateTopologyTree(data, chart) {
	var newTopologyTree = parseTopology(data, true);
	
	var option = chart.getOption();
	
	option.series[0].data = [];
	option.series[0].data.push(newTopologyTree);

	chart.setOption(option, true);
}

function startPolling(myChart) {
    var updateOnlineList = function() {
        return network.flushCache().then(function() {
            return Promise.all([
                callAppList(),
                callClmeshTopology()
            ]);
        }).then(function(results) {
            var applist = results[0].apps;
            var chartInfo = [];
            var rows = [];

            for (let i = 0; i < applist.length; i++) {
                rows.push([
                    E('span', {'style': s}, applist[i].name),
                    E('select', {
                        'id': 'cls-'+applist[i].name,
                        'change': function(event){
                            var selectedOption = event.target.value;
                            
                            if (selectedOption == 'normal')
                                delAppVIP(applist[i].name);
                            else
                                setAppToVIP(applist[i].name);
                        }}, [
                                E('option', {'value': applist[i].vip? 'vip': 'normal'}, applist[i].vip? 'VIP mode': 'Normal mode'),
                                E('option', {'value': applist[i].vip? 'normal': 'vip'}, applist[i].vip? 'Normal mode': 'VIP mode')
                            ]
                    )
                ]);
            }

            var table = document.querySelector('#online_applications_table');

            cbi_update_table(table, rows);

			chartInfo.push(myChart);
			chartInfo.push(results[1].network);

			return chartInfo;
        }).then(function(chartInfo) {
			var topology = chartInfo[1],
				myChart = chartInfo[0];

			updateTopologyTree(topology, myChart);
		});
    };

    return updateOnlineList().then(function() {
        poll.add(updateOnlineList);
    });
}

function handleModeChange(mode, mac) {
    if (mode === 'vip')
        setStaToVIP(mac);
    if (mode === 'normal')
        delStaVip(mac);
}

return view.extend({
    load: function() {
        return Promise.all([
                callClmeshTopology()
        ]);
    },

    render: function(initialData) {
        var table = E('table', { 'class': 'table online', 'id': 'online_applications_table' }, [
            E('tr', { 'class': 'tr table-titles' }, [
                E('th', { 'class': 'th nowrap' }, _('APP type')),
                E('th', { 'class': 'th hide-xs' }, _('State'))
            ])
        ]);

        var chartDiv = E('div', {'class':'chart-div', 'style': 'height:500px;width:900px'});
        var myChart = echarts.init(chartDiv);
        var option;

        var topologyTree = parseTopology(initialData[0].network, false);

        renderChartOption(topologyTree, myChart);

        myChart.on('click', function(params) {
            if (params.data.name === 'internet' || !params.data.parentName)
                return;

            document.getElementById('cls-tooltip').addEventListener('change', function(event) {
                handleModeChange(event.target.value, params.data.name);
            });
        });

        return startPolling(myChart).then(function() {
                return E([E('h3', _('Online applications')), table, E('br'), E('h3', _('Topology')), chartDiv]);
        });
    },

	handleSaveApply: null,
	handleSave: null,
	handleReset: null
});
