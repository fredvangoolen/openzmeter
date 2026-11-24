#!/usr/bin/env python3
"""Simulated Fluvius S1 meter on a pseudo terminal, to try s1monitor or the openZmeter S1 driver without a meter.

Prints the pseudo terminal to use as serial port, then streams S1 frames in real time until Ctrl-C (or -t seconds).
A pseudo terminal has no baud rate and no line errors, so wiring and speed problems can not be reproduced with it.
Linux only, no extra packages needed.
"""
import argparse, math, os, pty, time, tty

MODES = {
    "1p":     "single phase, 52 samples/period (like the example of the specification)",
    "3pn":    "three phases + neutral, unbalanced currents, 52 samples/period",
    "3p3900": "three phases without neutral, announces 3900 Hz but sends 3906.25 Hz, frequency field 0 (as a Sagemcom meter)",
    "noise":  "bytes that never form a valid frame",
    "stray":  "only two bytes, after 1.5 s",
    "silent": "nothing at all",
}

def x25(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0x8408 if crc & 1 else crc >> 1
    return crc ^ 0xFFFF

def volts(x): return (round(x / 0.025) & 0xFFFF).to_bytes(2, "big")
def amps(x):  return (round(x / 0.001) & 0xFFFFFF).to_bytes(3, "big")

HARMONICS = []   # (order, fraction of the fundamental), added to voltages and currents

def wave(x):
    return math.sin(x) + sum(k * math.sin(h * x) for h, k in HARMONICS)

def samples(mode, ph):
    """Info byte and measurement bytes for grid angle ph (radians)."""
    r2 = math.sqrt(2)
    if mode in ("1p", "noise"):
        return 0x0A, volts(231 * r2 * wave(ph)) + amps(5 * r2 * wave(ph - 0.2)) + bytes(13)
    currents = [6 * (1 + 0.3 * k) * r2 * wave(ph - 0.1 - k * 2 * math.pi / 3) for k in range(3)]
    data = b"".join(volts(230 * r2 * wave(ph - k * 2 * math.pi / 3)) + amps(currents[k]) for k in range(3))
    if mode == "3pn":
        return 0x1F, data + amps(-sum(currents))
    return 0x0D, data + bytes(3)

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter,
                                 epilog="modes:\n" + "\n".join("  %-8s %s" % m for m in MODES.items()))
    ap.add_argument("mode", nargs="?", default="1p", choices=MODES, metavar="mode", help="see below (default 1p)")
    ap.add_argument("-t", type=float, default=0, metavar="SECONDS", help="stop after SECONDS")
    ap.add_argument("-f", type=float, default=50.02, metavar="HZ", help="grid frequency (default 50.02)")
    ap.add_argument("--harmonics", default="", metavar="H:PCT,...", help="add harmonics, e.g. 3:3,5:4,7:2")
    ap.add_argument("--drop", type=int, default=0, metavar="N", help="leave out one frame every N")
    ap.add_argument("--corrupt", type=int, default=0, metavar="N", help="damage one frame every N")
    ap.add_argument("--link", metavar="PATH", help="also create a symbolic link to the pseudo terminal")
    args = ap.parse_args()
    for item in filter(None, args.harmonics.split(",")):
        order, pct = item.split(":")
        HARMONICS.append((int(order), float(pct) / 100))

    master, slave = pty.openpty()
    tty.setraw(slave)
    name = os.ttyname(slave)
    print(name, flush=True)
    if args.link:
        if os.path.islink(args.link):
            os.unlink(args.link)
        os.symlink(name, args.link)

    per_second = args.mode == "3p3900"
    rate = 3906.25 if per_second else 52 * args.f
    sampling = 39 if per_second else 52
    freq_field = 0 if per_second else round(args.f * 1000)
    step = 2 * math.pi * args.f / rate
    start, n = time.monotonic(), 0
    try:
        while not args.t or time.monotonic() - start < args.t:
            if args.mode in ("silent", "stray"):
                if args.mode == "stray" and n == 0 and time.monotonic() - start > 1.5:
                    os.write(master, b"\x00\xf0")
                    n = 1
                time.sleep(0.1)
                continue
            buf = bytearray()
            for _ in range(int(rate / 20)):
                info, data = samples(args.mode, (n * step) % (2 * math.pi))
                body = (b"\x80\x2b\xff\x03" + b"1SIM0000000001" + bytes([info, sampling])
                        + freq_field.to_bytes(2, "big") + bytes([n & 0xFF]) + data)
                frame = bytearray(b"\x7e" + body + x25(body).to_bytes(2, "little") + b"\x7e")
                n += 1
                if args.mode == "noise":
                    frame = bytes(b ^ 0xFF for b in frame)
                elif args.drop and n % args.drop == 0:
                    continue
                elif args.corrupt and n % args.corrupt == 0:
                    frame[30] ^= 0x10
                buf += frame
            view = memoryview(bytes(buf))
            while len(view):
                view = view[os.write(master, view):]
            ahead = n / rate - (time.monotonic() - start)
            if ahead > 0:
                time.sleep(ahead)
    except KeyboardInterrupt:
        pass
    finally:
        if args.link and os.path.islink(args.link):
            os.unlink(args.link)

if __name__ == "__main__":
    main()
