'use strict';
'require view';
'require dom';
'require rpc';
'require network';
'require poll';
'require tools.cls-images as clsimg';

const callClmeshTopology = () => rpc.declare({
    object: 'clmesh.api',
    method: 'topology',
    expect: { '': {} }
})();

const callDHCPLeases = () => rpc.declare({
    object: 'luci-rpc',
    method: 'getDHCPLeases',
    expect: { '': {} }
})();

const parseTopology = (obj) => {
    const result = {
        mac: obj.al_mac,
        symbol: obj.service ? `image://${clsimg.lookupImage('gateway')}` : `image://${clsimg.lookupImage('laptop')}`,
        symbolSize: [60, 60],
        ip: obj.ipv4 || null,
        lineStyle: { type: 'solid', color: 'black' },
        children: []
    };

    obj.interfaces?.forEach(({ type, peers = [], clients = [] }) => {
        peers.forEach(peer => {
            result.children.push(peer.interfaces
                ? parseTopology(peer)
                : {
                    mac: peer.al_mac,
                    type,
                    ip: peer.ipv4 || null,
                    symbol: peer.service ? `image://${clsimg.lookupImage('gateway')}` : `image://${clsimg.lookupImage('laptop')}`,
                    symbolSize: [60, 60],
                    lineStyle: {
                        type: type === 'ethernet' ? 'solid' : 'dashed',
                        color: type === 'ethernet' ? 'black' : 'green'
                    },
                    children: []
                }
            );
        });

        clients.forEach(({ mac, ul_rate, dl_rate }) => {
            result.children.push({
                mac,
                ul_rate: +(ul_rate / 1024).toFixed(1),
                dl_rate: +(dl_rate / 1024).toFixed(1),
                symbol: `image://${clsimg.lookupImage('phone')}`,
                symbolSize: [60, 60],
                type: 'wifi',
                lineStyle: { type: 'dashed', color: 'green' },
                children: []
            });
        });
    });

    return result;
};

const addInternetNode = ({ mac, ip, children }) => ({
    name: 'Internet',
    symbol: `image://${clsimg.lookupImage('root')}`,
    symbolSize: [280, 80],
    mac,
    ip,
    label: {},
    children
});

const getMaxBreadth = (node, level = 0, levelCount = {}) => {
    levelCount[level] = (levelCount[level] || 0) + 1;
    node.children?.forEach(child => getMaxBreadth(child, level + 1, levelCount));
    return Math.max(...Object.values(levelCount));
};

const renderTopology = (tree, chart) => {
    const option = {
        tooltip: {
            trigger: 'item',
            formatter: ({ data }) => {
                const { mac = 'N/A', ip, hostname, ul_rate, dl_rate } = data;
                return [
                    `MAC: ${mac}`,
                    ip ? `IP: ${ip}` : '',
                    hostname ? `Host: ${hostname}` : '',
                    (ul_rate !== undefined && dl_rate !== undefined) ? `上行: ${ul_rate} ↑ Mbps<br>下行: ${dl_rate} ↓ Mbps` : ''
                ].filter(Boolean).join('<br>');
            }
        },
        series: [{
            type: 'tree',
            data: [tree],
            top: '1%',
            left: '10%',
            symbolSize: 7,
            nodePadding: 50,
            initialTreeDepth: -1,
            orient: 'LR',
            lineStyle: { width: 2, curveness: 0.5 },
            label: { show: false },
            edgeShape: 'curve',
            edgeForkPosition: 1,
            emphasis: { focus: 'none' },
            expandAndCollapse: false,
            animationDuration: 500,
            animationDurationUpdate: 500
        }]
    };

    chart.setOption(option);
};

const matchDHCPInfo = (tree, leasesMap) => {
    const lease = leasesMap[tree.mac?.toUpperCase()];
    if (lease) {
        tree.ip = lease.ipaddr;
        tree.hostname = lease.hostname;
    }
    tree.children?.forEach(child => matchDHCPInfo(child, leasesMap));
};

const drawTopology = (data, leases, chart, elem) => {
    const topo = parseTopology(data.network);
    const tree = addInternetNode(topo);

    const leasesMap = Object.fromEntries(leases.map(l => [l.macaddr.toUpperCase(), l]));

    matchDHCPInfo(tree, leasesMap);

    const maxBreadth = getMaxBreadth(tree);
    elem.style.height = `${maxBreadth * 60 + 2000}px`;

    chart.resize();
    renderTopology(tree, chart);
};

const startPolling = (chart, elem) => {
    const updateTopology = () =>
        network.flushCache()
            .then(() => Promise.all([callClmeshTopology(), callDHCPLeases()]))
            .then(([topology, { dhcp_leases }]) => drawTopology(topology, dhcp_leases, chart, elem));

    return updateTopology().then(() => poll.add(updateTopology, 5));
};

return view.extend({
    load: () => Promise.all([callClmeshTopology(), callDHCPLeases()]),

    render: ([topologyData, { dhcp_leases }]) => {
        const container = E('div', { 'class': 'cbi-section' }, [
            E('div', { 'class': 'topo-section', 'style': 'width: 100%; height: 600px; overflow: auto;' })
        ]);

        const topoElem = container.querySelector('.topo-section');

        const observer = new ResizeObserver(entries => {
            for (const { contentRect } of entries) {
                if (contentRect.width > 0 && contentRect.height > 0) {
                    observer.disconnect();
                    const chart = echarts.init(topoElem);
                    drawTopology(topologyData, dhcp_leases, chart, topoElem);
                    startPolling(chart, topoElem);
                }
            }
        });

        observer.observe(topoElem);

        return container;
    },

    handleReset: null,
    handleSave: null,
    handleSaveApply: null
});

