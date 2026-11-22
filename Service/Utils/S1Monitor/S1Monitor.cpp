// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
// s1monitor: standalone check of a Fluvius S1 port connection (no database, no web server, no openZmeter install).
// Opens the RS-422 adapter at 2 Mbaud, decodes frames with the same code as the openZmeter S1 driver and prints once
// per second: frame rate, CRC errors, lost frames, meter info, frequency, RMS voltage/current and active power.
// ---------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include "DeviceS1Frame.h"
#include "DeviceS1Serial.h"
// ---------------------------------------------------------------------------------------------------------------------
static volatile bool Terminate = false;
static void OnSignal(int) { Terminate = true; }
static double Now() {
  timespec TS;
  clock_gettime(CLOCK_MONOTONIC, &TS);
  return TS.tv_sec + TS.tv_nsec / 1e9;
}
// Kernel line counters of the serial port (not available on every device, e.g. pseudo terminals)
static bool LineCounters(int fd, serial_icounter_struct &cnt) {
  memset(&cnt, 0, sizeof(cnt));
  return ioctl(fd, TIOCGICOUNT, &cnt) == 0;
}
// An active S1 port sends one 45 byte frame per sample, at least ~2 kHz: ~90000 bytes/s or more
static const double MIN_STREAM_RATE = 1000.0;  //Below this, received bytes are treated as stray (noise, plugging)
// Explain why no valid frame was received, from the received byte rate and the line counter changes
static void Diagnose(double bytesPerSec, bool haveCounters, const serial_icounter_struct &d) {
  if(bytesPerSec >= MIN_STREAM_RATE) {
    printf("Receiving %.0f bytes/s but no valid frames. Likely causes, most likely first:\n"
           "  1. No 120 ohm termination between RXD+ and RXD-: an unterminated line picks up interference that\n"
           "     looks like data. Fit the resistor first; if the data then stops, the meter is not sending.\n"
           "  2. Wrong pins: check RJ12 pins 3/4/5 with a continuity tester (flat RJ12 cords are mirrored).\n"
           "  3. RXD+ and RXD- swapped (RS-422 A/B naming differs between vendors).\n"
           "  4. The adapter is not running at exactly 2 Mbaud.\n"
           "  Use -x to see the raw bytes.\n", bytesPerSec);
  } else if(haveCounters && ((d.frame + d.brk + d.parity) > 100)) {
    printf("Line activity, but the bytes are rejected (%d framing errors, %d breaks in 3 s): RXD+ and RXD- are\n"
           "probably swapped, or the line is not terminated (120 ohm), or the speed is wrong.\n", d.frame, d.brk);
  } else if(haveCounters && (d.cts > 100)) {
    printf("No data on RXD, but %d changes on the CTS input in 3 s: the meter wires are probably on the cable's\n"
           "CTS+/CTS- pair (FTDI USB-RS422-WE: brown/grey) instead of RXD+/RXD- (yellow/white).\n", d.cts);
  } else {
    printf("No signal%s. The meter is not sending, or the wiring does not reach the receive pair:\n"
           "  - Check that Fluvius has activated the S1 port (P1 working does not mean S1 is on).\n"
           "  - Check RJ12 pins 3 (GND), 4 (BUS-), 5 (BUS+) with a continuity tester.\n"
           "  - With the cable plugged in, measure DC volts between RXD+ and RXD- (FTDI: yellow and white):\n"
           "    ~0 V means the meter does not drive the line, +2 V or more means it does.\n",
           (bytesPerSec > 0) ? " (only a few stray bytes)" : "");
  }
}
static void Usage(const char *name) {
  printf("Usage: %s [options] [port]\n\n"
         "  port            Serial device of the RS-422 adapter (default /dev/ttyUSB0)\n"
         "  -t SECONDS      Stop after SECONDS (default: run until Ctrl-C)\n"
         "  -x              Hex dump the first 256 received bytes (wiring/baudrate debugging)\n"
         "  -h              This help\n\n"
         "Exit code: 0 if valid frames were received, 1 otherwise.\n", name);
}
// ---------------------------------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
  const char *Port     = "/dev/ttyUSB0";
  double      Duration = 0;
  bool        HexDump  = false;
  int         Opt;
  while((Opt = getopt(argc, argv, "t:xh")) != -1) {
    if(Opt == 't') Duration = atof(optarg);
    else if(Opt == 'x') HexDump = true;
    else { Usage(argv[0]); return (Opt == 'h') ? 0 : 2; }
  }
  if(optind < argc) Port = argv[optind];
  signal(SIGINT, OnSignal);
  signal(SIGTERM, OnSignal);
  setvbuf(stdout, NULL, _IOLBF, 0);

  int FD = DeviceS1Serial::Open(Port, 2000000, 2);
  if(FD < 0) {
    int Err = errno;
    fprintf(stderr, "Can not open %s at 2000000 baud: %s\n", Port, strerror(Err));
    if(Err == EINVAL) fprintf(stderr, "The adapter does not support exactly 2 Mbaud (use an FTDI FT232R/FT232H or CP2102N based adapter).\n");
    if(Err == EACCES) fprintf(stderr, "Permission denied: add your user to the 'dialout' group (then log in again) or use sudo.\n");
    return 1;
  }
  printf("Listening on %s at 2 Mbaud. Ctrl-C to stop.\n", Port);

  DeviceS1Parser Parser;
  // Per interval accumulators
  uint64_t Bytes = 0, Frames = 0, Lost = 0, Invalid = 0;
  double   SumV2[3] = {0}, SumI2[4] = {0}, SumP[3] = {0};
  // Totals
  uint64_t TotBytes = 0, TotLost = 0, TotInvalid = 0;
  bool     HaveSeq = false, HaveInfo = false;
  uint8_t  LastSeq = 0;
  DeviceS1Frame::Frame_t Last, Info;
  memset(&Last, 0, sizeof(Last));
  memset(&Info, 0, sizeof(Info));
  serial_icounter_struct LineStart, LineNow;
  bool     HaveCounters = LineCounters(FD, LineStart);
  int      Dumped = 0;
  double   Start = Now(), Tick = Start, LastData = Start;
  bool     HintShown = false;

  auto OnFrame = [&](const DeviceS1Frame::Frame_t &f) {
    if(HaveSeq) {
      uint8_t Gap = f.Sequence - LastSeq - 1;
      Lost += Gap;
    }
    LastSeq = f.Sequence;
    HaveSeq = true;
    if(!f.Valid) Invalid++;
    for(int n = 0; n < 3; n++) {
      SumV2[n] += f.Voltage[n] * f.Voltage[n];
      SumP[n]  += f.Voltage[n] * f.Current[n];
    }
    for(int n = 0; n < 4; n++) SumI2[n] += f.Current[n] * f.Current[n];
    Frames++;
    Last = f;
  };

  while(!Terminate) {
    uint8_t Raw[4096];
    ssize_t Len = read(FD, Raw, sizeof(Raw));
    double  T   = Now();
    if(Len < 0) {
      if(errno == EINTR) continue;
      fprintf(stderr, "Read error: %s\n", strerror(errno));
      break;
    }
    if(Len > 0) {
      LastData = T;
      Bytes += Len;
      if(HexDump && (Dumped < 256)) {
        for(ssize_t n = 0; (n < Len) && (Dumped < 256); n++, Dumped++) printf("%02X%s", Raw[n], ((Dumped % 32) == 31) ? "\n" : " ");
        if(Dumped >= 256) printf("\n");
      }
      Parser.Push(Raw, Len, OnFrame);
    }
    if((T - Tick) >= 1.0) {
      double Elapsed = T - Tick;
      if(Frames > 0) {
        bool InfoChanged = !HaveInfo || (Info.PolyPhase != Last.PolyPhase) || (Info.PerPeriod != Last.PerPeriod) || (Info.Sampling != Last.Sampling) || (Info.Neutral != Last.Neutral) || strcmp(Info.MeterID, Last.MeterID);
        if(InfoChanged) {
          Info = Last;
          HaveInfo = true;
          printf("\nMeter ID '%s', %s%s, %s, sampling %d %s, data format version %u\n", Info.MeterID[0] ? Info.MeterID : "(empty)",
                 Info.PolyPhase ? "poly phase" : "single phase", Info.PolyPhase ? (Info.FourWires ? " 4 wires" : " 3 wires") : "",
                 Info.Neutral ? "neutral current provided" : "no neutral current",
                 Info.PerPeriod ? Info.Sampling : Info.Sampling * 100, Info.PerPeriod ? "samples/period" : "Hz", Info.Version);
          if(Info.PolyPhase)
            printf("%8s %7s %6s %6s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s\n", "frames/s", "freq", "crc", "lost", "V1", "V2", "V3", "I1", "I2", "I3", "IN", "P1", "P2", "P3");
          else
            printf("%8s %7s %6s %6s %8s %8s %8s\n", "frames/s", "freq", "crc", "lost", "V", "I", "P");
        }
        double V[3], I[4], P[3];
        for(int n = 0; n < 3; n++) { V[n] = sqrt(SumV2[n] / Frames); P[n] = SumP[n] / Frames; }
        for(int n = 0; n < 4; n++) I[n] = sqrt(SumI2[n] / Frames);
        static uint64_t PrevCRC = 0;
        uint64_t CRC = Parser.CRCErrors() - PrevCRC;
        PrevCRC = Parser.CRCErrors();
        if(Info.PolyPhase)
          printf("%8.0f %7.3f %6llu %6llu %8.2f %8.2f %8.2f %8.3f %8.3f %8.3f %8.3f %8.1f %8.1f %8.1f%s\n", Frames / Elapsed, Last.Frequency,
                 (unsigned long long)CRC, (unsigned long long)Lost, V[0], V[1], V[2], I[0], I[1], I[2], I[3], P[0], P[1], P[2], Invalid ? "  (invalid samples)" : "");
        else
          printf("%8.0f %7.3f %6llu %6llu %8.2f %8.3f %8.1f%s\n", Frames / Elapsed, Last.Frequency,
                 (unsigned long long)CRC, (unsigned long long)Lost, V[0], I[0], P[0], Invalid ? "  (invalid samples)" : "");
      } else if(!HintShown && ((T - Start) >= 3.0)) {
        HintShown = true;
        serial_icounter_struct Delta;
        memset(&Delta, 0, sizeof(Delta));
        bool Counters = HaveCounters && LineCounters(FD, LineNow);
        if(Counters) {
          Delta.frame  = LineNow.frame  - LineStart.frame;
          Delta.brk    = LineNow.brk    - LineStart.brk;
          Delta.parity = LineNow.parity - LineStart.parity;
          Delta.cts    = LineNow.cts    - LineStart.cts;
        }
        Diagnose((TotBytes + Bytes) / (T - Start), Counters, Delta);
      }
      TotBytes += Bytes; TotLost += Lost; TotInvalid += Invalid;
      Bytes = Frames = Lost = Invalid = 0;
      memset(SumV2, 0, sizeof(SumV2)); memset(SumI2, 0, sizeof(SumI2)); memset(SumP, 0, sizeof(SumP));
      Tick = T;
      if(HaveInfo && ((T - LastData) > 3.0)) printf("No data for %.0f s\n", T - LastData);
    }
    if((Duration > 0) && ((T - Start) >= Duration)) break;
  }
  bool HaveFinal = HaveCounters && LineCounters(FD, LineNow);    //Read before closing the port
  close(FD);
  TotBytes += Bytes; TotLost += Lost; TotInvalid += Invalid;
  printf("\nSummary: %.0f s, %llu bytes, %llu valid frames, %llu CRC errors, %llu discarded bytes, %llu lost frames, %llu invalid samples\n",
         Now() - Start, (unsigned long long)TotBytes, (unsigned long long)Parser.Frames(), (unsigned long long)Parser.CRCErrors(),
         (unsigned long long)Parser.Discarded(), (unsigned long long)TotLost, (unsigned long long)TotInvalid);
  if(HaveFinal)
    printf("Line counters: %d framing errors, %d breaks, %d overruns, %d CTS changes\n", LineNow.frame - LineStart.frame,
           LineNow.brk - LineStart.brk, (LineNow.overrun - LineStart.overrun) + (LineNow.buf_overrun - LineStart.buf_overrun), LineNow.cts - LineStart.cts);
  return (Parser.Frames() > 0) ? 0 : 1;
}
// ---------------------------------------------------------------------------------------------------------------------
