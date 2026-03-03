'use strict';
'require view';
'require dom';
'require rpc';
'require uci';
'require form';
'require tools.cls-verify as clsverify';

return view.extend({
    load: function() {
        return Promise.all([
            uci.load('network')
        ]);
    },

    render: function() {
        var m, s, o;

        m = new form.Map('network', _('WAN'));
        m.chain('cls-opmode');

        /* working mode switching */
        s = m.section(form.NamedSection, 'globals', 'globals', _('Operation Configuration'));
        s.uciconfig = 'cls-opmode';
        s.anonymous = false;

        o = s.option(form.ListValue, 'mode', _('Operation Mode'));
        o.value('router', _('Router'));
        o.value('bridge', _('Bridge'));

        /* WAN Settings */
        s = m.section(form.NamedSection, 'wan', 'interface', _('WAN Configuration'));
        s.addremove = false;

        o = s.option(form.ListValue, 'proto', _('WAN Connection Type'));
        o.value('dhcp', _('DHCP'));
        o.value('static', _('Static IP'));
        o.value('pppoe', _('PPPoE'));

        o = s.option(form.Value, 'username', _('User Name'));
        o.depends('proto', 'pppoe');

        o = s.option(form.Value, 'password', _('Password'));
        o.depends('proto', 'pppoe');

        o = s.option(form.Value, 'ipaddr', _('IP Address'));
        o.depends('proto', 'static');
        o.datatype = 'ipaddr';

        o = s.option(form.Value, 'netmask', _('Subnet Mask'));
        o.depends('proto', 'static');
        o.datatype = 'ip4addr';

        o = s.option(form.Value, 'gateway', _('Defalult Gateway'));
        o.depends('proto', 'static');
        o.datatype = 'ipaddr';

        o = s.option(form.Value, 'dns', _('DNS Sever'));
        o.depends('proto', 'static');
        o.datatype = 'ipaddr';

        return m.render();
    }
})                          
