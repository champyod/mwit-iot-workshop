/**
 * DHT_Webapp — Dashboard data functions
 * Called by DHT_Service.doGet() via HtmlService and google.script.run.
 * Uses CacheService to reduce SpreadsheetApp round-trips.
 */

var CACHE_TTL = 15; // seconds
var SPREADSHEET_ID = '12saPXGjrX9FgS8wwdGTdDPg30dSMJZHt9wh2gNV9qWc';

function getSS() { return SpreadsheetApp.openById(SPREADSHEET_ID); }

function toDate(v) {
  if (v instanceof Date) return v;
  // writing script stores UTC string: 'yyyy-MM-dd HH:mm:ss' → parse as UTC
  var pts = v.split(/[- :]/);
  return new Date(Date.UTC(pts[0], pts[1]-1, pts[2], pts[3], pts[4], pts[5]));
}

/**
 * List all DHT22-* sheets, sorted newest first.
 * Returns [{ name: "DHT22-2026-06-09_14:30:00", label: "Run …" }, ...]
 */
function getFormList() {
  var cache = CacheService.getScriptCache();
  var cached = cache.get('formList');
  if (cached) return JSON.parse(cached);

  var sheets = getSS().getSheets();
  var result = [];

  for (var i = 0; i < sheets.length; i++) {
    var name = sheets[i].getName();
    if (name.indexOf('DHT22-') === 0) {
      var runPart = name.substring(6); // strip "DHT22-"
      var label = 'Run ' + runPart.replace(/_/g, ' ');
      result.push({ name: name, label: label });
    }
  }

  result.sort(function(a, b) { return b.name.localeCompare(a.name); });
  cache.put('formList', JSON.stringify(result), CACHE_TTL * 2);
  return result;
}

/**
 * Get the minimum and maximum timestamps in the sheet (data time range).
 * @param {string} formName — optional sheet name.
 * @returns {object|null} { startISO, endISO }
 */
function getSheetTimeRange(formName) {
  var cacheKey = formName ? 'range_' + formName : 'range_';
  var cache = CacheService.getScriptCache();
  var cached = cache.get(cacheKey);
  if (cached) return JSON.parse(cached);

  var sheetInfo = formName ? resolveSheet(formName) : findLatestSheet();
  if (!sheetInfo) return null;

  var sheet = sheetInfo.sheet;
  var lastRow = sheet.getLastRow();
  if (lastRow < 2) return null;

  var firstVal = sheet.getRange(2, 1, 1, 1).getValue();
  var lastVal  = sheet.getRange(lastRow, 1, 1, 1).getValue();

  var result = {
    startISO: Utilities.formatDate(toDate(firstVal), 'UTC', "yyyy-MM-dd'T'HH:mm:ss'Z'"),
    endISO:   Utilities.formatDate(toDate(lastVal),   'UTC', "yyyy-MM-dd'T'HH:mm:ss'Z'")
  };
  cache.put(cacheKey, JSON.stringify(result), CACHE_TTL);
  return result;
}

/**
 * Get the latest sensor reading.
 * @param {string} formName — optional sheet name. Falls back to newest DHT22-* sheet if null/empty.
 */
function getLatestData(formName) {
  var cacheKey = formName ? 'latest_' + formName : 'latest';
  var cache = CacheService.getScriptCache();
  var cached = cache.get(cacheKey);
  if (cached) return JSON.parse(cached);

  var sheetInfo = formName ? resolveSheet(formName) : findLatestSheet();
  if (!sheetInfo) return null;

  var sheet = sheetInfo.sheet;
  var lastRow = sheet.getLastRow();
  if (lastRow < 2) return null;

  var row = sheet.getRange(lastRow, 1, 1, 4).getValues()[0];
  var result = {
    timestamp:     Utilities.formatDate(toDate(row[0]), 'UTC', "yyyy-MM-dd'T'HH:mm:ss'Z'"),
    temperature:   row[2],
    humidity:      row[1],
    heatIndex:     row[3],
    runStartTime:  sheetInfo.runStartTime
  };
  cache.put(cacheKey, JSON.stringify(result), CACHE_TTL);
  return result;
}

