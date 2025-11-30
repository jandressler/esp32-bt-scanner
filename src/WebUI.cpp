#include "WebUI.h"

// ============================================================================
// MAIN HTML GENERATION
// ============================================================================

String WebUI::generateMainHTML() {
    String html;
    html.reserve(32768); // Massiv erhöht: 32KB für große HTML mit JavaScript
    
    html += buildHTMLHeader();
    html += buildStyles();
    html += "</head><body>";
    html += "<div class=\"container\">";
    html += buildPageHeader();
    html += buildControlsSection();
    html += buildDevicesSection();
    html += buildKnownDevicesSection();
    html += buildOutputLogSection();
    html += "</div>"; // container
    html += buildNotificationContainer();
    html += buildMainScript();
    html += buildHTMLFooter();
    
    return html;
}

String WebUI::generateModeSetupHTML(const String& beaconName, const BeaconConfig& beaconConfig) {
    String html;
    html.reserve(8192);
    
    html += buildHTMLHeader();
    html += buildModeSetupStyles();
    html += "</head><body>";
    html += "<div class='container'>";
    html += buildModeSetupHeader();
    html += buildTabButtons();
    html += buildBeaconTab(beaconName, beaconConfig);
    html += buildWiFiTab();
    html += buildAPTab();
    html += "<div id='message'></div>";
    html += "</div>";
    html += buildModeSetupScript();
    html += buildHTMLFooter();
    
    return html;
}

// ============================================================================
// HTML STRUCTURE
// ============================================================================

String WebUI::buildHTMLHeader() {
    return String(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>🔍 ESP32-C3 BT Scanner</title>"
    );
}

String WebUI::buildHTMLFooter() {
    return "</body></html>";
}

// ============================================================================
// CSS STYLES - Main UI
// ============================================================================

String WebUI::buildStyles() {
    return String(
        "<style>"
        // Base styles
        "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;margin:0;padding:20px;background:#f5f5f7;color:#1d1d1f;}"
        ".container{max-width:1200px;margin:0 auto;}"
        
        // Header
        ".header{background:white;padding:20px;border-radius:12px;box-shadow:0 4px 12px rgba(0,0,0,0.1);margin-bottom:20px;}"
        ".header h1{margin:0 0 10px 0;color:#1d1d1f;font-size:2em;}"
        
        // Status bar
        ".status-bar{display:flex;gap:20px;flex-wrap:wrap;margin-top:15px;}"
        ".status-item{background:#f6f6f6;padding:10px 15px;border-radius:8px;font-size:14px;}"
        ".status-item.online{background:#d4edda;color:#155724;}"
        
        // Controls
        ".controls{background:white;padding:20px;border-radius:12px;box-shadow:0 4px 12px rgba(0,0,0,0.1);margin-bottom:20px;}"
        ".btn-group{display:flex;gap:10px;flex-wrap:wrap;}"
        
        // Buttons
        "button{background:#007aff;color:white;border:none;padding:12px 20px;border-radius:8px;cursor:pointer;font-size:14px;font-weight:500;transition:all 0.2s;}"
        "button:hover{background:#0056d3;transform:translateY(-1px);}"
        "button.secondary{background:#8e8e93;}button.secondary:hover{background:#6d6d70;}"
        "button.danger{background:#ff3b30;}button.danger:hover{background:#d70015;}"
        "button.success{background:#34c759;}button.success:hover{background:#248a3d;}"
        
        // Devices section
        ".devices-section{background:white;border-radius:12px;box-shadow:0 4px 12px rgba(0,0,0,0.1);overflow:hidden;margin-bottom:20px;}"
        ".devices-header{padding:20px;border-bottom:1px solid #e5e5e7;display:flex;justify-content:space-between;align-items:center;}"
        ".devices-header h2{margin:0;font-size:1.5em;}"
        ".filter-controls{display:flex;gap:10px;align-items:center;}"
        ".filter-controls select,.filter-controls input{padding:8px 12px;border:1px solid #d1d1d6;border-radius:6px;}"
        
        // Device card
        ".device{padding:15px 20px;border-bottom:1px solid #f2f2f7;transition:background 0.2s;}"
        ".device:hover{background:#f9f9f9;}"
        ".device:last-child{border-bottom:none;}"
        ".device.known{background:#f0f8ff;border-left:4px solid #007aff;}"
        ".device.active{background:#f9f9f9;}"
        ".device.proximity-green{border-left:4px solid #34c759;background:#e8f5e8!important;}"
        ".device.proximity-yellow{border-left:4px solid #ff9500;background:#fff8e8!important;}"
        ".device.proximity-red{border-left:4px solid #ff3b30;background:#ffeaea!important;}"
        
        // Device layout
        ".device-row{display:flex;justify-content:space-between;align-items:center;}"
        ".device-info{flex:1;}"
        ".device-name{font-weight:600;font-size:16px;margin-bottom:4px;}"
        ".device-details{display:flex;gap:15px;font-size:14px;color:#8e8e93;margin-bottom:4px;}"
        ".device-meta{font-size:12px;color:#a1a1a6;}"
        ".device-actions{display:flex;gap:8px;}"
        ".device-actions button{padding:6px 12px;font-size:12px;}"
        
        // Notification
        ".notification{position:fixed;top:20px;right:20px;padding:15px 20px;background:#34c759;color:white;border-radius:8px;box-shadow:0 4px 12px rgba(0,0,0,0.2);z-index:1001;transform:translateX(400px);transition:transform 0.3s;}"
        ".notification.show{transform:translateX(0);}"
        ".notification.error{background:#ff3b30;}"
        
        // File input
        ".file-input{display:none;}"
        ".file-label{background:#007aff;color:white;border:none;padding:12px 20px;border-radius:8px;cursor:pointer;font-size:14px;font-weight:500;text-align:center;transition:all 0.2s;display:inline-block;}"
        ".file-label:hover{background:#0056d3;transform:translateY(-1px);}"
        
        // Responsive
        "@media(max-width:768px){"
        ".status-bar{flex-direction:column;}"
        ".btn-group{flex-direction:column;}"
        ".device-row{flex-direction:column;align-items:flex-start;}"
        ".device-actions{margin-top:10px;}"
        "}"
        "</style>"
    );
}

