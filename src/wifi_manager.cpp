#include "wifi_manager.h"
#include "config.h"
#include "nvs_config.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

static bool           s_ap_mode         = false;
static bool           s_restart_req     = false;
static unsigned long  s_restart_at      = 0;
static char           s_device_name[32] = "";

static DNSServer      dns;
static AsyncWebServer server(80);

// ── Embedded portal HTML ──────────────────────────────────────────────────────
static const char PORTAL_HTML[] = R"rawhtml(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>NestGrow Setup</title>
<style>
*{box-sizing:border-box}
body{font-family:sans-serif;margin:0;padding:20px;background:#f0f7f4;color:#222}
h1{color:#2d6a4f;font-size:1.8em;margin-bottom:4px}
p{color:#555;margin-top:0;font-size:.95em}
label{display:block;margin-top:14px;font-weight:bold;font-size:1em}
input[type=text],input[type=password],input[type=number]{
  width:100%;padding:10px;font-size:1em;margin-top:4px;
  border:1px solid #b2d8c8;border-radius:6px;background:#fff}
.row{display:flex;gap:16px;margin-top:8px;align-items:center}
.row label{font-weight:normal;margin-top:0;cursor:pointer;display:flex;align-items:center;gap:6px}
#sf{display:none;background:#e8f5f0;padding:12px;border-radius:6px;margin-top:8px}
button{margin-top:24px;width:100%;padding:14px;background:#2d6a4f;color:#fff;
  font-size:1.1em;border:none;border-radius:8px;cursor:pointer}
button:active{background:#1b4332}
</style></head><body>
<h1>&#127807; NestGrow Setup</h1>
<p>Configura WiFi e MQTT, poi premi Salva.</p>
<form action="/save" method="POST">
<label>SSID WiFi</label>
<input type="text" name="ssid" placeholder="Nome rete WiFi" required>
<label>Password WiFi</label>
<input type="password" name="password" placeholder="(vuota = rete aperta)">
<label>Modalit&agrave; IP</label>
<div class="row">
  <label><input type="radio" name="ip_mode" value="dhcp" checked
    onchange="sf(this)"> DHCP (automatico)</label>
  <label><input type="radio" name="ip_mode" value="static"
    onchange="sf(this)"> IP Statico</label>
</div>
<div id="sf">
  <label>IP Statico</label>
  <input type="text" name="static_ip" placeholder="es. 192.168.1.100">
  <label>Gateway</label>
  <input type="text" name="gateway" placeholder="es. 192.168.1.1">
  <label>Subnet</label>
  <input type="text" name="subnet" value="255.255.255.0">
</div>
<label>MQTT Host</label>
<input type="text" name="mqtt_host" placeholder="IP o hostname del broker" required>
<label>MQTT Port</label>
<input type="number" name="mqtt_port" value="1883" min="1" max="65535">
<label>Nome Dispositivo</label>
<input type="text" name="device_name" value="%DN%">
<button type="submit">Salva e connetti</button>
</form>
<script>function sf(r){document.getElementById('sf').style.display=r.value==='static'?'block':'none';}</script>
</body></html>
)rawhtml";

static const char SAVED_HTML[] = R"rawhtml(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>NestGrow</title>
<style>body{font-family:sans-serif;text-align:center;padding:40px;background:#f0f7f4}
h2{color:#2d6a4f}p{color:#555;font-size:1.1em}</style>
</head><body>
<h2>&#9989; Configurazione salvata!</h2>
<p>Il dispositivo si riavvia e si connette alla rete...</p>
<p><small>Puoi chiudere questa pagina.</small></p>
</body></html>
)rawhtml";
// ─────────────────────────────────────────────────────────────────────────────

void wifi_start_ap(const NVSConfig &cfg) {
    s_ap_mode = true;
    strlcpy(s_device_name, cfg.device_name, sizeof(s_device_name));

    WiFi.mode(WIFI_AP);

    // SSID = "NestGrow-Setup-" + last 4 hex chars of device_name ("nestgrow-XXXX")
    char ap_ssid[32];
    const char *suffix = (strlen(s_device_name) >= 9) ? (s_device_name + 9) : s_device_name;
    snprintf(ap_ssid, sizeof(ap_ssid), "%s%s", AP_SSID_PREFIX, suffix);

    WiFi.softAP(ap_ssid);
    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));

    dns.start(53, "*", IPAddress(192,168,4,1));

    // GET / — captive portal form
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
        String html = PORTAL_HTML;
        html.replace("%DN%", s_device_name);
        req->send(200, "text/html", html);
    });

    // POST /save — persist config and reboot
    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *req) {
        NVSConfig nc;
        memset(&nc, 0, sizeof(nc));
        // Inizializza zona_interval al default — memset li azzererebbe a 0
        for (int i = 0; i < 4; i++) nc.zona_interval[i] = (int)DEFAULT_INTERVAL_MS;

        auto get = [&](const char *key, char *dst, size_t len) {
            if (req->hasParam(key, true))
                strlcpy(dst, req->getParam(key, true)->value().c_str(), len);
        };

        get("ssid",        nc.ssid,        sizeof(nc.ssid));
        get("password",    nc.password,    sizeof(nc.password));
        get("mqtt_host",   nc.mqtt_host,   sizeof(nc.mqtt_host));
        get("device_name", nc.device_name, sizeof(nc.device_name));
        get("static_ip",   nc.static_ip,   sizeof(nc.static_ip));
        get("gateway",     nc.gateway,     sizeof(nc.gateway));
        get("subnet",      nc.subnet,      sizeof(nc.subnet));

        nc.mqtt_port = req->hasParam("mqtt_port", true)
                     ? req->getParam("mqtt_port", true)->value().toInt()
                     : MQTT_PORT;

        nc.use_dhcp = !(req->hasParam("ip_mode", true) &&
                        req->getParam("ip_mode", true)->value() == "static");

        nc.configured = true;
        nvs_save(nc);

        req->send(200, "text/html", SAVED_HTML);
        s_restart_req = true;
        s_restart_at  = millis() + 2000UL;
    });

    // GET /reset — factory reset
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *req) {
        nvs_clear();
        req->send(200, "text/plain", "NVS cleared. Restarting...");
        s_restart_req = true;
        s_restart_at  = millis() + 1000UL;
    });

    // GET /status — simple JSON status
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *req) {
        char buf[128];
        snprintf(buf, sizeof(buf),
            "{\"mode\":\"ap\",\"ssid\":\"%s\",\"ip\":\"192.168.4.1\"}",
            WiFi.softAPSSID().c_str());
        req->send(200, "application/json", buf);
    });

    // Catch-all: redirect to captive portal (handles iOS/Android detection URLs)
    server.onNotFound([](AsyncWebServerRequest *req) {
        req->redirect("http://" AP_IP_STR "/");
    });

    server.begin();

    Serial.printf("[AP] SSID: %s  IP: %s\n", ap_ssid, AP_IP_STR);
}

bool wifi_connect_station(const NVSConfig &cfg) {
    s_ap_mode = false;
    WiFi.mode(WIFI_STA);

    if (!cfg.use_dhcp && strlen(cfg.static_ip) > 0) {
        IPAddress ip, gw, sn;
        if (ip.fromString(cfg.static_ip) &&
            gw.fromString(cfg.gateway)   &&
            sn.fromString(cfg.subnet)) {
            WiFi.config(ip, gw, sn);
        }
    }

    WiFi.begin(cfg.ssid, cfg.password);
    Serial.printf("[WiFi] Connecting to \"%s\"", cfg.ssid);

    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - t0 > 30000UL) {
            Serial.println("\n[WiFi] Timeout");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\n[WiFi] Connected — IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
}

void wifi_loop() {
    if (s_ap_mode) dns.processNextRequest();
    if (s_restart_req && millis() >= s_restart_at) {
        s_restart_req = false;
        ESP.restart();
    }
}

bool wifi_is_ap_mode() {
    return s_ap_mode;
}
