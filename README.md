# Mini_PSU_Core

Public portfolio repository for a Mini PSU control-firmware prototype.

This repository documents the architecture and development direction for a small programmable power-supply controller. It is useful as a power-electronics firmware portfolio project, but it is not a validated production release.

## Repository Role

- **Purpose:** public portfolio repository for Mini PSU control-firmware architecture and development.
- **Status:** public showcase and active prototype.
- **Scope:** non-confidential power-control firmware structure, documented interfaces, and reproducible examples.
- **Production status:** prototype/portfolio code; hardware use requires independent review, calibration, and protection validation.

## Project Goal

Implement a digital-control programmable power supply with:

- regulated output voltage and current
- fast transient response target
- over-voltage protection (OVP)
- over-current protection (OCP)
- over-temperature protection (OTP)
- host communication for configuration and monitoring

## Hardware Platform

The current README history describes the intended platform as:

- **Control board:** Mini PSU Control Board V1.0 or compatible hardware.
- **MCU family:** TI C2000 F28388D / F28377D class high-resolution PWM MCU.

## Power Topology

Target topology direction:

- synchronous buck
- bidirectional buck-boost candidate path
- digital dual-loop control: outer voltage loop plus inner current loop
- HRPWM-based switching control

## Firmware Architecture

The intended firmware architecture is foreground/background plus interrupt-driven real-time control:

```text
High-speed ISR, 100 kHz+ target
  - ADC conversion handling
  - 2P2Z / PID control calculation
  - HRPWM duty update
  - fast protection checks

Background / scheduled tasks
  - system state machine
  - LED/status reporting
  - thermal monitoring and fan control
  - host communication
  - parameter storage
```

## Build Guide

1. Install Code Composer Studio v12 or newer.
2. Install C2000Ware under the standard TI path when possible.
3. Import the project through CCS.
4. Select the intended build configuration:
   - `RAM` for debug
   - `FLASH` for standalone firmware image
5. Build and verify the generated `.out` file.

## Current Status

Current documented status:

- baseline clock / hardware configuration work exists
- HRPWM and ADC baseline direction is documented
- CCS project and build workflow are part of the repository intent
- closed-loop control and physical-board validation remain future work

## Safety Notes

This is power-electronics firmware. Do not run on hardware without reviewing:

- ADC scaling and calibration
- PWM polarity and dead-time
- current-sense polarity
- fault-trip configuration
- startup and shutdown state machine
- OVP / OCP / OTP limits
- load and supply boundaries

## Working Rules

1. Keep public content non-confidential.
2. Mark unverified hardware behavior clearly.
3. Do not present prototype code as production-ready.
4. Keep generated build output out of version control.
5. Document board version, toolchain, and test conditions for each hardware result.
