let screen = 'main';
let deferredPrompt = null;
let pin_id = null;
let started = false;
let show_version = false;
let cfg_changed = false;
let menu_f = false;
let updates = [];

let cfg = {
  prefix: 'MyDevices',
  use_ws: false, use_hook: true, client_ip: '192.168.1.1', netmask: 24,
  use_bt: false, use_serial: false,
  use_mqtt: false, mq_host: 'test.mosquitto.org', mq_port: '8081', mq_login: '', mq_pass: '',
  use_pin: false, hub_pin: '',
  hub_id: new Date().getTime().toString().hashCode().toString(16),
  ui_width: 450, theme: 'DARK', maincolor: 'GREEN', font: 'monospace', version: app_version, check_upd: true
};

document.addEventListener('keydown', function (e) {
  if (!started) return;
  switch (e.keyCode) {
    case 116: // refresh on F5
      if (!e.ctrlKey) {
        e.preventDefault();
        refresh_h();
      }
      break;

    case 192: // open cli on `
      if (focused) {
        e.preventDefault();
        toggleCLI();
      }
      break;

    default:
      break;
  }
  //log(e.keyCode);
});

// ============= STARTUP ============
window.onload = function () {
  /*NON-ESP*/
  if ('serviceWorker' in navigator && !isLocal() && !isApp()) {
    navigator.serviceWorker.register('/sw.js');
    window.addEventListener('beforeinstallprompt', (e) => deferredPrompt = e);
  }
  /*/NON-ESP*/

  window.history.pushState({ page: 1 }, "", "");    // back/refresh
  window.onpopstate = function (e) {
    window.history.pushState({ page: 1 }, "", "");
    back_h();
  }
  render_main(app_version);
  EL('title').innerHTML = app_title;
  load_cfg();
  if (typeof cfg.hub_id != 'string') cfg.hub_id = cfg.hub_id.toString(16);
  let title = 'GyverHub v' + app_version + ' [' + cfg.hub_id + '] ' + (isPWA() ? 'PWA ' : '') + (isSSL() ? 'SSL ' : '') + (isLocal() ? 'Local ' : '') + (isESP() ? 'ESP ' : '') + (isApp() ? 'App ' : '');
  EL('title').title = title;
  log(title);

  // pin
  if (cfg.use_pin) show_keypad(true);
  else startup();
  started = true;
}
function startup() {
  if (isESP()) cfg.use_ws = true;
  render_selects();
  render_info();
  show_screen('main');
  apply_cfg();
  update_ip();
  load_devices();
  render_devices();
  discover();

  /*NON-ESP*/
  if (isSSL()) {
    EL('http_only_http').style.display = 'block';
    EL('http_settings').style.display = 'none';
    EL('pwa_unsafe').style.display = 'none';
  }
  if (isPWA() || isLocal() || isApp()) {
    EL('app_block').style.display = 'none';
  }

  if (cfg.use_mqtt) mq_start();
  setInterval(() => {
    if (cfg.use_mqtt && !mq_state()) {
      log('MQTT reconnect');
      mq_start();
    }
    //if (!focused) sendDiscover();
  }, 5000);

  setTimeout(() => {
    if (show_version) alert('Версия ' + app_version + '!\n' + '__NOTES__');
  }, 1000);
  /*/NON-ESP*/
}
/*NON-ESP*/
async function checkUpdates(id) {
  if (!cfg.check_upd) return;
  if (updates.includes(id)) return;
  let ver = devices[id].version;
  if (!ver.includes('@')) return;
  let namever = ver.split('@');
  const resp = await fetch(`https://raw.githubusercontent.com/${namever[0]}/master/project.json`, { cache: "no-store" });
  let proj = await resp.text();
  try {
    proj = JSON.parse(proj);
  } catch (e) {
    return;
  }
  if (proj.version == namever[1]) return;
  if (id != focused) return;
  updates.push(id);
  if (confirm('Available new version v' + proj.version + ' for device [' + namever[0] + ']. Notes:\n' + proj.notes + '\n\nUpdate firmware?')) {
    otaUrl(`https://raw.githubusercontent.com/${namever[0]}/master/bin/firmware.bin${devices[id].gzip ? '.gz' : ''}`, 'flash');
  }
}
async function pwa_install(ssl) {
  if (ssl && !isSSL()) {
    if (confirm("Redirect to HTTPS?")) window.location.href = window.location.href.replace('http:', 'https:');
    else return;
  }
  if (!ssl && isSSL()) {
    if (confirm("Redirect to HTTP")) window.location.href = window.location.href.replace('https:', 'http:');
    else return;
  }
  if (!('serviceWorker' in navigator)) {
    alert('Error');
    return;
  }
  if (deferredPrompt !== null) {
    deferredPrompt.prompt();
    const { outcome } = await deferredPrompt.userChoice;
    if (outcome === 'accepted') deferredPrompt = null;
  }
}
async function loadProjects() {
  const resp = await fetch("https://raw.githubusercontent.com/GyverLibs/GyverHub-projects/main/projects.txt", { cache: "no-store" });
  let projects = await resp.text();
  projects = projects.split('\n');
  for (let proj of projects) {
    if (!proj) continue;
    let rep = proj.split('https://github.com/')[1];
    if (!rep) continue;
    loadProj(rep);
  }
}
async function loadProj(rep) {
  const resp = await fetch(`https://raw.githubusercontent.com/${rep}/master/project.json`, { cache: "no-store" });
  let proj = await resp.text();
  try {
    proj = JSON.parse(proj);
  } catch (e) {
    return;
  }
  if (!('version' in proj) || !('notes' in proj) || !('about' in proj)) return;
  let name = rep.split('/')[1];
  if (name.length > 30) name = name.slice(0, 30) + '..';
  EL('projects').innerHTML += `
  <div class="proj">
    <div class="proj_inn">
      <div class="proj_name">
        <a href="${'https://github.com/' + rep}" target="_blank" title="${rep} v${proj.version}">${name}</a>
      <div class="proj_about">${proj.about}</div>
    </div>
  </div>
  `;
}
/*/NON-ESP*/

