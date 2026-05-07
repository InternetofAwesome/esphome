# LD2410 component — auto-calibration notes

## Background

The upstream LD2410 component supports the full LD2410 family. This fork adds
auto-calibration to match the three modes offered by HLKRadarTool.

## Calibration modes

### Average
Samples the empty-room noise floor for `calibration_sample` seconds (default
60 s). Sets each gate's move and still threshold to the **mean** energy seen at
that gate over the sample window.

- Lower thresholds → more sensitive, more likely to trigger on small signals.
- Vulnerable to transient noise (vibration, air movement) pulling the mean up.
- Good default for quiet, stable environments.

### Maximum
Same host-side sampling as Average, but sets thresholds to the **peak** energy
seen per gate.

- Higher thresholds → less sensitive, requires a stronger signal to trigger.
- More robust in environments with intermittent background noise.
- Less likely to false-positive; may miss weak still targets.

### Intelligent
Delegates entirely to firmware (requires v2.44+). The host sends
`CMD_AUTO_THRESHOLD` (0x0B) with a 10 s pre-sampling delay, then polls
`CMD_AUTO_THRESHOLD_QUERY` (0x1B) every 2 s. The firmware samples internally
for ~120 s (fixed, not user-configurable) and applies its own threshold
algorithm with undocumented internal headroom. On completion the module
restarts and thresholds are read back via `CMD_QUERY`.

- `calibration_sample` is ignored in this mode.
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
4. Press **Start Calibration** and leave the room.
5. Status shows `firmware: in progress` for ~120 s, then `firmware: success`.
   The module restarts automatically and gate thresholds update in HA.

## Protocol notes

- `CMD_AUTO_THRESHOLD` (0x0B): payload `[0x0A, 0x00]` (10 s firmware-internal
  pre-sampling delay, fixed). Config mode must be enabled before sending;
  disabled in the ACK handler.
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
