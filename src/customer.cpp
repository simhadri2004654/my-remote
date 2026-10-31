#include "customer.h"

const char CUSTOMER_PAGE[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#f3f7f5">
<title>Customer Access</title>
<style>
*{box-sizing:border-box}
body{margin:0;padding:24px;font-family:Arial,Helvetica,sans-serif;background:#f3f7f5;color:#17211b}
.card{max-width:520px;margin:8vh auto;background:#fff;border:1px solid #e5ece7;border-radius:18px;padding:32px;box-shadow:0 12px 32px rgba(24,55,38,.08);text-align:center}
.mark{width:54px;height:54px;margin:0 auto 18px;border-radius:16px;background:#e6f6eb;color:#168443;display:grid;place-items:center;font-size:28px;font-weight:700}
h1{font-size:25px;margin:0 0 10px}
p{color:#637168;line-height:1.6;margin:8px 0}
.status{display:inline-block;margin-top:14px;padding:9px 14px;border-radius:999px;background:#e6f6eb;color:#168443;font-size:13px;font-weight:700}
small{display:block;margin-top:24px;color:#8a978f}
</style>
</head>
<body>
<div class="card">
  <div class="mark">✓</div>
  <h1>Scan successful</h1>
  <p>Your QR scan has been accepted. You have reached the customer website.</p>
  <div class="status">Access confirmed</div>
  <small>Scan ID: <span id="scanId">-</span></small>
</div>
<script>
const params=new URLSearchParams(location.search);
document.getElementById('scanId').textContent=params.get('id')||'accepted';
</script>
</body>
</html>
)rawliteral";

const char EXPIRED_PAGE[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>QR Expired</title>
<style>
body{margin:0;padding:24px;font-family:Arial,sans-serif;background:#f7f4f3;color:#29201e}
.card{max-width:480px;margin:10vh auto;background:#fff;border-radius:16px;padding:30px;text-align:center;box-shadow:0 10px 28px rgba(0,0,0,.07)}
h1{font-size:24px;color:#a33b32}p{color:#665d5b;line-height:1.6}
</style>
</head>
<body><div class="card"><h1>QR code expired</h1>
<p>This QR code has already been used or is no longer active.</p>
<p>Please scan the latest QR code displayed on the E-Paper screen.</p></div></body></html>
)rawliteral";

const char HOME_PAGE[] PROGMEM = R"rawliteral(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>QR Access</title></head>
<body style="font-family:Arial,sans-serif;max-width:560px;margin:12vh auto;padding:24px;text-align:center;background:#f3f7f5;color:#24332a">
<h2>Automatic QR Access System</h2><p>Scan the current QR code shown on the E-Paper display to continue.</p></body></html>
)rawliteral";

void customerSendPage(ESP8266WebServer &server, const String &scanId)
{
    (void)scanId; // Scan ID is also read from the URL by the page's JavaScript.
    server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
    server.send_P(200, PSTR("text/html; charset=utf-8"), CUSTOMER_PAGE);
}

void customerSendExpiredPage(ESP8266WebServer &server)
{
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(410, PSTR("text/html; charset=utf-8"), EXPIRED_PAGE);
}

void customerSendHomePage(ESP8266WebServer &server)
{
    server.sendHeader("Cache-Control", "no-store");
    server.send_P(200, PSTR("text/html; charset=utf-8"), HOME_PAGE);
}
