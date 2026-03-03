'use strict';
'require baseclass';
'require dom';
'require ui';
'require uci';
'require form';
'require network';
'require firewall';
'require tools.prng as random';

var protocols = [
    'ip', 0, 'IP',
    'hopopt', 0, 'HOPOPT',
    'icmp', 1, 'ICMP',
    'igmp', 2, 'IGMP',
    'ggp', 3 , 'GGP',
    'ipencap', 4, 'IP-ENCAP',
    'st', 5, 'ST',
    'tcp', 6, 'TCP',
    'egp', 8, 'EGP',
    'igp', 9, 'IGP',
    'pup', 12, 'PUP',
    'udp', 17, 'UDP',
    'hmp', 20, 'HMP',
    'xns-idp', 22, 'XNS-IDP',
    'rdp', 27, 'RDP',
    'iso-tp4', 29, 'ISO-TP4',
    'dccp', 33, 'DCCP',
    'xtp', 36, 'XTP',
    'ddp', 37, 'DDP',
    'idpr-cmtp', 38, 'IDPR-CMTP',
    'ipv6', 41, 'IPv6',
    'ipv6-route', 43, 'IPv6-Route',
    'ipv6-frag', 44, 'IPv6-Frag',
    'idrp', 45, 'IDRP',
    'rsvp', 46, 'RSVP',
    'gre', 47, 'GRE',
    'esp', 50, 'IPSEC-ESP',
    'ah', 51, 'IPSEC-AH',
    'skip', 57, 'SKIP',
    'icmpv6', 58, 'IPv6-ICMP',
    'ipv6-icmp', 58, 'IPv6-ICMP',
    'ipv6-nonxt', 59, 'IPv6-NoNxt',
    'ipv6-opts', 60, 'IPv6-Opts',
    'rspf', 73, 'RSPF',
    'rspf', 73, 'CPHB',
    'vmtp', 81, 'VMTP',
    'eigrp', 88, 'EIGRP',
    'ospf', 89, 'OSPFIGP',
    'ax.25', 93, 'AX.25',
    'ipip', 94, 'IPIP',
    'etherip', 97, 'ETHERIP',
    'encap', 98, 'ENCAP',
    'pim', 103, 'PIM',
    'ipcomp', 108, 'IPCOMP',
    'vrrp', 112, 'VRRP',
    'l2tp', 115, 'L2TP',
    'isis', 124, 'ISIS',
    'sctp', 132, 'SCTP',
    'fc', 133, 'FC',
    'mh', 135, 'Mobility-Header',
    'ipv6-mh', 135, 'Mobility-Header',
    'mobility-header', 135, 'Mobility-Header',
    'udplite', 136, 'UDPLite',
    'mpls-in-ip', 137, 'MPLS-in-IP',
    'manet', 138, 'MANET',
    'hip', 139, 'HIP',
    'shim6', 140, 'Shim6',
    'wesp', 141, 'WESP',
    'rohc', 142, 'ROHC',
];

function lookupProto(x) {
    if (x == null || x === '')
        return null;
        
    var s = String(x).toLowerCase();
        
    for (var i = 0; i < protocols.length; i += 3)
        if (s == protocols[i] || s == protocols[i+1])
            return [ protocols[i+1], protocols[i+2], protocols[i] ];
        
    return [ -1, x, x ];
}

