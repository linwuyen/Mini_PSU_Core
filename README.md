# Mini_PSU_Core

公開、非機密的 C2000 digital-power control firmware portfolio prototype。

這個 repository 的目標是建立一條可重現、可量測、可審查的控制證據鏈；它不是 production firmware，也不聲稱目前已完成閉迴路 power-stage qualification。

## Portfolio signal

```text
ePWM 100 kHz-class timing
        ↓ SOCA
ADC synchronous sampling
        ↓
real-time ISR and control calculation
        ↓
ePWM compare update
        ↓
CMPSS / XBAR / Trip-Zone shutdown
```

第一個公開 vertical reference 位於 [`portfolio_demo/`](portfolio_demo/)。它包含：

- portable C control/reference implementation；
- deterministic host tests；
- Q15 duty clamp 與 fault latch；
- C2000 SysConfig/DriverLib integration skeleton；
- GitHub Actions host validation；
- bench procedure 與 evidence template。

## Current evidence

| Evidence layer | Status |
|---|---|
| Portable control logic | Executable host reference; exact PR CI decides PASS/FAIL |
| C2000 integration | Public-safe skeleton |
| CPU clean build | Not claimed for the new reference |
| Program / flash | Not claimed |
| Oscilloscope / logic analyzer | Not claimed |
| Power-stage closed loop | Future work |
| Production qualification | Not claimed |

No waveform image or board result should be added without the exact source commit, build configuration, programmed artifact SHA-256, board revision, instrument setup, acceptance limit, and PASS/FAIL disposition.

## Intended platform

- TI C2000 F2838x / F2837x class MCU
- synchronous buck or bidirectional buck-boost learning path
- outer voltage loop plus inner current loop candidate architecture
- 100 kHz+ real-time ISR target
- ADC, 2P2Z/PID, HRPWM, OVP/OCP/OTP, and explicit safe startup/shutdown

The actual public demo is intentionally smaller than this target architecture so each claim can be verified independently.

## Run the host reference

```text
cd portfolio_demo
make test
```

The host test verifies event ordering, signed feedback direction, duty saturation, external/overcurrent trip latching, and explicit trip clearing. It does **not** prove C2000 timing, analog scaling, comparator polarity, power-stage stability, or safe hardware operation.

## Bench milestone

The next retained hardware result should use one exact commit and complete this sequence:

1. PWM frequency, duty, polarity, and dead-time.
2. ePWM SOCA → ADC SOC → ADC ISR timing.
3. ISR execution width and jitter using a GPIO marker.
4. Safe control-direction test with loopback or signal-generator input.
5. CMPSS/Trip-Zone or reviewed fault-injection shutdown latency.

Template: [`portfolio_demo/evidence/bench-result-template.md`](portfolio_demo/evidence/bench-result-template.md).

## Build guide for existing CCS projects

1. Install the required Code Composer Studio and C2000Ware versions for the project being imported.
2. Import the intended CPU project only; do not assume generated and legacy device-support files can be compiled together.
3. Select `RAM` for debugger bring-up or `FLASH` for standalone image generation.
4. Clean and build from a known workspace.
5. Retain the exact commit, build configuration, `.out` path, timestamp, and SHA-256.

## Safety boundary

Before enabling a real power stage, independently verify:

- ADC gain, offset, units, saturation, and current-sense polarity;
- PWM pin, polarity, time-base, compare limits, and dead-time;
- gate-driver and power-stage safe state;
- CMPSS/XBAR/Trip-Zone route and latch/clear behavior;
- startup, shutdown, relay, fan, and thermal policy;
- OVP/OCP/OTP limits against component and load boundaries;
- ISR worst-case execution time and interrupt interaction;
- control-loop design and stability.

## Working rules

1. Public content must remain non-confidential.
2. Unverified behavior is labeled `NOT RUN`, `BLOCKED`, or `NOT CLAIMED` rather than implied PASS.
3. Generated build output is not committed.
4. Every hardware result identifies board, toolchain, exact source, artifact hash, stimulus, expected result, measured result, tolerance, and limitation.
5. AI or static source review cannot replace clean build, programmed artifact, and board evidence.
