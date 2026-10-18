# Tasks

## 1. Settings: credentials and interval

- [x] 1.1 Append `mqttUser` and `mqttPassword` char fields at end of `GlobalSettings`, bump `GLOBAL_CURRENT_SETTINGS_VERSION`, set empty defaults and sanitize like other MQTT strings; verify load/save round-trip leaves empty defaults and persists non-empty values
- [x] 1.2 Clamp `send_data_to_mqtt_interval_ms` descriptor to 3000–30000 ms (default 6000 remains); verify navigator/descriptor min/max match and defaults still init to 6000 ms
- [x] 1.3 Expose MQTT user, password, and interval on MQTT web/TFT settings with interval label exactly `publis state interval (sec)`, UI in seconds (×1000 on save, /1000 on load); verify form show/edit round-trips 10 → 10000 ms storage

## 2. MQTT connect, topics, and STATE publish

- [x] 2.1 Connect with username/password only when both are non-empty; otherwise connect without credentials; verify both auth paths compile and follow the dual-empty vs both-set rule in `MqttController`
- [x] 2.2 On successful connect: build topics `{mqttDeviceName}/{deviceStateOutgoingTopicPrefix}` and `{mqttDeviceName}/{deviceIncomingCommandTopicPrefix}`, subscribe to command topic, and `LOGGER.info` the exact publish and subscribe topic strings; verify log strings match the strings passed to publish/subscribe
- [x] 2.3 While connected, publish STATE on `send_data_to_mqtt_interval_ms` as `{v1,...,v24}` with the exact field order/formatting from the mqtt-client spec (mode ≤15, pumps ON/OFF, floats 2 decimals, live PID terms); verify payload builder produces 24 comma-separated fields inside braces with no spaces after commas
- [x] 2.4 Increase PubSubClient buffer via `setBufferSize` only if needed for the STATE payload; verify a full STATE publish succeeds without truncation

## 3. Commands and pump override

- [x] 3.1 Register PubSubClient callback from `main.cpp` (no new `MqttController` singleton) forwarding payloads to command handling; verify `OPEN`/`CLOSE` call existing `openOxygenDoorForTime(900)` / `closeOxygenDoor()` with the same mode limits as UI
- [x] 3.2 Add thin MQTT pump override on `HeaterController` (flag + 300 s deadline): `PUMPS_ON` in STAND_BY/FINAL_COOLING forces 2 pumps; clear on expiry or leaving COLD; `PUMPS_OFF` only clears when override active in COLD; verify `handlePumps()` respects override and ignores commands outside rules

## 4. Web copy and docs

- [x] 4.1 Replace web caption text `Termosensor`/`termosensor` → `Thermosensor`/`thermosensor` in `data/index.html` only (leave parameter ids unchanged); verify heater core and internal captions match the settings-ui-copy scenarios
- [x] 4.2 Update `docs/HANDOFF.md` MQTT section for auth, topics, STATE publish, and commands; verify handoff describes the new runtime behavior

## Workflow follow-up

- Archive the change after review/verification on device (broker STATE + command smoke test).
- Remind `pio run -t uploadfs` after web UI changes.
