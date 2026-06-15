(() => {
  const TELEMETRY_TOPIC = CONFIG.telemetryTopic;

  // ── Toast ──────────────────────────────────────────────────────────
  function toast(message, type) {
    type = type || 'info';
    var icons = {
      success: '<svg class="toast-icon" viewBox="0 0 24 24"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14" stroke="currentColor" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/><path d="M22 4L12 14.01l-3-3" stroke="currentColor" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>',
      error: '<svg class="toast-icon" viewBox="0 0 24 24"><circle cx="12" cy="12" r="10" stroke="currentColor" fill="none" stroke-width="2"/><path d="M15 9l-6 6M9 9l6 6" stroke="currentColor" fill="none" stroke-width="2" stroke-linecap="round"/></svg>',
      info: '<svg class="toast-icon" viewBox="0 0 24 24"><circle cx="12" cy="12" r="10" stroke="currentColor" fill="none" stroke-width="2"/><path d="M12 16v-4M12 8h.01" stroke="currentColor" fill="none" stroke-width="2" stroke-linecap="round"/></svg>',
      warning: '<svg class="toast-icon" viewBox="0 0 24 24"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z" stroke="currentColor" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/><path d="M12 9v4M12 17h.01" stroke="currentColor" fill="none" stroke-width="2" stroke-linecap="round"/></svg>',
    };
    var el = document.createElement('div');
    el.className = 'toast ' + type;
    el.innerHTML = (icons[type] || icons.info) + '<span class="toast-message">' + escapeHtml(message) + '</span>';
    document.getElementById('toastContainer').appendChild(el);
    setTimeout(function () { if (el.parentNode) el.parentNode.removeChild(el); }, 4000);
  }

  // ── Helpers ─────────────────────────────────────────────────────────
  function escapeHtml(value) {
    return String(value).replace(/[&<>"']/g, function (c) {
      return ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' })[c];
    });
  }

  function formatPtBrDate(value) {
    if (!value) return '-';
    var parts = String(value).match(/^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})/);
    if (!parts) return value;
    return parts[3] + '/' + parts[2] + '/' + parts[1] + ' ' + parts[4] + ':' + parts[5] + ':' + parts[6];
  }

  function rssiBadge(value) {
    if (value == null || value === '-') return '<td>-</td>';
    var n = Number(value);
    var cls = n >= -50 ? 'rssi-good' : n >= -70 ? 'rssi-mid' : 'rssi-bad';
    var bars = n >= -50 ? '&#9646;&#9646;&#9646;&#9646;' : n >= -70 ? '&#9646;&#9646;&#9646;' : '&#9646;';
    return '<td><span class="rssi-badge ' + cls + '">' + bars + ' ' + n + ' dBm</span></td>';
  }

  function summaryIcon(name) {
    var icons = {
      time: '<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/></svg>',
      serial: '<svg viewBox="0 0 24 24"><rect x="4" y="5" width="16" height="14" rx="2"/><path d="M8 9h8M8 13h5"/></svg>',
      type: '<svg viewBox="0 0 24 24"><path d="M4 7h16M4 12h16M4 17h10"/></svg>',
      pin: '<svg viewBox="0 0 24 24"><path d="M9 3v5M15 3v5M8 8h8v7a4 4 0 0 1-8 0V8z"/><path d="M12 19v2"/></svg>',
      count: '<svg viewBox="0 0 24 24"><path d="M5 7h14M5 12h14M5 17h14"/><path d="M8 5v14M16 5v14"/></svg>',
    };
    return icons[name] || icons.type;
  }

  function summaryRow(icon, label, value, span) {
    return '<div class="summary-row span-' + span + '"><span class="summary-icon">' + summaryIcon(icon) + '</span><span class="summary-content"><span class="summary-label">' + label + '</span><span class="summary-value">' + escapeHtml(value ?? '-') + '</span></span></div>';
  }

  function eventSummaryBox(event, eventId) {
    return '<div class="payload-summary-card"><div class="payload-summary-title"><span style="color:var(--blue-light);">ID:' + escapeHtml(eventId) + '</span> <span style="color:var(--border-accent);margin:0 6px;">|</span> Dados do payload <span style="color:var(--border-accent);margin:0 6px;">|</span> <span style="color:var(--text-muted);font-weight:700;">' + escapeHtml(formatPtBrDate(event.at)) + '</span></div>'
      + '<div class="summary-grid">'
      + summaryRow('serial', 'Serial', event.serialNumber || '-', 2)
      + summaryRow('type', 'Tipo', event.type || '-', 2)
      + summaryRow('pin', 'Pino', event.pin ?? '-', 2)
      + summaryRow('count', 'Contagem', event.count ?? '-', 2)
      + summaryRow('type', 'RSSI', event.rssi ?? '-', 2)
      + summaryRow('type', 'Rede', event.network || '-', 2)
      + summaryRow('type', 'Versao', event.version || '-', 2)
      + summaryRow('type', 'Offline', event.data_off === undefined ? '-' : String(event.data_off), 2)
      + summaryRow('type', 'Topico', event.topic || '-', 6)
      + '</div></div>';
  }

  function payloadBox(value) {
    var displayValue = Object.assign({}, value);
    if (displayValue.at) displayValue.at = formatPtBrDate(displayValue.at);
    if (displayValue.lastSeenAt) displayValue.lastSeenAt = formatPtBrDate(displayValue.lastSeenAt);
    var text = JSON.stringify(displayValue, null, 2);
    return '<div class="payload-card"><div class="payload-toolbar"><span class="payload-title">Payload recebido</span><button type="button" class="copy-btn" data-copy="' + escapeHtml(text) + '">Copiar</button></div><pre class="payload-box">' + escapeHtml(text) + '</pre></div>';
  }

  // ── Loading State ──────────────────────────────────────────────────
  function setLoading(btn, loading) {
    if (loading) {
      btn.classList.add('btn-loading');
      var spinner = document.createElement('span');
      spinner.className = 'btn-spinner';
      btn.appendChild(spinner);
    } else {
      btn.classList.remove('btn-loading');
      var sp = btn.querySelector('.btn-spinner');
      if (sp) sp.remove();
    }
  }

  async function postCommand(device, command) {
    var res = await fetch('/api/devices/' + encodeURIComponent(device) + '/command', {
      method: 'POST',
      headers: { 'content-type': 'application/json' },
      body: JSON.stringify(command),
    });
    if (!res.ok) throw new Error('HTTP ' + res.status);
    return res.json();
  }

  // ── Clock ──────────────────────────────────────────────────────────
  function updateClock() {
    var now = new Date();
    var h = String(now.getHours()).padStart(2, '0');
    var m = String(now.getMinutes()).padStart(2, '0');
    var s = String(now.getSeconds()).padStart(2, '0');
    var el = document.getElementById('headerClock');
    if (el) el.textContent = h + ':' + m + ':' + s;
  }
  updateClock();
  setInterval(updateClock, 1000);

  // ── File Input ────────────────────────────────────────────────────
  document.getElementById('firmwareFile').addEventListener('change', function () {
    var name = this.files && this.files[0] ? this.files[0].name : 'Nenhum arquivo';
    document.getElementById('fileNameDisplay').textContent = name;
  });

  // ── Sync command fields ────────────────────────────────────────────
  function syncCommandFields() {
    var el = document.getElementById('fileName');
    if (el) el.style.display = document.getElementById('cmdType').value === 'update' ? '' : 'none';
  }
  document.getElementById('cmdType').addEventListener('change', syncCommandFields);
  syncCommandFields();

  // ── Command Form ───────────────────────────────────────────────────
  document.getElementById('cmdForm').addEventListener('submit', async function (e) {
    e.preventDefault();
    var btn = this.querySelector('button[type="submit"]');
    setLoading(btn, true);
    try {
      var body = { type: document.getElementById('cmdType').value };
      if (body.type === 'update') {
        body.fileName = document.getElementById('firmwareName').textContent || document.getElementById('fileName').value || 'firmware.bin';
      }
      await postCommand(document.getElementById('deviceId').value, body);
      toast('Comando ' + body.type + ' enviado para ' + document.getElementById('deviceId').value, 'success');
    } catch (err) {
      toast('Falha ao enviar comando: ' + err.message, 'error');
    } finally {
      setLoading(btn, false);
    }
  });

  // ── Relay Buttons ──────────────────────────────────────────────────
  document.querySelectorAll('button[data-relay]').forEach(function (btn) {
    btn.addEventListener('click', async function () {
      setLoading(btn, true);
      try {
        var device = document.getElementById('relayDeviceBlue').value || document.getElementById('relayDeviceBlack').value || document.getElementById('deviceId').value || '1';
        var body = {
          command: btn.getAttribute('data-relay'),
          duration_ms: Math.max(100, Math.min(5000, Number(document.getElementById('relayDuration').value || 1000))),
        };
        await postCommand(device, body);
        toast('Comando ' + body.command + ' enviado para placa ' + device, 'success');
      } catch (err) {
        toast('Falha no comando do relé: ' + err.message, 'error');
      } finally {
        setLoading(btn, false);
      }
    });
  });

  // ── Restart ────────────────────────────────────────────────────────
  document.getElementById('restartBtn').addEventListener('click', async function () {
    setLoading(this, true);
    try {
      var device = document.getElementById('restartDeviceId').value || '1';
      await postCommand(device, { type: 'RestartMachine' });
      toast('Comando Reiniciar enviado para placa ' + device, 'success');
    } catch (err) {
      toast('Falha ao reiniciar: ' + err.message, 'error');
    } finally {
      setLoading(this, false);
    }
  });

  // ── OTA Update ─────────────────────────────────────────────────────
  document.getElementById('updateBtn').addEventListener('click', async function () {
    setLoading(this, true);
    var device = document.getElementById('restartDeviceId').value || '1';
    var currentFirmware = document.getElementById('firmwareName').textContent || 'firmware.bin';
    if (!currentFirmware || currentFirmware === 'null') currentFirmware = 'firmware.bin';
    var statusEl = document.getElementById('firmwareStatus');
    statusEl.textContent = 'Disparando update para ' + device + ' usando ' + currentFirmware + '...';
    try {
      var res = await fetch('/api/devices/' + encodeURIComponent(device) + '/command', {
        method: 'POST',
        headers: { 'content-type': 'application/json' },
        body: JSON.stringify({ type: 'update', fileName: currentFirmware }),
      });
      if (!res.ok) throw new Error('HTTP ' + res.status);
      statusEl.textContent = 'Comando update enviado. A placa vai puxar o arquivo da VPS e reiniciar.';
      toast('Update OTA enviado para placa ' + device, 'success');
    } catch (err) {
      statusEl.textContent = 'Falha ao enviar update: ' + err.message;
      toast('Falha no update: ' + err.message, 'error');
    } finally {
      setLoading(this, false);
    }
  });

  // ── Clear ──────────────────────────────────────────────────────────
  document.getElementById('clearBtn').addEventListener('click', async function () {
    setLoading(this, true);
    try {
      var res = await fetch('/api/state/clear', { method: 'POST' });
      if (!res.ok) throw new Error('HTTP ' + res.status);
      toast('Dados limpos com sucesso', 'success');
      await refresh();
    } catch (err) {
      toast('Falha ao limpar: ' + err.message, 'error');
    } finally {
      setLoading(this, false);
    }
  });

  // ── Firmware Upload ────────────────────────────────────────────────
  document.getElementById('firmwareForm').addEventListener('submit', async function (e) {
    e.preventDefault();
    var file = document.getElementById('firmwareFile').files && document.getElementById('firmwareFile').files[0];
    if (!file) {
      toast('Selecione um arquivo .bin antes de enviar.', 'warning');
      return;
    }
    var btn = this.querySelector('button[type="submit"]');
    setLoading(btn, true);
    var statusEl = document.getElementById('firmwareStatus');
    statusEl.textContent = 'Enviando ' + file.name + ' para a VPS...';
    try {
      var uploadBody = await file.arrayBuffer();
      var res = await fetch('/api/firmware', {
        method: 'POST',
        headers: {
          'content-type': 'application/octet-stream',
          'x-firmware-name': file.name,
        },
        body: uploadBody,
      });
      if (!res.ok) throw new Error('HTTP ' + res.status);
      var result = await res.json();
      document.getElementById('firmwareFile').value = '';
      document.getElementById('fileNameDisplay').textContent = 'Nenhum arquivo';
      statusEl.textContent = 'Upload concluido: ' + result.name + ' (' + result.size + ' bytes). Depois envie o comando update para a placa.';
      toast('Firmware enviado com sucesso!', 'success');
      await refresh();
    } catch (err) {
      statusEl.textContent = 'Falha no upload: ' + err.message;
      toast('Falha no upload: ' + err.message, 'error');
    } finally {
      setLoading(btn, false);
    }
  });

  // ── Copy buttons ───────────────────────────────────────────────────
  function bindCopyButtons() {
    document.querySelectorAll('.copy-btn').forEach(function (button) {
      button.addEventListener('click', async function () {
        var value = button.getAttribute('data-copy') || '';
        try {
          await navigator.clipboard.writeText(value);
          button.textContent = 'Copiado';
          button.classList.add('copied');
          setTimeout(function () {
            button.textContent = 'Copiar';
            button.classList.remove('copied');
          }, 1200);
        } catch {
          button.textContent = 'Falha';
          setTimeout(function () { button.textContent = 'Copiar'; }, 1200);
        }
      });
    });
  }

  // ── Refresh ────────────────────────────────────────────────────────
  async function refresh() {
    try {
      var res = await fetch('/api/state');
      if (!res.ok) throw new Error('HTTP ' + res.status);
      var data = await res.json();

      // MQTT indicator (header)
      var mqttEl = document.getElementById('mqttStatus');
      mqttEl.className = 'mqtt-indicator' + (data.connected ? ' ok' : '');
      mqttEl.querySelector('.label').textContent = data.connected ? 'Conectado' : 'Desconectado';

      // MQTT detail (card)
      var detailEl = document.getElementById('mqttStatusDetail');
      detailEl.className = 'status-detail' + (data.connected ? ' ok' : '');
      detailEl.lastElementChild.textContent = data.connected ? 'Conectado' : 'Desconectado';

      var devCount = Object.keys(data.devices).length;
      var evtCount = data.events.filter(function (e) { return e.type !== 'ping'; }).length;

      document.getElementById('statDevices').textContent = devCount;
      document.getElementById('statEvents').textContent = evtCount;
      document.getElementById('statLast').textContent = formatPtBrDate(data.lastMessageAt);
      document.getElementById('deviceCount').textContent = devCount;
      document.getElementById('eventCount').textContent = evtCount;

      document.getElementById('firmwareName').textContent = data.firmware && data.firmware.name ? data.firmware.name : 'firmware.bin';
      document.getElementById('firmwareUploadedAt').textContent = data.firmware && data.firmware.uploadedAt ? formatPtBrDate(data.firmware.uploadedAt) : '-';
      document.getElementById('firmwareSize').textContent = data.firmware && data.firmware.size ? (data.firmware.size + ' bytes') : '-';

      var statusEl = document.getElementById('firmwareStatus');
      statusEl.textContent = data.firmware && data.firmware.uploadedAt
        ? ('Firmware salvo na VPS: ' + data.firmware.name + ' (' + data.firmware.size + ' bytes)')
        : 'Nenhum firmware enviado ainda.';

      // Devices table
      var devicesEl = document.getElementById('devices');
      devicesEl.innerHTML = Object.entries(data.devices).map(function (entry) {
        var id = entry[0], d = entry[1];
        return '<tr><td><strong>' + id + '</strong></td><td>' + formatPtBrDate(d.lastSeenAt) + '</td><td>' + (d.network || '-') + '</td>' + rssiBadge(d.rssi) + '<td>' + (d.version || '-') + '</td><td class="payload-cell">' + payloadBox(d) + '</td></tr>';
      }).join('') || '<tr><td colspan="6" class="empty-state">Nenhum dispositivo conectado</td></tr>';

      // Events table
      var filtered = data.events.filter(function (e) { return e.type !== 'ping'; });
      filtered.sort(function (a, b) { return (b.at || '').localeCompare(a.at || ''); });
      var total = filtered.length;
      var eventsEl = document.getElementById('events');
      eventsEl.innerHTML = filtered
        .map(function (e, index) {
          var eventId = String(total - index).padStart(4, '0');
          return '<tr class="event-detail-row"><td colspan="6" class="event-detail-cell">'
            + '<div class="event-payload-layout">' + eventSummaryBox(e, eventId) + payloadBox(e) + '</div>'
            + '</td></tr>';
        })
        .join('') || '<tr><td colspan="6" class="empty-state">Nenhum evento registrado</td></tr>';

      bindCopyButtons();
    } catch (err) {
      toast('Erro ao carregar dados: ' + err.message, 'error');
    }
  }

  // ── Render events table ────────────────────────────────────────────
  function renderEvents(events) {
    var filtered = events.filter(function (e) { return e.type !== 'ping'; });
    filtered.sort(function (a, b) { return (b.at || '').localeCompare(a.at || ''); });
    var total = filtered.length;
    document.getElementById('eventCount').textContent = total;
    var eventsEl = document.getElementById('events');
    eventsEl.innerHTML = filtered
      .map(function (e, index) {
        var eventId = String(total - index).padStart(4, '0');
        return '<tr class="event-detail-row"><td colspan="6" class="event-detail-cell">'
          + '<div class="event-payload-layout">' + eventSummaryBox(e, eventId) + payloadBox(e) + '</div>'
          + '</td></tr>';
      })
      .join('') || '<tr><td colspan="6" class="empty-state">Nenhum evento registrado</td></tr>';
    bindCopyButtons();
  }

  function prependEvent(event) {
    var e = event;
    if (!e || e.type === 'ping') return;
    var eventsEl = document.getElementById('events');
    var placeholder = eventsEl.querySelector('.empty-state');
    if (placeholder) eventsEl.innerHTML = '';
    var eventId = String(Date.now()).slice(-6);
    var row = '<tr class="event-detail-row"><td colspan="6" class="event-detail-cell">'
      + '<div class="event-payload-layout">' + eventSummaryBox(e, eventId) + payloadBox(e) + '</div>'
      + '</td></tr>';
    eventsEl.insertAdjacentHTML('afterbegin', row);
    var countEl = document.getElementById('eventCount');
    countEl.textContent = Number(countEl.textContent || 0) + 1;
    bindCopyButtons();
  }

  // ── SSE (Server-Sent Events) ──────────────────────────────────────
  function connectSSE() {
    var source = new EventSource('/api/events');
    source.addEventListener('connected', function (e) {
      console.log('[sse] connected');
    });
    source.addEventListener('telemetry', function (e) {
      try {
        var data = JSON.parse(e.data);
        prependEvent(data.event);
      } catch (err) {
        console.error('[sse] parse error', err);
      }
    });
    source.addEventListener('clear', function () {
      document.getElementById('events').innerHTML = '<tr><td colspan="6" class="empty-state">Nenhum evento registrado</td></tr>';
      document.getElementById('eventCount').textContent = '0';
    });
    source.onerror = function () {
      console.warn('[sse] connection lost, reconnecting...');
    };
  }

  var eventsRendered = false;

  // ── Refresh state (devices, firmware, mqtt) ────────────────────────
  async function refreshState() {
    try {
      var res = await fetch('/api/state');
      if (!res.ok) throw new Error('HTTP ' + res.status);
      var data = await res.json();

      var mqttEl = document.getElementById('mqttStatus');
      mqttEl.className = 'mqtt-indicator' + (data.connected ? ' ok' : '');
      mqttEl.querySelector('.label').textContent = data.connected ? 'Conectado' : 'Desconectado';

      var detailEl = document.getElementById('mqttStatusDetail');
      detailEl.className = 'status-detail' + (data.connected ? ' ok' : '');
      detailEl.lastElementChild.textContent = data.connected ? 'Conectado' : 'Desconectado';

      var devCount = Object.keys(data.devices).length;
      var evtCount = data.events.filter(function (e) { return e.type !== 'ping'; }).length;

      document.getElementById('statDevices').textContent = devCount;
      document.getElementById('statEvents').textContent = evtCount;
      document.getElementById('statLast').textContent = formatPtBrDate(data.lastMessageAt);
      document.getElementById('deviceCount').textContent = devCount;

      document.getElementById('firmwareName').textContent = data.firmware && data.firmware.name ? data.firmware.name : 'firmware.bin';
      document.getElementById('firmwareUploadedAt').textContent = data.firmware && data.firmware.uploadedAt ? formatPtBrDate(data.firmware.uploadedAt) : '-';
      document.getElementById('firmwareSize').textContent = data.firmware && data.firmware.size ? (data.firmware.size + ' bytes') : '-';

      var statusEl = document.getElementById('firmwareStatus');
      statusEl.textContent = data.firmware && data.firmware.uploadedAt
        ? ('Firmware salvo na VPS: ' + data.firmware.name + ' (' + data.firmware.size + ' bytes)')
        : 'Nenhum firmware enviado ainda.';

      var devicesEl = document.getElementById('devices');
      devicesEl.innerHTML = Object.entries(data.devices).map(function (entry) {
        var id = entry[0], d = entry[1];
        return '<tr><td><strong>' + id + '</strong></td><td>' + formatPtBrDate(d.lastSeenAt) + '</td><td>' + (d.network || '-') + '</td>' + rssiBadge(d.rssi) + '<td>' + (d.version || '-') + '</td><td class="payload-cell">' + payloadBox(d) + '</td></tr>';
      }).join('') || '<tr><td colspan="6" class="empty-state">Nenhum dispositivo conectado</td></tr>';

      // Initial events render (only on first load; SSE handles the rest)
      if (!eventsRendered && data.events && data.events.length > 0) {
        renderEvents(data.events);
        eventsRendered = true;
      }
    } catch (err) {
      toast('Erro ao carregar dados: ' + err.message, 'error');
    }
  }

  // ── Topic badge ────────────────────────────────────────────────────
  var topicEl = document.getElementById('topicBadge');
  if (topicEl) topicEl.textContent = TELEMETRY_TOPIC;

  // ── Init ───────────────────────────────────────────────────────────
  refreshState();
  setInterval(refreshState, 5000);
  connectSSE();
})();