/**
 * Get history of sensor readings (aggregated when > 500 rows).
 * @param {string} formName — optional sheet name.
 * @param {number} limit — max rows (0 = all). Ignored when startTime/endTime set.
 * @param {string} [startTime] — ISO UTC start (optional, enables date-range mode).
 * @param {string} [endTime]   — ISO UTC end (optional, enables date-range mode).
 */
function getHistoryData(formName, limit, startTime, endTime) {
  var hasDateRange = startTime && endTime;
  var cacheKey;
  if (hasDateRange) {
    cacheKey = formName ? 'history_' + formName + '_' + startTime + '_' + endTime : 'history__' + startTime + '_' + endTime;
  } else {
    cacheKey = formName ? 'history_' + formName + '_' + limit : 'history_' + limit;
  }
  var cache = CacheService.getScriptCache();
  var cached = cache.get(cacheKey);
  if (cached) return JSON.parse(cached);

  var sheetInfo = formName ? resolveSheet(formName) : findLatestSheet();
  if (!sheetInfo) return null;

  var sheet = sheetInfo.sheet;
  var lastRow = sheet.getLastRow();
  if (lastRow < 2) return null;

  // Gather raw rows (from date range or limit)
  var rawRows;
  if (hasDateRange) {
    var startDate = new Date(startTime);
    var endDate   = new Date(endTime);
    var allRows = sheet.getRange(2, 1, lastRow - 1, 4).getValues();
    rawRows = [];
    for (var i = 0; i < allRows.length; i++) {
      var rowDate = toDate(allRows[i][0]);
      if (rowDate >= startDate && rowDate <= endDate) {
        rawRows.push(allRows[i]);
      }
    }
  } else {
    var count = (limit > 0) ? Math.min(limit, lastRow - 1) : (lastRow - 1);
    var startRow = lastRow - count + 1;
    rawRows = sheet.getRange(startRow, 1, count, 4).getValues();
  }

  // Build result — aggregate into ~500 pts if over threshold, else raw
  var result = [];
  if (rawRows.length > 500) {
    var bucketSize = Math.ceil(rawRows.length / 500);
    for (var i = 0; i < rawRows.length; i += bucketSize) {
      var end = Math.min(i + bucketSize, rawRows.length);
      var n = end - i;
      var sumTemp = 0, sumHum = 0, sumHeat = 0;
      for (var j = i; j < end; j++) {
        sumTemp += rawRows[j][2];
        sumHum  += rawRows[j][1];
        sumHeat += rawRows[j][3];
      }
      result.push({
        timestamp:   Utilities.formatDate(toDate(rawRows[end - 1][0]), 'UTC', "yyyy-MM-dd'T'HH:mm:ss'Z'"),
        temperature: sumTemp / n,
        humidity:    sumHum / n,
        heatIndex:   sumHeat / n
      });
    }
  } else {
    for (var i = 0; i < rawRows.length; i++) {
      result.push({
        timestamp:   Utilities.formatDate(toDate(rawRows[i][0]), 'UTC', "yyyy-MM-dd'T'HH:mm:ss'Z'"),
        humidity:    rawRows[i][1],
        temperature: rawRows[i][2],
        heatIndex:   rawRows[i][3]
      });
    }
  }

  cache.put(cacheKey, JSON.stringify(result), CACHE_TTL);
  return result;
}

/**
 * Get global statistics for the entire dataset.
 * @param {string} formName — optional sheet name.
 * @returns {object|null} { count, temp: {min, max, avg}, humid: {min, max, avg}, heat: {min, max, avg} }
 */