// ============================================================================
// MAIN UI SECTIONS
// ============================================================================

String WebUI::buildPageHeader() {
    return String(
        "<div class=\"header\">"
        "<h1>🔍 ESP32-C3 BT Scanner</h1>"
    ) + buildStatusBar() + "</div>";
}

String WebUI::buildStatusBar() {
    return String(
        "<div class=\"status-bar\">"
        "<div id=\"status-wifi\" class=\"status-item\">WiFi: Verbinde...</div>"
        "<div id=\"status-wifimode\" class=\"status-item\">Modus: -</div>"
        "<div id=\"status-ever\" class=\"status-item\">📈 Geräte (ever): <span id=\"stat-ever\">0</span></div>"
        "<div id=\"status-known\" class=\"status-item\">⭐ Bekannt: <span id=\"stat-known\">0</span></div>"
        "<div id=\"status-active\" class=\"status-item\">🟢 Aktiv: <span id=\"stat-active\">0</span></div>"
        "<div id=\"status-present\" class=\"status-item\">✅ Anwesend: <span id=\"stat-present\">0</span></div>"
        "<div id=\"status-scan\" class=\"status-item\">🔍 <span id=\"scan-status\">Scan: Lade...</span></div>"
        "<div id=\"status-output\" class=\"status-item\">🔗 <span id=\"output-status\">Ausgang: Lade...</span></div>"
        "<div id=\"status-uptime\" class=\"status-item\">Uptime: Lade...</div>"
        "</div>"
    );
}

String WebUI::buildControlsSection() {
    return String(
        "<div class=\"controls\">"
        "<h3>Steuerung</h3>"
        "<div class=\"btn-group\">"
        "<button onclick=\"refreshData()\" class=\"success\">🔄 Aktualisieren</button>"
        "<button onclick=\"exportDevices()\">📥 Export</button>"
        "<label for=\"import-file\" class=\"file-label\">📤 Import</label>"
        "<input type=\"file\" id=\"import-file\" class=\"file-input\" accept=\".json\" onchange=\"importDevices(this)\">"
        "<button onclick=\"resetWiFi()\" class=\"danger\">⚠️ Factory Reset</button>"
        "<button onclick=\"resetBluetooth()\" class=\"danger\">🔵 Bluetooth Reset</button>"
        "<button onclick=\"resetSystem()\" class=\"danger\">🔄 Reboot</button>"
        "</div>"
        "</div>"
    );
}

String WebUI::buildDevicesSection() {
    return String(
        "<div class=\"devices-section\">"
        "<div class=\"devices-header\">"
        "<h2>Aktuell gefundene Geräte</h2>"
        "<div class=\"filter-controls\">"
        "<small>Nur aktive Geräte in Bluetooth-Reichweite</small>"
        "</div>"
        "</div>"
        "<div id=\"devices-list\"></div>"
        "</div>"
    );
}

String WebUI::buildKnownDevicesSection() {
    return String(
        "<div class=\"devices-section\">"
        "<div class=\"devices-header\">"
        "<h2>Bekannte Geräte</h2>"
        "<div class=\"filter-controls\">"
        "<small>Alle bekannten Geräte (auch wenn nicht anwesend)</small>"
        "</div>"
        "</div>"
        "<div id=\"known-devices-list\"></div>"
        "</div>"
    );
}

