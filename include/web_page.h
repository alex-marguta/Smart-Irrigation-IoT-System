#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char *MAIN_PAGE =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Connection: close\r\n"
    "\r\n"

    "<!DOCTYPE html>"
    "<html>"
    "<head>"

    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"

    "<style>"

    "body{"
    "background:#111;"
    "color:white;"
    "font-family:Arial;"
    "text-align:center;"
    "padding:25px;"
    "}"

    ".card{"
    "background:#1e1e1e;"
    "margin:auto;"
    "padding:25px;"
    "border-radius:22px;"
    "max-width:420px;"
    "}"

    "h1{font-size:30px;margin-bottom:25px;color:#ffffff;}"
    "h2{font-size:22px;margin:14px 0;}"

    ".muted{"
    "color:#bbbbbb;"
    "font-size:17px;"
    "line-height:1.5;"
    "}"

    ".box{"
    "background:#2a2a2a;"
    "border-radius:14px;"
    "padding:12px;"
    "margin:12px 0;"
    "}"

    ".on{color:#22dd66;font-weight:bold;}"
    ".off{color:#ff4444;font-weight:bold;}"

    ".bluebtn{"
    "width:240px;"
    "height:78px;"
    "font-size:25px;"
    "border:none;"
    "border-radius:18px;"
    "background:#007ACC;"
    "color:white;"
    "font-weight:bold;"
    "box-shadow:0 8px 0 #004f86;"
    "transition:all 0.12s ease;"
    "}"

    ".bluebtn:active{"
    "background:#777;"
    "transform:translateY(6px) scale(0.97);"
    "box-shadow:0 2px 0 #444;"
    "}"

    "input[type=range]{"
    "width:90%%;"
    "accent-color:#007ACC;"
    "}"

    "</style>"

    "<script>"

    "let watering=false;"

    "function syncTime(){"
    "let d=new Date();"
    "let t=d.toLocaleString('ro-RO');"
    "fetch('/time?now='+encodeURIComponent(t));"
    "}"

    "function udaAcum(){"
    "if(watering)return;"
    "watering=true;"
    "let btn=document.getElementById('waterBtn');"
    "btn.innerText='UDA...';"
    "let d=new Date();"
    "let t=d.toLocaleString('ro-RO');"
    "fetch('/water?time='+encodeURIComponent(t)).then(()=>{"
    "setTimeout(()=>{"
    "watering=false;"
    "btn.innerText='UDA ACUM';"
    "updateStatus();"
    "},3200);"
    "});"
    "}"

    "function setLimit(v){"
    "document.getElementById('limitValue').innerText=v+'%%';"
    "fetch('/setlimit?value='+v);"
    "}"

    "function updateStatus(){"
    "fetch('/status').then(r=>r.json()).then(data=>{"
    "document.getElementById('last').innerText=data.last;"
    "document.getElementById('soil').innerText=data.soil+'%%';"
    "document.getElementById('limitValue').innerText=data.limit+'%%';"
    "document.getElementById('slider').value=data.limit;"
    "let p=document.getElementById('pump');"
    "p.innerText=data.pump;"
    "p.className=data.pump=='ON'?'on':'off';"
    "});"
    "}"

    "setInterval(syncTime,1000);"
    "setInterval(updateStatus,1000);"

    "window.onload=function(){"
    "syncTime();"
    "updateStatus();"
    "};"

    "</script>"

    "</head>"

    "<body>"
    "<div class='card'>"

    "<h1>IRIGARE SMART</h1>"

    "<div class='box'>"
    "<h2>Ultima udare</h2>"
    "<h2 id='last'>%s</h2>"
    "</div>"

    "<div class='box'>"
    "<h2>Nivel umiditate: <span id='soil'>%d%%</span></h2>"
    "</div>"

    "<div class='box'>"
    "<h2>Udare automata sub: <span id='limitValue'>%d%%</span></h2>"
    "<input id='slider' type='range' min='0' max='100' value='%d' oninput='setLimit(this.value)'>"
    "</div>"

    "<p class='muted'>Planta se uda automat cand umiditatea scade sub valoarea aleasa.</p>"

    "<br>"

    "<button id='waterBtn' class='bluebtn' onclick='udaAcum()'>UDA ACUM</button>"

    "<p class='muted'>Daca apesi butonul, pompa porneste pentru 3 secunde.</p>"

    "<h2>Pompa: <span id='pump' class='%s'>%s</span></h2>"

    "</div>"
    "</body>"
    "</html>";

#endif