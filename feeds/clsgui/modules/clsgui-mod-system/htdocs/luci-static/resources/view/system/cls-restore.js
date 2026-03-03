'use strict';
'require baseclass';
'require view';
'require rpc';
'require dom';
'require uci';
'require ui';
'require fs';

document.querySelector('head').appendChild(E('link', {
    'rel': 'stylesheet',
    'type': 'text/css',
    'href': L.resource('view/system/css/cls-restore.css')
}));

return view.extend({
    handleRestore: function(ev) {
        return ui.uploadFile('/tmp/backup.tar.gz', ev.target)
            .then(L.bind(function(btn, res) {
                btn.firstChild.data = _('Checking archive…');
                return fs.exec('/bin/tar', [ '-tzf', '/tmp/backup.tar.gz' ]);
            }, this, ev.target))
                .then(L.bind(function(btn, res) {
                    if (res.code != 0) {
                        ui.addNotification(null, E('p', _('The uploaded backup archive is not readable')));
                        return fs.remove('/tmp/backup.tar.gz');
                    }

                    ui.showModal(_('Apply backup?'), [
                        E('p', _('The uploaded backup archive appears to be valid and contains the files listed below. Press "Continue" to restore the backup and reboot, or "Cancel" to abort the operation.')),
                        E('pre', {}, [ res.stdout ]),
                        E('div', { 'class': 'right' }, [
                            E('button', {
                                'class': 'btn',
                                'click': ui.createHandlerFn(this, function(ev) {
                                    return fs.remove('/tmp/backup.tar.gz').finally(ui.hideModal);
                                })
                            }, [ _('Cancel') ]), ' ',
                            E('button', {
                                'class': 'btn cbi-button-action important',
                                'click': ui.createHandlerFn(this, 'handleRestoreConfirm', btn)
                            }, [ _('Continue') ])
                        ])
                    ]);
                }, this, ev.target))
                .catch(function(e) { ui.addNotification(null, E('p', e.message)) })
                .finally(L.bind(function(btn, input) {
                        btn.firstChild.data = _('Restore');
                }, this, ev.target));
    },

    handleFirstboot: function(ev) {
        if (!confirm(_('Do you really want to erase all settings?')))
                return;

        ui.showModal(_('Erasing...'), [
                E('p', { 'class': 'spinning' }, _('The system is erasing the configuration partition now and will reboot itself when finished.'))
        ]);

        /* Currently the sysupgrade rpc call will not return, hence no promise handling */
        fs.exec('/sbin/firstboot', [ '-r', '-y' ]);

        ui.awaitReconnect('192.168.1.1', 'openwrt.lan');
    },

    handleRestoreConfirm: function(btn, ev) {
        return fs.exec('/sbin/sysupgrade', [ '--restore-backup', '/tmp/backup.tar.gz' ])
            .then(L.bind(function(btn, res) {
                if (res.code != 0) {
                    ui.addNotification(null, [
                        E('p', _('The restore command failed with code %d').format(res.code)),
                        res.stderr ? E('pre', {}, [ res.stderr ]) : ''
                    ]);
                    L.raise('Error', 'Unpack failed');
                }

                btn.firstChild.data = _('Rebooting…');
                return fs.exec('/sbin/reboot');
            }, this, ev.target))
                .then(L.bind(function(res) {
                    if (res.code != 0) {
                        ui.addNotification(null, E('p', _('The reboot command failed with code %d').format(res.code)));
                        L.raise('Error', 'Reboot failed');
                    }

                    ui.showModal(_('Rebooting…'), [
                        E('p', { 'class': 'spinning' }, _('The system is rebooting now. If the restored configuration changed the current LAN IP address, you might need to reconnect manually.'))
                    ]);

                    ui.awaitReconnect(window.location.host, '192.168.1.1', 'openwrt.lan');
            }, this))
            .catch(function(e) { ui.addNotification(null, E('p', e.message)) })
            .finally(function() { btn.firstChild.data = _('Restore') });
    },

    renderHtml: function() {
        var container = E('div', { 'class': 'system-container'});

        dom.content(container, [
            E('div', { 'class': 'cbi-section' }, [
                E('h3', {}, _('Restore Backup Configuration')),
                E('p', [
                    E('span', _('Restore previously saved default configuration from local PC. Reboot to apply changes.'))
                ]),
                E('hr'),
                E('button', { 'class': 'btn-restore', 'click': L.bind(this.handleRestore, this) }, _('Restore')),
            ]),
            E('div', { 'class': 'cbi-section' }, [
                E('h3', {}, _('Restore to Factory Default')),
                E('p', [
                    E('span', _('Reset the router configuration to the initial installation state, clear all user settings and custom configurations.'))
                ]),
                E('hr'),
                E('button', { 'class': 'btn-factory', 'click': L.bind(this.handleFirstboot, this)}, _('Restore Factory'))
            ]),
        ]);

        return container;
    },

    render: function() {
        return Promise.all([
            this.renderHtml()
        ]);
    },

    handleSave: null,
    handleReset: null,
    handleSaveApply: null
});
