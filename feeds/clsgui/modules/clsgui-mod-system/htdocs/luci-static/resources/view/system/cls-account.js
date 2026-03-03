'use strict';
'require view';
'require dom';
'require rpc';
'require uci';
'require form';
'require ui';

document.querySelector('head').appendChild(E('link', {
    'rel': 'stylesheet',
    'type': 'text/css',
    'href': L.resource('view/system/css/cls-general.css')
}));

var callUciSet = rpc.declare({
    object: 'uci',
    method: 'set',
    params: [ 'config', 'type', 'values' ]
});

var callUciCommit = rpc.declare({
    object: 'uci',
    method: 'commit',
    params: [ 'config' ]
});

var callSessionLogin = rpc.declare({
    object: 'session',
    method: 'login',
    params: [ 'username', 'password' ],
    expect: { '': {} }
});

var callSetPassword = rpc.declare({
    object: 'luci',
    method: 'setPassword',
    params: [ 'username', 'password' ],
    expect: { '': {} }
});

var updateAccount = {};

function setUsername() {
    var username = document.getElementById("userName");

    if (username.value == '')
        return;

    updateAccount['username'] = username.value;
}

function validateOldPassword() {
    var oldPasswordInput = document.getElementById("oldPassword"),
        oldPasswordError = document.getElementById("oldPasswordError"),
        usrname = uci.get_first('rpcd', 'login', 'username');

    return callSessionLogin(usrname, oldPasswordInput.value).then(function(res) {
        var array = Object.keys(res);

        if (array.length == 0) {
            oldPasswordInput.classList.add("error");
            oldPasswordError.classList.replace("hidden", "visible");
            return false;
        } else {
            oldPasswordInput.classList.remove("error");
            oldPasswordError.classList.replace("visible", "hidden");
            return true;
        }
    });
}

function validateNewPasswords() {
    var newPasswordInput = document.getElementById("newPassword"),
        confirmPasswordInput = document.getElementById("confirmPassword"),
        confirmPasswordError = document.getElementById("confirmPasswordError");

    if (newPasswordInput.value !== confirmPasswordInput.value) {
        confirmPasswordInput.classList.add("error");
        confirmPasswordError.classList.replace("hidden", "visible");
        return false;
    } else {
        confirmPasswordInput.classList.remove("error");
        confirmPasswordError.classList.replace("visible", "hidden");
        updateAccount['newpasswd'] = newPasswordInput.value;
        return true;
    }
}

function handleApplyButton() {
    var oldPasswordInput = document.getElementById("oldPassword"),
        newPasswordInput = document.getElementById("newPassword"),
        confirmPasswordInput = document.getElementById("confirmPassword"),
        successMessage = document.getElementById("successMessage"),
        isNewPasswordsValid = validateNewPasswords();

    if (oldPasswordInput.value == '' && newPasswordInput.value == '' && confirmPasswordInput.value == '') {

        if (updateAccount.hasOwnProperty('username') && updateAccount.username != '') {
            callUciSet('rpcd', 'login', { 'username': updateAccount.username });
            callUciCommit('rpcd');
            ui.changes.displayStatus('notice spinning',
            E('p', _('Starting configuration apply...')));

            window.setTimeout(function() {
                ui.changes.displayStatus(false);
            }, L.env.apply_display*2000);
        } else {
            ui.changes.displayStatus('notice',
                E('p', _('There are no changes to apply')));
            window.setTimeout(function() {
                ui.changes.displayStatus(false);
            }, L.env.apply_display*2000);
        }
    } else {
        validateOldPassword().then(function(res) {
            if (res && isNewPasswordsValid && newPasswordInput.value !== '') {
                if (updateAccount.hasOwnProperty('username') && updateAccount.username != '') {
                    callUciSet('rpcd', 'login', { 'username': updateAccount.username });
                    callUciCommit('rpcd');
                }
                callSetPassword(uci.get_first('rpcd', 'login', 'username'), updateAccount.newpasswd);
                ui.changes.displayStatus('notice spinning',
                    E('p', _('Starting configuration apply...')));

                window.setTimeout(function() {
                    ui.changes.displayStatus(false);
                }, L.env.apply_display*2000);

                successMessage.classList.replace("hidden", "visible");
                oldPasswordInput.value = "";
                newPasswordInput.value = "";
                confirmPasswordInput.value = "";
            } else {
                successMessage.classList.replace("visible", "hidden");
            }
        });
    }
}

return view.extend({
    load: function() {
        return Promise.all([
            uci.load('rpcd')
        ]);
    },

    render: function(data) {
        var elem = E('div');
        var usrn = uci.get_first('rpcd', 'login', 'username');

        dom.content(elem, [
            E('div', { 'class': 'cbi-section'}, [
                E('h2', { 'style': 'box-shadow: none;' }, _('Account Management')),
                E('div', { 'class': 'form-group' }, [
                    E('label', { 'for': 'userName' }, _('User Name')),
                    E('input', {
                        'id': 'userName',
                        'type': 'text',
                        'value': usrn,
                        'change': L.bind(setUsername)
                    })
                ]),
                E('div', { 'class': 'form-group' }, [
                    E('label', { 'for': 'oldPassword' },  _('Old Password')),
                    E('input', {
                        'id': 'oldPassword' ,
                        'type': 'password',
                        'placeholder': _('Please input old password'),
                        'change': L.bind(validateOldPassword)
                    }),
                    E('small', { 'id': 'oldPasswordError', 'class': 'hidden' }, _('Error Password'))
                ]),
                E('div', { 'class': 'form-group' }, [
                    E('label', { 'for': 'newPassword' }, _('New Password')),
                    E('input', {
                        'id': 'newPassword' ,
                        'type': 'password',
                        'placeholder': _('Please input new password')
                    })
                ]),
                E('div', { 'class': 'form-group last-group' }, [
                    E('label', { 'for': 'confirmPassword' },  _('Confirm New Password')),
                    E('input', {
                        'id': 'confirmPassword' ,
                        'type': 'password',
                        'placeholder': _('Please confirm new password'),
                        'change': L.bind(validateNewPasswords)
                    }),
                    E('small', { 'id': 'confirmPasswordError', 'class': 'hidden' }, _('The passwords entered twice are inconsistent'))
                ])
            ]),
            E('div', { 'class': 'cbi-page-actions' }, [
                E('button', {
                    'id': 'applyButton',
                    'type': 'button',
                    'click': L.bind(handleApplyButton)
                }, _('Save & Apply'))
            ])
        ]);

        return elem;
    },

    handleSave: null,
    handleSaveApply: null,
    handleReset: null
});
