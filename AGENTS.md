# AGENTS.md — boilercontroller

ESP32-WROOM wood boiler controller (PlatformIO / Arduino). Controls three damper doors (blowing / oxygen / smoke), boiler + accumulator temperatures, and accumulated thermal energy.

## Before changing firmware

1. Read and follow [`.cursor/skills/esp32/SKILL.md`](.cursor/skills/esp32/SKILL.md).
2. For EEPROM, LittleFS, OTA, or servos/doors, also read [`.cursor/skills/esp32/patterns.md`](.cursor/skills/esp32/patterns.md).
3. For product/runtime context, use [`docs/HANDOFF.md`](docs/HANDOFF.md).

## Always-on project rules

Enforced via [`.cursor/rules/`](.cursor/rules/):

| Rule | Meaning |
|------|---------|
| `esp32.mdc` | Apply the esp32 skill before firmware edits |
| `no-font-upscale.mdc` | No `setTextSize(n≠1)`; use sized GFXfonts; draw bitmaps 1:1 |
| `no-self-instance-ref.mdc` | No **new** ad-hoc `static Foo *instance` in class files; wire callbacks from `main.cpp` |
| `no-project-logic-in-universal.mdc` | No project key/unit special cases inside shared algorithms — put metadata on descriptors / registration |

## Domain → code (quick map)

| Domain | Code |
|--------|------|
| Blowing door | `upper_door_*` / `UPPER_SERVO_PIN` |
| Afterburning / oxygen door | `oxygen_door_*` / `OXYGEN_SERVO_PIN` |
| Smoke door | `smoke_pipe_*` / `SMOKE_SERVO_PIN` |
| Accumulated energy | `accumulated_energy_kwt_hour` |

Full glossary, FSM, pins, and energy formulas: [`docs/HANDOFF.md`](docs/HANDOFF.md).

## Stack (as-is)

- Board: `esp32dev` (ESP32-WROOM) — see [`platformio.ini`](platformio.ini)
- Pins: [`include/pins.h`](include/pins.h)
- Settings: EEPROM + versioned `GlobalSettings` — **not** Preferences/NVS
- Web assets: **LittleFS** under [`data/`](data/); flash with `pio run -t uploadfs`
- Web server: PlatformIO `me-no-dev/ESPAsyncWebServer` + `AsyncTCP` (not vendored under `src/lib/`)
- Display: custom local `lib/ILI9488` + XPT2046; Adafruit GFX / BusIO via `lib_deps`
- Temps: DS18x20 via `SensorController` + `paulstoffregen/OneWire` + `milesburton/DallasTemperature` (`lib_deps`)
- Doors: LEDC PWM servos via `DoorsController` / `ServoController`
- MQTT: settings fields exist; **runtime client not wired** — do not invent a publish loop unless asked

## Build / flash

```bash
pio run
pio run -t upload
pio run -t uploadfs    # after data/ changes
pio device monitor     # 921600
```

## Hard constraints (do not violate)

- **UI:** `setTextSize(1)` with GFXfonts; no bitmap upscaling at draw time.
- **Callbacks:** do not add new fake class singletons; register handlers from owners (e.g. `main.cpp`). Existing `HeaterController::instance` / `TouchDisplayController::instance` are legacy — do not expand.
- **Shared code:** extend neutral APIs / descriptor fields; do not add `if (paramName == "...")` lists in universal paths.
- **Filesystem:** **LittleFS** (`board_build.filesystem = littlefs`); keep SPIFFS out of new code.
- **Pins:** change only in `include/pins.h` with care (GPIO 32 is already contended).
- **Persistence:** EEPROM versioned struct — do not switch to Preferences/NVS casually.

## OTA (current)

- Production (`esp32dev`): HTTP Update only (`POST /update/code|firmware|data`).
- Lab (`esp32dev_ota` / `-DENABLE_ARDUINO_OTA`): `ArduinoOTA` started in `setup`; first **6 s** after boot early-returns so ArduinoOTA can run alone.
- No separate firmware/data HTTP endpoints beyond the paths above.

## Key sources

| Area | Path |
|------|------|
| Boot / OTA / loop | `src/main.cpp` |
| Heater FSM / power / energy | `src/HeaterController.*` |
| Doors | `src/DoorsController.*` |
| Sensors | `src/SensorController.*` |
| TFT gauges | `src/TouchDisplayController.*` |
| Settings / EEPROM | `src/SettingsManager.*`, `src/GlobalSettings.h`, `src/SettingsNavigator.*` |
| WiFi / web | `src/WiFiController.*`, `src/WebServerController.*` |
| Pins | `include/pins.h` |
| Constants / log / sensor indices | `src/Defines.h` |

## Working style

- Prefer small, focused diffs; match existing naming and patterns.
- Do not commit unless the user asks.
- After `data/` changes, remind to run `uploadfs` as well as firmware upload.
- Keep [`docs/HANDOFF.md`](docs/HANDOFF.md) in sync when behavior or APIs change.
- Do not “fix” known gaps listed in the handoff unless the user asks.