String WebUI::buildOutputLogSection() {
    return String(
        "<div class=\"devices-section\">"
        "<div class=\"devices-header\">"
        "<h2>Ausgangs-Log</h2>"
        "<div class=\"filter-controls\">"
        "<button id=\"clear-log-btn\" class=\"action-btn\">Log löschen</button>"
        "</div>"
        "</div>"
        "<div id=\"output-log-list\"></div>"
        "</div>"
    );
}

String WebUI::buildNotificationContainer() {
    return "<div id=\"notification\" class=\"notification\"></div>";
}

// ============================================================================
// JAVASCRIPT - Main Script Assembly
// ============================================================================

String WebUI::buildMainScript() {
    String script;
    script.reserve(16384); // JavaScript ist groß - 16KB reservieren
    
    script += "<script>\n";
    script += buildCoreFunctions();
    script += buildDevicesFunctions();
    script += buildKnownDevicesFunctions();
    script += buildOutputLogFunctions();
    script += buildActionFunctions();
    script += buildInitFunction();
    script += "</script>";
    
    return script;
}

// ============================================================================
// JAVASCRIPT - Core Functions
// ============================================================================

String WebUI::buildCoreFunctions() {
    return String(
        // State
        "let devices=[];let knownDevices=[];let filteredDevices=[];\n"
        "let isLoading=false;\n"
        
        // Notification
        "function showNotification(m,e){"
        "const n=document.getElementById('notification');"
        "n.textContent=m;"
        "n.className='notification'+(e?' error':'')+' show';"
        "setTimeout(()=>{n.className='notification';},4000);"
        "}\n"
        
        // Stats update (wird jetzt vom Server geliefert)
        "function updateStats(){"
        "}\n"
        
        // Proximity status text
        "function getProximityStatusText(s){"
        "switch(s){"
        "case 'green':return '<span style=\"color:#34c759;\">🟢 In Reichweite</span>';"
        "case 'yellow':return '<span style=\"color:#ff9500;\">🟡 Nah</span>';"
        "case 'red':return '<span style=\"color:#ff3b30;\">🔴 Nicht sichtbar</span>';"
        "default:return '<span style=\"color:#8e8e93;\">❓ Unbekannt</span>';"
        "}"
        "}\n"
    );
}

// ============================================================================
// JAVASCRIPT - Devices Functions
// ============================================================================

