# Proposal

## Why

MQTT currently only connects; nothing publishes state or accepts remote commands. Operators need a compact STATE feed and the same oxygen/pump actions already available on the UI, with optional broker credentials and clear topic logging.

## What Changes

- Add optional MQTT username and password settings (default empty). Use standard MQTT username/password auth only when **both** are non-empty; otherwise connect without credentials.
- Reuse existing `send_data_to_mqtt_interval_ms` for STATE publish period. Expose it in MQTT settings with UI label exactly `publis state interval (sec)`, shown/edited in **seconds** (min 3, max 30, default 6). Storage remains milliseconds (×1000) so existing EEPROM values stay valid. Tighten allowed range to 3000–30000 ms. Sensor poll continues to use this same field (existing coupling; do not invent a second interval).
- On successful connect: subscribe to the command topic; publish STATE on the configured interval; **log the exact full topic strings** used for subscribe and publish (as they appear on the broker).
- Topic construction (confirmed): `{mqttDeviceName}/{deviceStateOutgoingTopicPrefix}` for STATE publish; `{mqttDeviceName}/{deviceIncomingCommandTopicPrefix}` for command subscribe.
- STATE payload format: `{v1,v2,...,v24}` with the fixed field order in the mqtt-client spec (mode ≤15 chars human-readable; pump status `ON`/`OFF`; floats with 2 decimals where specified). PID fields are live calculated terms (P, I, D, PID output), not settings gains; no separate I-sum field.
- Command payloads (exact): `OPEN`, `CLOSE`, `PUMPS_ON`, `PUMPS_OFF`.
  - `OPEN` / `CLOSE`: call existing `openOxygenDoorForTime(900)` / `closeOxygenDoor()` (same mode limits as UI).
  - `PUMPS_ON`: if mode is STAND_BY or FINAL_COOLING, force **2** pumps on for 5 minutes; leave COLD → mode’s normal pump control takes over and MQTT override ends.
  - `PUMPS_OFF`: only if still in STAND_BY/FINAL_COOLING **and** pumps are under the MQTT override from `PUMPS_ON`.
- Fix web UI captions: `Termosensor` / `termosensor` → `Thermosensor` / `thermosensor` (label text only).
- **No** new heater entities, modes, or unrelated algorithm changes. Minimal hooks only where MQTT must call existing APIs or apply a time-bounded pump override in COLD modes.

## Capabilities

### New Capabilities

- `mqtt-client`: MQTT auth settings, topic construction, STATE publish payload/interval, command subscribe (`OPEN`/`CLOSE`/`PUMPS_ON`/`PUMPS_OFF`), and exact topic logging.
- `settings-ui-copy`: User-visible spelling fix for thermosensor labels in the web settings UI.

### Modified Capabilities

- (none — `openspec/specs/` has no existing capabilities yet)

## Impact

- `GlobalSettings` / `SettingsManager` / `SettingsNavigator`: append `mqttUser`, `mqttPassword`; bump settings version; defaults empty; interval descriptor limits; MQTT UI descriptors.
- Web (`data/index.html`) and TFT MQTT forms: user, password, interval label/seconds conversion; thermosensor caption spelling.
- `MqttController`: auth connect, subscribe, periodic STATE publish, command callback wiring from `main.cpp` (no new class singleton).
- `HeaterController`: thin MQTT pump override API used only for `PUMPS_ON`/`PUMPS_OFF` in STAND_BY/FINAL_COOLING; OPEN/CLOSE reuse existing oxygen APIs.
- EEPROM layout: new fields only at end of `GlobalSettings` (versioned migration).
- No `FIRMWARE_VERSION` symbol in this repo — version bump skipped for propose.
