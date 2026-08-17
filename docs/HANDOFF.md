# Wood boiler controller — handoff / context

PlatformIO project for **ESP32-WROOM** (`board = esp32dev`, `espressif32` / Arduino).  
Path: `/home/dsporynkhin/Projects/cpp/home/boilercontroller`

Use this file when starting a new chat (`@docs/HANDOFF.md`). Documented behavior is **as implemented today**.

## Domain glossary

| Domain | Code symbols |
|--------|----------------|
| Blowing / bottom air door | `upper_door_*`, `UPPER_SERVO_PIN` |
| Afterburning / oxygen door | `oxygen_door_*`, `OXYGEN_SERVO_PIN` |
| Smoke / chimney door | `smoke_pipe_*`, `SMOKE_SERVO_PIN` |
| Fuel-loading hatch sensor | `MAIN_DOOR_SENSOR_PIN`, `main_door_opened` |
| Boiler core / out / in | `T_SENS_INDEX_CORE` / `OUTPUT_FLOW` / `INPUT_FLOW` |
| Thermal accumulator | top, middle_hi, middle_low, bottom |
| House heating supply / return | forward / backward flow |
| Stored energy | `accumulated_energy_kwt_hour` |
| Instant / smoothed core power | `current_core_power`, `core_EMA_power`, `core_DEMA_power` |
| Power balance | `power_balance_kwt_hour` (rate of change of stored energy) |

Primary goal: drive the three damper servos to burn safely (avoid overheating) and estimate accumulated thermal energy.

## Hardware (current)

Pins from [`src/Defines.h`](../src/Defines.h):

| Function | Macro | GPIO |
|----------|--------|------|
| Smoke servo | `SMOKE_SERVO_PIN` | 25 |
| Oxygen servo | `OXYGEN_SERVO_PIN` | 26 |
| Upper (blowing) servo | `UPPER_SERVO_PIN` | 27 |
| Emergency valve | `EMERGENCY_VALVE_PIN` | 0 |
| Pump 1–4 | `PUMP_1_PIN`…`PUMP_4_PIN` | 4, 13, 33, 32 |
| OneWire bus 1 / 2 | `ONE_WIRE_PIN`, `ONE_WIRE_PIN_2` | 14, **32** |
| Flow sensor | `FLOW_SENSOR_PIN` | 39 |
| TFT CS / DC / RST | `DISPLAY_*` | 17 / 15 / 16 |
| Touch CS / PENIRQ | `TOUCH_CS`, `TOUCH_PEN` | 12 / 36 |
| Fuel hatch sensor | `MAIN_DOOR_SENSOR_PIN` | 34 |
| External WDT kick | (hardcoded in `main.cpp`) | 5 |
| I2C | `Wire.begin(21, 22)` | 21 / 22 |

- **MCU:** ESP32-WROOM (`esp32dev`)
- **Display:** custom local `lib/ILI9488` + XPT2046 touch (`TouchDisplayController`), rotation 1; Adafruit GFX / BusIO via `lib_deps`
- **Temps:** DS18x20 on two OneWire buses (`SensorController` + OneWire / DallasTemperature `lib_deps`); addresses in `GlobalSettings.ds18D20Addresses[]`
- **Doors:** three LEDC PWM servos (`Servo` / `ServoController` / `DoorsController`); all inverted; ~60 °/s rate limit
- **Pumps:** pumps 1–2 constructed in control path; 3–4 macros exist but are unused
- **FS:** **LittleFS** for web/`data/` assets (`board_build.filesystem = littlefs`)
- **Flash partitions:** custom [`partitions_4mb_ota_512fs.csv`](../partitions_4mb_ota_512fs.csv) — app0/app1 **1728 KB** each, LittleFS (`spiffs` subtype) **512 KB** (wired via `board_build.partitions` in `platformio.ini`). First cutover after changing this CSV needs a full reflash (`upload` + `uploadfs`), not app-only OTA.
- **Web UI:** vanilla JS (`fetch`) in `data/index.html` / `data/settings.html` — **no jQuery**. TFT BMP icons remain under `data/img/`.
- **Settings:** EEPROM 4096 bytes, marker `0x34 0x32 0x33 0x31`, `GLOBAL_CURRENT_SETTINGS_VERSION` = 2

