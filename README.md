# Wood boiler controller

ESP32-WROOM firmware (PlatformIO / Arduino) for a wood boiler with three servo dampers (blowing, afterburning/oxygen, smoke), DS18x20 temperature sensing (boiler + thermal accumulator + house loops), and accumulated energy estimation.

## Build / flash

```bash
pio run
pio run -t upload
pio run -t uploadfs    # after changes under data/
pio device monitor     # 921600 (see platformio.ini)
```

Board: `esp32dev`. Upload port defaults to `/dev/ttyUSB0` in [`platformio.ini`](platformio.ini).

## Docs for agents / contributors

| File | Purpose |
|------|---------|
| [`AGENTS.md`](AGENTS.md) | Agent entrypoint, constraints, key sources |
| [`docs/HANDOFF.md`](docs/HANDOFF.md) | Domain glossary, pins, FSM, energy math, known gaps |
| [`.cursor/skills/esp32/`](.cursor/skills/esp32/) | Project stack conventions |

## Stack snapshot

- ILI9488 TFT + XPT2046 touch
- DS18x20 on dual OneWire buses
- LEDC PWM servos for three doors
- LittleFS web UI under [`data/`](data/)
- EEPROM versioned settings (`GlobalSettings`)
- `ESPAsyncWebServer` + `AsyncTCP` via PlatformIO `lib_deps`
