'use strict';
'require view';
'require dom';
'require rpc';
'require network';
'require poll';
'require tools.cls-images as clsimg';

const parseTopology = (obj, type) => {
    const result = {
        mac: obj.al_mac,
        symbol: obj.service ? `image://${clsimg.lookupImage('gateway')}` : `image://${clsimg.lookupImage('laptop')}`,
        symbolSize: [60, 60],
        type,
        ip: obj.ipv4 || null,
        lineStyle: { type: 'solid', color: 'black' },
        children: []
    };

    obj.interfaces?.forEach(({ type: ifaceType, peers = [], clients = [] }) => {
        peers.forEach(peer => {
            if (peer.interfaces) {
                result.children.push(parseTopology(peer, 'ethernet'));
            } else {
                result.children.push({
                    mac: peer.al_mac,
                    type: ifaceType,
                    ip: peer.ipv4 || null,
                    symbol: peer.service ? `image://${clsimg.lookupImage('gateway')}` : `image://${clsimg.lookupImage('laptop')}`,
                    symbolSize: [60, 60],
                    lineStyle: {
                        type: ifaceType === 'ethernet' ? 'solid' : 'dashed',
                        color: ifaceType === 'ethernet' ? 'black' : 'green'
                    },
                    children: []
                });
            }
        });

        clients.forEach(({ mac, ul_rate, dl_rate }) => {
            result.children.push({
                mac,
                ul_rate,
                dl_rate,
                symbol: `image://${clsimg.lookupImage('phone')}`,
                symbolSize: [60, 60],
                type: 'wifi',
                lineStyle: { type: 'dashed', color: 'green' }
            });
        });
    });

    return result;
};

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

