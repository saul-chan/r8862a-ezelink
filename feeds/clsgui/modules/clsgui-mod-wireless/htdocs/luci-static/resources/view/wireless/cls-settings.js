'use strict';
"require view";
"require dom";
"require rpc";
"require uci";
"require form";
"require network";

var conf_state = [
    [ "24g_disabled", " " ],
    [ "24g_ssid", " " ],
    [ "24g_encryption", " " ],
    [ "24g_key", " " ],
    [ "5g_disabled", " " ],
    [ "5g_ssid", " " ],
    [ "5g_encryption", " " ],
    [ "5g_key", " " ],
    [ "dual_disabled", " " ],
    [ "dual_ssid", " " ],
    [ "dual_encryption", " " ],
    [ "dual_key", " " ],
];

var encryption_obj = {
    "none": _('No Encryption'),
    "psk": "WPA-PSK",
    "psk2": "WPA2-PSK",
    "sae": "WPA3-SAE",
    "psk-mixed": _('WPA-PSK/WPA2-PSK Mixed Mode'),
    "sae-mixed": _('WPA2-PSK/WPA3-SAE Mixed Mode'),
    "owe": "OWE"
    //"wep-open": "WEP Open System",
    //"wep-shared": "WEP Shared Key"
};

function set_list_value(o) {
    for (var key in encryption_obj)
        o.value(key, encryption_obj[key]);
}

function set_key_depends(o) {
    var prefix = o.option.split('_')[0],
        opt = prefix + '_encryption',
        res = [];

    if (prefix != '2g' && prefix != '5g' && prefix != 'dual')
        opt = prefix;

    for (var key in encryption_obj) {
        if (key != 'none' && key != 'owe') {
            var obj = {};

            obj[opt] = key;
            res.push(obj);
        }
    }

    res.forEach(function(subArray) {
        o.depends(subArray);
    });
}

function password_check(value) {
    if (value.length < 8)
            return _("Please set a password that exceeds 8 characters");
    else
            return true;
};

