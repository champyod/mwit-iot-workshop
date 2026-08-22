# Google Sheets logging

The firmware pushes one telemetry row every 10 s while running, plus an
immediate row on every tier change, through an Apps Script web app.
`Code.gs` in this directory is that script.

## Columns written (single `Log` tab)

Timestamp (UTC) · Run · Event (`heartbeat`/`tier_change`) · Running ·
Nearest cm · Tier · S1 Raw cm · S1 cm · S1 Status · S2 Raw cm · S2 cm ·
S2 Status · RSSI dBm · Heap B · Uptime ms

The tab is auto-created and styled (bold header, frozen top row, filter)
on the first received row.

## Setup (~3 minutes)

1. Create a spreadsheet: https://sheets.new
2. In the sheet: **Extensions → Apps Script**, replace the placeholder
   code with the full contents of `Code.gs`, Save.
   Bound scripts find their spreadsheet automatically — no ID editing.
3. **Deploy → New deployment → ⚙ Select type → Web app**
   - Execute as: `Me`
   - Who has access: `Anyone`
4. Authorize when prompted, then copy the web app URL:
   `https://script.google.com/macros/s/<ID>/exec`
5. Put it into `include/credentials.h`:

   ```c
   #define SHEETS_HOST "script.google.com"
   #define SHEETS_PATH "/macros/s/<ID>/exec"
   ```

Standalone alternative: create the project at script.google.com instead,
paste the same file, set `SPREADSHEET_ID` at the top to your sheet's ID.

## Smoke test (no hardware)

Paste this in a browser — a styled `Log` tab appears with one row:

```
https://script.google.com/macros/s/<ID>/exec?run=test&event=heartbeat&running=true&nearest_cm=42.0&tier=SAFE&s1_raw=50.0&s1_cm=50.0&s1_status=ok&s2_raw=60.0&s2_cm=60.0&s2_status=ok&rssi_dbm=-60&free_heap=200000&uptime_ms=123456
```

Expected response body: `OK`.

## Notes

- The transport is HTTP GET by design: Apps Script web apps answer the
  first request with a 302 redirect to `script.googleusercontent.com`,
  where only GET is accepted. The firmware follows redirects manually.
- Rows carry a server-side UTC timestamp from Apps Script; the device
  clock is not trusted for logging.