String WebUI::buildDevicesFunctions() {
    return String(
        // Load devices and status - SEQUENTIAL to avoid parallel overload
        "function loadDevices(){"
        "if(isLoading){console.log('Already loading, skip');return;}"
        "isLoading=true;"
        "fetch('/api/devices').then(r=>r.json()).then(d=>{"
        "devices=d.devices||[];"
        "knownDevices=d.knownDevices||[];"
        "filterDevices();"
        "renderKnownDevices();"
        "return fetch('/api/status');"
        "}).then(r=>r.json()).then(s=>{"
        "document.getElementById('scan-status').textContent=s.scanning?'Scannt...':'Pause';"
        "document.getElementById('output-status').textContent=s.outputActive?'AN':'AUS';"
        "document.getElementById('status-scan').className='status-item'+(s.scanning?' online':'');"
        "document.getElementById('status-output').className='status-item'+(s.outputActive?' online':'');"
        "const wifiStatus=s.wifi_connected?'WiFi: '+s.wifi_ssid:'WiFi: Nicht verbunden';"
        "document.getElementById('status-wifi').textContent=wifiStatus;"
        "if(document.getElementById('status-wifimode')){document.getElementById('status-wifimode').textContent='Modus: '+(s.wifi_mode||'---');}"
        "document.getElementById('status-wifi').className='status-item'+(s.wifi_connected?' online':'');"
        "document.getElementById('status-uptime').textContent='Uptime: '+s.uptime;"
        "document.getElementById('stat-ever').textContent=s.devices_ever||0;"
        "document.getElementById('stat-known').textContent=s.known||0;"
        "document.getElementById('stat-active').textContent=s.devices||0;"
        "document.getElementById('stat-present').textContent=s.present||0;"
        "loadOutputLog();"
        "}).catch(e=>{console.error(e);showNotification('Fehler beim Laden',true);}).finally(()=>{isLoading=false;});"
        "}\n"
        
        // Filter devices
        "function filterDevices(){"
        "filteredDevices=devices.filter(d=>d.active);"
        "renderDevices();"
        "}\n"
        
        // Render devices
        "function renderDevices(){"
        "const c=document.getElementById('devices-list');"
        "if(!filteredDevices.length){"
        "c.innerHTML='<div style=\"padding:40px;text-align:center;color:#8e8e93;\">Keine Geräte gefunden</div>';"
        "return;"
        "}"
        "c.innerHTML=filteredDevices.map(device=>{"
        "const lastSeen=device.lastSeenRelative||'nie';"
        "const manufacturerInfo=device.manufacturer||'Unbekannt';"
        "const comment=device.comment||'';"
        "const proximityClass=' proximity-'+(device.proximityStatus||'red');"
        "return '<div class=\"device'+(device.known?' known':'')+(device.active?' active':'')+proximityClass+'\">'"
        "+'<div class=\"device-row\"><div class=\"device-info\">'"
        "+'<div class=\"device-name\">'+(device.name||'Unbekanntes Gerät')+(comment?' ('+comment+')':'')+'</div>'"
        "+'<div class=\"device-details\">'"
        "+'<span>📱 '+device.address+'</span>'"
        "+'<span>📶 '+device.rssi+' dBm</span>'"
        "+'<span title=\"Payload: '+(device.payloadHex||'Keine Daten')+'\">🏭 '"
        "+(manufacturerInfo!=='Unbekannt'?manufacturerInfo:(device.payloadHex?'Raw: '+device.payloadHex.substring(0,8)+'...':'Unbekannt'))+'</span>'"
        "+(device.known?'<span>⭐ Bekannt (Schwelle: '+(device.rssiThreshold||-80)+' dBm)</span>':'')+"
        "getProximityStatusText(device.proximityStatus)+"
        "'</div>'"
        "+'<div class=\"device-meta\">Zuletzt gesehen: '+lastSeen+'</div>'"
        "+'</div>'"
        "+'<div class=\"device-actions\">'"
        "+'<button onclick=\"toggleKnown(\\''+device.address+'\\','+(device.known?'false':'true')+')\" class=\"'+(device.known?'danger':'success')+'\">'"
        "+(device.known?'❌ Entfernen':'⭐ Hinzufügen')+'</button>'"
        "+'</div></div></div>';"
        "}).join('');"
        "}\n"
    );
}

// ============================================================================
// JAVASCRIPT - Known Devices Functions
// ============================================================================

String WebUI::buildKnownDevicesFunctions() {
    return String(
        // Render known devices
        "function renderKnownDevices(){"
        "const c=document.getElementById('known-devices-list');"
        "if(!knownDevices.length){"
        "c.innerHTML='<div style=\"padding:40px;text-align:center;color:#8e8e93;\">Keine bekannten Geräte</div>';"
        "return;"
        "}"
        "c.innerHTML=knownDevices.map(device=>{"
        "const proximityClass=' proximity-'+(device.proximityStatus||'red');"
        "return '<div class=\"device known'+proximityClass+'\">'"
        "+'<div class=\"device-row\"><div class=\"device-info\">'"
        "+'<div class=\"device-name\">'+(device.name||'Unbekanntes Gerät')+' ('+(device.comment||'Kein Kommentar')+')</div>'"
        "+'<div class=\"device-details\">'"
        "+'<span>📱 '+device.address+'</span>'"
        "+(device.rssi?'<span>📶 '+device.rssi+' dBm</span>':'<span>📶 --- dBm</span>')+"
        "'<span>⚡ Schwelle: '+device.rssiThreshold+' dBm</span>'"
        "+getProximityStatusText(device.proximityStatus)+"
        "'</div></div>'"
        "+'<div class=\"device-actions\">'"
        "+'<button onclick=\"toggleKnown(\\''+device.address+'\\',false)\" class=\"danger\">❌ Entfernen</button>'"
        "+'</div></div></div>';"
        "}).join('');"
        "}\n"
        
        // Toggle known device
        "function toggleKnown(address,known){"
        "if(known){showDeviceDialog(address);}else{"
        "const params=new URLSearchParams({address:address,known:known});"
        "fetch('/api/device/known?'+params,{method:'POST'}).then(r=>r.json()).then(d=>{"
        "if(d.status==='success'){showNotification(d.message);loadDevices();}"
        "else{showNotification(d.message,true);}"
        "}).catch(e=>showNotification('Fehler beim Ändern des Status',true));"
        "}"
        "}\n"
        
        // Device dialog
        "function showDeviceDialog(address){"
        "const dialog=document.createElement('div');"
        "dialog.style.cssText='position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.5);z-index:1000;display:flex;align-items:center;justify-content:center';"
        "dialog.innerHTML='<div style=\"background:white;padding:20px;border-radius:12px;width:90%;max-width:400px;box-shadow:0 10px 30px rgba(0,0,0,0.3)\">'"
        "+'<h3 style=\"margin:0 0 15px\">Gerät hinzufügen</h3>'"
        "+'<p style=\"color:#666;margin-bottom:15px\">MAC: '+address+'</p>'"
        "+'<input type=\"text\" id=\"device-comment\" maxlength=\"32\" placeholder=\"Kommentar (z.B. iPhone von Max)\" style=\"width:100%;padding:10px;border:1px solid #ddd;border-radius:6px;margin-bottom:10px\">'"
        "+'<input type=\"number\" id=\"device-threshold\" value=\"-80\" placeholder=\"RSSI Schwellwert\" style=\"width:100%;padding:10px;border:1px solid #ddd;border-radius:6px;margin-bottom:15px\">'"
        "+'<div style=\"display:flex;gap:10px\">'"
        "+'<button onclick=\"addKnownDevice(\\''+address+'\\')\" style=\"flex:1;background:#007aff;color:white;border:none;padding:12px;border-radius:6px;cursor:pointer\">Hinzufügen</button>'"
        "+'<button onclick=\"closeDialog()\" style=\"flex:1;background:#8e8e93;color:white;border:none;padding:12px;border-radius:6px;cursor:pointer\">Abbrechen</button>'"
        "+'</div></div>';"
        "document.body.appendChild(dialog);"
        "document.getElementById('device-comment').focus();"
        "window.currentDialog=dialog;"
        "}\n"
        
        // Add known device
        "function addKnownDevice(address){"
        "const comment=document.getElementById('device-comment').value;"
        "const threshold=document.getElementById('device-threshold').value;"
        "if(!comment.trim()){showNotification('Bitte Kommentar eingeben',true);return;}"
        "const params=new URLSearchParams({address:address,known:true,comment:comment,rssiThreshold:threshold});"
        "fetch('/api/device/known?'+params,{method:'POST'}).then(r=>r.json()).then(d=>{"
        "if(d.status==='success'){showNotification(d.message);loadDevices();closeDialog();}"
        "else{showNotification(d.message,true);}"
        "}).catch(e=>showNotification('Fehler beim Hinzufügen',true));"
        "}\n"
        
        // Close dialog
        "function closeDialog(){"
        "if(window.currentDialog){document.body.removeChild(window.currentDialog);window.currentDialog=null;}"
        "}\n"
    );
}