function wifi_conf_sync_without_dualband(array) {
    let action_set_2g = [], action_del_2g = [],
        action_set_5g = [], action_del_5g = [],
        action_set_dual = [], action_del_dual = [];

    array.forEach(function(subArray) {
        if (subArray[0] == 'set' && subArray[2].split('_')[0] == '2g')
            action_set_2g.push(subArray);
        else if (subArray[0] == 'set' && subArray[2].split('_')[0] == '5g')
            action_set_5g.push(subArray);
        else if (subArray[0] == 'remove' && subArray[2].split('_')[0] == '2g')
            action_del_2g.push(subArray);
        else if (subArray[0] == 'remove' && subArray[2].split('_')[0] == '5g')
            action_del_5g.push(subArray);
        else if (subArray[0] == 'set' && subArray[2].split('_')[0] == 'dual')
            action_set_dual.push(subArray);
        else if (subArray[0] == 'remove' && subArray[2].split('_')[0] == 'dual')
            action_del_dual.push(subArray);
    });

    for (let i = 0; i < action_set_2g.length; i++) {
        if (action_set_2g[i][2].split('_')[1] == 'enable' && action_set_2g[i][3] == '0')
            uci.set('wireless', 'default_radio0', 'disabled', '1');
        else if (action_set_2g[i][2].split('_')[1] == 'enable' && action_set_2g[i][3] == '1')
            uci.set('wireless', 'default_radio0', 'disabled', '');
        else
            uci.set('wireless', 'default_radio0', action_set_2g[i][2].split('_')[1], action_set_2g[i][3]);
    }

    for (let i = 0; i < action_set_5g.length; i++) {
        if (action_set_5g[i][2].split('_')[1] == 'enable' && action_set_5g[i][3] == '0')
            uci.set('wireless', 'default_radio1', 'disabled', '1');
        else if (action_set_5g[i][2].split('_')[1] == 'enable' && action_set_5g[i][3] == '1')
            uci.set('wireless', 'default_radio1', 'disabled', '');
        else
            uci.set('wireless', 'default_radio1', action_set_5g[i][2].split('_')[1], action_set_5g[i][3]);
    }

    for (let i = 0; i < action_del_2g.length; i++)
        uci.set('wireless', 'default_radio0', action_del_2g[i][2].split('_')[1], '');
    
    for (let i = 0; i < action_del_5g.length; i++)
        uci.set('wireless', 'default_radio1', action_del_5g[i][2].split('_')[1], '');

    for (let i = 0; i < action_set_dual.length; i++) {
        if (action_set_dual[i][2].split('_')[1] == 'enable' && action_set_dual[i][3] == '0') {
            uci.set('wireless', 'default_radio0', 'disabled', '1');
            uci.set('wireless', 'default_radio1', 'disabled', '1');
        } else if (action_set_dual[i][2].split('_')[1] == 'enable' && action_set_dual[i][3] == '1') {
            uci.set('wireless', 'default_radio0', 'disabled', '');
            uci.set('wireless', 'default_radio1', 'disabled', '');
        } else if (action_set_dual[i][2].split('_')[1] == 'ssid'){
            uci.set('wireless', 'default_radio0', action_set_dual[i][2].split('_')[1], action_set_dual[i][3]);
            uci.set('wireless', 'default_radio1', action_set_dual[i][2].split('_')[1], action_set_dual[i][3] + '-5G');
        } else {
            uci.set('wireless', 'default_radio0', action_set_dual[i][2].split('_')[1], action_set_dual[i][3]);
            uci.set('wireless', 'default_radio1', action_set_dual[i][2].split('_')[1], action_set_dual[i][3]);
        }
    }

    for (let i = 0; i < action_del_dual.length; i++) {
        uci.set('wireless', 'default_radio0', action_del_dual[i][2].split('_')[1], '');
        uci.set('wireless', 'default_radio1', action_del_dual[i][2].split('_')[1], '');
    }
}

function wifi_conf_sync_within_dualband(wiface, luci) {
    let enDualband = false;

    wiface.forEach(function(subArray) {
        if (subArray[0] != 'remove')
            enDualband = true;
    });

    if (enDualband) {
        luci.forEach(function(subArray) {
            if (subArray[0] == 'set') {
                uci.set('wireless', 'default_radio0', subArray[2].split('_')[1], subArray[3]);
                uci.set('wireless', 'default_radio1', subArray[2].split('_')[1], subArray[3]);
            }
        })
    } else {
        luci.forEach(function(subArray) {
            if (subArray[0] == 'set' && subArray[2] == '2g_ssid')
                uci.set('wireless', 'default_radio1', 'ssid', subArray[3] + '-5G'); 
        })
    }
}

