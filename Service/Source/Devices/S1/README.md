# S1 port driver (Fluvius digital meter)

Reads raw voltage/current samples from the S1 high speed port of Flemish (Fluvius) digital electricity meters
(e-MUCS H ed. 1.3, section 4) and feeds them to a standard `AnalyzerSoft`. The specification is published by
[Fluvius](https://www.fluvius.be/sites/fluvius/files/2019-12/e-mucs_h_ed_1_3.pdf) (copy in
[`Docs/e-mucs_h_ed_1_3.pdf`](../../../../Docs/e-mucs_h_ed_1_3.pdf)).

## Hardware

* The S1 port must be **activated by Fluvius** (request it through the Fluvius customer portal). Without it the meter
  sends nothing.
* RJ12 connector on the meter: pin 3 = GND, pin 4 = BUS_M (RS-422 -), pin 5 = BUS_P (RS-422 +). Use a twisted pair
  for pins 4/5 and also connect GND.
* RS-422 at a fixed **2 Mbaud** 8N1, receive only. The receiving side must terminate the line (120 Ω between + and -).
  Check whether your adapter already has it: the FTDI USB-RS422-WE cable has it built in, others use a jumper or need
  an external resistor. Do not add a second one: two in parallel (~55 Ω) make the meter drive twice the load.
* Use a USB to RS-422 adapter that can run at exactly 2 Mbaud: FTDI FT232R/FT232H/FT2232H based (ex: FTDI
  USB-RS422-WE cable) or CP2102N + RS-422 receiver. The on-board UARTs of the NanoPi Air (H3) and Omega2 (MT7688) can
  not generate 2 Mbaud.

### Wiring an FTDI USB-RS422-WE cable

Wire colours from FTDI's datasheet (DS_USB_RS422_CABLES, FT_000116 version 1.3, table 5.1). The cable is
wire-ended and supports up to 3 Mbaud. It **has a built-in 120 Ω termination** across RXD+/RXD- (R1 in the
schematic, figure 7.1), so no external resistor is needed. To check it, unplug the cable from the meter and measure
between Yellow and White: about 120 Ω, whether or not the cable is plugged into USB.

| Meter RJ12 pin | S1 signal | Cable wire | Cable signal |
|---|---|---|---|
| 3 | GND | **Black** | GND |
| 4 | BUS_M (RS-422 -) | **White** | RXD- (A), input |
| 5 | BUS_P (RS-422 +) | **Yellow** | RXD+ (B), input |

Leave all other wires unconnected, each insulated separately:

| Cable wire | Cable signal | Why not connect it |
|---|---|---|
| Red, Orange | TXD-, TXD+ | **Outputs**: the cable drives them. Connected to the meter they would drive against the meter's own output. |
| Green, Blue | RTS+, RTS- | **Outputs**, same reason. |
| Brown, Grey | CTS+, CTS- | Inputs for flow control, not data. A meter connected here shows no data at all (the line activity is only visible as CTS changes). |

* Use a twisted pair for pins 4/5 between the meter and the cable.
* **Check the pin numbering on the meter itself.** On the Sagemcom meter tested here, the S1 socket is numbered
  from the right: facing the socket, pin 1 is the rightmost contact. Counting from the wrong side puts GND on BUS-
  and the data pins on GND and an unused pin. Typical symptoms: no data at all, activity only on CTS, or bytes that
  never form a valid frame. Flat RJ12 cords are also mirrored end to end, so check with a continuity tester from the
  plug that goes into the meter.
* If `s1monitor` receives bytes but no valid frames, swap Yellow and White (RS-422 A/B naming is not consistent
  between vendors).

## Checking the connection

Before installing openZmeter, check the link with the standalone monitor in `Service/Utils/S1Monitor`. It has no
dependencies beyond `g++`, and uses the same frame decoder as the driver:

```
cd Service/Utils/S1Monitor && make
./s1monitor /dev/serial/by-id/usb-FTDI_...   # Ctrl-C to stop, -t 60 to stop after 60 s, -x for a raw hex dump
```

Every second it prints frames/s (this should match the meter sampling rate), frequency, CRC errors, lost frames,
RMS voltage and current, and active power per phase. If no valid frames arrive after 3 s, it diagnoses the line from
the received byte rate and the serial port's line counters (framing errors, breaks, CTS changes):

* **No signal** (no or only a few stray bytes): the meter is not sending (S1 port not activated) or the wiring does not
  reach the receive pair. A multimeter between RXD+ and RXD- tells which: ~0 V means the meter does not drive the line.
* **Bytes but no valid frames**: a distorted or weak signal, wrong pins, RXD+/RXD- swapped, an adapter not at exactly
  2 Mbaud, or (with adapters without built-in termination) a missing 120 Ω termination: an unterminated line can pick
  up interference that looks like data. If the bytes disappear when the line is terminated, the meter's signal is too
  weak to drive a normal load: check it with an oscilloscope (it should stay above 2 V differential with 120 Ω).
* **Framing errors or breaks only**: RXD+/RXD- swapped, missing termination or wrong speed.
* **Changes on CTS only**: the meter is wired to the cable's CTS pair instead of RXD.

## Tests and simulated meter

`Service/Utils/S1Test` works without a meter, database or openZmeter build:

* `make test` runs the unit tests of the sample rate measurement, frame decoder and stream parser (specification example, sign and range of the
  values, every possible single bit error, split and shared-flag streams, corrupted and truncated frames).
* `./s1sim.py` simulates a meter on a pseudo terminal and prints its path, to use as serial port for `s1monitor` or the
  S1 device (the openZmeter service user needs access to it). `./s1sim.py 3p3900 --corrupt 3906` resembles the tested
  Sagemcom meter; `--help` lists all modes. Baud rate, wiring and line errors can not be simulated this way.

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
* Sample rate: the meter announces it in steps of 100 Hz (e-MUCS H 4.2.3), too coarse to measure frequency and energy.
  As it sends exactly one telegram per sample (4.2.4), the driver measures the real rate by counting telegrams against
  the system clock (`DeviceS1Rate`). The clock must be synchronized (NTP). The announced rate is used for the first
  20 seconds, until the measured one is known.
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

The Pi can be shared with other services. If something else already uses port 80 (Pi-hole's web interface, for
example), use `--http-port 8080`; the installer refuses to continue when the chosen port is taken. The build is the
heaviest part (several hundred MB of RAM for some minutes): on a Pi that serves DNS for the network, run it at a
quiet moment with `--jobs 1`.
