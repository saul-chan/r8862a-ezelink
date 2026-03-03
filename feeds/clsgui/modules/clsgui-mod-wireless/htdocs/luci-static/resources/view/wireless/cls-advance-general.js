'use strict';
'require view';
'require dom';
'require rpc';
'require uci';
'require form';
'require network';

document.querySelector('head').appendChild(E('link', {
    'rel': 'stylesheet',
    'type': 'text/css',
    'href': L.resource('view/wireless/css/advance-general.css')
}));

var CBIWifiFrequencyValue = form.ListValue.extend({
    callFrequencyList: rpc.declare({
        object: 'iwinfo',
        method: 'freqlist',
        params: [ 'device' ],
        expect: { results: []}
    }),

    load: function(section_id) {
        return Promise.all([
            network.getWifiDevice(section_id),
            this.callFrequencyList(section_id)
        ]).then(L.bind(function(data) {
            this.channels = {
                '2g': L.hasSystemFeature('hostapd', 'acs') ? [ 'auto', 'auto', true ] : [],
                '5g': L.hasSystemFeature('hostapd', 'acs') ? [ 'auto', 'auto', true ] : [],
                '6g': [],
                '60g': []
            };

            for (var i = 0; i < data[1].length; i++) {
                var band;

                if (data[1][i].mhz >= 2412 && data[1][i].mhz <= 2484)
                    band = '2g';
                else if (data[1][i].mhz >= 5160 && data[1][i].mhz <= 5885)
                    band = '5g';
                else if (data[1][i].mhz >= 5925 && data[1][i].mhz <= 7125)
                    band = '6g';
                else if (data[1][i].mhz >= 58320 && data[1][i].mhz <= 69120)
                    band = '60g';
                else
                    continue;

                this.channels[band].push(
                    data[1][i].channel,
                    '%d (%d Mhz)'.format(data[1][i].channel, data[1][i].mhz),
                    !data[1][i].restricted
                );
            }

            var hwmodelist = L.toArray(data[0] ? data[0].getHWModes() : null)
                .reduce(function(o, v) { o[v] = true; return o }, {});

            this.modes = [
                '', 'Legacy', true,
                'n', 'N', hwmodelist.n,
                'ac', 'AC', hwmodelist.ac,
                'ax', 'AX', hwmodelist.ax
            ];

            var htmodelist = L.toArray(data[0] ? data[0].getHTModes() : null)
                .reduce(function(o, v) { o[v] = true; return o }, {});

            this.htmodes = {
                '': [ '', '-', true ],
                'n': [
                    'HT20', '20 MHz', htmodelist.HT20,
                    'HT40', '40 MHz', htmodelist.HT40
                ],
                'ac': [
                    'VHT20', '20 MHz', htmodelist.VHT20,
                    'VHT40', '40 MHz', htmodelist.VHT40,
                    'VHT80', '80 MHz', htmodelist.VHT80,
                    'VHT160', '160 MHz', htmodelist.VHT160
                ],
                'ax': [
                    'HE20', '20 MHz', htmodelist.HE20,
                    'HE40', '40 MHz', htmodelist.HE40,
                    'HE80', '80 MHz', htmodelist.HE80,
                    'HE160', '160 MHz', htmodelist.HE160
                ]
            };

            this.channels_5g = {
                'ac': [
                    'VHT20', '20 MHz', htmodelist.VHT20,
                    'VHT40', '40 MHz', htmodelist.VHT40,
                    'VHT80', '80 MHz', htmodelist.VHT80
                ],
                'ax': [
                    'HE20', '20 MHz', htmodelist.HE20,
                    'HE40', '40 MHz', htmodelist.HE40,
                    'HE80', '80 MHz', htmodelist.HE80
                ]
            };

            this.bands = {
                '': [
                    '2g', '2.4 GHz', this.channels['2g'].length > 3,
                    '5g', '5 GHz', this.channels['5g'].length > 3,
                    '60g', '60 GHz', this.channels['60g'].length > 0
                ],
                'n': [
                    '2g', '2.4 GHz', this.channels['2g'].length > 3,
                    '5g', '5 GHz', this.channels['5g'].length > 3
                ],
                'ac': [
                    '5g', '5 GHz', true
                ],
                'ax': [
                    '2g', '2.4 GHz', this.channels['2g'].length > 3,
                    '5g', '5 GHz', this.channels['5g'].length > 3
                ]
            };
        }, this));
    },

    setValues: function(sel, vals) {
        if (sel.vals)
            sel.vals.selected = sel.selectedIndex;

        while (sel.options[0])
            sel.remove(0);

        for (var i = 0; vals && i < vals.length; i += 3)
            if (vals[i+2])
                sel.add(E('option', { value: vals[i+0] }, [ vals[i+1] ]));

            if (vals && !isNaN(vals.selected))
                sel.selectedIndex = vals.selected;

            sel.parentNode.style.display = (sel.options.length <= 1) ? 'none' : '';
            sel.vals = vals;
        },

    toggleWifiMode: function(elem) {
        this.toggleWifiHTMode(elem);
        this.toggleWifiBand(elem);
    },

    toggleWifiHTMode: function(elem) {
        var mode = elem.querySelector('.mode');
        var bwdt = elem.querySelector('.htmode');
        var chan = elem.querySelector('.channel');

        this.setValues(bwdt, this.htmodes[mode.value]);
    },

    toggleWifiBand: function(elem) {
        var mode = elem.querySelector('.mode');
        var band = elem.querySelector('.band');

        this.setValues(band, this.bands[mode.value]);
        this.toggleWifiChannel(elem);

        this.map.checkDepends();
    },

    toggleWifiChannel: function(elem) {
        var band = elem.querySelector('.band');
        var chan = elem.querySelector('.channel');
        var bwdt = elem.querySelector('.htmode');
        var mode = elem.querySelector('.mode');

        this.setValues(chan, this.channels[band.value]);

        if (chan.value >= 149)
            this.setValues(bwdt, this.channels_5g[mode.value]);
    },

    setInitialValues: function(section_id, elem) {
        var mode = elem.querySelector('.mode'),
            band = elem.querySelector('.band'),
            chan = elem.querySelector('.channel'),
            bwdt = elem.querySelector('.htmode'),
            htval = uci.get('wireless', section_id, 'htmode'),
            hwval = uci.get('wireless', section_id, 'hwmode'),
            chval = uci.get('wireless', section_id, 'channel'),
            bandval = uci.get('wireless', section_id, 'band');

        this.setValues(mode, this.modes);

        if (/HE20|HE40|HE80|HE160/.test(htval))
            mode.value = 'ax';
        else if (/VHT20|VHT40|VHT80|VHT160/.test(htval))
            mode.value = 'ac';
        else if (/HT20|HT40/.test(htval))
            mode.value = 'n';
        else
            mode.value = '';

        this.toggleWifiMode(elem);

        if (hwval != null) {
            this.useBandOption = false;

            if (/a/.test(hwval))
                band.value = '5g';
            else
                band.value = '2g';
        } else {
            this.useBandOption = true;
            band.value = bandval;
        }

        this.toggleWifiBand(elem);

        bwdt.value = htval;
        chan.value = chval || (chan.options[0] ? chan.options[0].value : 'auto');

        return elem;
    },

    renderWidget: function(section_id, option_index, cfgvalue) {
        var elem = E('div', { "class": "radio-conf" });

        dom.content(elem, [
            E('div', { "class": "form-group" }, [
                E('label', { 'class': 'cbi-value-title' }, _('Mode')),
                E('select', {
                    'class': 'mode',
                    'change': L.bind(this.toggleWifiMode, this, elem),
                    'disabled': (this.disabled != null) ? this.disabled : this.map.readonly
                })
            ]),
            E('div', { "class": "form-group" }, [
                E('label', { 'class': 'cbi-value-title' }, _('Band')),
                E('select', {
                    'class': 'band',
                    'change': L.bind(this.toggleWifiBand, this, elem),
                    'disabled': (this.disabled != null) ? this.disabled : this.map.readonly
                })
            ]),
            E('div', { "class": "form-group" }, [
                E('label', { 'class': 'cbi-value-title' }, _('Channel')),
                E('select', {
                    'class': 'channel',
                    'change': L.bind(this.toggleWifiChannel, this, elem),
                    'disabled': (this.disabled != null) ? this.disabled : this.map.readonly
                })
            ]),
            E('div', { "class": "form-group" }, [
                E('label', { 'class': 'cbi-value-title' }, _('Channel Width')),
                E('select', {
                    'class': 'htmode',
                    'change': L.bind(this.map.checkDepends, this.map),
                    'disabled': (this.disabled != null) ? this.disabled : this.map.readonly
                })
            ]),
        ]);

        return this.setInitialValues(section_id, elem);
    },

    cfgvalue: function(section_id) {
        return [
            uci.get('wireless', section_id, 'htmode'),
            uci.get('wireless', section_id, 'hwmode') || uci.get('wireless', section_id, 'band'),
            uci.get('wireless', section_id, 'channel')
        ];
    },

    formvalue: function(section_id) {
        var node = this.map.findElement('data-field', this.cbid(section_id));

        return [
            node.querySelector('.htmode').value,
            node.querySelector('.band').value,
            node.querySelector('.channel').value
        ];
    },

    write: function(section_id, value) {
        uci.set('wireless', section_id, 'htmode', value[0] || null);

        if (this.useBandOption)
            uci.set('wireless', section_id, 'band', value[1]);
        else
            uci.set('wireless', section_id, 'hwmode', (value[1] == '2g') ? '11g' : '11a');

        uci.set('wireless', section_id, 'channel', value[2]);
    }
});