// =============== PIN ================
function pass_type(v) {
  pass_inp.value += v;
  let hash = pass_inp.value.hashCode();

  if (pin_id) {   // device
    if (hash == devices[pin_id].PIN) {
      open_device(pin_id);
      pass_inp.value = '';
      devices_t[pin_id].granted = true;
    }
  } else {        // app
    if (hash == cfg.hub_pin) {
      EL('password').style.display = 'none';
      startup();
      pass_inp.value = '';
    }
  }
}
function check_type(arg) {
  if (arg.value.length > 0) {
    let c = arg.value[arg.value.length - 1];
    if (c < '0' || c > '9') arg.value = arg.value.slice(0, -1);
  }
}
function show_keypad(v) {
  if (v) {
    EL('password').style.display = 'block';
    EL('pass_inp').focus();
  } else {
    EL('password').style.display = 'none';
  }
}

// ============== RENDER ==============
function render_devices() {
  EL('devices').innerHTML = '';
  for (let id in devices) addDevice(id);
}
function render_info() {
  for (let id in info_labels_topics) {
    EL('info_topics').innerHTML += `
    <div class="cfg_row info">
      <label>${info_labels_topics[id]}</label>
      <label id="${id}" class="lbl_info info_small">-</label>
    </div>`;
  }

  for (let id in info_labels_version) {
    EL('info_version').innerHTML += `
    <div class="cfg_row info">
      <label>${info_labels_version[id]}</label>
      <label id="${id}" class="lbl_info">-</label>
    </div>`;
  }

  for (let id in info_labels_esp) {
    EL('info_esp').innerHTML += `
    <div class="cfg_row info">
      <label>${info_labels_esp[id]}</label>
      <label id="${id}" class="lbl_info">-</label>
    </div>`;
  }
  EL('info_esp').innerHTML += '<div style="padding:10px"><button class="c_btn btn_mini" onclick="reboot_h()"><span class="icon info_icon"></span>Reboot</button></div>';
  EL('info_l_ip').onclick = function () { window.open('http://' + devices[focused].ip, '_blank').focus(); };
  EL('info_l_ip').classList.add('info_link');
}
function update_info() {
  let id = focused;
  EL('info_break_sw').checked = devices[id].break_widgets;
  EL('info_names_sw').checked = devices[id].show_names;
  EL('info_cli_sw').checked = EL('cli_cont').style.display == 'block';

  EL('info_id').innerHTML = id;
  EL('info_set').innerHTML = devices[id].prefix + '/' + id + '/set/*';
  EL('info_read').innerHTML = devices[id].prefix + '/' + id + '/read/*';
  EL('info_get').innerHTML = devices[id].prefix + '/hub/' + id + '/get/*';
  EL('info_status').innerHTML = devices[id].prefix + '/hub/' + id + '/status';
}
function render_selects() {
  for (let color in colors) {
    EL('maincolor').innerHTML += `
    <option value="${color}">${color}</option>`;
  }

  for (let font of fonts) {
    EL('font').innerHTML += `
    <option value="${font}">${font}</option>`;
  }

  for (let theme in themes) {
    EL('theme').innerHTML += `
    <option value="${theme}">${theme}</option>`;
  }

  for (let i = 0; i < 33; i++) {
    let imask;
    if (i == 32) imask = 0xffffffff;
    else imask = ~(0xffffffff >>> i);
    EL('netmask').innerHTML += `
      <option value="${i}">${intToOctets(imask)}</option>`;
  }
}