// ============================================================================
// JAVASCRIPT - Output Log Functions
// ============================================================================

String WebUI::buildOutputLogFunctions() {
    return String(
        // Load output log
        "function loadOutputLog(){"
        "fetch('/api/output-log').then(r=>r.json()).then(d=>{"
        "renderOutputLog(d.outputLog||[]);"
        "}).catch(e=>console.error(e));"
        "}\n"
        
        // Render output log
        "function renderOutputLog(log){"
        "const c=document.getElementById('output-log-list');"
        "if(!log.length){"
        "c.innerHTML='<div style=\"padding:40px;text-align:center;color:#8e8e93;\">Keine Log-Einträge</div>';"
        "return;"
        "}"
        "c.innerHTML=log.map(entry=>{"
        "const stateIcon=entry.outputState?'🟢 AN':'🔴 AUS';"
        "const deviceInfo=entry.deviceName&&entry.deviceName!==''?entry.deviceName+' ('+entry.deviceAddress+')':entry.deviceAddress;"
        "const timeInfo=entry.timeAgo?'vor '+entry.timeAgo:'gerade eben';"
        "return '<div class=\"device\"><div class=\"device-row\"><div class=\"device-info\">'"
        "+'<div class=\"device-name\">'+timeInfo+' - '+stateIcon+'</div>'"
        "+'<div class=\"device-details\">'"
        "+'<span>📱 '+deviceInfo+'</span>'"
        "+'<span>📝 '+entry.reason+'</span>'"
        "+'</div></div></div></div>';"
        "}).join('');"
        "}\n"
        
        // Clear output log
        "function clearOutputLog(){"
        "if(!confirm('Log wirklich löschen?'))return;"
        "fetch('/api/output-log/clear',{method:'POST'}).then(r=>r.json()).then(d=>{"
        "if(d.status==='success'){showNotification('Log gelöscht');loadOutputLog();}"
        "else{showNotification('Fehler beim Löschen',true);}"
        "}).catch(e=>showNotification('Fehler beim Löschen',true));"
        "}\n"
    );
}

// ============================================================================
// JAVASCRIPT - Action Functions
// ============================================================================

