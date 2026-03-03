'use strict';
'require view';
'require rpc';
'require ui';
'require uci';

document.querySelector('head').appendChild(E('link', {
    'rel': 'stylesheet',
    'type': 'text/css',
    'href': L.resource('view/system/css/cls-restore.css')
}));

var callReboot = rpc.declare({
    object: 'system',
    method: 'reboot',
    expect: { result: 0 }
});

return view.extend({
    load: function() {
        return uci.changes();
    },

    render: function(changes) {
        var body = E('div', [
            E('h2', _('Reboot')),
            E('div', { 'class': 'cbi-section' }, [
                E('h3', {}, _('Reboot')),
                E('p', [
                    E('span', { 'class': 'reboot-span' }, _('Reboots the operating system of your device.'))
                ]),
                E('hr'),
                E('button', { 'class': 'btn-reboot', 'click': L.bind(this.handleRebootConfirm, this) }, _('Perform Reboot')),
            ])
        ]);

        for (var config in (changes || {})) {
            body.appendChild(E('p', { 'class': 'alert-message warning' },
                _('Warning: There are unsaved changes that will get lost on reboot!')));
            break;
        }

        return body;
    },

    handleRebootConfirm: function() {
        L.ui.showModal(_('Are you sure to reboot this device?'), [
            E('div', {
                'style': 'background-color: ;'}, [
                    E('button', {
                        'class': 'confirm-button',
                        'style': 'margin-left: 220px; margin-top: 40px; padding: 10px;',
                        'click': L.bind(this.handleReboot, this),
                    }, _('Confirm')),
                    E('button', {
                        'class': 'cancel-button',
                        'style': 'padding: 10px;',
                        'click': ui.hideModal
                    }, _('Cancel'))
                ])
            ]);
    },

    handleReboot: function(ev) {
        return callReboot().then(function(res) {
            if (res != 0) {
                L.ui.addNotification(null, E('p', _('The reboot command failed with code %d').format(res)));
                L.raise('Error', 'Reboot failed');
            }

            L.ui.showModal(_('Rebooting…'), [
                E('p', { 'class': 'spinning' }, _('Waiting for device...'))
            ]);

            window.setTimeout(function() {
                L.ui.showModal(_('Rebooting…'), [
                    E('p', { 'class': 'spinning alert-message warning' },
                        _('Device unreachable! Still waiting for device...'))
                ]);
            }, 150000);

            L.ui.awaitReconnect();
        })
        .catch(function(e) { L.ui.addNotification(null, E('p', e.message)) });
    },

    handleSaveApply: null,
    handleSave: null,
    handleReset: null
});