function getGlobalStats(formName) {
  var cacheKey = formName ? 'global_stats_' + formName : 'global_stats';
  var cache = CacheService.getScriptCache();
  var cached = cache.get(cacheKey);
  if (cached) return JSON.parse(cached);

  var sheetInfo = formName ? resolveSheet(formName) : findLatestSheet();
  if (!sheetInfo) return null;

  var sheet = sheetInfo.sheet;
  var lastRow = sheet.getLastRow();
  if (lastRow < 2) return { count: 0, temp: {min:0, max:0, avg:0}, humid: {min:0, max:0, avg:0}, heat: {min:0, max:0, avg:0} };

  var data = sheet.getRange(2, 1, lastRow - 1, 4).getValues();
  var count = data.length;

  var sumTemp = 0, minTemp = Infinity, maxTemp = -Infinity;
  var sumHum = 0, minHum = Infinity, maxHum = -Infinity;
  var sumHeat = 0, minHeat = Infinity, maxHeat = -Infinity;

  for (var i = 0; i < count; i++) {
    var temp = data[i][2];
    var hum = data[i][1];
    var heat = data[i][3];

    if (typeof temp === 'number') {
      sumTemp += temp;
      if (temp < minTemp) minTemp = temp;
      if (temp > maxTemp) maxTemp = temp;
    }
    if (typeof hum === 'number') {
      sumHum += hum;
      if (hum < minHum) minHum = hum;
      if (hum > maxHum) maxHum = hum;
    }
    if (typeof heat === 'number') {
      sumHeat += heat;
      if (heat < minHeat) minHeat = heat;
      if (heat > maxHeat) maxHeat = heat;
    }
  }

  var result = {
    count: count,
    temp: { min: minTemp === Infinity ? 0 : minTemp, max: maxTemp === -Infinity ? 0 : maxTemp, avg: sumTemp / count },
    humid: { min: minHum === Infinity ? 0 : minHum, max: maxHum === -Infinity ? 0 : maxHum, avg: sumHum / count },
    heat: { min: minHeat === Infinity ? 0 : minHeat, max: maxHeat === -Infinity ? 0 : maxHeat, avg: sumHeat / count }
  };

  cache.put(cacheKey, JSON.stringify(result), 150); // Aggressive cache: 150s
  return result;
}

/**
 * Get the latest N raw readings (for the table, with pagination).
 * @param {string} formName — optional sheet name.
 * @param {number} count — number of rows per page.
 * @param {number} offset — number of rows to skip.
 * @param {string} [startTime] — ISO UTC start (optional, enables date-range mode).
 * @param {string} [endTime]   — ISO UTC end (optional, enables date-range mode).
 */