return view.extend({
    render: function() {
        var m, s, o;

        m = new form.Map('wireless');

        s = m.section(form.NamedSection, 'radio0', 'wifi-device', _('2.4G Wi-Fi'));

        o = s.option(CBIWifiFrequencyValue, '_htmode');
        o.ucisection = s.section;

        o = s.option(form.Value, 'beacon_int', _('Beacon Interval'));
        o.default = o.enabled;
        o.datatype = 'range(15,65535)';
        o.placeholder = 100;
        o.rmempty = true;

        o = s.option(form.Value, 'dtim_period', _('DTIM Interval'));
        o.ucisection = 'default_radio0';
        o.optional = true;
        o.placeholder = 2;
        o.datatype = 'range(1,255)';

        o = s.option(form.Flag, 'wmm', _('WMM Mode'));
        o.ucisection = 'default_radio0';
        o.default = o.enabled;

        o = s.option(form.Flag, 'short_preamble', _('Short Preamble'));
        o.ucisection = 'default_radio0';
        o.default = o.enabled;

        s = m.section(form.NamedSection, 'radio1', 'wifi-device', _('5G Wi-Fi'));

        o = s.option(CBIWifiFrequencyValue, '_htmode');
        o.ucisection = s.section;

        o = s.option(form.Value, 'beacon_int', _('Beacon Interval'));
        o.default = o.enabled;
        o.datatype = 'range(15,65535)';
        o.placeholder = 100;
        o.rmempty = true;

        o = s.option(form.Value, 'dtim_period', _('DTIM Interval'));
        o.ucisection = 'default_radio1';
        o.optional = true;
        o.placeholder = 2;
        o.datatype = 'range(1,255)';

        o = s.option(form.Flag, 'wmm', _('WMM Mode'));
        o.ucisection = 'default_radio1';
        o.default = o.enabled;

        o = s.option(form.Flag, 'short_preamble', _('Short Preamble'));
        o.ucisection = 'default_radio1';
        o.default = o.enabled;

        return m.render();
    }
});
