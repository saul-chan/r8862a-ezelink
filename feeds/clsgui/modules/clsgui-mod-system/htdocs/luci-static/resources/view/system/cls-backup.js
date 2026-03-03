'use strict';
'require view';
'require dom';
'require rpc';

document.querySelector('head').appendChild(E('link', {
    'rel': 'stylesheet',
    'type': 'text/css',
    'href': L.resource('view/system/css/cls-restore.css')
}));

return view.extend({
    handleBackup: function(ev) {
        var form = E('form', {
            method: 'post',
            action: L.env.cgi_base + '/cgi-backup',
            enctype: 'application/x-www-form-urlencoded'
        }, E('input', { type: 'hidden', name: 'sessionid', value: rpc.getSessionID() }));

        ev.currentTarget.parentNode.appendChild(form);

        form.submit();
        form.parentNode.removeChild(form);
    },

    renderHtml: function() {
        var container = E('div', { 'class': 'system-container'});

        dom.content(container, [
            E('div', { 'class': 'cbi-section' }, [
                E('h3', {}, _('Configuration Backup')),
                E('p', [
                    E('span', _('Back up default configuration to local PC.'))
                ]),
                E('hr'),
                E('button', { 'class': 'btn-backup', 'click': L.bind(this.handleBackup, this) }, _('Backup')),
            ])
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
