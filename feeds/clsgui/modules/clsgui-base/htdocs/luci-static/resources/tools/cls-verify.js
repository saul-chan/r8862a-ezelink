'use strict';
'require baseclass';

return baseclass.extend({
    isDNSServerValid: function(address) {
        // 正则表达式模式，用于匹配 DNS 地址
        var pattern = /^(?:(?!-|_)[A-Za-z0-9-]{1,63}(?<!-)\.)+[A-Za-z]{2,6}$/;
          
        // 使用正则表达式匹配地址
        return pattern.test(address);
    }
});
