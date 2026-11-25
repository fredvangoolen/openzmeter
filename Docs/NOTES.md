# Project notes

Notes for this fork of openZmeter: Fluvius S1 port support, running on a Raspberry Pi 4, and how data flows through
the software. User-facing instructions are in the [main README](../README.md) and the
[S1 driver README](../Service/Source/Devices/S1/README.md).

## 1. Software architecture and data flow

```mermaid
flowchart TB
  subgraph ROW1[" "]
    direction LR
    subgraph HW["Hardware"]
      METER["Fluvius digital meter<br/>S1 port, RJ12"]
      ADAPTER["USB to RS-422 adapter<br/>2 Mbaud"]
      BOARDS["openZmeter capture boards<br/>V1 / V2 / V3, USB"]
    end

    subgraph DEV["Device drivers"]
      S1SER["DeviceS1Serial<br/>raw 8N1 at 2 Mbaud"]
      S1PARSE["DeviceS1Parser<br/>frame sync,<br/>X.25 CRC, decode"]
      S1PROC["DeviceS1::ProcessFrame<br/>mode detection, gap fill,<br/>invalid sample hold"]
      S1RATE["DeviceS1Rate<br/>real sample rate from<br/>telegram count and clock"]
      OTHERDEV["DeviceV1 / V2 / V3<br/>DeviceVirtual (test signal)"]
      SAMPLES(["Samples"])
    end

    METER -- "45 byte frame per sample,<br/>2-4 kHz, ~90-180 kB/s" --> ADAPTER --> S1SER --> S1PARSE --> S1PROC
    S1PROC -- "telegram count" --> S1RATE -- "sample rate" --> S1PROC
    BOARDS --> OTHERDEV
    S1PROC -- "V, I per sample +<br/>meter frequency +<br/>sample rate" --> SAMPLES
    OTHERDEV -- "V, I per sample" --> SAMPLES
  end

  subgraph ROW2[" "]
    direction LR
    subgraph AN["AnalyzerSoft"]
      INPUT(["Samples"])
      ZC["ZeroCrossing<br/>band-pass filter,<br/>10 cycle blocks<br/>(~200 ms)"]
      QUEUE[("block queue")]
      CALC["Analyzer thread<br/>RMS, P, Q, energy,<br/>frequency, FFT:<br/>harmonics up to Nyquist"]
      A200["200 ms result"]
      EVT["Events<br/>dips, swells, RVC,<br/>interruptions,<br/>raw waveform snippets"]
      AGG["Aggregations<br/>3 s, 1 min, 10 min,<br/>15 min, 1 h"]
      ALARM["Alarms"]
      CH["Cyclic histogram, Jolt"]
      LIVE["Live consumers<br/>now values, wave stream"]
    end

    subgraph OUT["Storage and outputs"]
      DB[("PostgreSQL<br/>one schema per analyzer")]
      HTTP["HTTP server<br/>JSON / CBOR API"]
      WEB["Web UI (Vue)"]
      MQTT["MQTT broker"]
      TG["Telegram"]
      MODBUS["Modbus TCP"]
      SYNC["Sync to a remote<br/>openZmeter"]
      TARIFF["Tariffs and costs<br/>(15 min energy)"]
      WATCHDOG["openzmeter-watchdog<br/>(separate service)"]
    end

    INPUT --> ZC --> QUEUE --> CALC --> A200
    A200 --> EVT
    A200 --> AGG
    A200 --> CH
    A200 --> LIVE
    AGG --> ALARM
    EVT --> DB
    AGG --> DB
    CH --> DB
    ALARM --> DB
    ALARM --> MQTT
    ALARM --> TG
    AGG --> MQTT
    DB --> SYNC
    DB --> TARIFF
    DB --> HTTP
    LIVE --> HTTP
    TARIFF --> HTTP
    HTTP --> WEB
    WATCHDOG -. "checks every minute,<br/>restarts openZmeter" .-> HTTP
    CALC -. "latest values" .-> MODBUS
  end

  ROW1 == "samples to the analyzer<br/>of each device" ==> ROW2

  style ROW1 fill:none,stroke:none
  style ROW2 fill:none,stroke:none
```

Capture is drawn above, analysis and outputs below; the two "Samples" are the same stream. There is one AnalyzerSoft per
measured installation.

### Blocks

