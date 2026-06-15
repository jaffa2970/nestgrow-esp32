# NestGrow Firmware 🌱
**by [lake8.dev](https://nestgrow.lake8.dev)**

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange.svg)](https://platformio.org)
[![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![MQTT](https://img.shields.io/badge/MQTT-PubSubClient-purple.svg)](https://github.com/knolleary/pubsubclient)

Firmware open source per **ESP32 WROOM-32** che gestisce culle di accrescimento vegetale automatizzate. Parte del progetto **NestGrow by lake8.dev**.

---

## Funzionalità

- **Captive portal** per configurazione WiFi da browser — nessuna modifica al codice necessaria
- **4 sensori umidità** suolo capacitivi (ADC1) con media mobile anti-rumore
- **Livello serbatoio** via galleggiante digitale
- **4 valvole/pompe** controllate via relè HW-316 (logica inversa)
- **Pubblicazione MQTT** con intervallo configurabile per zona in tempo reale
- **Sincronizzazione NTP** automatica (UTC+1 Italia, ora legale inclusa)
- **Heartbeat** ogni 60 secondi con stato sistema completo
- **Riconfigurazione remota** via MQTT senza accesso fisico
- **Safety timeout** valvole (5 minuti massimo)
- **Blocco automatico** se serbatoio vuoto
- **LED stato** sistema (AP / connessione / operativo / errore)
- **Comandi Serial Monitor** — `RESET` e `STATUS` da terminale seriale

---

## Hardware supportato

| Componente | Modello | Note |
|---|---|---|
| Microcontrollore | ESP32 WROOM-32 DevKit | Testato |
| Sensori umidità | Capacitive Soil Moisture V2.0 | Analogico 3.3V |
| Modulo relè | HW-316 4 canali | Logica inversa LOW=ON |
| Livello serbatoio | Float switch | Digitale, pullup esterno |
| Valvole | Elettrovalvole 12V | Via relè |
| Alimentazione | 12V 2A + step-down 5V | Per ESP32 e relè |

> Vedi [docs/WIRING.md](docs/WIRING.md) per lo schema di collegamento completo.

---

## Requisiti

- [VS Code](https://code.visualstudio.com/) + [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
- Driver **CP2102** o **CH340** (dipende dalla board)
- Git

---

## Quick Start

### 1. Clona il repository

```bash
git clone https://github.com/lake8dev/nestgrow-esp32.git
cd nestgrow-esp32
```

### 2. Apri in VS Code

```bash
code .
```

PlatformIO scarica automaticamente tutte le dipendenze alla prima compilazione.

### 3. Compila e flasha

Clicca **→ (Upload)** nella barra in basso di PlatformIO, oppure da terminale:

```bash
pio run --target upload --upload-port COM3
```

### 4. Configura il dispositivo

Al primo avvio il dispositivo entra in modalità AP:

1. Connetti al WiFi **`NestGrow-Setup-XXXX`** dal telefono o PC
2. Apri il browser su **`http://192.168.4.1`**
3. Compila: SSID, password WiFi, IP broker MQTT
4. Clicca **Salva e connetti** → il dispositivo si riavvia operativo

### 5. Verifica

Apri il Serial Monitor (115200 baud):

```bash
pio device monitor
```

Output atteso:

```
[NTP] Sincronizzazione...... OK
[NTP] Ora: 2026-05-08 19:32:13
[MQTT] Connected as nestgrow-a4b2
NestGrow v1.0.0 — Operativo
Device : nestgrow-a4b2
IP     : 192.168.1.45
Broker : 192.168.1.10:1883
```

---

## Topic MQTT

Sostituisci `{device_id}` con il nome del tuo dispositivo (es. `nestgrow-a4b2`).

| Topic | Direzione | Payload | Intervallo |
|---|---|---|---|
| `nestgrow/{device_id}/zona/{1-4}/umidita` | ESP32→Broker | `{"v":67.3,"ts":1714592400,"device_id":"..."}` | Configurabile |
| `nestgrow/{device_id}/serbatoio/livello` | ESP32→Broker | `{"v":100.0,"ts":1714592400}` | 30s fisso |
| `nestgrow/{device_id}/heartbeat` | ESP32→Broker | `{"uptime_sec":3600,"wifi_rssi":-65,...}` | 60s fisso |
| `nestgrow/{device_id}/zona/{1-4}/pompa` | Broker→ESP32 | `{"cmd":"on","sec":30}` / `{"cmd":"off"}` | On demand |
| `nestgrow/{device_id}/zona/{1-4}/pompa/stato` | ESP32→Broker | `{"state":"on","sec_remaining":25}` | On change |
| `nestgrow/{device_id}/cmd/config` | Broker→ESP32 | `{"zona":1,"intervallo_ms":15000,"salva_nvs":true}` | On demand |
| `nestgrow/{device_id}/cmd/config/ack` | ESP32→Broker | `{"zona":1,"intervallo_ms":15000,"salvato":true}` | On change |
| `nestgrow/{device_id}/cmd/reconfig` | Broker→ESP32 | `{"ssid":"...","password":"...","mqtt_host":"..."}` | On demand |

### Esempio: cambiare intervallo zona 1 a 10 secondi

```bash
mosquitto_pub -h localhost \
  -t "nestgrow/nestgrow-a4b2/cmd/config" \
  -m '{"zona":1,"intervallo_ms":10000,"salva_nvs":false}'
```

### Esempio: aprire valvola zona 2 per 30 secondi

```bash
mosquitto_pub -h localhost \
  -t "nestgrow/nestgrow-a4b2/zona/2/pompa" \
  -m '{"cmd":"on","sec":30}'
```

---

## Configurazione avanzata

### Calibrazione sensori umidità

In `src/config.h` modifica i valori raw ADC:

```c
#define SOIL_DRY  2800  // valore ADC con sensore in aria
#define SOIL_WET  1200  // valore ADC con sensore in acqua
```

Vedi [docs/CALIBRATION.md](docs/CALIBRATION.md) per la procedura completa.

### Comandi Serial Monitor

Con il Serial Monitor aperto a **115200 baud**, scrivi un comando e premi Invio:

| Comando | Effetto |
|---|---|
| `RESET` | Cancella tutta la NVS e riavvia — il dispositivo riparte come al primo boot |
| `STATUS` | Stampa device ID, IP, stato MQTT, NTP, intervalli zona (NVS vs RAM), heap libera |

Esempio output `STATUS`:

```
[CMD] Device  : nestgrow-a4b2
[CMD] WiFi    : 192.168.1.45
[CMD] MQTT    : connesso
[CMD] NTP     : 2026-05-08T19:32:13
[CMD] zona_interval (NVS): [30000, 30000, 30000, 30000] ms
[CMD] zona_interval (RAM): [10000, 30000, 30000, 30000] ms
[CMD] Free heap: 187432 bytes
[CMD] Uptime   : 142 s
```

> La colonna **NVS** mostra i valori che sopravvivono al riavvio; la colonna **RAM** mostra i valori correnti (modificabili via `cmd/config` senza `salva_nvs`).

### Modalità debug

In `src/config.h` decommentare:

```c
#define DEBUG_SENSORS
```

I sensori restituiranno valori simulati casuali — utile per testare senza hardware fisico.

### Pin mapping personalizzato

Tutti i pin sono definiti in `src/config.h` e modificabili senza toccare il codice applicativo.

---

## Struttura del progetto

```
nestgrow-esp32/
├── src/
│   ├── main.cpp           — Orchestrazione principale
│   ├── config.h           — Pin mapping e costanti
│   ├── nvs_config.cpp/h   — Configurazione persistente (NVS)
│   ├── wifi_manager.cpp/h — AP mode e captive portal
│   ├── mqtt_client.cpp/h  — Client MQTT
│   ├── ntp.cpp/h          — Sincronizzazione NTP
│   ├── sensors.cpp/h      — Lettura sensori ADC
│   ├── valves.cpp/h       — Controllo relè e safety
│   └── led.cpp/h          — Gestione LED stato
├── docs/
│   ├── WIRING.md          — Schema di collegamento
│   ├── CALIBRATION.md     — Calibrazione sensori
│   └── TROUBLESHOOTING.md — Risoluzione problemi
├── platformio.ini
├── CHANGELOG.md
├── CONTRIBUTING.md
└── LICENSE
```

---

## Dipendenze

| Libreria | Versione | Uso |
|---|---|---|
| `knolleary/PubSubClient` | ^2.8 | Client MQTT |
| `bblanchon/ArduinoJson` | ^7.0 | Parsing/serializzazione JSON |
| `esphome/ESPAsyncWebServer-esphome` | ^3.1.0 | Captive portal async |
| `esphome/AsyncTCP-esphome` | ^2.1.0 | TCP asincrono per ESP32 |

---

## Backend

Il backend NestGrow che riceve i dati MQTT, li archivia e li visualizza è disponibile (closed source) su **[nestgrow.lake8.dev](https://nestgrow.lake8.dev)**.

---

## Contribuire

Pull request benvenute! Vedi [CONTRIBUTING.md](CONTRIBUTING.md) per le linee guida.

---

## Licenza

**MIT License** — Copyright © 2026 lake8.dev

Vedi [LICENSE](LICENSE) per il testo completo.

---

## 🇬🇧 English

Open source firmware for **ESP32 WROOM-32** that manages automated plant-growth cradles. Part of the **NestGrow by lake8.dev** project.

---

## Features

- **Captive portal** for browser-based WiFi configuration — no code changes needed
- **4 capacitive soil moisture sensors** (ADC1) with anti-noise moving average
- **Tank level** via digital float switch
- **4 valves/pumps** controlled via HW-316 relay module (inverted logic)
- **MQTT publishing** with per-zone configurable interval in real time
- **Automatic NTP synchronisation** (UTC+1 Italy, DST included)
- **Heartbeat** every 60 seconds with full system status
- **Remote reconfiguration** via MQTT without physical access
- **Valve safety timeout** (5 minutes maximum)
- **Automatic lockout** when tank is empty
- **Status LED** (AP / connecting / operational / error)
- **Serial Monitor commands** — `RESET` and `STATUS` from the serial terminal

---

## Supported Hardware

| Component | Model | Notes |
|---|---|---|
| Microcontroller | ESP32 WROOM-32 DevKit | Tested |
| Moisture sensors | Capacitive Soil Moisture V2.0 | Analogue 3.3 V |
| Relay module | HW-316 4-channel | Inverted logic LOW=ON |
| Tank level | Float switch | Digital, external pull-up |
| Valves | 12 V solenoid valves | Via relay |
| Power supply | 12 V 2 A + 5 V step-down | For ESP32 and relay |

> See [docs/WIRING.md](docs/WIRING.md) for the full wiring diagram.

---

## Requirements

- [VS Code](https://code.visualstudio.com/) + [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
- **CP2102** or **CH340** driver (depends on the board)
- Git

---

## Quick Start

### 1. Clone the repository

```bash
git clone https://github.com/lake8dev/nestgrow-esp32.git
cd nestgrow-esp32
```

### 2. Open in VS Code

```bash
code .
```

PlatformIO automatically downloads all dependencies on the first build.

### 3. Build and flash

Click **→ (Upload)** in the PlatformIO bottom bar, or from the terminal:

```bash
pio run --target upload --upload-port COM3
```

### 4. Configure the device

On first boot the device enters AP mode:

1. Connect to the **`NestGrow-Setup-XXXX`** WiFi from your phone or PC
2. Open a browser at **`http://192.168.4.1`**
3. Fill in: SSID, WiFi password, MQTT broker IP
4. Click **Save and connect** → the device reboots into operational mode

### 5. Verify

Open the Serial Monitor (115200 baud):

```bash
pio device monitor
```

Expected output:

```
[NTP] Syncing...... OK
[NTP] Time: 2026-05-08 19:32:13
[MQTT] Connected as nestgrow-a4b2
NestGrow v1.0.0 — Operational
Device : nestgrow-a4b2
IP     : 192.168.1.45
Broker : 192.168.1.10:1883
```

---

## MQTT Topics

Replace `{device_id}` with your device name (e.g. `nestgrow-a4b2`).

| Topic | Direction | Payload | Interval |
|---|---|---|---|
| `nestgrow/{device_id}/zona/{1-4}/umidita` | ESP32→Broker | `{"v":67.3,"ts":1714592400,"device_id":"..."}` | Configurable |
| `nestgrow/{device_id}/serbatoio/livello` | ESP32→Broker | `{"v":100.0,"ts":1714592400}` | Fixed 30 s |
| `nestgrow/{device_id}/heartbeat` | ESP32→Broker | `{"uptime_sec":3600,"wifi_rssi":-65,...}` | Fixed 60 s |
| `nestgrow/{device_id}/zona/{1-4}/pompa` | Broker→ESP32 | `{"cmd":"on","sec":30}` / `{"cmd":"off"}` | On demand |
| `nestgrow/{device_id}/zona/{1-4}/pompa/stato` | ESP32→Broker | `{"state":"on","sec_remaining":25}` | On change |
| `nestgrow/{device_id}/cmd/config` | Broker→ESP32 | `{"zona":1,"intervallo_ms":15000,"salva_nvs":true}` | On demand |
| `nestgrow/{device_id}/cmd/config/ack` | ESP32→Broker | `{"zona":1,"intervallo_ms":15000,"salvato":true}` | On change |
| `nestgrow/{device_id}/cmd/reconfig` | Broker→ESP32 | `{"ssid":"...","password":"...","mqtt_host":"..."}` | On demand |

### Example: change zone 1 interval to 10 seconds

```bash
mosquitto_pub -h localhost \
  -t "nestgrow/nestgrow-a4b2/cmd/config" \
  -m '{"zona":1,"intervallo_ms":10000,"salva_nvs":false}'
```

### Example: open zone 2 valve for 30 seconds

```bash
mosquitto_pub -h localhost \
  -t "nestgrow/nestgrow-a4b2/zona/2/pompa" \
  -m '{"cmd":"on","sec":30}'
```

---

## Advanced Configuration

### Moisture sensor calibration

Edit the raw ADC values in `src/config.h`:

```c
#define SOIL_DRY  2800  // ADC value with sensor in air
#define SOIL_WET  1200  // ADC value with sensor in water
```

See [docs/CALIBRATION.md](docs/CALIBRATION.md) for the full procedure.

### Serial Monitor commands

With the Serial Monitor open at **115200 baud**, type a command and press Enter:

| Command | Effect |
|---|---|
| `RESET` | Clears all NVS and reboots — the device starts as on first boot |
| `STATUS` | Prints device ID, IP, MQTT status, NTP, zone intervals (NVS vs RAM), free heap |

Example `STATUS` output:

```
[CMD] Device  : nestgrow-a4b2
[CMD] WiFi    : 192.168.1.45
[CMD] MQTT    : connected
[CMD] NTP     : 2026-05-08T19:32:13
[CMD] zona_interval (NVS): [30000, 30000, 30000, 30000] ms
[CMD] zona_interval (RAM): [10000, 30000, 30000, 30000] ms
[CMD] Free heap: 187432 bytes
[CMD] Uptime   : 142 s
```

> The **NVS** column shows values that survive a reboot; the **RAM** column shows the current values (modifiable via `cmd/config` without `salva_nvs`).

### Debug mode

In `src/config.h` uncomment:

```c
#define DEBUG_SENSORS
```

Sensors will return random simulated values — useful for testing without physical hardware.

### Custom pin mapping

All pins are defined in `src/config.h` and can be changed without touching the application code.

---

## Project Structure

```
nestgrow-esp32/
├── src/
│   ├── main.cpp           — Main orchestration
│   ├── config.h           — Pin mapping and constants
│   ├── nvs_config.cpp/h   — Persistent configuration (NVS)
│   ├── wifi_manager.cpp/h — AP mode and captive portal
│   ├── mqtt_client.cpp/h  — MQTT client
│   ├── ntp.cpp/h          — NTP synchronisation
│   ├── sensors.cpp/h      — ADC sensor reading
│   ├── valves.cpp/h       — Relay control and safety
│   └── led.cpp/h          — Status LED management
├── docs/
│   ├── WIRING.md          — Wiring diagram
│   ├── CALIBRATION.md     — Sensor calibration
│   └── TROUBLESHOOTING.md — Troubleshooting guide
├── platformio.ini
├── CHANGELOG.md
├── CONTRIBUTING.md
└── LICENSE
```

---

## Dependencies

| Library | Version | Use |
|---|---|---|
| `knolleary/PubSubClient` | ^2.8 | MQTT client |
| `bblanchon/ArduinoJson` | ^7.0 | JSON parsing/serialisation |
| `esphome/ESPAsyncWebServer-esphome` | ^3.1.0 | Async captive portal |
| `esphome/AsyncTCP-esphome` | ^2.1.0 | Async TCP for ESP32 |

---

## Backend

The NestGrow backend that receives MQTT data, stores it, and visualises it is available (closed source) at **[nestgrow.lake8.dev](https://nestgrow.lake8.dev)**.

---

## Contributing

Pull requests welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for the guidelines.

---

## License

**MIT License** — Copyright © 2026 lake8.dev

See [LICENSE](LICENSE) for the full text.
