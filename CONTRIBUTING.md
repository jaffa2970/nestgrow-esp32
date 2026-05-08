# Come contribuire a NestGrow ESP32

NestGrow ESP32 firmware è open source (licenza MIT). Contributi di ogni tipo sono benvenuti!

---

## Cosa puoi fare

- Aggiungere supporto per nuovi tipi di sensori
- Implementare aggiornamenti OTA (Over The Air)
- Ottimizzare il consumo energetico (deep sleep tra le letture)
- Migliorare la stabilità della connessione MQTT/WiFi
- Aggiungere supporto per sensori I2C o SPI
- Tradurre il captive portal in altre lingue
- Scrivere test automatici
- Migliorare la documentazione

---

## Come inviare una Pull Request

1. **Fork** del repository su GitHub
2. **Crea un branch** con nome descrittivo:
   ```bash
   git checkout -b feature/ota-updates
   # oppure
   git checkout -b fix/mqtt-reconnect
   ```
3. **Modifica il codice** seguendo le linee guida sotto
4. **Testa su hardware fisico** ESP32 WROOM-32 (o almeno con `#define DEBUG_SENSORS`)
5. **Commit** con messaggio descrittivo:
   ```bash
   git commit -m "feat: add OTA update support via ArduinoOTA"
   ```
6. **Push** e apri una **Pull Request** su GitHub

---

## Linee guida per il codice

- Usa `millis()` al posto di `delay()` nel `loop()` principale
- Ogni modulo ha il suo `.h` / `.cpp` separato
- Costanti e pin mapping in `src/config.h`, non hardcoded nel codice
- Nessun `Serial.print` di debug nel codice finale (usa `#ifdef DEBUG_SENSORS`)
- Testa con `#define DEBUG_SENSORS` prima di collegare hardware fisico
- Mantieni la compatibilità con `espressif32@5.3.0` e le librerie in `platformio.ini`

---

## Segnalare un bug

Apri una [Issue su GitHub](https://github.com/lake8dev/nestgrow-esp32/issues) includendo:

- **Descrizione** del problema
- **Output del Serial Monitor** (115200 baud)
- **Versione hardware**: board, modello sensori, modulo relè
- **Contenuto di `platformio.ini`** se modificato
- **Passi per riprodurre** il problema

---

## Struttura dei commit

Usa il formato [Conventional Commits](https://www.conventionalcommits.org/):

| Prefisso | Uso |
|---|---|
| `feat:` | Nuova funzionalità |
| `fix:` | Correzione bug |
| `docs:` | Solo documentazione |
| `refactor:` | Refactoring senza cambi funzionali |
| `chore:` | Manutenzione (dipendenze, CI, ecc.) |

---

## Contatti

- **Progetto**: [nestgrow.lake8.dev](https://nestgrow.lake8.dev)
- **Issues**: [github.com/lake8dev/nestgrow-esp32/issues](https://github.com/lake8dev/nestgrow-esp32/issues)
