# Schema di collegamento — NestGrow ESP32

---

## Componenti necessari

| Componente | Quantità | Note |
|---|---|---|
| ESP32 WROOM-32 DevKit (38 pin) | 1 | Board principale |
| Capacitive Soil Moisture Sensor V2.0 | 4 | Sensori umidità suolo |
| Modulo relè 4 canali HW-316 | 1 | Controllo valvole |
| Float switch (galleggiante) | 1 | Livello serbatoio |
| Elettrovalvole 12V | 4 | O pompe sommerse 12V |
| Alimentatore 12V 2A (minimo) | 1 | Per valvole e step-down |
| Step-down DC-DC 12V→5V (LM2596) | 1 | Per ESP32 |
| Resistenza 10kΩ | 1 | Pull-up per galleggiante |
| Jumper wire, morsetti a vite | — | Cablaggio |

---

## Schema pin ESP32 WROOM-32

| GPIO | Funzione | Collegato a | Note |
|---|---|---|---|
| **25** | OUTPUT | IN0 modulo relè | Valvola zona 1 — logica inversa |
| **26** | OUTPUT | IN1 modulo relè | Valvola zona 2 — logica inversa |
| **27** | OUTPUT | IN2 modulo relè | Valvola zona 3 — logica inversa |
| **14** | OUTPUT | IN3 modulo relè | Valvola zona 4 — logica inversa |
| **32** | ADC1 CH4 | AOUT sensore 1 | Umidità zona 1 |
| **33** | ADC1 CH5 | AOUT sensore 2 | Umidità zona 2 |
| **34** | ADC1 CH6 | AOUT sensore 3 | Umidità zona 3 — input-only |
| **35** | ADC1 CH7 | AOUT sensore 4 | Umidità zona 4 — input-only |
| **36** | ADC1 CH0 | Galleggiante | Input-only — pullup 10kΩ esterno |
| **2** | OUTPUT | LED built-in | Stato sistema (integrato) |
| **3.3V** | PWR | VCC sensori | Tutti i sensori umidità V2.0 |
| **GND** | GND | GND comune | Tutti i componenti |

---

## Schema alimentazione

```
Alimentatore 12V 2A
├── 12V ──────────────────────────────────→ JD-VCC modulo relè (★ vedi nota)
├── 12V ──────────────────────────────────→ COM elettrovalvole (comune)
├── 12V → Step-down IN+                   Step-down IN- → GND
│         Step-down OUT 5V → ESP32 5V pin
└── GND ──────────────────────────────────→ GND comune (ESP32 + relè + sensori)

Segnali di controllo (3.3V logica ESP32):
  GPIO 25-27, 14 ──→ IN0-IN3 modulo relè
  VCC (3.3V ESP32) ──→ VCC modulo relè (logica)
```

---

## Collegamento sensori umidità V2.0

Ogni sensore ha 3 pin:

| Pin sensore | Collegato a | Nota |
|---|---|---|
| VCC | ESP32 3.3V | NON 5V |
| GND | GND comune | — |
| AOUT | GPIO 32/33/34/35 | Uno per zona |

---

## Collegamento galleggiante serbatoio

```
ESP32 3.3V ──→ resistenza 10kΩ ──→ GPIO 36
                                       │
                                    galleggiante
                                       │
                                      GND

Logica:
  GPIO 36 HIGH (3.3V) = galleggiante APERTO = serbatoio PIENO
  GPIO 36 LOW  (0V)   = galleggiante CHIUSO = serbatoio VUOTO
```

---

## Collegamento modulo relè HW-316

| Pin relè | Collegato a | Note |
|---|---|---|
| **JD-VCC** | 12V alimentatore | ★ Rimuovere jumper JD-VCC |
| **VCC** | ESP32 3.3V | Logica relè |
| **GND** | GND comune | — |
| **IN0** | GPIO 25 | Zona 1 |
| **IN1** | GPIO 26 | Zona 2 |
| **IN2** | GPIO 27 | Zona 3 |
| **IN3** | GPIO 14 | Zona 4 |
| **COM** | 12V alimentatore | Comune elettrovalvole |
| **NO** | Elettrovalvola+ | Normally Open (aperto a riposo) |

---

## Note importanti

### ★ Jumper JD-VCC — OBBLIGATORIO rimuovere

Sul modulo HW-316 c'è un jumper che collega **JD-VCC** a **VCC**.

**Rimuovere questo jumper** per separare l'alimentazione dei relè (12V) dalla logica (3.3V ESP32). Poi collegare:
- `JD-VCC` → **12V** dall'alimentatore (alimenta le bobine dei relè)
- `VCC` → **3.3V** da ESP32 (alimenta i transistor di pilotaggio)

Se non si rimuove il jumper i transistor ricevono 12V sulla base e si danneggiano.

### ADC2 non utilizzabile con WiFi attivo

Su ESP32 i pin **ADC2** (GPIO 0, 2, 4, 12, 13, 15, 25, 26, 27) non funzionano quando il WiFi è attivo. NestGrow usa esclusivamente **ADC1** (GPIO 32-39), sempre disponibili.

### GPIO 34-39 sono input-only

Non hanno resistenza di pull-up interna. Per il galleggiante (GPIO 36) è **obbligatoria** una resistenza pull-up esterna da **10kΩ** verso 3.3V.

### Tensione logica 3.3V

L'ESP32 opera a **3.3V**. Non collegare mai 5V ai GPIO — rischio di danno permanente. I sensori Capacitive V2.0 funzionano correttamente a 3.3V.