return view.extend({
    load() {
        return Promise.all([callClmeshTopology(), callDHCPLeases()]);
    },

    render(data) {
        const [clmeshData, { dhcp_leases }] = data;
        return this.renderList(clmeshData, dhcp_leases);
    },

    renderList(clmeshData, dhcpLeases) {
        let allClients = [];
        let currentPage = 1;
        const pageSize = 10;
        let currentFilter = 'all';
        let currentKeyword = '';

        const container = E('div', { class: 'cbi-section' });

        const controls = E('div', { style: 'margin-bottom: 5px; padding-top: 5px; padding-left: 5px;' }, [
            E('select', { id: 'typeFilter', style: 'margin-right: 10px;' }, [
                E('option', { value: 'all' }, _('All')),
                E('option', { value: 'wired' }, _('Wired link')),
                E('option', { value: 'wireless' }, _('Wi-Fi link')),
            ]),
            E('input', { type: 'text', id: 'searchInput', placeholder: _('Input hostname or MAC'), style: 'margin-right: 5px;' }),
            E('button', { id: 'searchBtn' }, _('Search'))
        ]);

        const table = E('table', { class: 'table', style: 'width:100%; margin-top: 10px;' });
        const thead = E('thead', { style: 'text-align: left;' }, [
            E('tr', {}, [
                E('th', {}, _('Hostname')),
                E('th', {}, _('Rate')),
                E('th', {}, _('MAC')),
                E('th', {}, _('IP')),
                E('th', {}, _('Link Type')),
            ])
        ]);
        const tbody = E('tbody');

        const pagination = E('div', { style: 'margin-top: 10px; display: flex; align-items: center; gap: 10px; position: absolute; right: 2%;' });
        const prevBtn = E('button', {}, '←');
        const nextBtn = E('button', {}, '→');
        const pageInfo = E('span');
        const pageInput = E('input', { type: 'number', min: 1, style: 'width: 70px;' });

        pagination.append(prevBtn, pageInfo, nextBtn, E('span', {}, _('Jump to page') + ':'), pageInput);

        table.append(thead, tbody);
        container.append(controls, table, pagination);

        const seenMACs = new Set();

        const traverse = (nodes, ctrlMac) => {
            nodes.forEach(node => {
                if (node?.mac && !seenMACs.has(node.mac.toLowerCase())) {
                    seenMACs.add(node.mac.toLowerCase());

                    const client = {
                        name: 'N/A',
                        mac: node.mac === ctrlMac ? `${node.mac}` + '(' + _('Local device') + ')' : node.mac,
                        ip: node.ip || 'N/A',
                        type: node.type === 'wifi' ? 'wireless' : 'wired',
                        up: node.ul_rate ? `${(node.ul_rate / 1024).toFixed(1)}Mbps` : '0 Mbps',
                        down: node.dl_rate ? `${(node.dl_rate / 1024).toFixed(1)}Mbps` : '0 Mbps'
                    };

                    allClients.push(client);

                    if (node.children?.length) {
                        traverse(node.children, ctrlMac);
                    }
                }
            });
        };

        const updateClientList = (clmeshData, dhcpLeases) => {
            const topology = parseTopology(clmeshData.network);
            const ctrlMac = topology.mac;

            seenMACs.clear();
            allClients = [];
            traverse([topology], ctrlMac);

            allClients.forEach(client => {
                const lease = dhcpLeases.find(l => l.mac?.toLowerCase() === client.mac?.toLowerCase());
                if (lease) {
                    client.name = lease.hostname || 'N/A';
                    client.ip = lease.ipaddr || client.ip;
                }
            });
        };

        const filterData = () =>
            allClients.filter(({ type, name, mac }) =>
                (currentFilter === 'all' || type === currentFilter) &&
                (!currentKeyword || name.includes(currentKeyword) || mac.includes(currentKeyword))
            );

        const renderTable = () => {
            const filtered = filterData();
            const totalPages = Math.ceil(filtered.length / pageSize);

            currentPage = Math.max(1, Math.min(currentPage, totalPages || 1));
            tbody.innerHTML = '';

            const start = (currentPage - 1) * pageSize;
            const pageItems = filtered.slice(start, start + pageSize);

            if (pageItems.length === 0) {
                tbody.appendChild(E('tr', {}, [
                    E('td', { colspan: 5, style: 'text-align:center;' }, _('No Data'))
                ]));
            } else {
                pageItems.forEach(({ name, up, down, mac, ip, type }) => {
                    tbody.appendChild(E('tr', {}, [
                        E('td', {}, name),
                        E('td', {}, [
                            E('span', { style: 'color: green; margin-right: 5px;' }, `↑ ${up}`),
                            E('span', { style: 'color: blue;' }, `↓ ${down}`)
                        ]),
                        E('td', {}, mac),
                        E('td', {}, ip),
                        E('td', {}, type === 'wireless' ? _('Wi-Fi link') : _('Wired link')),
                    ]));
                });
            }
			
            pageInfo.textContent = `第 ${currentPage} 页 / 共 ${totalPages} 页`;
			//pageInfo.textContent = _('Page') + `${currentPage} / ${totalPages}` + _('Pages in total') ;
            prevBtn.disabled = currentPage === 1;
            nextBtn.disabled = currentPage === totalPages || totalPages === 0;
        };

        controls.querySelector('#typeFilter').addEventListener('change', ({ target }) => {
            currentFilter = target.value;
            currentPage = 1;
            renderTable();
        });

        controls.querySelector('#searchBtn').addEventListener('click', () => {
            currentKeyword = controls.querySelector('#searchInput').value.trim();
            currentPage = 1;
            renderTable();
        });

        prevBtn.addEventListener('click', () => {
            if (currentPage > 1) {
                currentPage--;
                renderTable();
            }
        });

        nextBtn.addEventListener('click', () => {
            const totalPages = Math.ceil(filterData().length / pageSize);
            if (currentPage < totalPages) {
                currentPage++;
                renderTable();
            }
        });

        pageInput.addEventListener('change', ({ target }) => {
            const page = parseInt(target.value, 10);
            const totalPages = Math.ceil(filterData().length / pageSize);

            if (!isNaN(page) && page >= 1 && page <= totalPages) {
                currentPage = page;
                renderTable();
            }
        });

        updateClientList(clmeshData, dhcpLeases);
        renderTable();

        const polling = () =>
            Promise.all([callClmeshTopology(), callDHCPLeases()]).then(([clmesh, { dhcp_leases }]) => {
                updateClientList(clmesh, dhcp_leases);
                renderTable();
            });

        poll.add(polling);

        return container;
    },

    handleReset: null,
    handleSave: null,
    handleSaveApply: null
});

