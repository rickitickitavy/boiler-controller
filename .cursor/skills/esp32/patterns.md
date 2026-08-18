# ESP32 patterns (boilercontroller)

Project-derived detail only. Read when implementing persistence, LittleFS assets, OTA, or door/servo control.

## Settings persistence

- Store settings in **EEPROM** (`EEPROM.begin(4096)`) via `SettingsManager` — not `Preferences`/NVS.
- `GlobalSettings` uses a 4-byte marker (`0x34 0x32 0x33 0x31`) and `GLOBAL_CURRENT_SETTINGS_VERSION` (currently 2).
- First boot (bad/missing marker): apply defaults, write marker/version, save.
- On version mismatch, follow the existing upgrade path in `SettingsManager`; do not invent Preferences migration.

## LittleFS web assets

- Mount LittleFS in `setup` (`LittleFS.begin(false)`).
- Serve static files from `data/` via the async web server; UI icons also load from LittleFS (e.g. `/img/*.bmp`).
- After changing `data/`, run `pio run -t uploadfs` in addition to firmware upload.
- PlatformIO: `board_build.filesystem = littlefs`.

## OTA

- ArduinoOTA is started unconditionally after WiFi setup.
- Boot gate: for ~6 s after `setup` finishes, `loop` only runs `ArduinoOTA.handle()` / WiFi check / WDT — then enables the main UI/control path.
- There is no HTTP firmware/filesystem update endpoint in this repo today.

## Doors / servos

Ownership sketch:

```
HeaterController
  └─ DoorsController
        ├─ ServoController (smoke, LEDC ch 0)
        ├─ ServoController (oxygen, LEDC ch 1)
        └─ ServoController (upper/blowing, LEDC ch 2)
```

- Percent open is programmed on `DoorsController`; `applyStatus()` applies hatch override when the fuel door sensor reports open.
- `ServoController` rate-limits motion (~60 °/s); hardware angles come from `ServosHardwareSettings` in EEPROM settings.
- All three servos are inverted in the current `DoorsController` constructor.

## Callback wiring

Prefer registering handlers from `main.cpp` (or another owner) instead of a fake singleton inside the class:

```cpp
// GOOD — ownership outside the class
heaterController = new HeaterController(...);
touchDisplayController->setHeaterController(heaterController);
wiFiController->setHeaterController(heaterController);
```

```cpp
// BAD for new code — instance pointer only for a library callback
static Foo *instance;
instance = this;
```

## Temperatures

- Read via `SensorController` only (async Dallas conversion + SMA helpers).
- Sensor addresses live in `GlobalSettings`; indices are the `T_SENS_INDEX_*` macros in `Defines.h`.
- Do not hardcode ROM addresses in control logic.

## Pin care

- Edit pins only in `include/pins.h`.
- GPIO **32** is assigned to both `ONE_WIRE_PIN_2` and `PUMP_4_PIN` — treat as contended; do not add a third use casually.
- `EMERGENCY_VALVE_PIN` is `0`; some setup paths use truthiness checks and may skip `pinMode`.
