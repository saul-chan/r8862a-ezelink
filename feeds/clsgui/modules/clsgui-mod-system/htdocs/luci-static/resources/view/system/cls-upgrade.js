'use strict';
'require baseclass';
'require view';
'require rpc';
'require network';
'require dom';
'require uci';
'require ui';
'require fs';

document.querySelector('head').appendChild(E('link', {
    'rel': 'stylesheet',
    'type': 'text/css',
    'href': L.resource('view/system/css/cls-restore.css')
}));

var callSystemValidateFirmwareImage = rpc.declare({
    object: 'system',
    method: 'validate_firmware_image',
    params: [ 'path' ],
    expect: { '': { valid: false, forcable: true } }
});

function findStorageSize(procmtd, procpart) {
    var kernsize = 0, rootsize = 0, wholesize = 0;

    procmtd.split(/\n/).forEach(function(ln) {
        var match = ln.match(/^mtd\d+: ([0-9a-f]+) [0-9a-f]+ "(.+)"$/),
            size = match ? parseInt(match[1], 16) : 0;

        switch (match ? match[2] : '') {
        case 'linux':
        case 'firmware':
            if (size > wholesize)
                wholesize = size;
            break;

        case 'kernel':
        case 'kernel0':
            kernsize = size;
            break;

        case 'rootfs':
        case 'rootfs0':
        case 'ubi':
        case 'ubi0':
            rootsize = size;
            break;
        }
    });

    if (wholesize > 0)
        return wholesize;
    else if (kernsize > 0 && rootsize > kernsize)
        return kernsize + rootsize;

    procpart.split(/\n/).forEach(function(ln) {
        var match = ln.match(/^\s*\d+\s+\d+\s+(\d+)\s+(\S+)$/);
        if (match) {
            var size = parseInt(match[1], 10);

            if (!match[2].match(/\d/) && size > 2048 && wholesize == 0)
                wholesize = size * 1024;
        }
    });

    return wholesize;
}

