const COLORS = [
  { css: 'blue',   hex: '#3b82f6', off: '#0f1a2d' },
  { css: 'green',  hex: '#22c55e', off: '#0a2a14' },
  { css: 'yellow', hex: '#eab308', off: '#2d2408' },
];

const sd = document.getElementById('statusDot');
const st = document.getElementById('statusText');
const ipEl = document.getElementById('ipDisplay');
const row = document.getElementById('ledRow');

// Track current values (0–255)
let vals = [0, 0, 0];
let syncTimers = [null, null, null];

// ====== Build DOM ======

const domes = [];
const sliders = [];
const valDips = [];

for (let i = 0; i < 3; i++) {
  const col = document.createElement('div');
  col.className = 'led-col';

  const dome = document.createElement('div');
  dome.className = 'led-dome off ' + COLORS[i].css;
  col.appendChild(dome);
  domes.push(dome);

  const base = document.createElement('div');
  base.className = 'led-base';
  col.appendChild(base);

  const legs = document.createElement('div');
  legs.className = 'led-legs';
  legs.innerHTML = '<div class="led-leg"></div><div class="led-leg"></div>';
  col.appendChild(legs);

  const label = document.createElement('div');
  label.className = 'led-label';
  label.textContent = 'LED ' + (i + 1);
  col.appendChild(label);

  const wrap = document.createElement('div');
  wrap.className = 'slider-wrap';

  const slider = document.createElement('input');
  slider.type = 'range';
  slider.min = 0;
  slider.max = 255;
  slider.value = 0;
  wrap.appendChild(slider);
  sliders.push(slider);

  const vdisp = document.createElement('span');
  vdisp.className = 'slider-val';
  vdisp.textContent = '0';
  wrap.appendChild(vdisp);
  valDips.push(vdisp);

  col.appendChild(wrap);
  row.appendChild(col);
}

// ====== Update visual from value ======

function updateVisual(i) {
  const v = vals[i];
  const c = COLORS[i];

  if (v === 0) {
    domes[i].className = 'led-dome off ' + c.css;
    domes[i].style.boxShadow = 'none';
  } else {
    const pct = v / 255;
    domes[i].className = 'led-dome';
    domes[i].style.background = c.hex;
    domes[i].style.boxShadow = '0 0 ' + (22 * pct) + 'px ' + c.hex;
  }

  // Slider track accent
  const s = sliders[i];
  s.style.background = v > 0
    ? 'linear-gradient(to right, ' + c.hex + ' 0%, ' + c.hex + ' ' + Math.round(v / 255 * 100) + '%, rgba(255,255,255,.08) ' + Math.round(v / 255 * 100) + '%)'
    : 'rgba(255,255,255,.08)';
  s.style.accentColor = v > 0 ? c.hex : 'rgba(255,255,255,.25)';

  valDips[i].textContent = v;
}

// ====== Sync to server (throttled) ======

function syncLed(i) {
  if (syncTimers[i]) clearTimeout(syncTimers[i]);
  syncTimers[i] = setTimeout(() => {
    fetch('/api/led/' + (i + 1) + '/set?b=' + vals[i]);
  }, 25);
}

// ====== Slider input ======

for (let i = 0; i < 3; i++) {
  sliders[i].addEventListener('input', () => {
    vals[i] = parseInt(sliders[i].value);
    updateVisual(i);
    syncLed(i);
  });
}

// ====== Refresh from server ======

async function refresh() {
  try {
    const r = await fetch('/api/status');
    const d = await r.json();
    d.leds.forEach((v, i) => {
      vals[i] = v;
      sliders[i].value = v;
      updateVisual(i);
    });
    sd.className = 'dot online';
    st.textContent = 'ONLINE';
    if (d.ip) ipEl.textContent = d.ip;
  } catch (_) {
    sd.className = 'dot offline';
    st.textContent = 'OFFLINE';
  }
}

refresh();
setInterval(refresh, 2000);