return baseclass.extend({
    fmt: function(fmtstr, args, values) {
        var repl = [],
            wrap = false,
            tokens = [];
        
        if (values == null) {
            values = [];
            wrap = true;
        }

        var get = function(args, key) {
            var names = key.trim().split(/\./),
                obj = args,
                ctx = obj;

            for (var i = 0; i < names.length; i++) {
                if (!L.isObject(obj))
                    return null;
        
                ctx = obj;
                obj = obj[names[i]];
            }

            if (typeof(obj) == 'function')
                return obj.call(ctx);

            return obj;
        };

        var isset = function(val) {
            if (L.isObject(val) && !dom.elem(val)) {
                for (var k in val)
                    if (val.hasOwnProperty(k))
                        return true;

                return false;
            }
            else if (Array.isArray(val)) {
                return (val.length > 0);
            }
            else {
                return (val !== null && val !== undefined && val !== '' && val !== false);
            }
        };

        var parse = function(tokens, text) {
            if (dom.elem(text)) {
                tokens.push('<span data-fmt-placeholder="%d"></span>'.format(values.length));
                values.push(text);
            }
            else {
                tokens.push(String(text).replace(/\\(.)/g, '$1'));
            }
        };

        for (var i = 0, last = 0; i <= fmtstr.length; i++) {
            if (fmtstr.charAt(i) == '%' && fmtstr.charAt(i + 1) == '{') {
                if (i > last)
                    parse(tokens, fmtstr.substring(last, i));
        
                var j = i + 1,  nest = 0;
        
                var subexpr = [];
        
                for (var off = j + 1, esc = false; j <= fmtstr.length; j++) {
                    var ch = fmtstr.charAt(j);
        
                    if (esc) {
                        esc = false;
                    }
                    else if (ch == '\\') {
                        esc = true;
                    }
                    else if (ch == '{') {
                        nest++;
                    }
                    else if (ch == '}') {
                        if (--nest == 0) {
                            subexpr.push(fmtstr.substring(off, j));
                            break;
                        }
                    }
                    else if (ch == '?' || ch == ':' || ch == '#') {
                        if (nest == 1) {
                            subexpr.push(fmtstr.substring(off, j));
                            subexpr.push(ch);
                            off = j + 1;
                        }
                    }
                }
        
                var varname  = subexpr[0].trim(),
                    op1      = (subexpr[1] != null) ? subexpr[1] : '?',
                    if_set   = (subexpr[2] != null && subexpr[2] != '') ? subexpr[2] : '%{' + varname + '}',
                    op2      = (subexpr[3] != null) ? subexpr[3] : ':',
                    if_unset = (subexpr[4] != null) ? subexpr[4] : '';
               /* Invalid expression */
                if (nest != 0 || subexpr.length > 5 || varname == '') {
                    return fmtstr;
                }
                /* enumeration */
                else if (op1 == '#' && subexpr.length == 3) {
                    var items = L.toArray(get(args, varname));

                    for (var k = 0; k < items.length; k++) {
                        tokens.push.apply(tokens, this.fmt(if_set, Object.assign({}, args, {
                            first: k == 0,
                            next:  k > 0,
                            last:  (k + 1) == items.length,
                            item:  items[k]
                        }), values));
                    }
                }
                /* ternary expression */
                else if (op1 == '?' && op2 == ':' && (subexpr.length == 1 || subexpr.length == 3 || subexpr.length == 5)) {
                    var val = get(args, varname);
                
                    if (subexpr.length == 1)
                        parse(tokens, isset(val) ? val : '');
                    else if (isset(val))
                        tokens.push.apply(tokens, this.fmt(if_set, args, values));
                    else
                        tokens.push.apply(tokens, this.fmt(if_unset, args, values));
                }
                /* unrecognized command */
                else {
                    return fmtstr;
                }

                last = j + 1;
                i = j;
            }
            else if (i >= fmtstr.length) {
                if (i > last)
                    parse(tokens, fmtstr.substring(last, i));
            }
        }

        if (wrap) {
            var node = E('span', {}, tokens.join('')),
                repl = node.querySelectorAll('span[data-fmt-placeholder]');

            for (var i = 0; i < repl.length; i++)
                repl[i].parentNode.replaceChild(values[repl[i].getAttribute('data-fmt-placeholder')], repl[i]);

            return node;
        }
        else {
            return tokens;
        }
    },

    map_invert: function(v, fn) {
        return L.toArray(v).map(function(v) {
            v = String(v);

            if (fn != null && typeof(v[fn]) == 'function')
                v = v[fn].call(v);

            return {
                ival: v,
                inv: v.charAt(0) == '!',
                val: v.replace(/^!\s*/, '')
            };
        });
    },

    lookupProto: lookupProto,

    transformHostHints: function(family, hosts) {
        var choice_values = [],
            choice_labels = {},
            ip6addrs = {},
            ipaddrs = {};

        for (var mac in hosts) {
            L.toArray(hosts[mac].ipaddrs || hosts[mac].ipv4).forEach(function(ip) {
                ipaddrs[ip] = mac;
            });

            L.toArray(hosts[mac].ip6addrs || hosts[mac].ipv6).forEach(function(ip) {
                ip6addrs[ip] = mac;
            });
        }

        if (!family || family == 'ipv4') {
            L.sortedKeys(ipaddrs, null, 'addr').forEach(function(ip) {
                var val = ip,
                    txt = hosts[ipaddrs[ip]].name || ipaddrs[ip];

                choice_values.push(val);
                choice_labels[val] = E([], [ val, ' (', E('strong', {}, [txt]), ')' ]);
            });
        }

        if (!family || family == 'ipv6') {
            L.sortedKeys(ip6addrs, null, 'addr').forEach(function(ip) {
                var val = ip,
                    txt = hosts[ip6addrs[ip]].name || ip6addrs[ip];

                choice_values.push(val);
                choice_labels[val] = E([], [ val, ' (', E('strong', {}, [txt]), ')' ]);
            });
        }

        return [choice_values, choice_labels];
    },

    addIPOption: function(s, name, label, description, family, hosts, multiple) {
        var o = s.option(multiple ? this.CBIDynamicMultiValueList : form.Value, name, label, description);

        o.modalonly = true;
        o.datatype = 'list(neg(ipmask("true")))';
        o.placeholder = multiple ? _('-- add IP --') : _('any');

        if (family != null) {
            var choices = this.transformHostHints(family, hosts);
                
            for (var i = 0; i < choices[0].length; i++)
                o.value(choices[0][i], choices[1][choices[0][i]]);
        }

        /* force combobox rendering */
        o.transformChoices = function() {
            return this.super('transformChoices', []) || {};
        };

        return o;
    },

    CBIProtocolSelect: form.MultiValue.extend({
        __name__: 'CBI.ProtocolSelect',
                
        addChoice: function(value, label) {
            if (!Array.isArray(this.keylist) || this.keylist.indexOf(value) == -1)
                this.value(value, label);
        },

        load: function(section_id) {
            var cfgvalue = L.toArray(this.super('load', [section_id]) || this.default).sort();
                
            ['all', 'tcp', 'udp', 'icmp'].concat(cfgvalue).forEach(L.bind(function(value) {
                switch (value) {
                    case 'all':
                    case 'any':
                    case '*':
                        this.addChoice('all', _('Any'));
                        break;
                    case 'tcpudp':
                        this.addChoice('tcp', 'TCP');
                        this.addChoice('udp', 'UDP');
                        break;
                    default:
                        var m = value.match(/^(0x[0-9a-f]{1,2}|[0-9]{1,3})$/),
                            p = lookupProto(m ? +m[1] : value);

                        this.addChoice(p[2], p[1]);
                        break;
                    }
                }, this));

                if (cfgvalue == '*' || cfgvalue == 'any' || cfgvalue == 'all')
                    cfgvalue = 'all';

                return cfgvalue;
        },

        renderWidget: function(section_id, option_index, cfgvalue) {
            var value = (cfgvalue != null) ? cfgvalue : this.default,
                choices = this.transformChoices();

            var widget = new ui.Dropdown(L.toArray(value), choices, {
                id: this.cbid(section_id),
                sort: this.keylist,
                multiple: true,
                optional: false,
                display_items: 10,
                dropdown_items: -1,
                create: true,
                disabled: (this.readonly != null) ? this.readonly : this.map.readonly,
                validate: function(value) {
                    var v = L.toArray(value);

                    for (var i = 0; i < v.length; i++) {
                        if (v[i] == 'all')
                            continue;
                
                        var m = v[i].match(/^(0x[0-9a-f]{1,2}|[0-9]{1,3})$/);
                
                        if (m ? (+m[1] > 255) : (lookupProto(v[i])[0] == -1))
                            return _('Unrecognized protocol');
                    }

                    return true;
                }
            });

            widget.createChoiceElement = function(sb, value) {
                var p = lookupProto(value);

                return ui.Dropdown.prototype.createChoiceElement.call(this, sb, p[2], p[1]);
            };

            widget.createItems = function(sb, value) {
                var values = L.toArray(value).map(function(value) {
                    var m = value.match(/^(0x[0-9a-f]{1,2}|[0-9]{1,3})$/),
                        p = lookupProto(m ? +m[1] : value);

                    return (p[0] > -1) ? p[2] : p[1];
                });

                values.sort();

                return ui.Dropdown.prototype.createItems.call(this, sb, values.join(' '));
            };

            widget.toggleItem = function(sb, li) {
                var value = li.getAttribute('data-value'),
                    toggleFn = ui.Dropdown.prototype.toggleItem;

                toggleFn.call(this, sb, li);

                if (value == 'all') {
                    var items = li.parentNode.querySelectorAll('li[data-value]');

                    for (var j = 0; j < items.length; j++)
                        if (items[j] !== li)
                            toggleFn.call(this, sb, items[j], false);
                }
                else {
                    toggleFn.call(this, sb, li.parentNode.querySelector('li[data-value="all"]'), false);
                }
            };

            return widget.render();
        }
    })
});

