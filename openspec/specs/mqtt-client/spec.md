# mqtt-client Specification

## Purpose

Defines MQTT broker authentication, STATE telemetry publish, inbound command handling, and exact topic logging for the boiler controller. Behavior below matches the human-approved decisions in the propose conversation for change mqtt-state-and-commands.

## Requirements

### Requirement: Optional MQTT username and password
The system SHALL persist MQTT username and password settings that default to empty. The system SHALL use MQTT username/password authentication when connecting only when both username and password are non-empty. When either field is empty, the system SHALL connect without username/password credentials.

#### Scenario: Both credentials set
- **WHEN** MQTT username and password are both non-empty and the client connects
- **THEN** the connection MUST use those credentials for MQTT authentication

#### Scenario: Credentials empty
- **WHEN** MQTT username or password is empty and the client connects
- **THEN** the connection MUST proceed without username/password authentication

### Requirement: Publish interval setting in seconds
The system SHALL use the existing `send_data_to_mqtt_interval_ms` setting as the STATE publish interval. MQTT settings UI MUST show the label exactly `publis state interval (sec)` and present the value in seconds with minimum 3, maximum 30, and default 6. Stored value MUST remain in milliseconds (seconds × 1000). Allowed stored range MUST be 3000–30000 ms.

#### Scenario: Default interval
- **WHEN** settings are initialized to defaults
- **THEN** the interval MUST be 6 seconds (6000 ms)

#### Scenario: UI edit in seconds
- **WHEN** the operator sets the MQTT interval field to 10
- **THEN** the stored value MUST be 10000 ms and STATE MUST publish about every 10 seconds while connected

### Requirement: Topic path construction
The system SHALL publish STATE to topic `{mqttDeviceName}/{deviceStateOutgoingTopicPrefix}` and SHALL subscribe for commands on topic `{mqttDeviceName}/{deviceIncomingCommandTopicPrefix}`.

#### Scenario: Example topics
- **WHEN** device name is `heater-test`, state prefix is `STATE`, and command prefix is `command`
- **THEN** publish topic MUST be `heater-test/STATE` and subscribe topic MUST be `heater-test/command`

### Requirement: Exact topic logging
After a successful MQTT connect, the system SHALL log the exact full publish topic and the exact full subscribe topic strings as they are used with the broker (no abbreviated or differently constructed forms in that log).

#### Scenario: Log on connect
- **WHEN** MQTT connects successfully and topics are armed for publish/subscribe
- **THEN** the log MUST contain those exact topic strings

### Requirement: Periodic STATE publish
While MQTT is connected, the system SHALL publish to the STATE topic at the configured interval. The payload MUST be a single brace-wrapped comma-separated list `{v1,v2,...,v24}` using this exact field order and formatting:

1. current mode — human-readable string, length ≤ 15
2. oxygen door open timer remaining — integer seconds
3. pump status — `ON` or `OFF`
4. time online — integer seconds since boot
5. internal thermosensor — float, 2 decimal places
6. heater core thermosensor — float, 2 decimal places
7. heater output flow thermosensor — float, 2 decimal places
8. heater input flow thermosensor — float, 2 decimal places
9. thermo accumulator top thermosensor — float, 2 decimal places
10. thermo accumulator middle higher thermosensor — float, 2 decimal places
11. thermo accumulator middle lower thermosensor — float, 2 decimal places
12. thermo accumulator bottom thermosensor — float, 2 decimal places
13. forward flow to all contours thermosensor — float, 2 decimal places
14. back flow from contour 1 thermosensor — float, 2 decimal places
15. oxygen door percentage — float, 2 decimal places
16. afterburning (upper oxygen) door percentage — float, 2 decimal places
17. smoke door percentage — float, 2 decimal places
18. core power — integer
19. accumulate/spend power — float, 2 decimal places
20. accumulated energy — float, 2 decimal places
21. P (PID) — live calculated P term, float, 2 decimal places
22. I (PID) — live calculated I term (integrator), float, 2 decimal places
23. D (PID) — live calculated D term, float, 2 decimal places
24. PID output — live calculated output, float, 2 decimal places

Pump status MUST be `ON` when at least one pump is on, otherwise `OFF`. PID fields MUST be current calculated regulator values, not PID gain settings.

#### Scenario: Publish while connected
- **WHEN** MQTT remains connected for longer than the configured interval
- **THEN** the system MUST publish at least one STATE payload on the STATE topic in that brace-list format with 24 fields

#### Scenario: Mode string length
- **WHEN** STATE is published
- **THEN** the mode field MUST be a human-readable string of at most 15 characters

### Requirement: Command OPEN
When a message with payload exactly `OPEN` arrives on the command topic, the system SHALL invoke the same open-oxygen-for-15-minutes behavior as the UI (`openOxygenDoorForTime(900)`), including the same mode restrictions.

#### Scenario: OPEN in allowed mode
- **WHEN** mode is STAND_BY or FINAL_COOLING and payload `OPEN` is received
- **THEN** the oxygen timed-open behavior MUST run as on the UI

#### Scenario: OPEN in disallowed mode
- **WHEN** mode is not STAND_BY or FINAL_COOLING and payload `OPEN` is received
- **THEN** the system MUST NOT open the oxygen door via this command (same failure as UI)

### Requirement: Command CLOSE
When a message with payload exactly `CLOSE` arrives on the command topic, the system SHALL invoke the same close-oxygen behavior as the UI (`closeOxygenDoor()`), including the same mode restrictions.

#### Scenario: CLOSE in allowed mode
- **WHEN** mode is STAND_BY or FINAL_COOLING and payload `CLOSE` is received
- **THEN** close-oxygen MUST behave as on the UI

### Requirement: Command PUMPS_ON
When a message with payload exactly `PUMPS_ON` arrives and the heater mode is STAND_BY or FINAL_COOLING, the system SHALL turn on 2 pumps for 5 minutes under an MQTT override. If the heater leaves STAND_BY/FINAL_COOLING for any other mode, pump control MUST follow that mode’s existing logic and the MQTT override MUST end. If mode is not STAND_BY or FINAL_COOLING, `PUMPS_ON` MUST be ignored.

#### Scenario: PUMPS_ON in COLD
- **WHEN** mode is STAND_BY or FINAL_COOLING and payload `PUMPS_ON` is received
- **THEN** 2 pumps MUST turn on and remain under MQTT override for up to 5 minutes while mode stays STAND_BY or FINAL_COOLING

#### Scenario: Mode leaves COLD during override
- **WHEN** MQTT pump override is active and the heater switches to a non-COLD mode
- **THEN** pumps MUST be controlled by that mode’s existing logic and the MQTT override MUST no longer apply

#### Scenario: PUMPS_ON outside COLD
- **WHEN** mode is not STAND_BY or FINAL_COOLING and payload `PUMPS_ON` is received
- **THEN** the command MUST have no pump effect

### Requirement: Command PUMPS_OFF
When a message with payload exactly `PUMPS_OFF` arrives, the system SHALL clear the MQTT pump override and turn pumps off only if mode is STAND_BY or FINAL_COOLING and pumps are currently under that MQTT override. Otherwise `PUMPS_OFF` MUST be ignored.

#### Scenario: PUMPS_OFF while override active in COLD
- **WHEN** MQTT pump override is active, mode is STAND_BY or FINAL_COOLING, and payload `PUMPS_OFF` is received
- **THEN** the override MUST end and pumps MUST turn off

#### Scenario: PUMPS_OFF without override
- **WHEN** MQTT pump override is not active and payload `PUMPS_OFF` is received
- **THEN** the command MUST have no pump effect