// ============== SCREEN ==============
function test_h() {
  show_screen('test');
}
function projects_h() {
  EL('projects').innerHTML = '';
  show_screen('projects');
  loadProjects();
}
function refresh_h() {
  if (screen == 'device') post('focus');
  else if (screen == 'info') post('info');
  else if (screen == 'fsbr') post('fsbr');
  else discover();
}
function back_h() {
  if (menu_f) {
    menu_deact();
    menu_show(0);
    return;
  }
  switch (screen) {
    case 'device':
      release_all();
      close_device();
      break;
    case 'info':
    case 'fsbr':
      menu_deact();
      showControls(devices_t[focused].controls);
      show_screen('device');
      break;
    case 'config':
      config_h();
      break;
    case 'pin':
    case 'projects':
    case 'test':
      show_screen('main');
      break;
  }
}
function config_h() {
  if (screen == 'config') {
    if (cfg_changed) save_cfg();
    cfg_changed = false;
    show_screen('main');
    discover();
  } else {
    show_screen('config');
  }
}
function menu_show(state) {
  menu_f = state;
  let cl = EL('menu').classList;
  if (menu_f) cl.add('menu_show');
  else cl.remove('menu_show');
  EL('icon_menu').innerHTML = menu_f ? '' : '';
  EL('menu_overlay').style.display = menu_f ? 'block' : 'none';
}
function menu_h() {
  menu_show(!menu_f);
}
function info_h() {
  menu_deact();
  menu_show(0);
  post('info');
  show_screen('info');
  EL('menu_info').classList.add('menu_act');
}
function fsbr_h() {
  menu_deact();
  menu_show(0);
  post('fsbr');
  EL('fsbr_inner').innerHTML = '<div class="fsbr_wait"><span style="font-size:50px;color:var(--prim)" class="icon spinning"></span></div>';
  show_screen('fsbr');
  EL('menu_fsbr').classList.add('menu_act');
}
function device_h(id) {
  if (discovering) return;
  if (!(id in devices_t) || devices_t[id].conn == Conn.NONE) {
    delete_h(id);
    return;
  }
  if (!devices[id]) return;
  if (devices[id].PIN && !devices_t[id].granted) {
    pin_id = id;
    show_screen('pin');
  } else open_device(id);
}
function open_device(id) {
  /*NON-ESP*/
  checkUpdates(id);
  /*/NON-ESP*/
  focused = id;

  switch (devices_t[id].conn) {
    case Conn.SERIAL:
      break;

    case Conn.BT:
      break;

    case Conn.WS:
      refreshSpin(true);
      ws_start(id);
      ws_focus_flag = true;
      break;

    case Conn.MQTT:
      post('focus');
      break;
  }
  log('Open device #' + id + ' via ' + ConnNames[devices_t[id].conn]);

  EL('menu_user').innerHTML = '';
  showControls(devices_t[id].controls);
  show_screen('device');
  reset_ping();
}
function close_device() {
  showErr(false);
  switch (devices_t[focused].conn) {
    case Conn.SERIAL:
      break;

    case Conn.BT:
      break;

    case Conn.WS:
      // 'unfocus' forbidden
      ws_stop(focused);
      refreshSpin(false);
      break;

    case Conn.MQTT:
      post('unfocus');
      break;
  }
  log('Close device #' + focused);
  focused = null;
  show_screen('main');
  //sendDiscover();
  stop_ping();
  stop_tout();
}
function clear_all() {
  EL('devices').innerHTML = "";
  devices = {};
  devices_t = {};
  save_devices();
  show_screen('main');
}
function show_screen(nscreen) {
  screen = nscreen;
  stopFS();
  show_keypad(false);
  let proj_s = EL('projects_cont').style;
  let test_s = EL('test_cont').style;
  let main_s = EL('main_cont').style;
  let config_s = EL('config').style;
  let devices_s = EL('devices').style;
  let controls_s = EL('controls').style;
  let info_s = EL('info').style;
  let fsbr_s = EL('fsbr').style;
  let icon_cfg_s = EL('icon_cfg').style;
  let icon_menu_s = EL('icon_menu').style;
  let icon_refresh_s = EL('icon_refresh').style;
  let back_s = EL('back').style;
  let version_s = EL('version').style;
  let title_row_s = EL('title_row').style;

  main_s.display = 'block';
  test_s.display = 'none';
  proj_s.display = 'none';
  config_s.display = 'none';
  devices_s.display = 'none';
  controls_s.display = 'none';
  info_s.display = 'none';
  icon_menu_s.display = 'none';
  icon_cfg_s.display = 'none';
  fsbr_s.display = 'none';
  back_s.display = 'none';
  icon_refresh_s.display = 'none';
  version_s.display = 'none';
  title_row_s.cursor = 'pointer';
  EL('title').innerHTML = app_title;

  if (screen == 'main') {
    version_s.display = 'unset';
    devices_s.display = 'grid';
    icon_cfg_s.display = 'inline-block';
    icon_refresh_s.display = 'inline-block';
    title_row_s.cursor = 'unset';
    EL('conn').innerHTML = '';
    showCLI(false);

  } else if (screen == 'test') {
    main_s.display = 'none';
    test_s.display = 'block';
    back_s.display = 'inline-block';
    EL('title').innerHTML = 'UI Test';

  } else if (screen == 'projects') {
    main_s.display = 'none';
    proj_s.display = 'block';
    back_s.display = 'inline-block';
    EL('title').innerHTML = 'Projects';

  } else if (screen == 'device') {
    controls_s.display = 'block';
    icon_menu_s.display = 'inline-block';
    back_s.display = 'inline-block';
    icon_refresh_s.display = 'inline-block';
    EL('title').innerHTML = devices[focused].name;

  } else if (screen == 'config') {
    config_s.display = 'block';
    icon_cfg_s.display = 'inline-block';
    back_s.display = 'inline-block';
    EL('title').innerHTML = 'Config';

  } else if (screen == 'info') {
    info_s.display = 'block';
    icon_menu_s.display = 'inline-block';
    back_s.display = 'inline-block';
    EL('title').innerHTML = devices[focused].name + '/info';
    update_info();

  } else if (screen == 'fsbr') {
    fsbr_s.display = 'block';
    icon_menu_s.display = 'inline-block';
    back_s.display = 'inline-block';
    EL('title').innerHTML = devices[focused].name + '/fs';

  } else if (screen == 'pin') {
    back_s.display = 'inline-block';
    show_keypad(true);
  }
}
function delete_h(id) {
  if (confirm('Delete ' + id + '?')) {
    document.getElementById("device#" + id).remove();
    delete devices[id];
    save_devices();
    return 1;
  }
  return 0;
}