| Block | Source | Role |
|---|---|---|
| **DeviceS1Serial** | `Service/Source/Devices/S1/DeviceS1Serial.cpp` | Opens the adapter read-only, raw 8N1 at exactly 2 Mbaud (`termios2`). Refuses adapters that can't reach that speed. |
| **DeviceS1Parser** | `DeviceS1Frame.cpp` | Finds 45-byte frames in the byte stream, checks the X.25 checksum, decodes the meter ID, flags, sampling, frequency, sequence number, voltages (25 mV/step) and currents (1 mA/step). Shared with the `s1monitor` tool. |
| **DeviceS1::ProcessFrame** | `DeviceS1.cpp` | Creates the analyzer when the first valid frame arrives (`S1_1P`, `S1_3P` or `S1_3PN`, auto-detected or forced), by default called "Fluvius S1 meter" with time zone Europe/Brussels. Fills up to 16 lost frames and replaces samples flagged invalid with the last valid sample. Passes the meter's frequency and actual sample rate along with every sample. |
| **DeviceS1Rate** | `DeviceS1Rate.cpp` | Measures the real sample rate: straight line through the telegram count of the last 10 minutes against the system clock, one point per second. Needed because the announced rate has steps of 100 Hz. |
| **Other devices** | `Devices/V1`, `V2`, `V3`, `Virtual`, `Remote` | openZmeter's own USB capture boards (6–24 kHz), a test signal generator, and analyzers received from another openZmeter. |
| **ZeroCrossing** | `Analyzers/Soft/AnalyzerSoftZeroCrossing.cpp` | Cuts the sample stream into blocks of 10 cycles (12 at 60 Hz), about 200 ms, as IEC 61000-4-30 requires. Detects cycles on a band-pass filtered copy of voltage L1. |
| **Analyzer thread** | `AnalyzerSoft.cpp` | Per block: RMS, active/reactive/apparent power, power factor, energy, frequency, FFT harmonics (zero above the Nyquist limit), THD, unbalance. For S1, frequency is the one sent by the meter, or calculated with the measured sample rate if the meter sends 0 (as the tested one). Energy uses the measured sample rate (per-period meters: samples per period × frequency). |
| **Events** | `AnalyzerSoftEvents.cpp` | Detects dips, swells, interruptions and rapid voltage changes; stores RMS and raw waveform snippets around each event. |
| **Aggregations** | `AnalyzerSoftAggreg*.cpp` | Average the 200 ms results into 3 s, 1 min, 10 min, 15 min and 1 h values and store them. |
| **Alarms** | `AnalyzerSoftAlarms.cpp` | User-defined conditions on aggregates. Notify via MQTT and Telegram. |
| **Cyclic histogram, Jolt** | `AnalyzerSoftCyclicHistogram.cpp`, `AnalyzerSoftJoltAlgorithm.cpp` | Additional statistics computed on each block. |
| **Live consumers** | `ConsumerQueue` | Let web requests read the newest block (now values, raw wave stream) without blocking analysis. |
| **PostgreSQL** | `Database/` | One schema per analyzer; plain PostgreSQL, no extensions. |
| **HTTP server + web UI** | `HTTPServer/`, `Service/WEB` | JSON/CBOR API and the Vue web interface (port 80, or `--http-port`). |
| **MQTT, Telegram, Modbus TCP, Sync** | `AnalyzerSoftMQTT.cpp`, `System/` | Publishing to other systems; Sync pushes stored data to a remote openZmeter. |
| **Tariffs** | `Tariffs/` | Cost calculation from stored 15-minute energy. |
| **Watchdog** | `Service/Packaging/raspberrypi/openzmeter-watchdog.sh` | Separate service outside openZmeter: asks the web interface for an answer every 60 s and restarts openZmeter after three failures in a row. Raspberry Pi installer only. |

### Threads
Each device has its own capture thread and each analyzer its own analysis thread. Blocks are passed through a
queue. FFTs run in a task pool, one task per channel. The HTTP server, MQTT, Sync and database maintenance run on
their own threads. With the systemd service, capture and analysis threads get realtime priority (`CAP_SYS_NICE`;
checked on the Pi: round robin, priority 40); without it they fall back to `nice` levels.

## 2. Data rates and storage

The high-rate sample stream never reaches storage. It is processed in RAM block by block; only aggregated values
are stored.

| Data | Retention | Size (3 phases) |
|---|---|---|
| Raw samples (2–4 kHz, ~90–180 kB/s from the meter) | not stored | — |
| 3-second values (V, I, P, Q, f) | kept indefinitely | ~210 bytes/row, **~6 MB/day, ~2.2 GB/year** |
| 1 min, 10 min, 15 min, 1 h aggregates | kept indefinitely | together ~0.5 MB/day |
| Jolt statistics | kept indefinitely | ~0.5 MB/day |
| 1-minute harmonics | deleted after 1 day | ~3.5 MB, rolling |
| Events, with a short raw-waveform snippet | kept | only when an event happens |
| 200 ms values (`Save200MS`, **off by default**) | deleted after 1 hour | ~35 MB on disk, but **~1 GB/day of writes** |

