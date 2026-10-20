# SLC Execution Gate — ESP32-S3 prototype

This repository is being repurposed on branch `feat/slc-esp32-execution-gate` for an experimental software SLC (Semantic Logic Core) execution gate targeting the Waveshare ESP32-S3-LCD-1.47 (SKU 28317).

## Scope of this first milestone

- Build firmware for ESP32-S3 with ESP-IDF in GitHub Actions, avoiding a local ESP toolchain on macOS.
- Exercise a small, deterministic COMMIT/REJECT gate against a harmless virtual effect.
- Show a visual hardware smoke test on the ST7789 LCD.
- Run host-side C tests against the same `slc_gate.c` implementation used by the firmware.

## Important security boundary

This is an experimental prototype, not an EABC conformance claim and not a production safety controller.

In this first milestone, `signature_valid` and `issuer_authorized` are inputs to the gate representing results that a future EGA/crypto adapter must establish. They are **not cryptographically verified by this implementation**. Never connect this prototype to a real actuator or use it to control a physical process.

The gate checks:
- authorization/signature and issuer-policy result flags;
- action and target binding;
- payload-digest binding;
- epoch and validity interval;
- replay of a bounded set of nonces;
- that the virtual effect callback is called only after the checks pass.

Replay protection is a bounded RAM cache for demonstration. It is not persistent across reboot and is not a substitute for a production replay/nonce design.

## Hardware

Board: Waveshare ESP32-S3-LCD-1.47, not the 1.47B variant.

According to the Waveshare model-specific documentation:
- LCD: ST7789, 172 × 320
- MOSI GPIO45, SCLK GPIO40, CS GPIO42, DC GPIO41, RST GPIO39, BL GPIO48

## Build

GitHub Actions builds with Espressif ESP-IDF v5.3.2 for `esp32s3`. The workflow also compiles and runs host-side tests for the gate core.

The CI artifact is a build output, not yet validated on physical hardware.

## Local host test

On a machine with a C compiler:

```sh
cc -std=c11 -Wall -Wextra -Werror -I main main/slc_gate.c tests/test_slc_gate.c -o /tmp/test_slc_gate
/tmp/test_slc_gate
```

## First hardware validation

After a successful CI build, download the artifact and inspect the generated flash arguments before flashing. Preserve the current flash dump. Do not erase the chip. Hardware flashing and LCD verification are separate steps and require explicit review of the produced artifact and flash offsets.