// ============== CLI =============
function printCLI(text, color) {
  if (EL('cli_cont').style.display == 'block') {
    if (EL('cli').innerHTML) EL('cli').innerHTML += '\n';
    let st = color ? `style="color:${intToCol(color)}"` : '';
    EL('cli').innerHTML += `><span ${st}">${text}</span>`;
    EL('cli').scrollTop = EL('cli').scrollHeight;
  }
}
function toggleCLI() {
  EL('cli').innerHTML = "";
  EL('cli_input').value = "";
  showCLI(!(EL('cli_cont').style.display == 'block'));
}
function showCLI(v) {
  EL('bottom_space').style.height = v ? '170px' : '70px';
  EL('cli_cont').style.display = v ? 'block' : 'none';
  if (v) EL('cli_input').focus();
  EL('info_cli_sw').checked = v;
}
function checkCLI() {
  if (event.key == 'Enter') sendCLI();
}
function sendCLI() {
  post('cli', 'cli', EL('cli_input').value);
  EL('cli_input').value = "";
}

// ============== DISCOVER =============
function sendDiscover() {
  /*NON-ESP*/
  if (cfg.use_mqtt) mq_discover();
  /*/NON-ESP*/
  if (cfg.use_ws && !isSSL()) ws_discover();
}
function discover() {
  if (isESP()) {
    let has = false;
    for (let id in devices) {
      if (window.location.href.includes(devices[id].ip)) has = true;
    }
    if (!has && checkIP(window_ip())) ws_discover_ip(window_ip());
  }
  for (let id in devices) {
    if (id in devices_t) devices_t[id].conn = Conn.NONE;
    EL(`device#${id}`).className = "device offline";

    EL(`Serial#${id}`).style.display = 'none';
    EL(`BT#${id}`).style.display = 'none';
    EL(`WS#${id}`).style.display = 'none';
    EL(`MQTT#${id}`).style.display = 'none';
  }
  sendDiscover();
}
function discover_all() {
  /*NON-ESP*/
  if (cfg.use_mqtt) mq_discover_all();
  /*/NON-ESP*/
  if (cfg.use_ws && !isSSL()) ws_discover_all();
  back_h();
}

