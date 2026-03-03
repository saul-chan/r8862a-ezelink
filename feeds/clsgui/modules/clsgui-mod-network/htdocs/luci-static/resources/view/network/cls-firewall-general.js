'use strict';
'require view';
'require rpc';
'require uci';
'require form';

return view.extend({
    render: function() {
        var m, s, o;

        m = new form.Map('firewall');

        s = m.section(form.TypedSection, 'rule', _('Traffic rule'));
        s.addremove = false;
        s.anonymous = true;

        s.filter = function(section_id) {
            return (uci.get('firewall', section_id, 'name') == 'Allow-Ping');
        };

        o = s.option(form.Flag, 'enabled', _('Allow Ping WAN'));
        o.default = o.enabled;

        return m.render();
    }
});
