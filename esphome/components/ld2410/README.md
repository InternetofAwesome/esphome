# LD2410 component — auto-calibration notes

## Background

The upstream LD2410 component supports the full LD2410 family. This fork adds
auto-calibration to match the three modes offered by HLKRadarTool.

Gate numbers map to distance ranges at 0.75 m per gate: G0 = 0.00-0.75 m,
G1 = 0.75-1.50 m, G2 = 1.50-2.25 m, G3 = 2.25-3.00 m, G4 = 3.00-3.75 m,
G5 = 3.75-4.50 m, G6 = 4.50-5.25 m, G7 = 5.25-6.00 m, G8 = 6.00-6.75 m.

## Calibration modes

Thresholds are always clamped to the sensor's valid 0-100 range.

### Average
Samples the empty-room noise floor for `calibration_sample` seconds (default
60 s). For each gate (G0 0.00-0.75 m through G8 6.00-6.75 m), sets the move
and still threshold to `mean + 2*stddev` of the energy seen at that gate over
the sample window, computed from running sum / sum-of-squares accumulators
(no per-sample buffer is kept, so RAM use is O(1) per gate).

A threshold at the bare mean would sit in the middle of the observed noise —
roughly half of all empty-room samples would exceed it, i.e. false-triggers
by construction. Adding 2 standard deviations pushes the threshold above
~97.7% of a roughly-normal noise distribution while keeping it, in most
cases, below the sampled peak — hence "more sensitive" relative to Maximum.

- Lower thresholds → more sensitive, more likely to trigger on small signals.
- Vulnerable to transient noise (vibration, air movement) inflating the
  mean/stddev.
- Good default for quiet, stable environments.
- Declaring per-gate `move_energy` / `still_energy` sensors (the `gX:` sensor
  blocks) is *not* required — the component captures per-gate energies
  internally as engineering frames arrive, independent of whether any gate
  energy sensor entities exist.

### Maximum
Same host-side sampling as Average, but sets each gate's threshold to the
**peak** energy seen at that gate, plus a fixed margin of 5 (on the 0-100
scale). The margin exists because detection is a strict `energy > threshold`
comparison: a threshold set exactly at the sampled peak would still
false-trigger on the next noise excursion that's even fractionally larger,
and the sampling window is short enough that it may not have caught the
true peak.

- Higher thresholds → less sensitive, requires a stronger signal to trigger.
- More robust in environments with intermittent background noise.
- Less likely to false-positive; may miss weak still targets.
- Same as Average: no per-gate `move_energy` / `still_energy` sensor entities
  need to be declared for calibration to work.

### Intelligent
Delegates entirely to firmware (requires v2.44+). The host sends
`CMD_AUTO_THRESHOLD` (0x0B) with `calibration_sample` seconds as the
firmware's sampling duration, then polls `CMD_AUTO_THRESHOLD_QUERY` (0x1B)
every 2 s. The firmware samples internally for that long and applies its own
threshold algorithm with undocumented internal headroom. On completion the
module restarts and thresholds are read back via `CMD_QUERY`.

- `calibration_sample` sets the firmware's sampling duration directly (see
  "Protocol notes" below) — it is *not* ignored in this mode.
- Recommended starting point — mirrors the vendor tool default.

## Workflow

### Average / Maximum
1. Make sure the room is empty.
2. Set **Calibration Mode** to Average or Maximum.
3. Set **Calibration Delay** (seconds to leave the room after pressing Start).
4. Set **Calibration Sample Duration** (seconds to collect data).
5. Press **Start Calibration** and leave the room.
6. Watch **Calibration Status** count through `delay X/Ys` → `sampling X/Ys`
   → `ready to apply`.
7. Press **Apply Calibration** to write thresholds to the sensor, or
   **Discard Calibration** to abandon without changing anything.

### Intelligent
1. Make sure the room is empty.
2. Set **Calibration Mode** to Intelligent.
3. Set **Calibration Delay** (countdown before the firmware command is sent).
4. Set **Calibration Sample Duration** (seconds the firmware spends sampling).
5. Press **Start Calibration** and leave the room.
6. Status shows `firmware: in progress` for the sampling duration, then
   `firmware: success`. The module restarts automatically and gate
   thresholds update in HA.

## Protocol notes

- `CMD_AUTO_THRESHOLD` (0x0B): payload is `duration_sec` as a little-endian
  uint16 (2 bytes) — the firmware's **sampling duration in seconds**, not a
  pre-sampling delay as earlier revisions of this doc claimed. This fork
  sends `calibration_sample` (clamped to uint8_t, which always fits in the
  uint16 field) for that payload. This reading is corroborated by the
  independent `Megarushing/ha-ld2410` HA integration, which sends
  `CMD_START_AUTO_THRESH + duration_sec.to_bytes(2, 'little')` and tells the
  user to keep the room empty for the given duration while calibration runs —
  re-check that project if this ever needs re-verifying. Config mode must be
  enabled before sending; disabled in the ACK handler.
- `CMD_AUTO_THRESHOLD_QUERY` (0x1B): no payload. Response byte 10:
  `0` = not started, `1` = in progress, `2` = completed.
- The `CMD_QUERY` (0x61) response has `0x01, 0x00` at bytes 8–9 (value head
  flag). The generic ACK guard in `handle_ack_data_()` exempts this command
  to avoid discarding threshold data.

## Upstream PR checklist

- [ ] Docs PR to `esphome/esphome-docs` (separate repo, separate PR)
- [ ] Tag `@sebcaps` and `@regevbr` in PR description (CODEOWNERS)
- [ ] Step 5: actions in `automation.h` filling `CALIBRATION_ACTION_SCHEMA`
- [ ] Remove debug `ESP_LOGD` calls from `ld2410.cpp` before submitting
