/**
 * DHT_Service — request router
 *
 * ESP32 posting data → calls DHT_Data_Writing.handleDataWrite(e)
 * Browser visit       → serves Index.html (dashboard uses DHT_Webapp.getLatestData)
 */

function doGet(e) {
  if (e.parameter.humidity !== undefined) {
    return handleDataWrite(e);
  }

  return HtmlService.createHtmlOutputFromFile('webapp')
      .setTitle('DHT22 Sensor Dashboard')
      .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL)
      .addMetaTag('viewport', 'width=device-width, initial-scale=1');
}
