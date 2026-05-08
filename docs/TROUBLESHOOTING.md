# Risoluzione problemi

---

## ESP32 non appare su porta COM

**Windows:** Gestione dispositivi → Porte (COM e LPT)

| Sintomo | Soluzione |
|---|---|
| Nessuna porta COM presente | Installa driver **CP2102** o **CH340** (dipende dalla board) |
| Porta presente ma upload fallisce | Riduci `upload_speed = 115200` in `platformio.ini` |
| Porta presente ma scompare | Cavo USB difettoso o solo alimentazione (non dati) — prova un altro cavo |

---

## Captive portal non si apre automaticamente

1. Connetti **manualmente** al WiFi `NestGrow-Setup-XXXX`
2. Apri il browser e vai su **`http://192.168.4.1`** (scrivi esplicitamente `http://`)
3. Se ancora non funziona: **disabilita i dati mobili** sul telefono — alcuni Android bloccano reti senza accesso Internet

---

## LED indica errore dopo configurazione

Il LED lampeggia 3 volte veloci ogni 2s → `ERROR state`. Cause possibili:

| Causa | Diagnosi | Soluzione |
|---|---|---|
| Credenziali WiFi errate | Serial Monitor: `[WiFi] Timeout` | Reset config (vedi sotto) e reinserisci le credenziali |
| Broker MQTT non raggiungibile | Serial Monitor: `[MQTT] Connect failed` | Verifica IP broker e porta 1883 |
| Broker richiede autenticazione | `rc=-4` nel Serial Monitor | PubSubClient non è configurato per auth — apri una Issue |

---

## Reset configurazione completo

**Metodo 1 — Via web (se in AP mode):**

```
http://192.168.4.1/reset
```

**Metodo 2 — Via MQTT (se connesso):**

```bash
mosquitto_pub -h <broker_ip> \
  -t "nestgrow/<device_id>/cmd/reconfig" \
  -m '{"ssid":"new_ssid","password":"new_pass","mqtt_host":"192.168.1.10"}'
```

**Metodo 3 — Tieni premuto BOOT (GPIO 0) per 5 secondi** durante l'avvio
(non implementato nel firmware, da aggiungere in una future release)

Dopo il reset il dispositivo riappare come `NestGrow-Setup-XXXX`.

---

## Umidità sempre 0% o 100%

- **Sempre 0%**: `SOIL_DRY` troppo basso o sensore non collegato → verifica i pin e la calibrazione
- **Sempre 100%**: `SOIL_WET` troppo alto → ricalibra con il sensore in acqua
- Verifica che `SOIL_DRY > SOIL_WET` in `src/config.h`
- Vedi [CALIBRATION.md](CALIBRATION.md) per la procedura completa

---

## Valvola non si apre/chiude

1. Verifica che l'alimentazione 12V arrivi alle elettrovalvole (misura con tester)
2. Verifica che il **jumper JD-VCC sia rimosso** dal modulo relè HW-316 e che 12V sia collegato a JD-VCC
3. La logica è **inversa**: `LOW = ON`, `HIGH = OFF` — verificare che il firmware usi `LOW` per aprire
4. Testa manualmente via MQTT:
   ```bash
   mosquitto_pub -h <broker_ip> \
     -t "nestgrow/<device_id>/zona/1/pompa" \
     -m '{"cmd":"on","sec":5}'
   ```
5. Senti/vedi il relè cliccare? Se sì, il problema è nell'elettrovalvola o nell'alimentazione

---

## Timestamp errato (valori piccoli tipo 100-500)

NTP non sincronizzato — il firmware usa `millis()/1000` come fallback (secondi dall'avvio).

Cause:
- Il router non ha accesso a Internet
- Firewall blocca UDP porta 123 (NTP)

Verifica dal Serial Monitor:
```
[NTP] Sincronizzazione............. FALLITO - uso uptime
```

Soluzione:
- Verifica che `pool.ntp.org` sia raggiungibile dalla rete del router
- Il firmware riprova al prossimo avvio; nel frattempo usa il timestamp di uptime

---

## MQTT: messaggi non ricevuti

- Verifica che il dispositivo sia iscritto al topic corretto: il `device_id` è nel Serial Monitor all'avvio
- `device_id` formato: `nestgrow-XXXX` dove XXXX sono le ultime 4 cifre esadecimali del MAC
- Usa `mosquitto_sub -v -h <broker> -t "nestgrow/#"` per monitorare tutti i messaggi

---

## Dispositivo si riavvia continuamente (bootloop)

Cause comuni:
- NVS corrotta: esegui un reset via web o MQTT
- Firmware corrotto: riflasha con `pio run --target upload`
- Problema hardware: verifica alimentazione stabile 3.3V sull'ESP32

---

## Compilazione fallisce

```bash
# Pulisci la cache di build
pio run --target clean

# Ricompila da zero
pio run
```

Se il problema persiste, verifica che le versioni delle librerie in `platformio.ini` siano compatibili con `espressif32@5.3.0`.
