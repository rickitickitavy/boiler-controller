# Design

## Context

See `proposal.md` for motivation. Today `MqttController` only connects/reconnects on STA Wi-Fi; HANDOFF notes no publish/subscribe. Settings already include `mqttDeviceName`, state/command topic prefixes, and `send_data_to_mqtt_interval_ms` (default 6000), which `SensorController` already uses as the sensor poll interval. Oxygen open/close APIs and UI limits already exist on `HeaterController`.

## Goals / Non-Goals

**Goals:**
- Optional MQTT user/password on connect
- STATE publish + command subscribe with exact topic logging
- Seconds UI for the existing interval field (ms storage)
- Minimal COLD-mode MQTT pump override for `PUMPS_ON` / `PUMPS_OFF`
- Web label spelling fix for thermosensor

**Non-Goals:**
- New interval setting or decoupling sensor poll from MQTT publish rate
- Changing heater FSM, PID math, door algorithms, or pump cooling algorithms beyond a thin override guard
- Home Assistant discovery, retained messages, QoS beyond PubSubClient defaults, or new MQTT topic settings fields
- New class singletons for callbacks (`no-self-instance-ref`)

## Decisions

### 1. Topic strings
**Choice:** `String(deviceName) + "/" + prefix` for state publish and command subscribe.  
**Why:** Matches existing prefix settings without new topic parameters.  
**Alternative:** Hard-code leaf `STATE` / `COMMAND` and ignore prefixes — rejected (would leave UI fields unused).

### 2. Auth
**Choice:** `mqtt.connect(clientId, user, pass)` only when both `mqttUser` and `mqttPassword` are non-empty; else `mqtt.connect(clientId)`.  
**Why:** Standard MQTT optional credentials; matches approved rule.

### 3. Settings layout for credentials
**Choice:** Append `char mqttUser[...]` and `char mqttPassword[...]` at end of `GlobalSettings`; bump `GLOBAL_CURRENT_SETTINGS_VERSION`; defaults empty; sanitize like other MQTT strings.  
**Why:** EEPROM compatibility pattern already used in this project.

### 4. Interval field
**Choice:** Keep `send_data_to_mqtt_interval_ms` storage in ms. MQTT web/TFT UI show/edit seconds with ×1000 on save and /1000 on load. Label exactly `publis state interval (sec)`. Clamp descriptor to 3000–30000. Place control on MQTT settings surfaces; do not add a second EEPROM field.  
**Why:** Human chose reuse; preserves EEPROM values (e.g. 6000 → 6 sec).  
**Trade-off:** Sensor poll rate remains coupled to MQTT publish rate (already true for this field’s name/usage).

### 5. STATE payload assembly
**Choice:** Build `{...}` in `MqttController` (or a small helper it owns) from `HeaterController` telemetry + existing getters: mode via existing English names (≤15), oxygen timer via `getOxygenDoorOpenedForATime()`, pumps ON if `getOnPumpsCount()>0`, time online `millis()/1000`, SMA/temps and door % from telemetry record, core power / power balance cast to int, energy and PID from telemetry (`pid_p/i/d/output`).  
**Why:** All values already exist; no new sensors or PID math.  
**Note:** Float formatting with 2 decimals; no spaces after commas in the list.

### 6. Command callback ownership
**Choice:** Register PubSubClient callback from `main.cpp` (or a free function owned there) that forwards to `MqttController` / `HeaterController` methods. Do not add a new `static MqttController *instance` inside `MqttController`.  
**Why:** Project `no-self-instance-ref` rule.

### 7. OPEN / CLOSE
**Choice:** Call existing `openOxygenDoorForTime(900)` and `closeOxygenDoor()` only.  
**Why:** Same behavior and limits as UI/web.

### 8. MQTT pump override (minimal)
**Choice:** Add a small override window on `HeaterController` (deadline ms + active flag).  
- `PUMPS_ON` in STAND_BY/FINAL_COOLING: set flag, deadline = now+300s, `setOnPumpsCount(2)`.  
- At start of `handlePumps()`: if override active and mode still COLD and not expired → keep 2 pumps and skip normal COLD pump logic for that cycle; if expired → clear flag and fall through; if mode left COLD → clear flag and fall through to existing mode logic.  
- `PUMPS_OFF`: only if flag active and mode COLD → clear flag and `setOnPumpsCount(0)`.  
**Why:** Localizes change; does not rewrite cooling/two-pump algorithms.  
**Alternative:** Separate MQTT pump task — rejected (more surface area).

### 9. Logging
**Choice:** After successful connect + subscribe, `LOGGER.info` the exact publish and subscribe topic C-strings used for `publish`/`subscribe`.  
**Why:** Human-requested broker-visible topic audit.

### 10. UI thermosensor spelling
**Choice:** Replace caption text in `data/index.html` only (`Termosensor`→`Thermosensor`, `termosensor`→`thermosensor`). Leave parameter ids (`sensors>termoaccumulator>...`) unchanged.  
**Why:** Label-only; avoid breaking form keys.

## Risks / Trade-offs

- [Risk] Large STATE payload vs PubSubClient buffer → Mitigation: verify buffer size; increase via `setBufferSize` only if publish fails.  
- [Risk] Existing devices with `send_data_to_mqtt_interval_ms` outside 3–30 s after clamp on save → Mitigation: clamp on load/save; document.  
- [Risk] Pump override interacts with cooling logic → Mitigation: override only in COLD; clear on mode leave; no changes to non-COLD pump paths.  
- [Risk] Callback + heap String topic build on ESP32 → Mitigation: stack/char buffers where practical; avoid blocking in callback (queue command to `handle()` if needed).

## Migration Plan

1. Flash firmware with bumped settings version; new user/password fields default empty.  
2. Set state prefix to `STATE` if operators want that leaf (default remains whatever is already in EEPROM / current defaults).  
3. `uploadfs` after web UI label/interval changes.  
4. Rollback: previous firmware ignores trailing settings fields if version handling allows; otherwise restore prior firmware build.

## Open Questions

None for planning — PID field list (option A), topics, COLD modes, pumps=2, and auth rules are human-approved.