Measured on the Pi (`S1_3PN` and `S1_3P`, together 32 hours, indexes included), except events and 200 ms values, which
are estimates from the table definitions in `Service/Resources/SQL/Database_Analyzer.sql`. The whole database was 23 MB
after those 32 hours. PostgreSQL's transaction log roughly doubles or triples the writes (estimate), so with default
settings the total is a few tens of MB per day.

Only the 1-minute harmonics and the 200 ms values are deleted automatically, and only for an analyzer that is running.
The 3-second table grows without limit.

### Storage media on a Raspberry Pi 4
- **SD card**: fine for testing and default settings; a "high endurance" card is safer for long-term use. Do not
  enable `Save200MS` on an SD card: it keeps little data but rewrites it constantly, which wears out flash.
- **USB memory stick**: not recommended; most sticks handle wear worse than a good SD card.
- **USB SSD**: best for permanent use. Use a 2.5" SATA SSD in a USB 3 enclosure on a blue USB 3 port, and keep the
  RS-422 adapter on a USB 2 port. Two options:
  1. Boot the whole system from the SSD (Pi 4 USB boot: copy the SD card with "SD Card Copier" or `rpi-clone`,
     update the bootloader with `sudo rpi-eeprom-update`, remove the SD card). Also moves other services' writes off
     the SD card.
  2. Keep booting from SD and move only PostgreSQL's data directory to the SSD. Less change to a running system, but
     the SD card remains a single point of failure.
- **Power**: the Pi 4's USB ports share about 1.2 A. A SATA SSD with the official 3 A supply is normally fine;
  NVMe enclosures may need a powered hub. Some cheap USB-SATA enclosures misbehave with UAS on the Pi: prefer ones
  known to work on a Pi.

## 3. Running on a Pi shared with other services

Tested target: Raspberry Pi 4 (2 GB), Debian 13 "trixie" 64-bit, GCC 14, also running Pi-hole v6.

- Pi-hole's web server already uses ports 80 and 443. Install with `--http-port 8080`; the installer refuses a port
  that another program uses.
- Running openZmeter is light (a few percent of one core, a small PostgreSQL). The build is the heavy part (web
  build plus link-time optimized link, several hundred MB of RAM for some minutes). On a Pi that serves DNS for the
  network, build at a quiet moment with `--jobs 1`, or build elsewhere and install with `--skip-build`.
- The clone (`~/openzmeter`, about 500 MB with the build) is only used for building and for the test programs. The only
  running copy is `/usr/local/sbin/openZmeter`; `git pull` alone does not change it.
- No compiled version is kept in the repository: it would grow every clone, go out of date unnoticed and only run on some
  systems (the one built here needs arm64 and glibc 2.38 or newer, so Debian 13, not 12). If binaries are ever wanted, a
  GitHub Release built from a tag is the better place.
- Check the S1 link with `Service/Utils/S1Monitor` before installing: it needs nothing beyond `g++` and does not
  affect other services.

## 4. Verification done so far

On an x86 machine, with simulated S1 frames through a pseudo-terminal and a temporary PostgreSQL 16:
- Single phase (230 V, 10 A, 0.3 rad, 50.1 Hz, 52 samples/period): 230.00 V, 10.00 A, 2197.29 W, 679.69 var,
  50.100 Hz, PF 0.9553, harmonics above the 26th exactly 0.
- Three phases + neutral (230 V, 6 / 7.8 / 9.6 A): exact per-phase values, neutral 3.118 A.
- Full optimized build with GCC 13 and GCC 14.
- Unit tests, repeated on 2026-11-25: `S1Test` 427 checks (decoder, parser, sample rate), `p1monitor.py --selftest` 621
  checks, no failures.

On a real meter (Raspberry Pi 4, FTDI USB-RS422-WE cable with its built-in 120 Ω termination, `s1monitor`):
- Sagemcom meter, poly phase 4 wires, no neutral current, **fixed per-second sampling**, announced as 3900 Hz, data
  format 0.
- 3905 valid frames/s, plausible values (about 230 V per phase, currents and power consistent with the household).
- The **frequency field is always 0** on this meter (e-MUCS H 4.2.3 defines it as the currently measured network
  frequency, without special meaning for 0). The analyzer then measures it from the zero crossings.
