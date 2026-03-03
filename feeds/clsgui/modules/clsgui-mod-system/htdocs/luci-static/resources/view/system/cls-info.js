'use strict';
'require baseclass';
'require view';
'require dom';
'require uci';
'require rpc';

document.querySelector('head').appendChild(E('link', {
    'rel': 'stylesheet',
    'type': 'text/css',
    'href': L.resource('view/system/css/cls-restore.css')
}));

var callSystemBoard = rpc.declare({
    object: 'system',
    method: 'board',
    expect: { '': {} }
});

return view.extend({
    load: function() {
        return Promise.all([
            callSystemBoard()
        ]);    
    },

    renderHtml: function(data) {
        var container = E('div', { 'class': 'firmware-information' });

        dom.content(container, [
            E('div', { 'class': 'cbi-section' }, [
                E('h3', _('Firmware Version')),
                E('p', [
                    E('span', _('The current firmware version number.'))
                ]),
                E('hr'),
                E('p', { 'class': 'version-info' }, [
                    E('span', _('Current Version')),
                    E('span', data[0].release.version)
                ])
            ])
        ]);

        return container;
    },

    render: function(data) {
        return Promise.all([
            this.renderHtml(data)
        ]);
    },

    handleSave: null,
    handleReset: null,
    handleSaveApply: null
});