String WebUI::buildActionFunctions() {
    return String(
        // Refresh data
        "function refreshData(){"
        "loadDevices();"
        "showNotification('Daten aktualisiert');"
        "}\n"
        
        // Export devices
        "function exportDevices(){"
        "window.location.href='/api/export-devices-file';"
        "showNotification('Export gestartet...');"
        "}\n"
        
        // Import devices
        "function importDevices(input){"
        "if(!input||!input.files||!input.files[0]){showNotification('Keine Datei ausgewählt',true);return;}"
        "const file=input.files[0];"
        "const reader=new FileReader();"
        "reader.onload=function(){"
        "try{"
        "const data=JSON.parse(reader.result);"
        "fetch('/api/import-devices',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(data)}).then(r=>r.json()).then(d=>{"
        "if(d.status==='success'){showNotification('Import erfolgreich');loadDevices();}"
        "else{showNotification('Import fehlgeschlagen',true);}"
        "});"
        "}catch(e){showNotification('Ungültige JSON',true);}"
        "};"
        "reader.readAsText(file);"
        "}\n"
        
        // Reset WiFi
        "function resetWiFi(){"
        "fetch('/api/factory-reset',{method:'POST'}).then(()=>showNotification('Factory Reset - Gerät wird offline geschaltet')).catch(()=>showNotification('Factory Reset fehlgeschlagen',true));"
        "}\n"
        
        // Reset Bluetooth
        "function resetBluetooth(){"
        "fetch('/api/bluetooth/reset',{method:'POST'}).then(()=>showNotification('Bluetooth Reset ausgeführt')).catch(()=>showNotification('Bluetooth Reset fehlgeschlagen',true));"
        "}\n"
        
        // Reset System
        "function resetSystem(){"
        "fetch('/api/system/reboot',{method:'POST'}).then(()=>showNotification('System wird neu gestartet')).catch(()=>showNotification('Neustart fehlgeschlagen',true));"
        "}\n"
    );
}

// ============================================================================
// JAVASCRIPT - Initialization
// ============================================================================

String WebUI::buildInitFunction() {
    return String(
        "window.onload=function(){"
        "document.getElementById('clear-log-btn').onclick=clearOutputLog;"
        "loadDevices();"
        "};\n"
    );
}

// ============================================================================
// MODE SETUP HTML - Styles (Beacon/WiFi/AP)
// ============================================================================

String WebUI::buildModeSetupStyles() {
    return String(
        "<style>"
        "body{font-family:Arial,sans-serif;margin:20px;background:#f5f5f5;}"
        ".container{max-width:600px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}"
        "h1{text-align:center;color:#333;margin-bottom:30px;}"
        ".tab-buttons{display:flex;margin-bottom:20px;gap:5px;}"
        ".tab-button{flex:1;padding:12px;background:#e9ecef;border:none;cursor:pointer;border-radius:5px;font-weight:bold;}"
        ".tab-button.active{background:#007bff;color:white;}"
        ".tab{display:none;}"
        ".tab.active{display:block;}"
        ".form-group{margin-bottom:20px;}"
        "label{display:block;margin-bottom:5px;font-weight:bold;}"
        "input,select{width:100%;padding:10px;border:1px solid #ddd;border-radius:5px;box-sizing:border-box;}"
        "button{width:100%;padding:12px;background:#007bff;color:white;border:none;border-radius:5px;cursor:pointer;font-size:16px;margin-top:10px;}"
        "button:hover{background:#0056b3;}"
        ".networks{max-height:200px;overflow-y:auto;border:1px solid #ddd;border-radius:5px;margin-bottom:10px;}"
        ".network{padding:10px;border-bottom:1px solid #eee;cursor:pointer;}"
        ".network:hover{background:#f8f9fa;}"
        ".network:last-child{border-bottom:none;}"
        "#message{margin-top:20px;padding:10px;border-radius:5px;display:none;}"
        ".success{background:#d4edda;color:#155724;border:1px solid #c3e6cb;}"
        ".error{background:#f8d7da;color:#721c24;border:1px solid #f5c6cb;}"
        "</style>"
    );
}

// ============================================================================
// MODE SETUP HTML - Header
// ============================================================================

String WebUI::buildModeSetupHeader() {
    return "<h1>ESP32 BT Scanner Setup</h1>";
}

// ============================================================================
// MODE SETUP HTML - Tab Buttons
// ============================================================================

String WebUI::buildTabButtons() {
    return String(
        "<div class='tab-buttons'>"
        "<button class='tab-button active' onclick='showTab(\"beacon\")'>Beacon</button>"
        "<button class='tab-button' onclick='showTab(\"wifi\")'>Scanner + WiFi</button>"
        "<button class='tab-button' onclick='showTab(\"ap\")'>Scanner + AP</button>"
        "</div>"
    );
}

// ============================================================================
// MODE SETUP HTML - Beacon Tab
// ============================================================================