return view.extend({
    load: function() {
        return Promise.all([
            uci.changes(),
            uci.load('wireless'),
            uci.load('luci')
        ]);
    },

    render: function() {
        let m, s, o,
            keyG2 = uci.get('wireless', 'default_radio0', 'key'),
            keyG5 = uci.get('wireless', 'default_radio1', 'key');

        m = new form.Map('luci', _('Wireless Settings'));
        m.chain('wireless');

        s = m.section(form.NamedSection, 'dualband', 'wifi-dualband');
        s.uciconfig = 'wireless';
        s.anonymous = false;

        o = s.option(form.Flag, 'enable', _('Unified Wi-Fi Dual-band'), _('2.4G and 5G signals are displayed together, and the faster 5G is preferred for the same signal strength. Turn off this switch to set it separately.'));

        /* 2.4G Wi-Fi configuration */
        s = m.section(form.NamedSection, 'wifi_dualband', 'internal');
        s.anonymous = false;

        o = s.option(form.Flag, '2g_enable', _('2.4G Wi-Fi'), _('Turn on/off 2.4G Wi-Fi.'));
        o.default = uci.get('wireless', 'default_radio0', 'disabled')? '0': '1';
        o.depends("wireless.dualband.enable", '0');

        o = s.option(form.Value, '2g_ssid', _('SSID'));
        o.default = uci.get('wireless', 'default_radio0', 'ssid');
        o.depends("wireless.dualband.enable", '0');

        o = s.option(form.ListValue, '2g_encryption', _('Encryption'));
        o.default = uci.get('wireless', 'default_radio0', 'encryption');
        o.depends("wireless.dualband.enable", '0');
        set_list_value(o);

        o = s.option(form.Value, '2g_key', _('Key'));
        o.default = keyG2? keyG2: '';
        o.password = true;
        o.validate = function(secid, value) { let res = password_check(value); return res;};
        set_key_depends(o);

        /* 5G Wi-Fi configuration */
        s = m.section(form.NamedSection, 'wifi_dualband', 'internal');
        s.anonymous = false;

        o = s.option(form.Flag, '5g_enable', _('5G Wi-Fi'), _('Turn on/off 5G Wi-Fi.'));
        o.default = uci.get('wireless', 'default_radio1', 'disabled')? '0': '1';
        o.depends("wireless.dualband.enable", '0');

        o = s.option(form.Value, '5g_ssid', _('SSID'));
        o.default = uci.get('wireless', 'default_radio1', 'ssid');
        o.depends("wireless.dualband.enable", '0');

        o = s.option(form.ListValue, '5g_encryption', _('Encryption'));
        o.default = uci.get('wireless', 'default_radio1', 'encryption');
        o.depends("wireless.dualband.enable", '0');
        set_list_value(o);

        o = s.option(form.Value, '5g_key', _('Key'));
        o.default = keyG5? keyG5: '';
        o.password = true;
        o.validate = function(secid, value) { let res = password_check(value); return res;};
        set_key_depends(o);

        /* Wi-Fi configuration with dualband enabled */
        s = m.section(form.NamedSection, 'wifi_dualband', 'internal');
        s.anonymous = false;

        o = s.option(form.Flag, 'dual_enable', _('Wi-Fi'), _('Turn on/off Wi-Fi.'));
        o.depends("wireless.dualband.enable", '1');
        o.default = uci.get('wireless', 'default_radio0', 'disabled')? '0': '1';

        o = s.option(form.Value, 'dual_ssid', _('SSID'));
        o.default = uci.get('wireless', 'default_radio0', 'ssid');
        o.depends("wireless.dualband.enable", '1');

        o = s.option(form.ListValue, 'dual_encryption', _('Encryption'));
        o.default = uci.get('wireless', 'default_radio0', 'encryption');
        o.depends("wireless.dualband.enable", '1');
        set_list_value(o);

        o = s.option(form.Value, 'dual_key', _('Key'));
        o.default = keyG2? keyG2: '';
        o.password = true;
        o.validate = function(secid, value) { let res = password_check(value); return res;};
        set_key_depends(o);

        return m.render();
    },

    handleSaveApply: function(ev, mode){
        return this.handleSave(ev).then(() => {
            var luciChanges = classes.ui.changes.changes.luci, noDualenable = false,
                wifiChanges = classes.ui.changes.changes.wireless,
                luci = luciChanges == null? null: JSON.parse(JSON.stringify(luciChanges)),
                wiface = wifiChanges == null? null: JSON.parse(JSON.stringify(wifiChanges));

            if (wiface) {
                for (let i = 0; i < wiface.length; i++) {
                    if (wiface[i][1] == 'dualband' && wiface[i][2] == 'enable')
                        noDualenable = true;
                }
            }

            if (!noDualenable)
                wifi_conf_sync_without_dualband(luci);
            else
                wifi_conf_sync_within_dualband(wiface, luci);

            this.handleSave(ev).then(function() {
                classes.ui.changes.apply(mode == '0');
            });
        });
    }
});
