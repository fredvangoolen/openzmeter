# S1 port driver (Fluvius digital meter)

Reads raw voltage/current samples from the S1 high speed port of Flemish (Fluvius) digital electricity meters
(e-MUCS H ed. 1.3, section 4) and feeds them to a standard `AnalyzerSoft`. The specification, as published on the
Fluvius website, is in [`Docs/e-mucs_h_ed_1_3.pdf`](../../../../Docs/e-mucs_h_ed_1_3.pdf).

## Hardware

* The S1 port must be **activated by Fluvius** (request it through the Fluvius customer portal). Without it the meter
  sends nothing.
* RJ12 connector on the meter: pin 3 = GND, pin 4 = BUS_M (RS-422 -), pin 5 = BUS_P (RS-422 +). Use a twisted pair
  for pins 4/5 and also connect GND.
* RS-422 at a fixed **2 Mbaud** 8N1, receive only. The receiving side must terminate the line (120 Ω between + and -,
  many adapters have a jumper/switch for it).
* Use a USB to RS-422 adapter that can run at exactly 2 Mbaud: FTDI FT232R/FT232H/FT2232H based (ex: FTDI
  USB-RS422-WE cable) or CP2102N + RS-422 receiver. The on-board UARTs of the NanoPi Air (H3) and Omega2 (MT7688) can
  not generate 2 Mbaud.

## Configuration

Device `S1` in *Settings » Devices*:

* **Serial port**: e.g. `/dev/ttyUSB0`. Prefer a stable `/dev/serial/by-id/...` path.
* **Mode**: `AUTO` picks the mode from the frame flags: single phase gives `1PHASE`, poly phase with neutral current
  gives `3PHASE+N`, and poly phase without it gives `3PHASE`. You can also force a mode.

The analyzer (`S1_1P`, `S1_3P` or `S1_3PN`) is created when the first valid frame arrives. The device dialog shows
the meter ID, sampling mode, frequency and frame counters (OK / CRC errors / lost / invalid).

## Notes / limitations

* Sample rate is 2-4 kHz, much lower than openZmeter capture boards: harmonics above the Nyquist limit (roughly the
  25th-40th harmonic, depending on the meter) are reported as 0.
* Meters sampling at a fixed number of samples per period: grid frequency is taken from the frequency field in each
  frame, and energy uses the effective samplerate (samples per period × frequency). Without this, frequency would
  always read nominal.
* Lost frames (sequence gaps up to 16) are filled with the last sample to keep timing; samples flagged invalid by
  the meter are replaced by the last valid sample.

## Raspberry Pi 4

`Packaging/raspberrypi/install.sh` builds openZmeter on the Pi (`Configs/raspberrypi4`) and installs it with a local
PostgreSQL database and a systemd service (`openzmeter.service`), with the S1 device preconfigured but disabled. Run
it from a clone of the repository: `sudo Service/Packaging/raspberrypi/install.sh`. A 2 GB board is enough. If the
build runs out of memory, enlarge the swap or use `--jobs 1`.
