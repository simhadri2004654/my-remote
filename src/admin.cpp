#include "admin.h"

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <math.h>
#include <stdlib.h>

#include "app_config.h"
#include "product_store.h"

extern ESP8266WebServer webServer;

static bool CheckAdminAuthentication()
{
    if (webServer.authenticate(ADMIN_USERNAME, ADMIN_PASSWORD)) {
        return true;
    }

    webServer.requestAuthentication(
        BASIC_AUTH,
        "Lakshiv Admin",
        "Administrator login required"
    );
    return false;
}

// A lightweight professional dashboard. CSS and JavaScript remain in flash (PROGMEM).
static const char ADMIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="theme-color" content="#111827">
<title>Lakshiv | Admin Console</title>
<style>
:root{--navy:#111827;--navy2:#1d293b;--green:#168451;--text:#17212f;--muted:#748094;--border:#e6eaf0;--page:#f5f7fa;--shadow:0 8px 28px rgba(24,39,61,.06)}
*{box-sizing:border-box}
body{margin:0;background:var(--page);color:var(--text);font-family:Inter,-apple-system,BlinkMacSystemFont,"Segoe UI",Arial,sans-serif;font-size:14px}
button,input{font:inherit}.shell{min-height:100vh;display:flex}.sidebar{width:242px;flex:0 0 242px;background:var(--navy);color:#eef3f8;padding:24px 17px;display:flex;flex-direction:column}
.brand{display:flex;align-items:center;gap:11px;padding:0 8px 26px;border-bottom:1px solid #ffffff1a}.brand-mark{width:39px;height:39px;border-radius:11px;background:#1e9a68;color:white;display:grid;place-items:center;font-size:21px;font-weight:800}.brand-name{font-size:12px;font-weight:800;line-height:1.4;letter-spacing:.4px}.brand-sub{display:block;margin-top:3px;color:#9caabd;font-size:9px;letter-spacing:1.1px}
.nav-label{padding:26px 11px 10px;color:#8290a4;font-size:10px;font-weight:700;letter-spacing:1.5px;text-transform:uppercase}.nav-link{display:flex;align-items:center;gap:11px;padding:12px;border-radius:9px;color:#b9c4d2;text-decoration:none;font-weight:600;margin:3px 0}.nav-link.active{background:#253449;color:#fff}.nav-icon{width:19px;text-align:center;color:#a9ddc2}.side-foot{margin-top:auto;padding:15px 10px 3px;border-top:1px solid #ffffff1a;color:#d5deea;font-size:12px}.dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#42d392;margin-right:8px}.side-note{padding:8px 0 0 16px;color:#8f9caf;font-size:10px}
.main{min-width:0;flex:1}.topbar{height:70px;background:white;border-bottom:1px solid var(--border);display:flex;align-items:center;justify-content:space-between;padding:0 34px}.crumb{color:#8490a1;font-size:12px}.crumb strong{color:#344154}.profile{display:flex;align-items:center;gap:10px}.avatar{width:35px;height:35px;border-radius:50%;background:#e7f4ec;color:#167749;display:grid;place-items:center;font-weight:800;font-size:12px}.profile-name{font-size:12px;font-weight:750}.profile-role{font-size:10px;color:var(--muted);margin-top:3px}
.content{max-width:1160px;margin:0 auto;padding:32px 34px 24px}.heading{display:flex;align-items:flex-end;justify-content:space-between;gap:18px;margin-bottom:24px}.eyebrow{margin:0 0 8px;color:var(--green);font-size:10px;font-weight:800;letter-spacing:1.5px}h1{margin:0;color:var(--text);font-size:29px;letter-spacing:-.7px}.desc{margin:8px 0 0;color:#778397;font-size:13px;line-height:1.5}.btn{min-height:38px;display:inline-flex;align-items:center;justify-content:center;gap:7px;padding:0 13px;border-radius:8px;border:1px solid #dce3eb;background:#fff;color:#344154;text-decoration:none;font-size:12px;font-weight:750;cursor:pointer}.btn:hover{background:#f8fafc}.btn:disabled{opacity:.55;cursor:wait}
.metrics{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:14px;margin-bottom:20px}.metric{background:#fff;border:1px solid var(--border);border-radius:12px;padding:17px 18px;box-shadow:var(--shadow)}.metric-label{display:flex;justify-content:space-between;align-items:center;color:#778397;font-size:12px;font-weight:650}.metric-icon{width:30px;height:30px;border-radius:9px;background:#eef7f1;color:var(--green);display:grid;place-items:center}.metric-value{margin-top:10px;font-size:25px;line-height:1.1;font-weight:800;color:#1c2939}.metric-note{margin-top:6px;color:#8b96a6;font-size:10px}.live{display:inline-flex;align-items:center;gap:6px;padding:5px 8px;border-radius:20px;background:#eaf7ef;color:var(--green);font-size:10px;font-weight:800}.live:before{content:"";width:6px;height:6px;border-radius:50%;background:#20a36a}
.panel{background:#fff;border:1px solid var(--border);border-radius:13px;box-shadow:var(--shadow);overflow:hidden}.panel-head{display:flex;align-items:center;justify-content:space-between;gap:15px;padding:19px 21px;border-bottom:1px solid var(--border)}.panel-title{margin:0;font-size:15px;font-weight:800}.panel-sub{margin:5px 0 0;color:#8591a2;font-size:11px}.tools{display:flex;align-items:center;gap:8px}.search{height:37px;width:190px;padding:0 11px;border:1px solid #e0e6ed;border-radius:8px;outline:none;font-size:12px}.search:focus,.price-input:focus{border-color:#6dbb91;box-shadow:0 0 0 3px #1f995e1a}.table-wrap{width:100%;overflow-x:auto}table{width:100%;border-collapse:collapse;min-width:640px}thead{background:#fafbfd}th{padding:12px 18px;text-align:left;color:#8a95a5;font-size:10px;font-weight:800;letter-spacing:.8px;text-transform:uppercase;border-bottom:1px solid var(--border)}td{padding:13px 18px;border-bottom:1px solid #edf0f4;vertical-align:middle}tbody tr:last-child td{border-bottom:0}tbody tr:hover{background:#fcfdfd}.product{display:flex;align-items:center;gap:10px}.product-mark{width:34px;height:34px;border-radius:9px;background:#eef5f1;color:#16784a;display:grid;place-items:center;font-weight:800}.product-name{font-size:12px;font-weight:800;color:#293548}.product-id{margin-top:4px;color:#97a1af;font-size:10px}.unit{display:inline-block;padding:5px 8px;border-radius:6px;background:#f2f4f7;color:#687589;font-size:10px;font-weight:700}.current{font-size:13px;color:#334155;font-weight:800;font-variant-numeric:tabular-nums}.edit{display:flex;align-items:center;gap:7px}.price-input{width:106px;height:35px;padding:0 9px;border:1px solid #dfe5ec;border-radius:7px;outline:none;font-size:12px;font-weight:700}.save{height:34px;padding:0 11px;border-radius:7px;background:#eaf6ef;border:1px solid #d7ecdf;color:#167747;font-size:11px;font-weight:800;cursor:pointer}.save:hover{background:#dff1e6}.save:disabled{opacity:.5;cursor:wait}.state{display:block;margin-top:5px;color:#9aa4b1;font-size:10px}.state.changed{color:#a46b16}.state.saved{color:#168451}.empty{padding:30px;text-align:center;color:#8a95a5;font-size:12px}.panel-foot{display:flex;justify-content:space-between;gap:12px;padding:13px 19px;background:#fcfdfd;border-top:1px solid var(--border);color:#8994a3;font-size:10px}.footer{padding:19px 3px 0;color:#a0a9b5;font-size:10px;text-align:center;letter-spacing:.2px}.toast{position:fixed;right:22px;bottom:22px;z-index:10;max-width:calc(100% - 32px);padding:13px 17px;border-radius:9px;background:#172536;color:#fff;box-shadow:0 8px 25px #0003;font-size:12px;font-weight:700;opacity:0;transform:translateY(8px);pointer-events:none;transition:.2s}.toast.show{opacity:1;transform:translateY(0)}.toast.error{background:#a8323b}.mobile-brand{display:none}
@media(max-width:900px){.sidebar{width:210px;flex-basis:210px}.content{padding:26px 22px}.topbar{padding:0 22px}}
@media(max-width:680px){.shell{display:block}.sidebar{display:none}.topbar{height:60px;padding:0 15px}.crumb{display:none}.mobile-brand{display:flex;align-items:center;gap:8px;font-size:12px;font-weight:850}.mobile-brand .brand-mark{width:30px;height:30px;font-size:16px}.content{padding:22px 13px}.heading{align-items:flex-start;flex-direction:column}h1{font-size:25px}.metrics{grid-template-columns:1fr;gap:9px}.metric{padding:13px 15px}.metric-value{font-size:22px;margin-top:7px}.panel-head{align-items:flex-start;flex-direction:column;padding:16px}.tools{width:100%}.search{flex:1;min-width:0;width:100%}.panel-foot{padding:12px}.profile-text{display:none}.toast{right:14px;bottom:14px}}
</style>
</head>
<body>
<div class="shell">
  <aside class="sidebar">
    <div class="brand"><div class="brand-mark">L</div><div><div class="brand-name">LAKSHIV TECHNOLOGIES</div><span class="brand-sub">&amp; SERVICES PVT LTD.</span></div></div>
    <div class="nav-label">Workspace</div>
    <a class="nav-link" href="/"><span class="nav-icon">⌂</span> Customer website</a>
    <a class="nav-link active" href="/admin"><span class="nav-icon">▦</span> Price management</a>
    <div class="side-foot"><span class="dot"></span> Device connected<div class="side-note">Local network administration</div></div>
  </aside>
  <div class="main">
    <header class="topbar">
      <div class="mobile-brand"><span class="brand-mark">L</span> LAKSHIV ADMIN</div>
      <div class="crumb">Workspace&nbsp; / &nbsp;<strong>Price management</strong></div>
      <div class="profile"><div class="avatar">AD</div><div class="profile-text"><div class="profile-name">Administrator</div><div class="profile-role">Store control</div></div></div>
    </header>
    <main class="content">
      <section class="heading">
        <div><p class="eyebrow">STORE OPERATIONS / CATALOGUE</p><h1>Price management</h1><p class="desc">Manage product prices and publish the latest rates to your customer website.</p></div>
        <a class="btn" href="/" target="_blank" rel="noopener">View customer website ↗</a>
      </section>
      <section class="metrics">
        <div class="metric"><div class="metric-label">Products in catalogue <span class="metric-icon">▤</span></div><div class="metric-value" id="productCount">—</div><div class="metric-note">Products configured on this device</div></div>
        <div class="metric"><div class="metric-label">Price list status <span class="live">LIVE</span></div><div class="metric-value">Active</div><div class="metric-note">Customer page reads the latest saved prices</div></div>
        <div class="metric"><div class="metric-label">Customer refresh <span class="metric-icon">↻</span></div><div class="metric-value">15 sec</div><div class="metric-note">Customer page checks for price changes</div></div>
      </section>
      <section class="panel">
        <div class="panel-head"><div><h2 class="panel-title">Product prices</h2><p class="panel-sub">Edit a price and select Save to update the customer view.</p></div><div class="tools"><input class="search" id="searchInput" type="search" placeholder="Search products..." aria-label="Search products"><button class="btn" id="refreshButton" type="button">↻ Refresh</button></div></div>
        <div class="table-wrap"><table><thead><tr><th>Product</th><th>Unit</th><th>Current price</th><th>New price</th></tr></thead><tbody id="productRows"><tr><td colspan="4" class="empty">Loading products...</td></tr></tbody></table></div>
        <div class="panel-foot"><span id="resultCount">Loading catalogue...</span><span>Prices are saved on the ESP8266 device</span></div>
      </section>
      <footer class="footer">LAKSHIV TECHNOLOGIES &amp; SERVICES PVT LTD. &nbsp;•&nbsp; LOCAL ADMIN CONSOLE</footer>
    </main>
  </div>
</div>
<div class="toast" id="toast" role="status" aria-live="polite"></div>
<script>
'use strict';
let products=[];
let toastTimer=0;
function showToast(message,isError){
  const toast=document.getElementById('toast');
  toast.textContent=message;
  toast.className='toast show'+(isError?' error':'');
  if(toastTimer)clearTimeout(toastTimer);
  toastTimer=setTimeout(function(){toast.className='toast';},3000);
}
function makeCell(text,className){
  const cell=document.createElement('td');
  if(className)cell.className=className;
  cell.textContent=text;
  return cell;
}
function renderProducts(){
  const tbody=document.getElementById('productRows');
  const term=document.getElementById('searchInput').value.trim().toLowerCase();
  tbody.textContent='';
  const visible=products.filter(function(p){return p.name.toLowerCase().includes(term)||p.id.toLowerCase().includes(term);});
  if(visible.length===0){
    const row=document.createElement('tr');const cell=makeCell('No products match your search.','empty');cell.colSpan=4;row.appendChild(cell);tbody.appendChild(row);
  }
  visible.forEach(function(product){
    const row=document.createElement('tr');
    const productCell=document.createElement('td');
    const wrap=document.createElement('div');wrap.className='product';
    const mark=document.createElement('div');mark.className='product-mark';mark.textContent=product.name.charAt(0);
    const nameWrap=document.createElement('div');
    const name=document.createElement('div');name.className='product-name';name.textContent=product.name;
    const sku=document.createElement('div');sku.className='product-id';sku.textContent='SKU: '+product.id.toUpperCase();
    nameWrap.appendChild(name);nameWrap.appendChild(sku);wrap.appendChild(mark);wrap.appendChild(nameWrap);productCell.appendChild(wrap);row.appendChild(productCell);

    const unitCell=document.createElement('td');const unit=document.createElement('span');unit.className='unit';unit.textContent='per '+product.unit;unitCell.appendChild(unit);row.appendChild(unitCell);
    const currentCell=makeCell(Number(product.price).toFixed(2),'current');row.appendChild(currentCell);

    const editCell=document.createElement('td');const edit=document.createElement('div');edit.className='edit';
    const input=document.createElement('input');input.className='price-input';input.type='number';input.min='0';input.max='1000000';input.step='0.01';input.value=Number(product.price).toFixed(2);input.setAttribute('aria-label','New price for '+product.name);
    const save=document.createElement('button');save.className='save';save.type='button';save.textContent='Save';
    const state=document.createElement('span');state.className='state saved';state.textContent='Saved';
    input.addEventListener('input',function(){const changed=input.value.trim()===''||Number(input.value)!==Number(product.price);state.textContent=changed?'Unsaved changes':'Saved';state.className=changed?'state changed':'state saved';});
    save.addEventListener('click',function(){saveProductPrice(product.id,input,currentCell,save,state);});
    edit.appendChild(input);edit.appendChild(save);editCell.appendChild(edit);editCell.appendChild(state);row.appendChild(editCell);tbody.appendChild(row);
  });
  document.getElementById('productCount').textContent=products.length;
  document.getElementById('resultCount').textContent='Showing '+visible.length+' of '+products.length+' products';
}
async function loadProducts(showMessage){
  const button=document.getElementById('refreshButton');button.disabled=true;button.textContent='Loading...';
  try{
    const response=await fetch('/api/products?t='+Date.now(),{cache:'no-store'});
    if(!response.ok)throw new Error('Could not load product list');
    products=await response.json();renderProducts();
    if(showMessage)showToast('Product prices refreshed.');
  }catch(error){
    showToast('Unable to load products. Check the device connection.',true);
    document.getElementById('productRows').innerHTML='<tr><td colspan="4" class="empty">Could not load products. Use Refresh to try again.</td></tr>';
  }finally{button.disabled=false;button.textContent='↻ Refresh';}
}
async function saveProductPrice(id,input,currentCell,button,state){
  const value=input.value.trim();const price=Number(value);
  if(value===''||!Number.isFinite(price)||price<0||price>1000000){showToast('Enter a valid price from 0 to 1,000,000.',true);input.focus();return;}
  button.disabled=true;button.textContent='Saving...';state.textContent='Saving';state.className='state';
  const body=new URLSearchParams();body.append('id',id);body.append('price',value);
  try{
    const response=await fetch('/admin/update',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body.toString()});
    const result=await response.json();
    if(!response.ok||!result.ok)throw new Error(result.message||'Price update failed');
    const saved=Number(result.price);const product=products.find(function(p){return p.id===id;});if(product)product.price=saved;
    input.value=saved.toFixed(2);currentCell.textContent=saved.toFixed(2);state.textContent='Saved successfully';state.className='state saved';showToast(result.message||'Price updated successfully.');
  }catch(error){state.textContent='Not saved';state.className='state changed';showToast(error.message||'Could not save the price.',true);}
  finally{button.disabled=false;button.textContent='Save';}
}
document.getElementById('searchInput').addEventListener('input',renderProducts);
document.getElementById('refreshButton').addEventListener('click',function(){loadProducts(true);});
loadProducts(false);
</script>
</body>
</html>
)rawliteral";

void HandleAdminPage()
{
    if (!CheckAdminAuthentication()) return;

    webServer.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.send_P(200, PSTR("text/html; charset=utf-8"), ADMIN_PAGE);
}

void HandleAdminUpdate()
{
    if (!CheckAdminAuthentication()) return;

    const String id = webServer.arg("id");
    String priceText = webServer.arg("price");
    priceText.trim();

    const int index = FindProductById(id);
    if (index < 0) {
        webServer.send(400, "application/json",
                       "{\"ok\":false,\"message\":\"Unknown product.\"}");
        return;
    }

    char* endPointer = nullptr;
    const double newPrice = strtod(priceText.c_str(), &endPointer);
    if (priceText.length() == 0 ||
        endPointer == priceText.c_str() ||
        *endPointer != '\0' ||
        !isfinite(newPrice) ||
        newPrice < 0.0 ||
        newPrice > 1000000.0) {
        webServer.send(400, "application/json",
                       "{\"ok\":false,\"message\":\"Invalid price. Enter a value from 0 to 1,000,000.\"}");
        return;
    }

    const float oldPrice = gProducts[index].price;
    gProducts[index].price = static_cast<float>(newPrice);

    if (!SavePrices()) {
        gProducts[index].price = oldPrice;
        webServer.send(500, "application/json",
                       "{\"ok\":false,\"message\":\"Could not save the price to LittleFS.\"}");
        return;
    }

    Serial.println();
    Serial.println(F("PRODUCT PRICE UPDATED"));
    Serial.print(F("Product: "));
    Serial.println(gProducts[index].name);
    Serial.print(F("New price: "));
    Serial.println(gProducts[index].price, 2);

    // Product names are fixed constants in this firmware.
    String response;
    response.reserve(150);
    response = F("{\"ok\":true,\"message\":\"");
    response += gProducts[index].name;
    response += F(" price updated successfully!\",\"price\":");
    response += String(gProducts[index].price, 2);
    response += '}';

    webServer.send(200, "application/json; charset=utf-8", response);
}