return view.extend({
    load: function() {
        var tasks = [
            L.resolveDefault(fs.stat('/lib/upgrade/platform.sh'), {}),
            fs.trimmed('/proc/sys/kernel/hostname'),
            fs.trimmed('/proc/mtd'),
            fs.trimmed('/proc/partitions'),
            fs.trimmed('/proc/mounts')
        ];

        return Promise.all(tasks);
    },

    handleSysupgrade: function(storage_size, has_rootfs_data, ev) {
        return ui.uploadFile('/tmp/firmware.bin', ev.target.firstChild)
            .then(L.bind(function(btn, reply) {
                btn.firstChild.data = _('Checking image…');

                ui.showModal(_('Checking image…'), [
                    E('span', { 'class': 'spinning' }, _('Verifying the uploaded image file.'))
                ]);

                return callSystemValidateFirmwareImage('/tmp/firmware.bin')
                    .then(function(res) { return [ reply, res ]; });
            }, this, ev.target))
            .then(L.bind(function(btn, reply) {
                return fs.exec('/sbin/sysupgrade', [ '--test', '/tmp/firmware.bin' ])
                    .then(function(res) { reply.push(res); return reply; });
            }, this, ev.target))
            .then(L.bind(function(btn, res) {
                /* sysupgrade opts table  [0]:checkbox element [1]:check condition [2]:args to pass */
                var opts = {
                    keep : [ E('input', { type: 'checkbox' }), false, '-n' ],
                    force : [ E('input', { type: 'checkbox' }), true, '--force' ],
                    skip_orig : [ E('input', { type: 'checkbox' }), true, '-u' ],
                    backup_pkgs : [ E('input', { type: 'checkbox' }), true, '-k' ],
                    },
                    is_valid = res[1].valid,
                    is_forceable = res[1].forceable,
                    allow_backup = res[1].allow_backup,
                    is_too_big = (storage_size > 0 && res[0].size > storage_size),
                    body = [];

                    body.push(E('p', _("The flash image was uploaded. Below is the checksum and file size listed, compare them with the original file to ensure data integrity. <br /> Click 'Continue' below to start the flash procedure.")));
                    body.push(E('ul', {}, [
                        res[0].size ? E('li', {}, '%s: %1024.2mB'.format(_('Size'), res[0].size)) : '',
                        res[0].checksum ? E('li', {}, '%s: %s'.format(_('MD5'), res[0].checksum)) : '',
                        res[0].sha256sum ? E('li', {}, '%s: %s'.format(_('SHA256'), res[0].sha256sum)) : ''
                    ]));

                    body.push(E('p', {}, E('label', { 'class': 'btn' }, [
                        opts.keep[0], ' ', _('Keep settings and retain the current configuration')
                    ])));

                    if (!is_valid || is_too_big)
                        body.push(E('hr'));

                    if (is_too_big)
                        body.push(E('p', { 'class': 'alert-message' }, [
                            _('It appears that you are trying to flash an image that does not fit into the flash memory, please verify the image file!')
                        ]));

                    if (!is_valid)
                        body.push(E('p', { 'class': 'alert-message' }, [
                            res[2].stderr ? res[2].stderr : '',
                            res[2].stderr ? E('br') : '',
                            res[2].stderr ? E('br') : '',
                            _('The uploaded image file does not contain a supported format. Make sure that you choose the generic image format for your platform.')
                        ]));

                    if (!allow_backup) {
                        body.push(E('p', { 'class': 'alert-message' }, [
                            _('The uploaded firmware does not allow keeping current configuration.')
                        ]));
                        opts.keep[0].disabled = true;
                    } else {
                        opts.keep[0].checked = true;

                        if (has_rootfs_data) {
                            body.push(E('p', {}, E('label', { 'class': 'btn' }, [
                                opts.skip_orig[0], ' ', _('Skip from backup files that are equal to those in /rom')
                            ])));
                        }

                        body.push(E('p', {}, E('label', { 'class': 'btn' }, [
                            opts.backup_pkgs[0], ' ', _('Include in backup a list of current installed packages at /etc/backup/installed_packages.txt')
                        ])));
                    };

                    var cntbtn = E('button', {
                        'class': 'btn cbi-button-action important',
                        'click': ui.createHandlerFn(this, 'handleSysupgradeConfirm', btn, opts),
                    }, [ _('Continue') ]);

                    if (res[2].code != 0) {
                        body.push(E('p', { 'class': 'alert-message danger' }, E('label', {}, [
                            _('Image check failed:'),
                            E('br'), E('br'),
                            res[2].stderr
                        ])));
                    };

                    fs.stat('/usr/bin/ucert')
                    .then(function() {
                        if (res[2].code != 0)
                            cntbtn.disabled = true;
                    })
                    .catch(function() {
                        if ((!is_valid || is_too_big || res[2].code != 0) && is_forceable) {
                            body.push(E('p', {}, E('label', { 'class': 'btn alert-message danger' }, [
                                opts.force[0], ' ', _('Force upgrade'),
                                E('br'), E('br'),
                                _('Select \'Force upgrade\' to flash the image even if the image format check fails. Use only if you are sure that the firmware is correct and meant for your device!')
                            ])));
                            cntbtn.disabled = true;
                        };
                    });

                    body.push(E('div', { 'class': 'right' }, [
                        E('button', {
                            'class': 'btn',
                            'click': ui.createHandlerFn(this, function(ev) {
                                return fs.remove('/tmp/firmware.bin').finally(ui.hideModal);
                            })
                        }, [ _('Cancel') ]), ' ', cntbtn
                    ]));

                    opts.force[0].addEventListener('change', function(ev) {
                        cntbtn.disabled = !ev.target.checked;
                    });

                    opts.keep[0].addEventListener('change', function(ev) {
                        opts.skip_orig[0].disabled = !ev.target.checked;
                        opts.backup_pkgs[0].disabled = !ev.target.checked;
                    });

                    ui.showModal(_('Flash image?'), body);
            }, this, ev.target))
            .catch(function(e) { ui.addNotification(null, E('p', e.message)) })
            .finally(L.bind(function(btn) {
                btn.firstChild.data = _('Manual upgrade');
            }, this, ev.target));
    },

    handleSysupgradeConfirm: function(btn, opts, ev) {
        btn.firstChild.data = _('Flashing…');

        ui.showModal(_('Flashing…'), [
                E('p', { 'class': 'spinning' }, _('The system is flashing now.<br /> DO NOT POWER OFF THE DEVICE!<br /> Wait a few minutes before you try to reconnect. It might be necessary to renew the address of your computer to reach the device again, depending on your settings.'))
        ]);

        var args = [];

        for (var key in opts)
                /* if checkbox == condition add args to sysupgrade */
                if (opts[key][0].checked == opts[key][1])
                        args.push(opts[key][2]);

        args.push('/tmp/firmware.bin');

        /* Currently the sysupgrade rpc call will not return, hence no promise handling */
        fs.exec('/sbin/sysupgrade', args);

        if (opts['keep'][0].checked)
                ui.awaitReconnect(window.location.host);
        else
                ui.awaitReconnect('192.168.1.1', 'openwrt.lan');
    },

    renderHtml: function(res) {
        var container = E('div', { 'class': 'firmware-upgrade'});

        dom.content(container, [
            E('div', { 'class': 'cbi-section' }, [
                E('h3', {}, _('Firmware Upgrade')),
                E('p', [
                    E('span', _('Update the version of the operating system on the router.'))
                ]),
                E('hr'),
                E('button', { 'class': 'btn-factory', 'click': L.bind(this.handleSysupgrade, this, res[4], res[3])}, _('Manual upgrade'))
            ]),
        ]);

      return container;
    },

    render: function(rpc_replies) {
        var res = [];

            res.push(rpc_replies[2]);
            res.push(rpc_replies[3]);
            res.push(rpc_replies[4]);
            res.push((res[0].match(/"rootfs_data"/) != null) || (res[2].match("overlayfs:\/overlay \/ ") != null));
            res.push(findStorageSize(res[0], res[2]));

        return Promise.all([
            this.renderHtml(res)
        ]);
    },

    handleSave: null,
    handleReset: null,
    handleSaveApply: null
});
