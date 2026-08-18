---
name: esp32
description: >-
  Applies ESP32 PlatformIO Arduino conventions for the wood boiler controller:
  GFX UI rules, callback ownership, LittleFS/OTA/EEPROM patterns, DS18x20 sensors,
  and LEDC servo doors. Use when working on ESP32-WROOM, PlatformIO, platformio.ini,
  LittleFS, ArduinoOTA, ILI9488/Adafruit GFX, DS18x20, PWM servos, WiFi/web firmware,
  or when the user mentions the esp32 skill.
---

# ESP32 (boilercontroller conventions)

Rules below are from **this** project’s stack. Document as-is behavior; do not invent pressure-controller / C3 guidance here.

## UI (GFX / ILI9488)

- Never use `setTextSize(n)` with `n != 1` to enlarge text.
- Choose a properly sized Adafruit GFX font; keep `setTextSize(1)` when using GFXfonts.
- Never scale bitmap icons at draw time; draw native-resolution BMPs from LittleFS 1:1.
- Prefer partial gauge/widget redraws in normal updates; avoid full-screen clear in the hot path.

## Class instance references

- **Not allowed for new code:** ad-hoc `static Foo *instance` inside a class’s `.h`/`.cpp` only so a static/C callback can reach `this`.
- **Allowed:** intentional logical singletons (e.g. `LOGGER`).
- Wire ownership and callbacks from outside the class (e.g. `main.cpp`).
- Legacy: `HeaterController::instance` and `TouchDisplayController::instance` exist — do not expand the pattern.

## Universal algorithms vs project config

- **Not allowed:** project-specific parameter names, units, or special cases inside shared algorithms.
- **Allowed:** neutral metadata on shared types (e.g. `ParamDescriptor::decimalPlaces`) and project-specific values only at registration/config sites.
- Extend the shared API; do not grow `if (paramName == "...")` lists in the generic path.

## Layout and stack

- Pins as macros in [`include/pins.h`](../../../include/pins.h).
- PlatformIO `espressif32` + Arduino; board `esp32dev` — see `platformio.ini`.
- Filesystem: **LittleFS** (`board_build.filesystem = littlefs`); web assets under `data/`.
- Web server: `me-no-dev/ESPAsyncWebServer@3.6.0` + `me-no-dev/AsyncTCP@3.3.2` via `lib_deps` (not under `src/lib/`).
- Display: custom local [`lib/ILI9488`](../../../lib/ILI9488/); `adafruit/Adafruit GFX Library@1.11.11` + `adafruit/Adafruit BusIO@1.16.2` via `lib_deps`.
- Persistence: EEPROM + versioned `GlobalSettings` — **not** Preferences/NVS; do not switch casually.
- Temps: DS18x20 via `SensorController` / `paulstoffregen/OneWire@2.3.8` + `milesburton/DallasTemperature@3.11.0` (`lib_deps`); addresses in settings.
- Doors: LEDC PWM via `src/lib/servo/Servo.*`, rate-limited by `ServoController`, owned by `DoorsController`.

## Domain map (doors)

| Domain | Code |
|--------|------|
| Blowing | `upper_door_*` / `UPPER_SERVO_PIN` |
| Oxygen / afterburning | `oxygen_door_*` / `OXYGEN_SERVO_PIN` |
| Smoke | `smoke_pipe_*` / `SMOKE_SERVO_PIN` |

Runtime FSM, energy math, and gaps: [`docs/HANDOFF.md`](../../../docs/HANDOFF.md).

## Build / flash

```bash
pio run
pio run -t upload
pio run -t uploadfs    # after data/ changes
pio device monitor     # 921600
```

## OTA (this project)

- `ArduinoOTA.begin()` in `setup`; `ArduinoOTA.handle()` in `loop`.
- First **6 s** after boot: early-return so OTA can finish without heater/UI work.
- No HTTP `/update` path today — do not assume pressure-controller dual OTA.

## MQTT

Settings and ParamDescriptors exist; no app-level MQTT client loop. Do not add publish/subscribe wiring unless the user asks.

## Deeper notes

- For EEPROM / LittleFS / door ownership detail, see [patterns.md](patterns.md).
