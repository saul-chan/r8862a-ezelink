(function () {
  const WIZARD_SESSION_KEY = 'clsemi:mesh-gui-onboarding';
  const STORAGE_KEY = 'lastPageId';
  const SCAN_LIST = [];

  const $ = (selector, ctx = document) => ctx.querySelector(selector);
  const $$ = (selector, ctx = document) => Array.from(ctx.querySelectorAll(selector));

  const on = (selectorOrElement, event, handler, ctx = document) => {
    let el = selectorOrElement;
    if (typeof selectorOrElement === 'string') el = $(selectorOrElement, ctx);
    if (el) el.addEventListener(event, handler);
  };

  const normalizeId = (x) => (x || '').replace(/^#/, '');

  function Request(method, url, data) {
    return new Promise((resolve, reject) => {
      const xhr = new XMLHttpRequest();
      xhr.open(method, url, true);
      xhr.setRequestHeader('Content-Type', 'application/json');
      if (window.__TOKEN) xhr.setRequestHeader('rest_token', window.__TOKEN);
      xhr.onload = () => {
        if (xhr.status >= 200 && xhr.status < 300) resolve(JSON.parse(xhr.responseText));
        else reject({ status: xhr.status, statusText: xhr.statusText });
      };
      xhr.onerror = () => reject({ status: xhr.status, statusText: 'Network Error' });
      xhr.send(data ? JSON.stringify(data) : null);
    });
  }

  function showModal(id) {
    const m = document.getElementById(id);
    if (!m) return;
    m.classList.add('active');
    m.setAttribute('aria-hidden', 'false');
  }

  function hideModal(id) {
    const m = document.getElementById(id);
    if (!m) return;
    m.classList.remove('active');
    m.setAttribute('aria-hidden', 'true');
  }

  function pingOnce(host, path = '/luci-static/argon/img/internet.png', timeoutMs = 2000) {
    return new Promise((resolve) => {
      const img = new Image();
      const proto = window.location.protocol.replace(':', '');
      const url = `${proto}://${host}${path}?_=${Date.now()}`;
      const timer = setTimeout(() => {
        cleanup();
        resolve(false);
      }, timeoutMs);
      function cleanup() {
        clearTimeout(timer);
        img.onload = img.onerror = null;
      }
      img.onload = () => {
        cleanup();
        resolve(true);
      };
      img.onerror = () => {
        cleanup();
        resolve(true);
      };
      img.src = url;
    });
  }

  function awaitConnectivity(hosts = [], opt = {}) {
    const {
      path = '/luci-static/argon/img/internet.png',
      delayMs = 3000,
      intervalMs = 2500,
      maxRounds = 60,
      onTick,
      onDone,
      onTimeout
    } = opt;
    const ipaddrs = hosts.length ? hosts : [window.location.host];
    let round = 0;
    let timer = null;
    function stop() {
      clearInterval(timer);
      timer = null;
    }
    function tryRound() {
      round++;
      Promise.all(ipaddrs.map((h) => pingOnce(h, path))).then((results) => {
        const idx = results.findIndex((ok) => ok);
        const ok = idx !== -1;
        onTick && onTick(round, ok);
        if (ok) {
          stop();
          onDone && onDone(ipaddrs[idx]);
        } else if (round >= maxRounds) {
          stop();
          onTimeout && onTimeout();
        }
      });
    }
    setTimeout(() => {
      tryRound();
      timer = setInterval(tryRound, intervalMs);
    }, delayMs);
  }

  const DataStore = (() => {
    let state = { mode: null, static: null, wifi: null, pppoe: null, backhaul: null, fronthaul: null };
    function save() {
      try {
        localStorage.setItem(WIZARD_SESSION_KEY, JSON.stringify(state));
      } catch {}
    }
    function load() {
      try {
        const raw = localStorage.getItem(WIZARD_SESSION_KEY);
        if (raw) state = Object.assign(state, JSON.parse(raw));
      } catch {}
    }
    function clearAll() {
      state = { mode: null, static: null, wifi: null, pppoe: null, mesh: null };
      save();
    }
    function clearStep(step) {
      if (step in state) {
        state[step] = null;
        save();
      }
    }
    function setStep(step, data) {
      state[step] = data;
      save();
    }
    function getStep(step) {
      return state[step];
    }
    function getAll() {
      return JSON.parse(JSON.stringify(state));
    }
    load();
    return { setStep, getStep, getAll, clearAll, clearStep };
  })();

  function collectGroupInputs(container) {
    const obj = {};
    container.querySelectorAll('.group input').forEach((input) => {
      const name = input.name;
      if (!name) return;
      obj[name] = (input.value || '').trim();
    });
    return obj;
  }

  function goBack() {
    const cur = getActivePageId();
    const mode = DataStore.getStep('mode');
    switch (cur) {
      case 'wifi':
        if (mode === 'static') return route('static');
        if (mode === 'pppoe') return route('pppoe');
        return route('mode');
      case 'pppoe':
      case 'static':
        return route('mode');
      case 'mesh':
        DataStore.clearStep('backhaul');
        return route('repeater');
      case 'scan':
        DataStore.clearStep('fronthaul');
        const scanContainer = $('.scan-container');
        const scanResults = $('.scan-results');
        if (scanResults && !scanResults.classList.contains('hidden')) {
          scanResults.classList.add('hidden');
          if (scanContainer) scanContainer.classList.remove('hidden');
          const list = $('#wifi-list');
          if (list) list.innerHTML = '';
        }
        return route('repeater');
      case 'repeater':
        return route('mode');
      case 'mode':
        return route('manual');
      default:
        return;
    }
  }

  function getActivePageId() {
    const cur = $$('.page.active');
    return cur.length ? cur[0].id : null;
  }

  function shouldSkip(input) {
    return input.disabled || input.readOnly || isHidden(input);
  }

  function isHidden(el) {
    if (!el) return true;
    if (el.offsetParent !== null) return false;
    const style = getComputedStyle(el);
    return style.display === 'none' || style.visibility === 'hidden' || style.opacity === '0';
  }

  function toggleError(input, ok) {
    const formGroup = input.closest('.form-group');
    const errorMsg = formGroup && formGroup.querySelector('.error-message');
    if (formGroup) formGroup.dataset.valid = ok ? 'true' : 'false';
    if (errorMsg) errorMsg.style.display = ok ? 'none' : 'block';
  }

  function isMAC(mac) {
    const regex = /^([0-9a-f]{2}:){5}[0-9a-f]{2}$/i;
    return regex.test(mac.trim());
  }

  function isIPv4(s) {
    if (typeof s !== 'string') return false;
    const parts = s.trim().split('.');
    if (parts.length !== 4) return false;
    for (let p of parts) {
      if (p === '' || !/^\d{1,3}$/.test(p)) return false;
      const n = +p;
      if (n < 0 || n > 255) return false;
    }
    return true;
  }

  function ipv4ToInt(s) {
    return s.split('.').reduce((acc, v) => (acc << 8) | +v, 0) >>> 0;
  }

  function isValidNetmask(s) {
    if (!isIPv4(s)) return false;
    const m = ipv4ToInt(s);
    const inv = ~m >>> 0;
    return (inv & (inv + 1)) === 0;
  }

  function makeCheckRule(overrides = {}) {
    return function checkRule(input) {
      if (overrides[input.name]) return !!overrides[input.name](input);
      const v = (input.value || '').trim();
      return v.length > 0;
    };
  }

  function validateContainer(container, checkRule) {
    const inputs = container.querySelectorAll('.group input');
    let allOk = true,
      firstBad = null;
    inputs.forEach((input) => {
      if (shouldSkip(input)) return;
      const ok = checkRule(input);
      toggleError(input, ok);
      if (!ok && !firstBad) firstBad = input;
      allOk = allOk && ok;
    });
    return { allOk, firstBad };
  }

  function bindSubmitWithValidation(
    btnSelector,
    { scopeSelector = '.container', rules = {}, onValid = () => {} } = {}
  ) {
    const btn = document.querySelector(btnSelector);
    if (!btn) return;
    const checkRule = makeCheckRule(rules);
    on(btn, 'click', function (e) {
      e.preventDefault();
      if (btnSelector === '#btn-confirm') scopeSelector = '.modal-card';
      const container = this.closest(scopeSelector);
      if (!container) return;
      const { allOk, firstBad } = validateContainer(container, checkRule);
      if (!allOk) {
        firstBad && firstBad.focus();
        firstBad && firstBad.scrollIntoView({ behavior: 'smooth', block: 'center' });
        return;
      }
      onValid(container, btn);
    });
    on(document, 'input', function (e) {
      const input = e.target;
      if (!(input instanceof HTMLInputElement)) return;
      const scope = input.closest(scopeSelector);
      if (!scope) return;
      if (!input.closest('.group')) return;
      if (shouldSkip(input)) return;
      toggleError(input, checkRule(input));
    });
  }

  function startButtonCountdown(
    btn,
    seconds,
    { runningText = (s) => `正在拨号中... 剩余${s}s`, doneText = '开始拨号', onTick = null, onDone = null } = {}
  ) {
    let remain = seconds;
    btn.disabled = true;
    btn.textContent = runningText(remain);
    const timer = setInterval(() => {
      remain--;
      if (remain <= 0) {
        clearInterval(timer);
        btn.disabled = false;
        btn.textContent = doneText;
        onDone && onDone();
        return;
      }
      btn.textContent = runningText(remain);
      onTick && onTick(remain);
    }, 1000);
    return () => {
      clearInterval(timer);
      btn.disabled = false;
      btn.textContent = doneText;
    };
  }

  function openModalWithSSID(ssidValue) {
    const form = $('#backhaul-form');
    const ssidInput = form.querySelector('input[name="ssid"]');
    const pwdInput = form.querySelector('input[name="password"]');
    const bssidInput = form.querySelector('input[name="bssid"]');
    ssidInput.value = ssidValue || '';
    ssidInput.disabled = true;
    pwdInput.value = '';
    bssidInput.value = '';
    form.querySelectorAll('.form-group').forEach((fg) => (fg.dataset.valid = 'true'));
    $('#wifi-modal').classList.add('active');
    $('#wifi-modal').setAttribute('aria-hidden', 'false');
    setTimeout(() => pwdInput.focus(), 50);
  }

  function closeModal() {
    $('#wifi-modal').classList.remove('active');
    $('#wifi-modal').setAttribute('aria-hidden', 'true');
  }

  function parseSignalValue(signalStr) {
    const match = signalStr.match(/-?\d+/);
    return match ? parseInt(match[0], 10) : null;
  }

  function dbmToPercent(dbm) {
    const val = parseSignalValue(dbm);
    const max = -30,
      min = -90;
    const x = Math.max(min, Math.min(max, val));
    return Math.round(((x - min) / (max - min)) * 100);
  }

  function signalColor(dbm) {
    const val = parseSignalValue(dbm);
    if (val >= -55) return '#22c55e';
    if (val >= -70) return '#f59e0b';
    return '#ef4444';
  }

  function signalCell(dbm) {
    const pct = dbmToPercent(dbm);
    const color = signalColor(dbm);
    const label = `${dbm}`;
    return `
			<div class="signal" aria-label="Signal ${label}">
				<div class="bar" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="${pct}" aria-valuetext="${label}">
					<span style="width:${pct}%; background:${color};"></span>
				</div>
				<span class="val">${label}</span>
			</div>
		`;
  }

  function renderWifiList(data) {
    const tbody = $('#wifi-list');
    const containerTbody = tbody;
    tbody.innerHTML = '';
    data.forEach((item) => {
      const tr = document.createElement('tr');
      tr.innerHTML = `
				<td>${item.ssid}</td>
				<td>${item.channel}</td>
				<td>${signalCell(item.signal)}</td>
				<td>${item.encryption}</td>
			`;
      tr.addEventListener('click', () => openModalWithSSID(item.ssid));
      tbody.appendChild(tr);
    });
    if (data.length > 10) containerTbody.classList.add('scroll');
    else containerTbody.classList.remove('scroll');
  }

  function bubbleSortByRssiAbs(data) {
    const n = data.length;
    for (let i = 0; i < n - 1; i++) {
      for (let j = 0; j < n - 1 - i; j++) {
        if (Math.abs(data[j].rssi) > Math.abs(data[j + 1].rssi)) {
          const tmp = data[j];
          data[j] = data[j + 1];
          data[j + 1] = tmp;
        }
      }
    }
    return data;
  }

  function parseScanEntry(array) {
    const wifiData = [];
    const data = bubbleSortByRssiAbs(array);
    data.forEach(function (item) {
      if (item.ssid != '')
        wifiData.push({
          bssid: item.bssid,
          ssid: item.ssid,
          channel: item.channel,
          signal: item.rssi + ' ' + 'dBm',
          encryption: getEncryption(item.crypto.sec_protos)
        });
    });
    return wifiData;
  }

  function getEncryption(num) {
    let encr;
    const mapping = { 0: 'None', 1: 'wep-mixed', 2: 'WPA', 4: 'WPA2', 8: 'WPA3', 16: 'OWE' };
    if (mapping.hasOwnProperty(num)) return mapping[num];
    const n = parseInt(num, 10);
    if (isNaN(n)) return undefined;
    const keys = Object.keys(mapping)
      .map((k) => parseInt(k, 10))
      .filter((k) => k > 0)
      .sort((a, b) => a - b);
    for (let i = 0; i < keys.length; i++) {
      for (let j = i + 1; j < keys.length; j++) {
        if (keys[i] + keys[j] === n) {
          if (mapping[keys[j]] == 'psk2') encr = 'psk-mixed';
          else encr = mapping[keys[j]].slice(0, 3) + '-mixed';
          return encr;
        }
      }
    }
    return undefined;
  }

  function startScan() {
    const phy1 = { phyname: 'phy1', flags: 0, band: 1, channels: [], channels_len: 0 };
    Request('POST', '/restful/wifi/start_scan', phy1);
  }

  function getScanEntry() {
    const entry = { phyname: 'phy1', ap_idx: -1 };
    return Request('POST', '/restful/wifi/get_scan_entry', entry);
  }

  function backhaulDetect() {
    return Promise.race([
      Request('POST', '/restful/base/get_conf_param', { cfg: 'cls-netmanager', section: 'backhaul', param: 'backhaul' }),
      new Promise(function (_, rej) {
        setTimeout(function () {
          rej(new Error('backhaul timeout'));
        }, 3000);
      })
    ]).then(function (d) {
      const val = d?.data?.value ?? d?.value;
      if (d?.code && d.code !== 0) throw new Error(`API code ${d.code}`);
      if (val === undefined) throw new Error('invalid payload');
      return { value: val };
    });
  }

  function waitForBackhaul(intervalMs = 1000, timeoutMs = 60000) {
    var elapsed = 0;
    var countdownTimer = null;
    var remain = Math.ceil(timeoutMs / 1000);
    var countdownEl = $('#repeater-countdown');
    showModal('repeater-modal');
    function startCountdown() {
      if (countdownEl) countdownEl.textContent = remain;
      countdownTimer = setInterval(function () {
        remain -= 1;
        if (remain < 0) remain = 0;
        if (countdownEl) countdownEl.textContent = remain;
      }, 1000);
    }
    function stopCountdown() {
      if (countdownTimer) {
        clearInterval(countdownTimer);
        countdownTimer = null;
      }
    }
    function doneSuccess() {
      stopCountdown();
      hideModal('repeater-modal');
      showModal('modal-success');
    }
    function doneFail() {
      stopCountdown();
      hideModal('repeater-modal');
      showModal('modal-fail');
    }
    let tickTimer = null;
    let settled = false;
    function finalize(cb) {
      if (settled) return;
      settled = true;
      if (tickTimer) {
        clearTimeout(tickTimer);
        tickTimer = null;
      }
      cb && cb();
    }
    return new Promise(function (resolve, reject) {
      function tick() {
        backhaulDetect()
          .then(function (res) {
            if (settled) return;
            if (res && res.value !== 'None') {
              finalize(doneSuccess);
              resolve(res);
            } else {
              elapsed += intervalMs;
              if (elapsed >= timeoutMs) {
                finalize(doneFail);
                reject(new Error('Timeout waiting for backhaul'));
              } else {
                tickTimer = setTimeout(tick, intervalMs);
              }
            }
          })
          .catch(function () {
            finalize(doneSuccess);
            resolve({ reason: 'network-lost' });
          });
      }
      startCountdown();
      tick();
    });
  }

  function wifiToSetParams(
    wifi,
    {
      cfg = 'wireless',
      keyToSection = { ssid2: 'wifinet0_1', ssid5: 'wifinet1_2' },
      keyToParam = { ssid2: 'ssid', ssid5: 'ssid' },
      order = ['ssid2', 'ssid5']
    } = {}
  ) {
    const params = [];
    order.forEach((key) => {
      const val = wifi && wifi[key] != null ? String(wifi[key]).trim() : '';
      if (!val) return;
      const section = keyToSection[key];
      const param = keyToParam[key];
      if (!section || !param) return;
      params.push({ cfg, section, param, value: val });
    });
    Object.keys(wifi || {}).forEach((key) => {
      if (order.includes(key)) return;
      const val = wifi[key] != null ? String(wifi[key]).trim() : '';
      if (!val) return;
      const section = keyToSection[key];
      const param = keyToParam[key];
      if (!section || !param) return;
      params.push({ cfg, section, param, value: val });
    });
    return params;
  }

  function awaitApiReady(url, opt) {
    opt = opt || {};
    var method = opt.method || 'POST';
    var body = opt.body || null;
    var intervalMs = opt.intervalMs || 1000;
    var maxTries = opt.maxTries || 60;
    var tries = 0;
    return new Promise(function (resolve, reject) {
      function tick() {
        tries += 1;
        Request(method, url, body)
          .then(resolve)
          .catch(function () {
            if (tries >= maxTries) reject(new Error('REST not ready'));
            else setTimeout(tick, intervalMs);
          });
      }
      tick();
    });
  }

  function setApplyConfParam(params) {
    const tasks = params.map((p) => () => Request('POST', '/restful/base/set_apply_conf_param', p).catch((err) => { throw err; }));
    const chain = tasks.reduce((prev, task) => prev.then(() => task()), Promise.resolve());
    return chain.catch((err) => {
      throw err;
    });
  }

  function switchOpmode(mode) {
    const params = buildConfParams('cls-opmode', { mode: mode });
    setApplyConfParam(params);
  }

  function applyProtocol(proto) {
    const params = buildConfParams('network', { proto: proto });
    setApplyConfParam(params);
  }

  function setOnboardingDone() {
    const params = buildConfParams('cls-netmanager', { onboarding_done: '1' });
    setApplyConfParam(params);
  }

  function applyMeshParams(data) {
    const cfg = 'mesh';
    const params = buildConfParams(cfg, { ssid: data.ssid, key: data.password });
    setApplyConfParam(params);
  }

  function applyWifiParams(data) {
    const params = buildConfParams('wireless', { ssid2: data.ssid2, ssid5: data.ssid5, key: data.wifiPassword });
    setApplyConfParam(params);
  }

  function applyNetworkParams(data) {
    const params = buildConfParams('network', {
      proto: data.proto,
      ipaddr: data.ipaddr,
      netmask: data.netmask,
      gateway: data.gateway,
      dns1: data.dns1,
      ...(data.dns2 ? { dns2: data.dns2 } : {})
    });
    setApplyConfParam(params);
  }

  const sectionMap = {
    wireless: {
      ssid2: { section: 'wifinet0_1', param: 'ssid' },
      ssid5: { section: 'wifinet1_2', param: 'ssid' },
      key: { section: 'both', param: 'key' }
    },
    network: { default: 'wan' },
    'cls-netmanager': { default: 'config' },
    'cls-opmode': { default: 'globals' }
  };

  function buildConfParams(conf, params) {
    const result = [];
    const rules = sectionMap[conf];
    if (conf === 'wireless') {
      if (params.ssid2) {
        result.push({ cfg: conf, section: 'wifinet0_1', param: 'ssid', value: params.ssid2 });
        if (params.key) {
          result.push({ cfg: conf, section: 'wifinet0_1', param: 'key', value: params.key });
        }
      }
      if (params.ssid5) {
        result.push({ cfg: conf, section: 'wifinet1_2', param: 'ssid', value: params.ssid5 });
        if (params.key) {
          result.push({ cfg: conf, section: 'wifinet1_2', param: 'key', value: params.key });
        }
      }
    } else if (conf === 'mesh') {
      result.push({ cfg: 'wireless', section: 'station_iface', param: 'ssid', value: params.ssid });
      result.push({ cfg: 'wireless', section: 'station_iface', param: 'key', value: params.key });
    } else {
      const section = rules.default;
      Object.entries(params).forEach(([k, v]) => {
        result.push({ cfg: conf, section, param: k, value: v });
      });
    }
    return result;
  }

  function registerEventListener() {
    on('#agreement', 'click', function (e) {
      if (e.target.checked) {
        $('#btn-start').removeAttribute('disabled');
        $('#btn-start').style.backgroundColor = '#5e72e4';
      } else {
        $('#btn-start').setAttribute('disabled', true);
        $('#btn-start').style.backgroundColor = '#c1c2ca';
      }
    });

    on('#btn-start', 'click', function () {
      backhaulDetect().then(function (res) {
        if (res.value != 'None') {
          route('mode');
        } else {
          route('detect');
          setTimeout(() => {
            route('manual');
          }, 3000);
        }
      });
    });

    on('#btn-recheck', 'click', function () {
      backhaulDetect().then(function (res) {
        if (res.value != 'None') {
          route('detect');
          setTimeout(() => {
            route('mode');
          }, 3000);
        } else {
          route('detect');
          setTimeout(() => {
            route('manual');
          }, 3000);
        }
      });
    });

    on('#manual-configuration', 'click', function () {
      route('mode');
    });

    on(document, 'click', function (e) {
      const btn = e.target.closest('.btn-return');
      if (btn) {
        e.preventDefault();
        goBack();
      }
    });

    $$('.mode-item').forEach(function (item) {
      on(item, 'click', function () {
        const page = this.getAttribute('data-page');
        if (['static', 'pppoe', 'dhcp', 'bridge', 'repeater'].includes(page)) {
          DataStore.clearAll();
          DataStore.setStep('mode', page);
        }
        switch (page) {
          case 'dhcp':
            route('wifi');
            break;
          case 'bridge':
            route('wifi');
            break;
          default:
            route(page);
            break;
        }
      });
    });

    $$('.repeater-item').forEach(function (item) {
      on(item, 'click', function () {
        const page = this.getAttribute('data-page');
        route(page);
        if (page === 'scan') {
          startScan();
          setTimeout(function () {
            getScanEntry().then(function (res) {
              renderWifiList(parseScanEntry(res.scan_ap_array));
              $('.scan-container').classList.add('hidden');
              $('.scan-results').classList.remove('hidden');
            });
          }, 3000);
        }
      });
    });

    on('#btn-success-now', 'click', function () {
      window.location.href = '/cgi-bin/luci';
    });

    on('#btn-fail-close', 'click', function () {
      hideModal('modal-fail');
    });

    $$('#btn-othermode').forEach(function (item) {
      on(item, 'click', function () {
        route('mode');
      });
    });

    on('#btn-skip', 'click', function () {
      DataStore.clearAll();
      DataStore.setStep('mode', 'bridge');
      route('wifi');
    });

    on(document, 'input', function (e) {
      const input = e.target;
      if (!(input instanceof HTMLInputElement)) return;
      if (input.name !== 'ssid2') return;
      const container = input.closest('.container');
      if (!container) return;
      const ssid5El = container.querySelector('input[name="ssid5"]');
      if (!ssid5El || !ssid5El.disabled) return;
      const base = (input.value || '').trim();
      const cur5 = ssid5El.value || '';
      const m = cur5.match(/^(.*?)(-5G.*)$/i);
      let next5 = '';
      if (!base) next5 = '';
      else if (m) next5 = base + m[2];
      else next5 = base + '-5G';
      if (ssid5El.value !== next5) ssid5El.value = next5;
    });

    bindSubmitWithValidation('#btn-done', {
      rules: { wifiPassword: (el) => (el.value || '').length >= 8 },
      onValid(container) {
        let data = {};
        const wifiData = collectGroupInputs(container);
        DataStore.setStep('wifi', wifiData);
        route('info');
        data = DataStore.getAll();
        switch (data.mode) {
          case 'dhcp':
          case 'bridge':
            let opmode = data.mode == 'dhcp' ? 'router' : 'bridge';
            applyWifiParams(data.wifi);
            setOnboardingDone();
            switchOpmode(opmode);
            setTimeout(function () {
              awaitApiReady('/restful/base/get_conf_param', {
                body: { cfg: 'wireless', section: 'wifinet0_1', param: 'ssid' },
                intervalMs: 800,
                maxTries: 80
              }).then(function () {
                route('home');
                window.location.href = '/cgi-bin/luci';
              });
            }, 5000);
            break;
          case 'static':
            applyWifiParams(data.wifi);
            setOnboardingDone();
            switchOpmode('router');
            setTimeout(function () {
              awaitApiReady('/restful/base/get_conf_param', {
                body: { cfg: 'wireless', section: 'wifinet0_1', param: 'ssid' },
                intervalMs: 800,
                maxTries: 80
              }).then(function () {
                data.static.proto = 'static';
                applyNetworkParams(data.static);
                route('home');
                window.location.href = '/cgi-bin/luci';
              });
            }, 5000);
            break;
        }
      }
    });

    let cancelDialCountdown = null;
    bindSubmitWithValidation('#btn-pppoe', {
      onValid(container, btn) {
        cancelDialCountdown = startButtonCountdown(btn, 60, {
          runningText: (s) => `正在拨号中... 剩余${s}s`,
          doneText: '开始拨号'
        });
        switchOpmode('bridge');
        if (true) {
          const staticData = collectGroupInputs(container);
          DataStore.setStep('static', staticData);
        } else {
        }
      }
    });

    bindSubmitWithValidation('#btn-next', {
      rules: {
        ipaddr: (el) => isIPv4(el.value),
        netmask: (el) => isValidNetmask(el.value),
        gateway: (el) => isIPv4(el.value),
        dns1: (el) => isIPv4(el.value),
        dns2: (el) => (el.value.trim() === '' ? true : isIPv4(el.value))
      },
      onValid(container) {
        const staticData = collectGroupInputs(container);
        DataStore.setStep('static', staticData);
        route('wifi');
      }
    });

    bindSubmitWithValidation('#btn-mesh', {
      rules: {
        password: (el) => (el.value || '').length >= 8,
        bssid: (el) => (el.value.trim() === '' ? true : isMAC(el.value))
      },
      onValid(container) {
        let data = {};
        const backhaulData = collectGroupInputs(container);
        DataStore.setStep('backhaul', backhaulData);
        data = DataStore.getAll();
        applyMeshParams(data.backhaul);
        waitForBackhaul();
      }
    });

    bindSubmitWithValidation('#btn-confirm', {
      rules: {
        password: (el) => (el.value || '').length >= 8,
        bssid: (el) => (el.value.trim() === '' ? true : isMAC(el.value))
      },
      onValid(container) {
        let data = {};
        const fronthaulData = collectGroupInputs(container);
        DataStore.setStep('fronthaul', fronthaulData);
        $('#wifi-modal').classList.remove('active');
        $('#wifi-modal').setAttribute('aria-hidden', 'true');
        $('#repeater-modal').classList.add('active');
        $('#repeater-modal').setAttribute('aria-hidden', 'false');
        data = DataStore.getAll();
        applyMeshParams(data.fronthaul);
        waitForBackhaul();
      }
    });

    on('#wifi-modal', 'click', function (e) {
      if (e.target === $('#wifi-modal')) closeModal();
    });

    on('#btn-cancel', 'click', closeModal);
  }

  function route(toId = null, { pushHash = true, remember = true, fallbackId = 'home' } = {}) {
    let id = normalizeId(
      toId != null ? toId : (location.hash || '').slice(1) || localStorage.getItem(STORAGE_KEY) || fallbackId
    );
    let target = $('#' + id);
    if (!target) {
      if (id !== fallbackId) return route(fallbackId, { pushHash, remember, fallbackId });
      return;
    }
    $$('.page').forEach((p) => p.classList.remove('active'));
    target.classList.add('active');
    if (pushHash && location.hash !== `#${id}`) location.hash = `#${id}`;
    if (remember) {
      try {
        localStorage.setItem(STORAGE_KEY, id);
      } catch {}
    }
  }

  function initTemplateEngine() {
    $$('[data-include]').forEach((el) => {
      const tplId = el.getAttribute('data-include');
      const tpl = document.getElementById(tplId);
      if (tpl) el.replaceWith(tpl.content.cloneNode(true));
    });
  }

  on(document, 'DOMContentLoaded', () => {
    initTemplateEngine();
    route();
    on(window, 'hashchange', () => route(null, { pushHash: false }));
    registerEventListener();
  });
})();

