# Calibrazione sensori umidità

---

## Perché calibrare

I sensori capacitivi V2.0 hanno piccole variazioni tra un'unità e l'altra dovute alle tolleranze produttive. La calibrazione garantisce letture accurate in percentuale per il tuo specifico set di sensori.

La formula di conversione usata dal firmware è:

```
pct = (SOIL_DRY - raw_adc) / (SOIL_DRY - SOIL_WET) × 100
```

dove `raw_adc` è il valore grezzo a 12 bit (0–4095) letto dall'ADC.

---

## Procedura di calibrazione

### Step 1 — Abilita la lettura raw sul Serial Monitor

Modifica temporaneamente `src/sensors.cpp`, nella funzione `sensors_read_soil`, aggiungendo:

```cpp
Serial.printf("[CAL] Zona %d raw ADC: %d\n", zona, avg);
```

subito prima del `return pct;`.

Oppure abilita `#define DEBUG_SENSORS` in `src/config.h` e usa un broker MQTT per vedere i valori simulati — ma per la calibrazione servono i valori reali.

### Step 2 — Misura in aria (SOIL_DRY)

1. Collega il sensore all'ESP32 ma **lascialo in aria**, completamente asciutto
2. Flasha il firmware e apri il Serial Monitor (115200 baud)
3. Annota il valore raw ADC che appare → questo è il tuo **SOIL_DRY**

### Step 3 — Misura in acqua (SOIL_WET)

1. Immergi **solo la parte sensibile** del sensore in acqua distillata
2. Annota il valore raw ADC → questo è il tuo **SOIL_WET**

### Step 4 — Aggiorna `src/config.h`

```c
#define SOIL_DRY  2800  // ← sostituisci con il valore misurato in aria
#define SOIL_WET  1200  // ← sostituisci con il valore misurato in acqua
```

Verifica sempre che `SOIL_DRY > SOIL_WET`. Se è il contrario, il sensore è collegato al contrario o danneggiato.

### Step 5 — Rimuovi la riga di debug, ricompila e riflasha

---

## Valori di riferimento tipici — Capacitive V2.0

| Condizione suolo | Valore ADC tipico | Percentuale |
|---|---|---|
| In aria (secco) | 2700 – 2900 | 0% |
| Terreno molto secco | 2300 – 2600 | 10 – 25% |
| Terreno asciutto | 2000 – 2300 | 25 – 45% |
| Terreno umido | 1500 – 2000 | 45 – 70% |
| Terreno bagnato | 1200 – 1500 | 70 – 90% |
| Terreno saturo / in acqua | 1000 – 1200 | 90 – 100% |

I valori possono variare di ±200 tra sensori diversi dello stesso lotto.

---

## Calibrazione per zona indipendente

Se i sensori sono molto diversi tra loro, puoi definire valori separati per zona modificando `src/sensors.cpp`:

```cpp
static const int SOIL_DRY_PER_ZONE[4] = {2800, 2750, 2820, 2790};
static const int SOIL_WET_PER_ZONE[4] = {1200, 1180, 1220, 1195};
```

e adattando la formula nella funzione `sensors_read_soil`.

---

## Verifica calibrazione

Dopo la calibrazione, la lettura MQTT sul topic `nestgrow/{device_id}/zona/1/umidita` deve restituire:

- **~0%** con sensore in aria
- **~100%** con sensore completamente in acqua
- **40-60%** in terreno ben innaffiato