String WebUI::buildBeaconTab(const String& beaconName, const BeaconConfig& beaconConfig) {
    // Determine display information
    bool isDefault = (beaconName == "BT-beacon" || beaconName.length() == 0);
    
    // Get BLE MAC for suffix
    uint8_t bleMac[6];
    esp_read_mac(bleMac, ESP_MAC_BT);
    char bleMacSuffix[5];
    snprintf(bleMacSuffix, sizeof(bleMacSuffix), "%02X%02X", bleMac[4], bleMac[5]);
    
    String displayHint = isDefault ? String("BT-beacon_") + String(bleMacSuffix) : beaconName;
    String fieldValue = isDefault ? "" : beaconName;
    
    String html = "<div id='beacon-tab' class='tab active'>";
    html += "<h3>Beacon-Modus</h3>";
    html += "<p>BLE-Beacon ohne WiFi (stromsparend)</p>";
    html += "<p style='color:#666;font-size:0.9em;'>Aktuell gespeichert: " + displayHint + "</p>";
    html += "<p style='color:#ff9500;font-size:0.9em;font-weight:bold;'>⚠️ Namensänderung: Stromlos-Neustart erforderlich!</p>";
    html += "<p style='color:#999;font-size:0.85em;'>Leer lassen = Auto-Name (BT-beacon_MAC)</p>";
    html += "<form onsubmit='return setupBeacon(event)'>";
    html += "<div class='form-group'><label>Name (optional):</label><input type='text' id='beacon-name' value='" + fieldValue + "' maxlength='20' placeholder='Leer = BT-beacon_MAC'></div>";
    html += "<div class='form-group'><label>Intervall (ms):</label><input type='number' id='beacon-interval' value='" + String(beaconConfig.intervalMs) + "' min='100' max='10000'></div>";
    html += "<div class='form-group'><label>TX Power:</label><select id='beacon-power'>";
    
    // TX Power options
    html += String("<option value='-12'") + (beaconConfig.txPower == -12 ? " selected" : "") + ">-12 dBm (2m)</option>";
    html += String("<option value='-9'") + (beaconConfig.txPower == -9 ? " selected" : "") + ">-9 dBm (3m)</option>";
    html += String("<option value='-6'") + (beaconConfig.txPower == -6 ? " selected" : "") + ">-6 dBm (5m)</option>";
    html += String("<option value='-3'") + (beaconConfig.txPower == -3 ? " selected" : "") + ">-3 dBm (7m)</option>";
    html += String("<option value='0'") + (beaconConfig.txPower == 0 ? " selected" : "") + ">0 dBm (10m)</option>";
    html += String("<option value='3'") + (beaconConfig.txPower == 3 ? " selected" : "") + ">3 dBm (15m)</option>";
    html += String("<option value='6'") + (beaconConfig.txPower == 6 ? " selected" : "") + ">6 dBm (20m)</option>";
    html += String("<option value='9'") + (beaconConfig.txPower == 9 ? " selected" : "") + ">9 dBm (30m)</option>";
    
    html += "</select></div>";
    html += "<button type='submit'>Beacon starten</button>";
    html += "</form>";
    html += "</div>";
    
    return html;
}

// ============================================================================
// MODE SETUP HTML - WiFi Tab
// ============================================================================

String WebUI::buildWiFiTab() {
    return String(
        "<div id='wifi-tab' class='tab'>"
        "<h3>Scanner + WLAN</h3>"
        "<p>Mit bestehendem WLAN verbinden</p>"
        "<div class='form-group'><label>Netzwerke:</label><div id='networks' class='networks'>Lade...</div></div>"
        "<form onsubmit='return setupWiFi(event)'>"
        "<div class='form-group'><label>SSID:</label><input type='text' id='wifi-ssid'></div>"
        "<div class='form-group'><label>Passwort:</label><input type='password' id='wifi-password'></div>"
        "<div class='form-group'><label><input type='checkbox' id='use-static-ip' onchange='toggleStaticIP()'> Statische IP</label></div>"
        "<div id='static-ip-fields' style='display:none;'>"
        "<div class='form-group'><label>IP:</label><input type='text' id='static-ip' placeholder='192.168.1.100'></div>"
        "<div class='form-group'><label>Gateway:</label><input type='text' id='gateway' placeholder='192.168.1.1'></div>"
        "<div class='form-group'><label>Subnet:</label><input type='text' id='subnet' value='255.255.255.0'></div>"
        "<div class='form-group'><label>DNS:</label><input type='text' id='dns' value='8.8.8.8'></div>"
        "</div>"
        "<button type='submit'>Verbinden</button>"
        "</form>"
        "</div>"
    );
}

