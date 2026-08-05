# F28388D board-evidence execution checklist

This checklist prepares and retains one exact-head hardware result. It does not authorize enabling an unreviewed power stage.

## 1. Freeze identity

From a clean local clone:

```powershell
powershell -ExecutionPolicy Bypass -File .\portfolio_demo\evidence\start-board-session.ps1
```

The script creates a dedicated `board/...` branch plus:

- a result Markdown file copied from `bench-result-template.md`;
- a JSON identity file containing the frozen source commit;
- explicit `NOT RUN` evidence boundaries.

Do not change control source after the session is frozen. If source changes, discard the session and start another one.

## 2. Record the build environment

Before building, write down:

- CCS version;
- C2000 compiler version;
- C2000Ware version;
- SysConfig version;
- imported CPU project;
- RAM or FLASH build configuration;
- board model and revision.

Run a clean build. Retain the complete build log. A successful host test is not a C2000 build result.

## 3. Retain the programmed artifact

Use the exact `.out` file produced by the frozen source commit. After build and before collecting measurements:

```powershell
powershell -ExecutionPolicy Bypass -File .\portfolio_demo\evidence\complete-board-session.ps1 `
  -ResultFile .\portfolio_demo\evidence\results\<session>.md `
  -ArtifactPath <path-to-out-file> `
  -BuildConfiguration RAM `
  -Toolchain "CCS <version>; compiler <version>; C2000Ware <version>; SysConfig <version>" `
  -Instrument "RIGOL DHO814" `
  -EvidenceFiles <waveform-file-1>,<waveform-file-2>
```

This records SHA-256 identity. It does not prove the artifact was programmed; that must be stated in the result with the actual programming procedure.

## 4. Safe measurement order

Use a current-limited supply and keep the gate driver or power stage disabled until PWM polarity, dead-time, trip routing, and safe state are reviewed.

1. **PWM baseline** — frequency, duty, polarity, complementary relationship, dead-time.
2. **ADC timing** — PWM/SOCA reference and ADC ISR GPIO marker on the same acquisition.
3. **ISR budget** — marker width and cycle-to-cycle jitter under minimum and maximum intended background load.
4. **Control direction** — safe loopback or signal-generator input; verify sign and duty clamp without an energized power stage.
5. **Protection** — reviewed software fault or CMPSS/Trip-Zone injection; measure shutdown latency and latch/clear behavior.

## 5. Minimum channel plan

| Channel | Signal | Purpose |
|---|---|---|
| CH1 | ePWM A | frequency, duty, polarity |
| CH2 | ePWM B | complementary output and dead-time |
| CH3 | PWM/SOCA timing reference | ADC sample phase reference |
| CH4 | ADC ISR GPIO marker | ISR phase, width, and jitter |

For trip latency, replace one channel with the reviewed fault-injection signal or capture it in a second acquisition.

## 6. Acceptance and disposition

Every row in the result file needs:

- predicted value or direction;
- numerical tolerance or explicit qualitative criterion;
- measured value;
- `PASS`, `FAIL`, `BLOCKED`, or `NOT RUN`;
- test conditions and instrument setup.

A screenshot without the exact source commit, artifact SHA-256, board, channel mapping, scales, trigger, and acceptance limit is not retained engineering evidence.

## 7. Commit evidence

Commit only public-safe evidence. Do not publish company schematics, proprietary pin maps, private board documents, credentials, or confidential measurements.

The evidence commit may follow the frozen source commit because Markdown, manifests, and waveform images are not part of the programmed firmware. The result must continue to identify the original frozen source commit and programmed artifact hash.
