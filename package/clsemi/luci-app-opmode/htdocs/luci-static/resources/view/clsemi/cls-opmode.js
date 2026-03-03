'use strict';
'require view';
'require dom';
'require form';
'require uci';

return view.extend({
        load: function() {
                return Promise.all([
                        uci.load('cls-opmode'),
                        uci.load('cls-mesh')
                ]);
        },

        render: function(data) {
                var m, s, o;

                m = new form.Map('cls-opmode', _('Operation Mode Settings'));
                m.chain('cls-mesh');

                s = m.section(form.NamedSection, 'globals');
                s.addremove = false;
                s.anonymous = true;

                o = s.option(form.ListValue, 'mode', _('Operation Mode'));
                o.modalonly = false;
                o.value('router', _('Router'));
                o.value('bridge', _('Bridge'));
                o.value('repeater', _('Repeater'));

                o = s.option(form.ListValue, 'sta_phyname', _('Station work on'));
                o.modalonly = false;
                o.depends('mode', 'repeater');
                o.value('radio0', _('2.4G Radio'));
                o.value('radio1', _('5G Radio'));

                o = s.option(form.ListValue, '_mode', _('UltraMesh Mode'));
                o.uciconfig = 'cls-mesh';
                o.ucisection = 'default';
                o.ucioption = 'mode';
		o.value('auto', _('Auto'));
                o.value('none', _('Disable'));
                o.value('controller', _('Controller'));
                o.value('agent', _('Agent'));

		o = s.option(form.ListValue, '_profile', _('Profile'));
                o.uciconfig = 'cls-mesh';
                o.ucisection = 'default';
                o.ucioption = 'profile';
                o.depends('_mode', 'auto');
                o.value('2', _('Default'));
                o.value('1', _('1'));

                return m.render();
        }
});
