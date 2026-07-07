/**
 * DHT_Data_Writing — Sheet writer functions
 * Called by DHT_Service.doGet() when ESP32 posts sensor data.
 */

function handleDataWrite(e) {
  var spreadsheet = SpreadsheetApp.openById(`12saPXGjrX9FgS8wwdGTdDPg30dSMJZHt9wh2gNV9qWc`);

  var run = e.parameter.run;
  var humidity = e.parameter.humidity;
  var temperature = e.parameter.temperature;
  var heatIndex = e.parameter.heatIndex;

  if (!run || humidity === undefined || temperature === undefined || heatIndex === undefined) {
    return ContentService.createTextOutput(
      'Error: Missing parameters (run, humidity, temperature, heatIndex)')
      .setMimeType(ContentService.MimeType.TEXT);
  }

  humidity = parseFloat(humidity);
  temperature = parseFloat(temperature);
  heatIndex = parseFloat(heatIndex);

  if (isNaN(humidity) || isNaN(temperature) || isNaN(heatIndex)) {
    return ContentService.createTextOutput('Error: Invalid number format')
      .setMimeType(ContentService.MimeType.TEXT);
  }

  var sheetName = 'DHT22-' + run;
  var sheet = spreadsheet.getSheetByName(sheetName);

  if (!sheet) {
    sheet = spreadsheet.insertSheet(sheetName);
    setupSheet(sheet);
  }

  var now = new Date();
  var timestamp = Utilities.formatDate(now, 'UTC', 'yyyy-MM-dd HH:mm:ss');
  var dataRow = [timestamp, humidity, temperature, heatIndex];
  var nextRow = sheet.getLastRow() + 1;
  var dataRange = sheet.getRange(nextRow, 1, 1, dataRow.length);
  dataRange.setValues([dataRow]);
  dataRange.setHorizontalAlignment('center');
  dataRange.setNumberFormats([['@', '0.0', '0.0', '0.0']]);

  return ContentService.createTextOutput('OK')
    .setMimeType(ContentService.MimeType.TEXT);
}

function setupSheet(sheet) {
  var headers = ['Timestamp (UTC)', 'Humidity (%)', 'Temperature (°C)', 'Heat Index (°C)'];
  var headerRange = sheet.getRange(1, 1, 1, headers.length);
  headerRange.setValues([headers]);
  headerRange.setFontWeight('bold');
  headerRange.setBackground('#4285f4');
  headerRange.setFontColor('#ffffff');
  headerRange.setHorizontalAlignment('center');
  headerRange.setFontSize(11);
  sheet.setColumnWidth(1, 180);
  sheet.setColumnWidth(2, 120);
  sheet.setColumnWidth(3, 140);
  sheet.setColumnWidth(4, 140);
  sheet.setFrozenRows(1);
  sheet.getRange(1, 1, 1, headers.length).createFilter();
}