**Pin clash:** `ONE_WIRE_PIN_2` and `PUMP_4_PIN` both use GPIO **32**.

## Temperature sensors

| Index | `#define` | Settings path (approx.) | Meaning |
|------:|-----------|-------------------------|---------|
| 0 | `T_SENS_INDEX_CORE` | `sensors>heater>core` | Boiler core |
| 1 | `T_SENS_INDEX_OUTPUT_FLOW` | `sensors>heater>output_flow` | Boiler output |
| 2 | `T_SENS_INDEX_INPUT_FLOW` | `sensors>heater>input_flow` | Boiler return/input |
| 3 | `T_SENS_INDEX_ACC_TOP` | `sensors>termoaccumulator>top` | Acc top |
| 4 | `T_SENS_INDEX_ACC_MID_HI` | `…>middle_hi` | Acc above middle |
| 5 | `T_SENS_INDEX_ACC_MID_LO` | `…>middle_low` | Acc below middle |
| 6 | `T_SENS_INDEX_ACC_BOTTOM` | `…>bottom` | Acc bottom |
| 7 | `T_SENS_INDEX_FORWARD_FLOW` | `sensors>forward>temperature` | House supply (“Warm T”) |
| 8 | `T_SENS_INDEX_BACKWARD_FLOW` | `sensors>backward>temperature` | House return |
| 9 | `T_SENS_INDEX_INTERNAL` | `sensors>internal>temperature` | Board/internal |

## Door control

### Fuel hatch override (`DoorsController::applyStatus`)

When `door_opened`: smoke **100%**, oxygen **0%** (immediate), upper **100%**. Otherwise programmed percentages apply.

### Heater modes (`HeaterMode` in `HeaterController.h`)

```
STAND_BY → WARMING → PID → FINAL_COOLING → STAND_BY
                ↘ OVERHEATED ↔ CRITICAL
Hard failsafe: core SMA ≥ 100 °C → CRITICAL
```

Comment also mentions `ALARM (BANG)` — **not implemented**.

| Mode | Smoke | Oxygen | Upper (blowing) | Notes |
|------|-------|--------|-----------------|-------|
| `STAND_BY` | 33% | 0% | 0% | Init-fire can temporarily open O₂ |
| `WARMING` | warming smoke % | 100% | warming upper % (gated) | Pumps typically on |
| `PID` | burning smoke % | PID `%` | burning upper % (gated) | Oxygen from `PidRegulator` |
| `FINAL_COOLING` | 33% | 0% | 0% | |
| `OVERHEATED` | 100% | 0% | 100% | Cool aggressively |
| `CRITICAL` | 100% | 0% | 100% | + emergency valve HIGH |

**Defaults:** `core_overheat` 96 °C → OVERHEATED; `core_critical` 99 °C → CRITICAL.

**Init fire** (`openOxygenDoorForTime`, default `TIME_FOR_INIT_FIRE_SEC` = 900): O₂ 100%, smoke 66%; on close → O₂ 0%, smoke 33%. UI button + web `/openDoorFor15Min`.

**Upper-door safety** (`calculateUpperDoorValue`): if power acceleration ≤ 0 **and** `core_EMA_power < core_power_to_close_upper_door_if_ICPD_W`, force upper **0%**.

**Stub:** `handle_DOOR_OPENED_mode()` is empty TODO.

## Power and accumulated energy

Constants in `Defines.h`: `WATER_ENERGY_PER_LTR_PER_GRAD` = 4200, `JOUL_PER_KWTCH` = 3_600_000, `LOWEST_TEMPERATURE_FOR_ACCUMULATOR` = 35 °C.

**Core power** (`calcMainCoreCharacteristics`):

- Pumps off: ΔT_core × `heater_core_ltr` × 4200 / Δt
- Pumps on: (T_out − T_in) × flow × 4200 / Δt  
  Then EMA / DEMA smoothing into `core_EMA_power` / `core_DEMA_power`.

**Stored energy** (`collectTelemetry`):

