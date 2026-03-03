'use strict';
'require view';
'require rpc';
'require form';
'require uci';
'require request';

return view.extend({
    render: function() {
        var m, s, o;

        m = new form.Map('uhttpd');

        s = m.section(form.NamedSection, 'main', 'uhttpd', _('WebGUI Switching'));

        o = s.option(form.ListValue, 'home', _('WebGUI Switching'));
        o.value('/www', _('Original WebGUI'));
        o.value('/clsgui', _('Clsemi WebGUI'));

        return m.render();
    },

    handleSaveApply: function(ev, mode) {
        return this.handleSave(ev).then(function() {
            classes.ui.changes.apply(mode == '0');
        }).then(function() {
            var status = classes.ui.changes.changes.uhttpd;

            if (status != 'undefined') {
                window.setTimeout(function() {
                    request.post(L.url('admin/logout'));
                }, 5000);
            }
        });
    }
});
