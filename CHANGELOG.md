# Changelog

Tutte le modifiche rilevanti al progetto sono documentate in questo file.
Formato basato su [Keep a Changelog](https://keepachangelog.com/it/1.0.0/).

---

## [1.1.0] — 2026-05-08

### Aggiunto

- **Comandi Serial Monitor** — invia `RESET` o `STATUS` a 115200 baud:
  - `RESET` → cancella tutta la NVS e riavvia (primo boot)
  - `STATUS` → stampa device, WiFi, MQTT, NTP, intervalli zona (NVS vs RAM), heap libera, uptime

### Corretto

- **Bug captive portal NVS** — il POST `/save` azzerava `zona_interval[4]` con `memset` prima di chiamare `nvs_save`. Questo scriveva `0 ms` per tutte le zone in NVS, sovrascrivendo qualsiasi intervallo configurato via MQTT. Fix: inizializzazione a `DEFAULT_INTERVAL_MS` dopo `memset`.
- **Debug NVS** — aggiunto read-back immediato dopo ogni `nvs_save` degli intervalli zona e print dei valori letti all'avvio per diagnostica.

---

## [1.0.0] — 2026-05-08

### Aggiunto

- **Captive portal** per configurazione WiFi/MQTT da browser (nessun codice da modificare)
- **4 sensori umidità** suolo capacitivi (ADC1) con media mobile su 5 campioni
- **Livello serbatoio** via galleggiante digitale con blocco automatico se vuoto
- **4 valvole/pompe** controllate via relè HW-316 (logica inversa LOW=ON)
- **Pubblicazione MQTT** umidità per zona con intervallo configurabile in tempo reale
- **Salvataggio intervalli** in NVS (sopravvive al riavvio)
- **Sincronizzazione NTP** automatica — UTC+1 Italia con ora legale
- **Heartbeat MQTT** ogni 60 secondi con uptime, RSSI, heap libera, NTP status, intervalli zona
- **Safety timeout** valvole — chiusura forzata dopo 5 minuti
- **Riconfigurazione remota** via MQTT (`cmd/reconfig`) senza accesso fisico
- **Cambio intervallo per zona** via MQTT (`cmd/config`) con ACK di conferma
- **LED stato** — AP mode / connessione / operativo / errore
- **Modalità debug** con sensori simulati (`#define DEBUG_SENSORS`)
- **Reset configurazione** via web (`/reset`) o MQTT
- Documentazione completa: WIRING, CALIBRATION, TROUBLESHOOTING