- The **real sample rate is 3906.25 Hz**, steady over 21 hours against an NTP synchronized clock. This agrees with the
  specification: the field has a resolution of 100 Hz (4.2.3) and one telegram is sent per sample (4.2.4). Earlier
  versions used the announced 3900 Hz, so frequency read 0.16 % low (49.913 Hz on average instead of 49.993 Hz) and
  live/MQTT active energy 0.16 % high. Voltage, current and power were not affected. Fixed by measuring the rate.
  Stored frequencies from before the fix (2026-11-23 23:56 to 2026-11-24 21:03, 25288 rows of 3 s in `S1_3PN` and 21 in
  `S1_3P`, plus their aggregates) were deliberately left as they are. The old graph therefore has a step of 0.08 Hz; the
  true value is the stored one times 1.0016.
- The analyzer was `S1_3PN` (mode forced) until 2026-11-24 21:26, and is `S1_3P` (`AUTO`) since then: this meter has no
  neutral current. The two histories are separate.
- **One corrupted frame per second** (CRC error + one lost frame, at a steady 1/s rate), most likely caused by the
  meter itself. About 0.03 % of the samples; the driver fills the gap with the previous sample.
- Harmonics up to about the 39th order are measurable at 3906 Hz.

Lesson from getting there: the meter's S1 socket is numbered from the right (pin 1 is the rightmost contact when
facing it). Counting from the left put GND on BUS- and the data wires on GND and an unused pin, which produced
CTS activity, garbled bytes or silence depending on the wiring. With the correct numbering it worked at once, without
an extra termination resistor.

openZmeter itself runs on the real meter since 2026-11-23, installed with the install script on a Pi 4 (about 34
minutes, up to 1.65 GB of memory used during the web build; about 4 % of one core and 25 MB when running).

Compared with the P1 port of the same meter (`p1monitor.py`, one hour on 2026-11-24, 1200 periods of 3 s, 3600 valid
telegrams, total load about 530 W): voltage +0.02 % on all phases, power L1 and L3 within 0.1 %, total power +0.15 %,
energy -0.19 % against the registers (528 Wh, resolution ±0.19 % over one hour). Voltage and current differences on most
phases are about half a step of the P1 display (0.1 V, 0.01 A), as if P1 cuts off digits. On the lightly loaded L2
openZmeter reads 0.016 A and 0.7 W more than P1 (0.24 A, 29 W). About 0.07 A RMS of components that P1 leaves out would
explain the current (it disappears at higher loads), but not the power. Not confirmed. An earlier run (160 periods) gave the
same L2 difference (0.015 A, 0.8 W). P1 has no frequency, reactive power or harmonics, so these could not be compared.

The watchdog was tested on the Pi by freezing openZmeter (with a shorter interval): three failed checks, then a restart.
It did not intervene during the one hour comparison or the following night.

Errors found by checking its results, not covered by the specification (S1 only delivers samples) and also present in
the original openZmeter for all devices:
- **THD** added the harmonics up linearly and divided by the peak value of the fundamental. It is now the root of the
  sum of squares, as in EN 50160 / IEC 61000-4-7. On this meter: 3.7 % stored, 2.1 % real (voltage L1). THD stored before
  the fix is not comparable with THD stored after it.
- **Reactive energy** (live values and MQTT) was always 0: calculated before reactive power was known. Fixed, but the
  live value has not been checked on the Pi yet.

Other changes to the original openZmeter: FFT bounds (it read outside its buffer and showed aliases at low sample rates),
build with GCC 13 and newer, and the disabled "Virtual" test device no longer creates an (empty) analyzer. Selecting
that one in the web interface instead of the meter showed empty pages.

## 5. Open items

- Stored voltage and current harmonics are peak values, not RMS (324 V for a fundamental of 229 V). The EN 50160
  harmonics check in `AnalyzerSoftAggreg1H.cpp` compares them in a way that looks wrong as well. Not changed yet.
- Seen once on x86 (not reproduced in about 340 further starts, cause unknown): openZmeter hung during startup, the main
  thread and the S1 thread both waiting for a lock. systemd does not notice this: the service stays active but the web
  interface does not answer. `systemctl restart openzmeter` solves it; `openzmeter-watchdog.service` (installed by the
  Raspberry Pi installer) does this after three failed checks, one per minute. It only notices that the web interface
  does not answer, not that measurements stop.
- Look into the one corrupted frame per second (raw capture around the bad frame).
- Explain the small difference on L2 with P1 (see section 4).
- The log shows "Operation not permitted" at startup (`TaskPool.cpp:46`, raising the realtime limit). Realtime priority
  works anyway, but `TaskPool` then skips the periodic task that removes idle worker threads. No problem seen
  (12 workers after 10 hours). `LimitRTPRIO=99` in the service would probably solve it. Not tried.
- Optional installer `--data-dir` to put PostgreSQL's data on an SSD while booting from SD.
- Existing behavior, not changed: openZmeter retries the database forever at startup, and stopping it during that
  loop crashes it; the 3-second table has no automatic cleanup.