function getLatestRawData(formName, count, offset, startTime, endTime) {
  var hasDateRange = startTime && endTime;
  var cacheKey;
  if (hasDateRange) {
    cacheKey = formName ? 'raw_' + formName + '_' + startTime + '_' + endTime : 'raw__' + startTime + '_' + endTime;
  } else {
    if (!count || count < 1) count = 100;
    if (offset < 0) offset = 0;
    cacheKey = formName ? 'raw_' + formName + '_' + count + '_' + offset : 'raw_' + count + '_' + offset;
  }
  var cache = CacheService.getScriptCache();
  var cached = cache.get(cacheKey);
  if (cached) return JSON.parse(cached);

  var sheetInfo = formName ? resolveSheet(formName) : findLatestSheet();
  if (!sheetInfo) return null;

  var sheet = sheetInfo.sheet;
  var lastRow = sheet.getLastRow();
  if (lastRow < 2) return [];

  // Date-range mode: read all, filter by time, return latest 500 raw rows descending
  if (hasDateRange) {
    var startDate = new Date(startTime);
    var endDate   = new Date(endTime);
    var allRows = sheet.getRange(2, 1, lastRow - 1, 4).getValues();
    var filtered = [];
    for (var i = 0; i < allRows.length; i++) {
      var rowDate = toDate(allRows[i][0]);
      if (rowDate >= startDate && rowDate <= endDate) {
        filtered.push(allRows[i]);
      }
    }
    var slice = filtered.slice(-500).reverse();
    var result = [];
    for (var i = 0; i < slice.length; i++) {
      result.push({
        timestamp:   Utilities.formatDate(toDate(slice[i][0]), 'UTC', "yyyy-MM-dd'T'HH:mm:ss'Z'"),
        humidity:    slice[i][1],
        temperature: slice[i][2],
        heatIndex:   slice[i][3]
      });
    }
    cache.put(cacheKey, JSON.stringify(result), CACHE_TTL);
    return result;
  }

  // We want the 'offset' most recent records, but starting from the 'count' before that.
  // Wait, pagination usually goes from start to end. 
  // For "Recent Readings", page 1 should be the most recent rows.
  // Page 1: lastRow-1 down to lastRow-count
  // Page 2: lastRow-count-1 down to lastRow-2*count
  
  // Let's use a standard: offset 0 is the most recent 'count' rows.
  // offset 1 is the next 'count' rows.
  
  var totalAvailable = lastRow - 1;
  var startIdx = Math.max(0, totalAvailable - (offset * count) - count); // Start from the end of the requested block
  var endIdx = Math.max(0, totalAvailable - (offset * count)); // End at the start of the requested block
  
  // Actually, simpler:
  // Let's treat the sheet as an array where index 0 is row 2, index N is lastRow.
  // Page 1 (offset 0): indices [totalAvailable - count, totalAvailable - 1]
  // Page 2 (offset 1): indices [totalAvailable - 2*count, totalAvailable - count - 1]
  
  var targetEnd = totalAvailable - (offset * count);
  var targetStart = Math.max(0, targetEnd - count);
  
  if (targetEnd <= 0) return [];

  var numToFetch = targetEnd - targetStart;
  var startRow = targetStart + 2; // +2 because 1-indexed and header
  var rows = sheet.getRange(startRow, 1, numToFetch, 4).getValues();

  // But wait, the user wants "Recent Readings". Usually this means newest at the top.
  // So Page 1 = rows [lastRow, lastRow-count+1]
  // Page 2 = rows [lastRow-count, lastRow-2*count+1]
  
  // Let's implement:
  // offset 0: [lastRow - count + 1, lastRow]
  // offset 1: [lastRow - 2*count + 1, lastRow - count]
  
  var fetchCount = count;
  var fetchStart = lastRow - (offset * count) - count + 1;
  if (fetchStart < 2) {
    fetchStart = 2;
    fetchCount = (lastRow - (offset * count)) - 1;
  }
  
  if (fetchStart > lastRow) return [];
  
  var rows = sheet.getRange(fetchStart, 1, fetchCount, 4).getValues();
  // But we want them in descending order (newest first) for the table
  rows.reverse();

  var result = [];
  for (var i = 0; i < rows.length; i++) {
    result.push({
      timestamp:   Utilities.formatDate(toDate(rows[i][0]), 'UTC', "yyyy-MM-dd'T'HH:mm:ss'Z'"),
      humidity:    rows[i][1],
      temperature: rows[i][2],
      heatIndex:   rows[i][3]
    });
  }
  
  cache.put(cacheKey, JSON.stringify(result), CACHE_TTL);
  return result;
}


/**
 * Resolve a named DHT22-* sheet.
 */
function resolveSheet(formName) {
  var sheet = getSS().getSheetByName(formName);
  if (!sheet) return null;
  var runStartTime = null;
  if (formName.indexOf('DHT22-') === 0) {
    runStartTime = formName.substring(6);
  }
  return { sheet: sheet, runStartTime: runStartTime };
}

/**
 * Find the most recent DHT22-* sheet (fallback when no formName given).
 */
function findLatestSheet() {
  var cache = CacheService.getScriptCache();
  var cached = cache.get('sheetName');
  if (cached) {
    var sheet = getSS().getSheetByName(cached);
    if (sheet) {
      var runStartTime = cached.substring(6);
      return { sheet: sheet, runStartTime: runStartTime };
    }
  }

  var sheets = getSS().getSheets();
  var dhtSheets = [];
  for (var i = 0; i < sheets.length; i++) {
    var name = sheets[i].getName();
    if (name.indexOf('DHT22-') === 0) {
      dhtSheets.push(sheets[i]);
    }
  }

  if (dhtSheets.length === 0) return null;

  dhtSheets.sort(function(a, b) {
    return b.getName().localeCompare(a.getName());
  });

  var sheet = dhtSheets[0];
  var sheetName = sheet.getName();
  var runStartTime = null;
  if (sheetName.indexOf('DHT22-') === 0) {
    runStartTime = sheetName.substring(6);
  }

  cache.put('sheetName', sheetName, CACHE_TTL * 2);
  return { sheet: sheet, runStartTime: runStartTime };
}
