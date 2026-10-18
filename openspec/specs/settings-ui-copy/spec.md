# settings-ui-copy Specification

## Purpose

Keeps user-visible settings copy spelled consistently for thermosensor labels in the web UI.

## Requirements

### Requirement: Thermosensor spelling in web UI
Web settings captions that currently say `Termosensor` or `termosensor` SHALL instead say `Thermosensor` or `thermosensor`. This change is label text only and MUST NOT alter parameter ids, storage keys, or control behavior.

#### Scenario: Heater core caption
- **WHEN** the operator opens the thermal sensors settings section in the web UI
- **THEN** the heater core caption MUST read with `Thermosensor`, not `Termosensor`

#### Scenario: Internal caption
- **WHEN** the operator views the internal sensor caption
- **THEN** it MUST read `Internal thermosensor`
