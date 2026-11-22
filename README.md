Description
===============

openzmeter (oZm) is an Open Source system designed for power quality analysis and smart energy metering in power networks.
It has been designed to comply with international standards IEC 61000-4-30 and EN 50160. It is able to measure RMS voltages with an accuracy up to 0.1%, frequency up to 10 mHz (between 42.5 and 57.5 Hz) and RMS currents according to different sensor probes (the basic version can handle up to 400 V and 50 A RMS using an onboard Hall effect sensor with a precision of 1%).

See http://www.openzmeter.com

Fluvius digital meter (S1 port) support
===============

This fork adds support for the **S1 port** of the digital electricity meters installed by Fluvius in Flanders
(Belgium). Instead of openZmeter's own capture hardware, the meter itself provides the measurements: it continuously
streams instantaneous voltage and current samples of all phases (and the neutral, if the meter provides it), at
2 to 4 kHz. openZmeter analyzes these samples like those from its own hardware, giving RMS values, active/reactive
power and energy, frequency, harmonics, events, alarms, cyclic histograms, MQTT and tariffs.

* **Specification**: e-MUCS H ed. 1.3, section 4
  ([Fluvius](https://www.fluvius.be/sites/fluvius/files/2019-12/e-mucs_h_ed_1_3.pdf), copy in
  [`Docs/e-mucs_h_ed_1_3.pdf`](Docs/e-mucs_h_ed_1_3.pdf)).
* **Activation**: the S1 port is off by default. Ask Fluvius to activate it for your meter.
* **Hardware**: a USB to RS-422 adapter that can receive at exactly 2 Mbaud (e.g. FTDI FT232R/FT232H based) connected
  to the RJ12 S1 socket of the meter: pin 3 = GND, pin 4 = BUS- and pin 5 = BUS+. The line needs one 120 Ω termination
  on the adapter side (built into the FTDI USB-RS422-WE cable). Wire colours for the FTDI USB-RS422-WE cable are in the
  [S1 driver README](Service/Source/Devices/S1/README.md#wiring-an-ftdi-usb-rs422-we-cable).
* **Check the connection first**: [`Service/Utils/S1Monitor`](Service/Utils/S1Monitor) is a small standalone tool
  (only needs `g++`). It shows frame rate, errors, meter info, frequency, RMS values and power every second, and
  explains what is wrong when nothing valid arrives. Build it with `make` and run `./s1monitor /dev/ttyUSB0`.
* **Single board computer**: a Raspberry Pi 4 (2 GB is enough) works well. Run
  `sudo Service/Packaging/raspberrypi/install.sh` to build and install openZmeter with a local PostgreSQL database
  and a systemd service. Then enable the device under *Settings » Devices » S1*. If port 80 is already taken (for
  example by Pi-hole), add `--http-port 8080`.
* **Limitations**: because of the lower sample rate, harmonics above roughly the 25th to 40th order (depending on the
  meter) are reported as 0.

Details are in the [S1 driver README](Service/Source/Devices/S1/README.md). Architecture, data flow and storage
notes are in [`Docs/NOTES.md`](Docs/NOTES.md).
