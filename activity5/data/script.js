const cols = document.querySelectorAll('.led-col');
const domes = [
  document.getElementById('dome1'),
  document.getElementById('dome2'),
  document.getElementById('dome3')
];
const sd = document.getElementById('statusDot');
const st = document.getElementById('statusText');
const ipEl = document.getElementById('ipDisplay');

const colors = ['blue', 'green', 'yellow'];
let busy = [false, false, false];

async function refresh() {
  try {
    const r = await fetch('/api/status');
    const d = await r.json();
    d.leds.forEach((s, i) => {
      domes[i].className = 'led-dome ' + (s ? 'on ' : 'off ') + colors[i];
    });
    sd.className = 'dot online';
    st.textContent = 'ONLINE';
    if (d.ip) ipEl.textContent = d.ip;
  } catch (_) {
    sd.className = 'dot offline';
    st.textContent = 'OFFLINE';
  }
}

for (let i = 0; i < 3; i++) {
  cols[i].style.cursor = 'pointer';
  cols[i].onclick = async () => {
    if (busy[i]) return;
    busy[i] = true;
    cols[i].style.opacity = '0.5';
    try {
      await fetch('/api/led/' + (i + 1) + '/toggle');
      await refresh();
    } catch (_) {}
    cols[i].style.opacity = '1';
    busy[i] = false;
  };
}

refresh();
setInterval(refresh, 2000);