// ============================================================================
// MODE SETUP HTML - AP Tab
// ============================================================================

String WebUI::buildAPTab() {
    return String(
        "<div id='ap-tab' class='tab'>"
        "<h3>Scanner + Sicherer AP</h3>"
        "<p>Eigenen Access Point erstellen</p>"
        "<form onsubmit='return setupAP(event)'>"
        "<div class='form-group'><label>AP-Passwort:</label><input type='password' id='ap-password' placeholder='Mind. 8 Zeichen' minlength='8'></div>"
        "<button type='submit'>AP erstellen</button>"
        "</form>"
        "</div>"
    );
}

// ============================================================================
// MODE SETUP HTML - Script
// ============================================================================

String WebUI::buildModeSetupScript() {
    return String(
        "<script>"
        "function showTab(tab){"
        "document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));"
        "document.querySelectorAll('.tab-button').forEach(b=>b.classList.remove('active'));"
        "document.getElementById(tab+'-tab').classList.add('active');"
        "event.target.classList.add('active');"
        "if(tab==='wifi')loadNetworks();"
        "}"
        "function showMessage(text,isError){"
        "const msg=document.getElementById('message');"
        "msg.textContent=text;"
        "msg.className=isError?'error':'success';"
        "msg.style.display='block';"
        "setTimeout(()=>msg.style.display='none',8000);"
        "}"
        "function toggleStaticIP(){"
        "document.getElementById('static-ip-fields').style.display="
        "document.getElementById('use-static-ip').checked?'block':'none';"
        "}"
        "function setupBeacon(e){"
        "e.preventDefault();"
        "const name=document.getElementById('beacon-name').value;"
        "const interval=document.getElementById('beacon-interval').value;"
        "const power=document.getElementById('beacon-power').value;"
        "const params=new URLSearchParams({name:name,interval:interval,power:power});"
        "fetch('/setup/beacon?'+params,{method:'POST'})"
        ".then(()=>{"
        "showMessage('Beacon wird gestartet...');"
        "setTimeout(()=>location.reload(),3000);"
        "})"
        ".catch(()=>{"
        "showMessage('Beacon wird gestartet...');"
        "setTimeout(()=>location.reload(),3000);"
        "});"
        "return false;"
        "}"
        "function setupWiFi(e){"
        "e.preventDefault();"
        "const ssid=document.getElementById('wifi-ssid').value;"
        "const password=document.getElementById('wifi-password').value;"
        "if(!ssid){showMessage('SSID eingeben',true);return false;}"
        "let body={ssid:ssid,password:password,scanner:true};"
        "if(document.getElementById('use-static-ip').checked){"
        "body.static_ip=document.getElementById('static-ip').value;"
        "body.gateway=document.getElementById('gateway').value;"
        "body.subnet=document.getElementById('subnet').value;"
        "body.dns=document.getElementById('dns').value;"
        "}"
        "fetch('/setup/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})"
        ".then(r=>r.json()).then(d=>{"
        "showMessage(d.message||'Verbinde...',d.status!=='success');"
        "if(d.status==='success')setTimeout(()=>location.reload(),5000);"
        "}).catch(()=>{"
        "showMessage('Verbinde mit WLAN...');"
        "setTimeout(()=>location.reload(),5000);"
        "});"
        "return false;"
        "}"
        "function setupAP(e){"
        "e.preventDefault();"
        "const password=document.getElementById('ap-password').value;"
        "if(password.length<8){showMessage('Mind. 8 Zeichen',true);return false;}"
        "fetch('/setup/ap',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({password:password,scanner:true})})"
        ".then(r=>r.json()).then(d=>{"
        "showMessage(d.message||'AP wird erstellt...',d.status!=='success');"
        "setTimeout(()=>location.reload(),5000);"
        "}).catch(()=>{"
        "showMessage('AP wird erstellt...');"
        "setTimeout(()=>location.reload(),5000);"
        "});"
        "return false;"
        "}"
        "function loadNetworks(){"
        "const container=document.getElementById('networks');"
        "container.innerHTML='Lade Netzwerke...';"
        "fetch('/api/scan').then(r=>r.json()).then(d=>{"
        "container.innerHTML='';"
        "d.networks.forEach(n=>{"
        "const div=document.createElement('div');"
        "div.className='network';"
        "div.innerHTML='<strong>'+n.ssid+'</strong> ('+n.rssi+' dBm) '+(n.auth?'🔒':'🔓');"
        "div.onclick=()=>document.getElementById('wifi-ssid').value=n.ssid;"
        "container.appendChild(div);"
        "});"
        "}).catch(()=>container.innerHTML='Fehler beim Laden');"
        "}"
        "</script>"
    );
}
