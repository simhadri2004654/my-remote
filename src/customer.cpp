#include "customer.h"

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "product_store.h"

extern ESP8266WebServer webServer;

// Customer website: simple, responsive product selector and large mint price card.
// The displayed price has no currency prefix, preventing the previous encoding issue.
static const char CUSTOMER_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="theme-color" content="#f3f5f7">
<title>Lakshiv | Product Prices</title>
<style>
:root{color-scheme:light;--green:#168443;--mint:#e8f7ed;--ink:#171717;--muted:#596273}
*{box-sizing:border-box}
body{margin:0;padding:18px;background:#f3f5f7;color:var(--ink);font-family:Inter,-apple-system,BlinkMacSystemFont,"Segoe UI",Arial,sans-serif}
.page{width:100%;max-width:620px;margin:34px auto;padding:26px;background:#fff;border-radius:18px;box-shadow:0 8px 30px rgba(20,35,50,.07)}
.brand{text-align:center;margin:0 0 4px;font-size:12px;font-weight:800;letter-spacing:1.2px;color:#168443}
h1{text-align:center;font-size:26px;margin:8px 0;color:#202938}
.hint{text-align:center;color:#778194;font-size:14px;margin:0 0 20px}
select{display:block;width:100%;max-width:440px;margin:0 auto;padding:14px 15px;border:1px solid #d9e0e7;border-radius:10px;background:white;color:#202938;font-size:16px;outline:none}
select:focus{border-color:#65ad82;box-shadow:0 0 0 3px rgba(22,132,67,.11)}
.price-card{display:none;min-height:232px;margin:24px auto 0;padding:27px 12px 24px;border-radius:16px;background:var(--mint);text-align:center;flex-direction:column;align-items:center;justify-content:center}
.product-name{margin:0 0 12px;color:#171717;font-size:36px;line-height:1.2;font-weight:400;overflow-wrap:anywhere}
.price{margin:0;color:var(--green);font-size:58px;line-height:1.15;font-weight:750;letter-spacing:.2px;font-variant-numeric:tabular-nums}
.unit{margin:10px 0 0;color:#555;font-size:24px;line-height:1.25;font-weight:400}
.status{min-height:18px;margin-top:16px;text-align:center;color:#7d8795;font-size:12px}
@media(max-width:480px){body{padding:10px}.page{margin:12px auto;padding:17px}.price-card{min-height:220px;padding:24px 8px}.product-name{font-size:31px}.price{font-size:48px}.unit{font-size:21px}}
</style>
</head>
<body>
<main class="page">
  <p class="brand">LAKSHIV TECHNOLOGIES &amp; SERVICES</p>
  <h1>Product Price</h1>
  <p class="hint">Choose a product to view its latest price.</p>
  <select id="productSelect" aria-label="Select a product" onchange="showSelectedProduct()">
    <option value="">Select Product</option>
  </select>
  <section class="price-card" id="priceCard" aria-live="polite">
    <div class="product-name" id="productName"></div>
    <div class="price" id="productPrice"></div>
    <div class="unit" id="productUnit"></div>
  </section>
  <div class="status" id="status">Loading current prices...</div>
</main>
<script>
'use strict';
let products=[];
async function loadPrices(){
  const select=document.getElementById('productSelect');
  const previous=select.value;
  try{
    const response=await fetch('/api/products?t='+Date.now(),{cache:'no-store'});
    if(!response.ok) throw new Error('Price service unavailable');
    products=await response.json();
    select.innerHTML='<option value="">Select Product</option>';
    products.forEach(function(p){
      const option=document.createElement('option');
      option.value=p.id; option.textContent=p.name; select.appendChild(option);
    });
    if(products.some(function(p){return p.id===previous;})) select.value=previous;
    showSelectedProduct();
    document.getElementById('status').textContent='Prices are up to date.';
  }catch(error){
    document.getElementById('status').textContent='Unable to load prices. Please try again.';
  }
}
function showSelectedProduct(){
  const id=document.getElementById('productSelect').value;
  const product=products.find(function(p){return p.id===id;});
  const card=document.getElementById('priceCard');
  if(!product){card.style.display='none';return;}
  document.getElementById('productName').textContent=product.name;
  // Deliberately no currency symbol: example output is 100.00.
  document.getElementById('productPrice').textContent=Number(product.price).toFixed(2);
  document.getElementById('productUnit').textContent='per '+product.unit;
  card.style.display='flex';
}
loadPrices();
setInterval(loadPrices,15000);
</script>
</body>
</html>
)rawliteral";

void HandleCustomerPage()
{
    webServer.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
    webServer.send_P(200, PSTR("text/html; charset=utf-8"), CUSTOMER_PAGE);
}

void HandleProductsApi()
{
    webServer.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
    webServer.sendHeader("Pragma", "no-cache");
    const String json = BuildProductsJson();
    webServer.send(200, "application/json; charset=utf-8", json);
}
