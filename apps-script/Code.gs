/**
 * MiniProject Danger Zone Alert — telemetry writer.
 *
 * Receives one row per detection heartbeat (10 s) plus immediate rows on
 * tier changes, appended to the 'Log' tab. The tab is created and styled
 * on the first row automatically.
 *
 * Deploy — bound mode (recommended, zero configuration):
 *   1. Open your Google Sheet → Extensions → Apps Script.
 *   2. Delete the placeholder code, paste this file, Save.
 *      Bound scripts use SpreadsheetApp.getActiveSpreadsheet() — nothing to edit.
 *   3. Deploy → New deployment → ⚙ Select type: Web app.
 *        Execute as:        Me
 *        Who has access:    Anyone
 *   4. Copy the web app URL (https://script.google.com/macros/s/<ID>/exec)
 *      into include/credentials.h:
 *          #define SHEETS_HOST "script.google.com"
 *          #define SHEETS_PATH "/macros/s/<ID>/exec"
 *
 * Standalone alternative (script at script.google.com, not inside a sheet):
 *   set SPREADSHEET_ID below to your spreadsheet ID from its URL.
 */

var SPREADSHEET_ID = ''; // standalone mode only; leave '' in bound mode

var TAB_NAME = 'Log';
var HEADERS = [
  'Timestamp (UTC)', 'Run', 'Event', 'Running', 'Nearest cm', 'Tier',
  'S1 Raw cm', 'S1 cm', 'S1 Status',
  'S2 Raw cm', 'S2 cm', 'S2 Status',
  'RSSI dBm', 'Heap B', 'Uptime ms'
];

function getSS() {
  if (SPREADSHEET_ID) return SpreadsheetApp.openById(SPREADSHEET_ID);
  return SpreadsheetApp.getActiveSpreadsheet();
}

function textOut(s) {
  return ContentService.createTextOutput(s)
      .setMimeType(ContentService.MimeType.TEXT);
}

function num(v) {
  var n = parseFloat(v);
  return isNaN(n) ? '' : n;
}

function doGet(e) {
  var p = e.parameter || {};
  var required = [
    'run', 'event', 'running', 'nearest_cm', 'tier',
    's1_raw', 's1_cm', 's1_status',
    's2_raw', 's2_cm', 's2_status'
  ];
  var missing = [];
  for (var i = 0; i < required.length; i++) {
    if (p[required[i]] === undefined || p[required[i]] === '') {
      missing.push(required[i]);
    }
  }
  if (missing.length) {
    return textOut('Error: missing parameters (' + missing.join(', ') + ')');
  }

  var lock = LockService.getScriptLock();
  lock.waitLock(10000);
  try {
    var ss = getSS();
    if (!ss) return textOut('Error: no spreadsheet bound and SPREADSHEET_ID is empty');
    var sheet = ss.getSheetByName(TAB_NAME) || createLogSheet(ss);
    appendRow(sheet, p);
  } finally {
    lock.releaseLock();
  }
  return textOut('OK');
}

function appendRow(sheet, p) {
  var ts = Utilities.formatDate(new Date(), 'UTC', 'yyyy-MM-dd HH:mm:ss');
  var row = [
    ts,
    p.run,
    p.event,
    p.running === 'true' ? 'yes' : 'no',
    num(p.nearest_cm),
    p.tier,
    num(p.s1_raw), num(p.s1_cm), p.s1_status,
    num(p.s2_raw), num(p.s2_cm), p.s2_status,
    num(p.rssi_dbm), num(p.free_heap), num(p.uptime_ms)
  ];
  var range = sheet.getRange(sheet.getLastRow() + 1, 1, 1, row.length);
  range.setValues([row]);
  range.setHorizontalAlignment('center');
}

function createLogSheet(ss) {
  var sheet = ss.insertSheet(TAB_NAME);
  var header = sheet.getRange(1, 1, 1, HEADERS.length);
  header.setValues([HEADERS]);
  header.setFontWeight('bold');
  header.setBackground('#4285f4');
  header.setFontColor('#ffffff');
  header.setHorizontalAlignment('center');
  header.setFontSize(11);
  sheet.setFrozenRows(1);
  sheet.getRange(1, 1, 1, HEADERS.length).createFilter();
  sheet.setColumnWidth(1, 150);
  for (var c = 2; c <= HEADERS.length; c++) sheet.setColumnWidth(c, 100);
  return sheet;
}
