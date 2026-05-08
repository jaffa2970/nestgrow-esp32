# Changelog

Tutte le modifiche rilevanti al progetto sono documentate in questo file.
Formato basato su [Keep a Changelog](https://keepachangelog.com/it/1.0.0/).

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