```
avg_acc = mean(top, bottom, mid_lo, mid_hi) SMA
E_kWh = ((T_top − 35) × boiler_ltr + (avg_acc − 35) × accumulator_ltr)
        × 4200 / 3_600_000
```

→ `accumulated_energy_kwt_hour` (UI: “Energy kWh”).

**Power balance:** `Intervals(30)` tracks stored energy over time; code then reads `getInterval(39)` while capacity is 30 — **likely buggy**; treat balance values cautiously.

## TFT UI

- Active path: `screen_index == 0` — 3×3 gauge grid + pump / init-fire buttons (`TouchDisplayController`)
- Gauges include Core T, Core P, Warm T, Output T, Pwr P, Top T, Input T, Energy kWh, Bottom T
- Mid-acc sensors feed energy math but are not dedicated main gauges
- `drawScreen1` (door % layout) largely commented out
- BMP icons from LittleFS (`/img/…`)
- **No** multilevel TFT settings menu; configuration is **web** (`data/settings.html`, `/settingsApi`)

## Network / OTA / persistence

| Concern | As-is behavior |
|---------|----------------|
| WiFi | STA to `network.ssid/password`; fail → AP `{mqttDeviceName}-WiFi` / `00000000`, IP `192.168.0.1`, mDNS HTTP; connect wait kicks `esp_task_wdt_reset()` |
| TWDT | Arduino may pre-init TWDT; `setup` uses `esp_task_wdt_reconfigure` for **25 s** if `init` returns `ESP_ERR_INVALID_STATE` |
| Web | Async server (`ESPAsyncWebServer` via `lib_deps`): `/`, `/settings.html`, `/settingsApi`, modelling / door / telemetry endpoints; files from LittleFS |
| Display libs | Adafruit GFX + BusIO via `lib_deps`; customized driver in `lib/ILI9488/` |
| Temps libs | `paulstoffregen/OneWire` + `milesburton/DallasTemperature` via `lib_deps` |
| MQTT | Fields + ParamDescriptors exist; `PubSubClient` vendored — **not constructed or looped in app code** |
| OTA | `ArduinoOTA` in `main.cpp`; **6 s OTA-only window** after boot; TFT progress; no HTTP `/update` |
| EEPROM | `SettingsManager`: versioned `GlobalSettings` + 4-byte marker |
| NTP | Implemented but call site commented out |

## Key sources

| Area | Path |
|------|------|
| Boot / OTA / loop | `src/main.cpp` |
| Heater FSM / power / energy | `src/HeaterController.*` |
| Doors / hatch override | `src/DoorsController.*` |
| Servo PWM rate limit | `src/ServoController.*`, `src/lib/servo/Servo.*` |
| DS18x20 | `src/SensorController.*` |
| TFT + gauges | `src/TouchDisplayController.*` |
| Settings / EEPROM | `src/SettingsManager.*`, `src/GlobalSettings.h`, `src/SettingsNavigator.*` |
| WiFi / web | `src/WiFiController.*`, `src/WebServerController.*` |
| Pins / constants | `src/Defines.h` |
| Web assets | `data/` |

## Known gaps (do not “fix” unless asked)

- MQTT runtime unused
- Pins live in `Defines.h` (no `include/pins.h`)
- GPIO 32 shared by OW2 and `PUMP_4`
- `EMERGENCY_VALVE_PIN == 0` makes some `if (EMERGENCY_VALVE_PIN)` setup paths skip
- `handle_DOOR_OPENED_mode` empty; ALARM / refuel UX incomplete
- Power-balance `getInterval(39)` vs `Intervals(30)`
- Legacy `static *instance` on `HeaterController` and `TouchDisplayController` — do not expand this pattern
- `SettingsManager.cpp` includes a broken absolute ESP8266 `Esp.h` path (legacy smell)

## Working notes

- Prefer small diffs; match existing naming.
- After `data/` changes, flash filesystem (`pio run -t uploadfs`) as well as firmware.
- After changing `partitions_4mb_ota_512fs.csv`, erase/full-flash once so app and FS offsets match.
- Keep this handoff in sync when behavior or APIs change.