// ============= CFG ==============
/*NON-ESP*/
function mq_change(start = false) {
  mq_show_err(1);
  mq_stop();
  if (start) mq_start();
}
/*/NON-ESP*/
function update_cfg(el) {
  cfg_changed = true;
  if (el.type == 'text') el.value = el.value.trim();
  if (el.type == 'checkbox') cfg[el.id] = el.checked;
  else cfg[el.id] = el.value;
  updateTheme();
}
function save_cfg() {
  localStorage.setItem('config', JSON.stringify(cfg));
}
function load_cfg() {
  if (localStorage.hasOwnProperty('config')) {
    let cfg_r = JSON.parse(localStorage.getItem('config'));
    let dif = false;

    for (let key in cfg) {
      if (cfg_r[key] === undefined) {
        cfg_r[key] = cfg[key];
        dif = true;
      }
    }

    if (cfg_r['version'] != cfg['version']) {
      cfg_r['version'] = cfg['version'];
      dif = true;
      show_version = true;
    }

    cfg = cfg_r;
    if (dif) save_cfg();
  } else {
    if ('Notification' in window && Notification.permission == 'default') Notification.requestPermission();
  }
  updateTheme();
}
function apply_cfg() {
  for (let key in cfg) {
    if (key == 'version') continue;
    let el = EL(key);
    if (el == undefined) continue;
    if (el.type == 'checkbox') el.checked = cfg[key];
    else el.value = cfg[key];
  }
  updateTheme();
}

// ============ IMPORT =============
async function cfg_export() {
  try {
    const text = btoa(JSON.stringify(cfg)) + ',' + btoa(encodeURIComponent(JSON.stringify(devices)));
    await navigator.clipboard.writeText(text);
    showPopup('Copied to clipboard');
  } catch (err) {
    showPopupError('Export error');
  }
}
async function cfg_import() {
  try {
    let text = await navigator.clipboard.readText();
    try {
      cfg = JSON.parse(atob(text.split(',')[0]));
    } catch (e) {
    }
    apply_cfg();
    save_cfg();

    try {
      devices = JSON.parse(decodeURIComponent(atob(text.split(',')[1])));
    } catch (e) {
    }
    render_devices();
    save_devices();

    showPopup('Import done');
  } catch (e) {
    showPopupError('Wrong data');
  }
}

// ============ THEME =============
function updateTheme() {
  let v = themes[cfg.theme];
  let r = document.querySelector(':root');
  r.style.setProperty('--back', theme_cols[v][0]);
  r.style.setProperty('--tab', theme_cols[v][1]);
  r.style.setProperty('--font', theme_cols[v][2]);
  r.style.setProperty('--font2', theme_cols[v][3]);
  r.style.setProperty('--dark', theme_cols[v][4]);
  r.style.setProperty('--thumb', theme_cols[v][5]);
  r.style.setProperty('--black', theme_cols[v][6]);
  r.style.setProperty('--scheme', theme_cols[v][7]);
  r.style.setProperty('--font_inv', theme_cols[v][8]);
  r.style.setProperty('--shad', theme_cols[v][9]);
  r.style.setProperty('--ui_width', cfg.ui_width + 'px');
  r.style.setProperty('--prim', intToCol(colors[cfg.maincolor]));
  r.style.setProperty('--font_f', cfg.font);

  let b = 'block';
  let n = 'none';
  let f = 'var(--font)';
  let f3 = 'var(--font3)';

  EL('ws_block').style.display = cfg.use_ws ? b : n;
  EL('ws_label').style.color = cfg.use_ws ? f : f3;
  EL('pin_block').style.display = cfg.use_pin ? b : n;
  EL('pin_label').style.color = cfg.use_pin ? f : f3;

  /*NON-ESP*/
  EL('mq_block').style.display = cfg.use_mqtt ? b : n;
  EL('mqtt_label').style.color = cfg.use_mqtt ? f : f3;
  EL('bt_block').style.display = cfg.use_bt ? b : n;
  EL('bt_label').style.color = cfg.use_bt ? f : f3;
  EL('serial_block').style.display = cfg.use_serial ? b : n;
  EL('serial_label').style.color = cfg.use_serial ? f : f3;
  /*/NON-ESP*/
}