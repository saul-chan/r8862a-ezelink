'use strict';
'require view';
'require dom';
'require rpc';
'require uci';
'require form';
'require network';
'require fs';

function sync_to_5g(array) {
    var n = 0, hasRemove = false,
        radio1 = [];

    array.forEach(function(subArray) {
        if (subArray[0] == 'set')
            n++;
        else if (subArray[0] == 'remove')
            hasRemove = true;
    });

    if (!hasRemove && n <= 1) {
        for (var i = 0; i < array.length; i++)
            radio1.push(array[i][3]);
    } else if (!hasRemove && n > 1) {
        for (var i = 1; i < array.length; i++)
            radio1.push(array[i][3]);
    } else if (hasRemove && n <= 1) {
        for (var i = array.length - 1; i >= 0; i--) {
            if (array[i][0] == 'remove') {
                radio1.splice(0, 0, array[i][0]);
                radio1.push(array[0][3]);
                break;
            }
            radio1.push(array[i][3]);
        }
    } else {
        for (var i = array.length - 1; i >= 0; i--) {
            if (array[i][0] == 'remove') {
                radio1.splice(0, 0, array[i][3]);
                break;
            }
            radio1.push(array[i][3]);
        }
    }

    fs.exec('/usr/libexec/macfilter', radio1);
}


return view.extend({
    render: function() {
        var m, s, o;

        m = new form.Map('wireless');

        s = m.section(form.NamedSection, 'default_radio0', 'wifi-iface', _('Access Permision'));

        o = s.option(form.ListValue, 'macfilter', _('MAC Address Filter'));
        o.value('', _('Disabled'));
        o.value('allow', _('Allow listed only'));
        o.value('deny', _('Allow all except listed'));

        o = s.option(form.DynamicList, 'maclist', _('MAC-List'));
        o.datatype = 'macaddr';
        o.retain = true;
        o.depends('macfilter', 'allow');
        o.depends('macfilter', 'deny');
        o.load = function(section_id) {
            return network.getHostHints().then(L.bind(function(hints) {
                hints.getMACHints().map(L.bind(function(hint) {
                    this.value(hint[0], hint[1] ? '%s (%s)'.format(hint[0], hint[1]) : hint[0]);
                }, this));

                return form.DynamicList.prototype.load.apply(this, [section_id]);
            }, this));
        };

        return m.render();
    },
    handleSaveApply: function(ev, mode) {
        return this.handleSave(ev).then(() => {
            var changes = classes.ui.changes.changes.wireless,
                wiface = changes == null? null: JSON.parse(JSON.stringify(changes));

            sync_to_5g(wiface);

            this.handleSave(ev).then(function() {
                classes.ui.changes.apply(mode == '0');
            });
        });
    },

});

