'use strict';
'require view';
'require rpc';
'require network';
'require poll';

var getDHCPClientList = rpc.declare({
    object: 'luci-rpc',
    method: 'getDHCPLeases',
    expect: { '': {} }
});

function startPolling() {
    var updateDHCPClientList = function() {
        return network.flushCache().then(function() {
            return Promise.all([
                getDHCPClientList()
            ]);
        }).then(function(result) {
            /* update DHCP clients list */
            var table = document.querySelector('#dhcp_clients_list'),
                rows = [],
                hosts = result[0].dhcp_leases;

            for (var i = 0; i < hosts.length; i++) {
                rows.push([
                        hosts[i].hostname || '-',
                        hosts[i].ipaddr,
                        hosts[i].macaddr,
                        hosts[i].expires + ' ' + _('seconds')
                ]);
            }

            cbi_update_table(table, rows);
        });
    };

    return updateDHCPClientList().then(function() {
        poll.add(updateDHCPClientList);
    });
}

return view.extend({
    renderTable: function() {
        var table = E('table', {'class':'dhcp_clients_list', 'id':'dhcp_clients_list'}, [
            E('tr', {'class':'tr table-titles'}, [
                E('th', {'class': 'th '}, _('Hostname')),
                E('th', {'class': 'th '}, _('IPv4 address')),
                E('th', {'class': 'th '}, _('MAC address')),
                E('th', {'class': 'th '}, _('Lease time remaining'))
            ])
        ]);

        return startPolling().then(function() {
            return E([E('br'), E('h3', _('DHCP Clients')), table]);
        });
    },

    render: function(data) {
        return Promise.all([
            this.renderTable()
        ]);
    },

    handleSave: null,
    handleSaveApply: null,
    handleReset: null
})
