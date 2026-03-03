'use strict';
'require view';
'require rpc';
'require uci';
'require form';

var callTimezone = rpc.declare({
    object: 'luci',
    method: 'getTimezones',
    expect: { '': {} }
});

return view.extend({
    load: function() {
        return Promise.all([
            callTimezone(),
            uci.load('rpcd')
        ]);
    },

    render: function(data) {
        var m, s, o, timezones = data[0];

        m = new form.Map('system');

        s = m.section(form.TypedSection, 'system', _('Time Zone'));
        s.anonymous = true;

        o = s.option(form.ListValue, 'zonename', _('Timezone'));

        var zones = Object.keys(timezones || {}).sort();
                for (var i = 0; i < zones.length; i++)
                        o.value(zones[i]);

                o.write = function(section_id, formvalue) {
                        var tz = timezones[formvalue] ? timezones[formvalue].tzstring : null;
                        uci.set('system', section_id, 'zonename', formvalue);
                        uci.set('system', section_id, 'timezone', tz);
                };


        return m.render();
    }
});
