# Mini PSU vertical control demo

This public-safe reference makes one complete control path reviewable:

```text
ePWM period / SOCA
        ↓
ADC sample
        ↓
ADC ISR timing marker
        ↓
proportional control and duty clamp
        ↓
ePWM compare update
        ↓
latched hardware/software trip
```

## What is executable now

The portable C reference under `include/`, `src/`, and `tests/` verifies:

- event ordering for PWM period → ADC sample → ISR → duty update or trip;
- signed error handling;
- Q15 proportional correction;
- duty saturation;
- overcurrent and external-fault latching;
- explicit, authorized trip clearing.

Run:

```text
cd portfolio_demo
make test
```

The GitHub Actions workflow runs the same command.

## C2000 integration boundary

`target/f2838x_signal_chain_example.c` is an integration skeleton, not board-qualified firmware. It shows the attachment points to a SysConfig/DriverLib project but deliberately does not define the actual board's:

- PWM pin and polarity;
- frequency, dead-time, or safe compare limits;
- ADC channel, acquisition window, gain, offset, or unit conversion;
- current-sense polarity and threshold;
- CMPSS/XBAR/Trip-Zone route;
- startup, shutdown, relay, fan, and thermal policy.

Those values must come from the target schematic, component limits, official TI documentation, clean build, and measurement.

## Bench validation sequence

Freeze one exact commit before collecting evidence.

1. **PWM baseline**
   - Measure frequency, duty, complementary polarity, and dead-time.
   - Record time-base clock assumptions and expected values.
2. **Synchronous ADC sampling**
   - Observe PWM/SOCA reference and ISR marker together.
   - Confirm the sample point is in the intended low-noise region.
3. **ISR budget**
   - Measure marker width and cycle-to-cycle jitter at minimum and maximum expected background load.
   - Compare worst-case execution time with the control-period budget.
4. **Control direction**
   - Apply a safe signal-generator or loopback input.
   - Confirm increasing feedback moves duty in the expected direction and remains clamped.
5. **Protection**
   - Inject the reviewed comparator or software fault.
   - Measure shutdown latency and verify the output remains latched off until the explicit clear policy is satisfied.

Use `evidence/bench-result-template.md` for the retained result.

## Evidence status

| Layer | Status |
|---|---|
| Portable host logic | CI will determine PASS/FAIL for the exact PR head |
| C2000 target integration | Skeleton only |
| CPU build | Not claimed |
| Program / flash | Not claimed |
| Oscilloscope / logic analyzer | Not claimed |
| Power-stage validation | Not claimed |
| Production safety | Not claimed |
