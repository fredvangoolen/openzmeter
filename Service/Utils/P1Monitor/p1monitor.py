#!/usr/bin/env python3
# This file is part of openZmeter, licensed under the GNU General Public License, version 3 or (at your option) any later
# version, without any warranty. See the LICENSE file. SPDX-License-Identifier: GPL-3.0-or-later
"""Standalone check of the P1 port of a Fluvius digital meter, and reference for openZmeter (S1) measurements.

Without options: shows voltage, current and active power per phase, once per telegram, and checks the telegram CRC.
With --compare:  records P1 telegrams and compares them with what openZmeter measured on S1 in the same period (read
                 from its database), including the energy registers of the meter.

The port is only read. No extra packages needed (Linux, Python 3.8 or newer). P1 does not provide frequency, reactive
power or harmonics, so those can not be checked with it.
"""
import argparse, errno, glob, json, math, os, re, select, shlex, statistics, subprocess, sys, termios, time, tty

# --- Telegrams --------------------------------------------------------------------------------------------------------
def crc16(data):
    """CRC16 of P1 telegrams (polynomial x16+x15+x2+1, LSB first, no inversion), from '/' up to and including '!'."""
    crc = 0
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if crc & 1 else crc >> 1
    return crc

FIELDS = {                                                  # name: OBIS codes of phase 1..3
    "v": ("32.7.0", "52.7.0", "72.7.0"), "i": ("31.7.0", "51.7.0", "71.7.0"),
    "imp": ("21.7.0", "41.7.0", "61.7.0"), "exp": ("22.7.0", "42.7.0", "62.7.0"),
}
LINE = re.compile(r"^1-0:(\d+\.\d+\.\d+)\(([-0-9.]+)\*([A-Za-z]+)\)\s*$", re.M)

def decode(text):
    """Values of one telegram: volts, amperes, watts (import - export) per phase and registers in Wh."""
    raw = {code: float(val) * (1000 if unit in ("kW", "kWh") else 1) for code, val, unit in LINE.findall(text)}
    r = {"phases": [n for n in range(3) if FIELDS["v"][n] in raw]}
    for name in ("v", "i"):
        r[name] = [raw.get(code, 0.0) for code in FIELDS[name]]
    r["p"] = [raw.get(FIELDS["imp"][n], 0.0) - raw.get(FIELDS["exp"][n], 0.0) for n in range(3)]
    r["total"] = raw.get("1.7.0", 0.0) - raw.get("2.7.0", 0.0)
    if not any(code in raw for code in FIELDS["imp"]):      # some single phase meters only send the total
        r["p"] = [r["total"], 0.0, 0.0]
    codes = [c for c in ("1.8.1", "1.8.2", "2.8.1", "2.8.2") if c in raw]
    r["energy"] = sum(raw[c] * (1 if c[0] == "1" else -1) for c in codes) if codes else None
    m = re.search(r"^0-0:1\.0\.0\((\d{12})([SW])\)", text, re.M)
    r["clock"] = m.group(1) if m else ""
    m = re.search(r"^0-0:96\.1\.1\(([0-9A-Fa-f]+)\)", text, re.M)
    r["id"] = bytes.fromhex(m.group(1)).decode("ascii", "replace") if m and len(m.group(1)) % 2 == 0 else ""
    return r

class Parser:
    """Extracts telegrams from the byte stream. Gives (arrival time of the first byte, values)."""
    def __init__(self):
        self.buf, self.start, self.bytes, self.ok, self.bad = bytearray(), 0.0, 0, 0, 0
    def push(self, data, now):
        out = []
        self.bytes += len(data)
        had = len(self.buf) > 0
        self.buf += data
        if not had:
            self.start = now
        while True:
            m = re.search(rb"!([0-9A-Fa-f]{4})\r?\n", self.buf)
            if not m:
                break
            body, crc = bytes(self.buf[:m.start() + 1]), int(m.group(1), 16)
            end = m.end()
            del m                                           # a match locks the buffer
            start, i = self.start, max([x.start() for x in re.finditer(rb"/[A-Za-z]{3}5", body)], default=-1)
            del self.buf[:end]
            self.start = now
            if i < 0:
                self.bad += self.ok > 0                     # started in the middle of the first telegram: not an error
                continue
            if crc16(body[i:]) != crc:
                self.bad += 1
                continue
            self.ok += 1
            out.append((start, decode(body[i:].decode("ascii", "replace").replace("\r", ""))))
        if len(self.buf) > 65536:
            del self.buf[:-4096]
        return out

def open_port(path):
    fd = os.open(path, os.O_RDONLY | os.O_NOCTTY | os.O_NONBLOCK | os.O_CLOEXEC)
    try:
        a = termios.tcgetattr(fd)
        a[0], a[1], a[3] = termios.IGNPAR, 0, 0
        a[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
        a[4] = a[5] = termios.B115200
        a[6][termios.VMIN], a[6][termios.VTIME] = 1, 0
        termios.tcsetattr(fd, termios.TCSANOW, a)
        termios.tcflush(fd, termios.TCIFLUSH)
    except termios.error as e:
        os.close(fd)
        raise OSError(e.args[0], e.args[1])
    return fd

def find_port():
    """The only USB serial adapter that is not the RS-422 one of the S1 port."""
    ports = [p for p in sorted(glob.glob("/dev/serial/by-id/*")) if "RS422" not in p.upper().replace("-", "")]
    if len(ports) != 1:
        sys.exit("Give the serial port of the P1 cable. Found: " + (", ".join(ports) or "none"))
    return ports[0]

def telegrams(fd, parser, seconds, idle):
    """Yields telegrams until 'seconds' passed (0: forever). Calls idle() about once per second."""
    end = time.monotonic() + seconds if seconds else math.inf
    tick = time.monotonic() + 1
    while time.monotonic() < end:
        ready, _, _ = select.select([fd], [], [], 0.2)
        if ready:
            try:
                data = os.read(fd, 4096)
            except BlockingIOError:
                data = b""
            except OSError as e:
                sys.exit("Read error: " + os.strerror(e.errno or errno.EIO))
            if not data and not os.isatty(fd):
                return
            yield from parser.push(data, time.time())
        if time.monotonic() >= tick:
            tick += 1
            idle()

class Hint:
    """Explains once why nothing valid is received. The meter sends a telegram every second (old meters every 10 s)."""
    def __init__(self, parser):
        self.parser, self.start, self.done = parser, time.monotonic(), False
    def __call__(self):
        if self.done or self.parser.ok or time.monotonic() - self.start < 12:
            return
        self.done = True
        if self.parser.bytes == 0:
            print("Nothing received. Check that the P1 port is activated (mijn.fluvius.be), that the cable is in the P1\n"
                  "socket, and that it connects the data request pin (2) to +5 V (pin 1).")
        else:
            print("%d bytes but no valid telegram (%d with wrong CRC). P1 is inverted 115200 8N1: use a P1 cable with an\n"
                  "inverting input, and check that no other program reads the same port."
                  % (self.parser.bytes, self.parser.bad))

# --- Comparison with openZmeter ---------------------------------------------------------------------------------------
def read_rows(psql, analyzer, start, end):
    """3 second rows of openZmeter: (start, end, V[], I[], P[]). The row time is the start of its last 200 ms block."""
    def query(sql):
        run = subprocess.run(shlex.split(psql) + ["-qtA", "-c", sql], capture_output=True, text=True, cwd="/")
        if run.returncode != 0:
            sys.exit("Can not read the openZmeter database: " + run.stderr.strip())
        return run.stdout.strip()
    if not analyzer:
        names = [n for n in query("SELECT nspname FROM pg_namespace WHERE nspname LIKE 'S1\\_%'").split()
                 if re.fullmatch(r"S1_\w+", n)]
        last = {n: int(query('SELECT COALESCE(MAX(sampletime), 0) FROM "%s".aggreg_3s' % n)) for n in names}
        if not last:
            sys.exit("No S1 analyzer found in the openZmeter database")
        analyzer = max(last, key=last.get)
    sql = ('SELECT COALESCE(json_agg(json_build_array(sampletime, rms_v, rms_i, active_power) ORDER BY sampletime), \'[]\') '
           'FROM "%s".aggreg_3s WHERE sampletime BETWEEN %d AND %d' % (analyzer, start * 1000, end * 1000))
    data = json.loads(query(sql))
    rows = []
    for n, (ts, v, i, p) in enumerate(data):
        length = (ts - data[n - 1][0]) / 1000 if n > 0 else 3.0
        end_row = ts / 1000 + 0.2
        rows.append((end_row - (length if length < 4 else 3.0), end_row, v, i, p))
    return analyzer, rows

def match(samples, rows, lag):
    """Pairs (row, average of the telegrams that arrived 'lag' seconds after the row's period)."""
    pairs, k = [], 0
    for row in rows:
        while k < len(samples) and samples[k][0] - lag < row[0]:
            k += 1
        n, part = k, []
        while n < len(samples) and samples[n][0] - lag < row[1]:
            part.append(samples[n][1])
            n += 1
        if len(part) >= 2:
            pairs.append((row, {q: [sum(s[q][ph] for s in part) / len(part) for ph in range(3)] for q in ("v", "i", "p")}))
    return pairs

def find_lag(samples, rows):
    """Delay of the telegrams, from the changes in total power. None if the load is too constant to tell."""
    if len(rows) < 20 or statistics.pstdev([sum(r[4]) for r in rows]) < 5:
        return None
    best = None
    for step in range(-12, 25):
        pairs = match(samples, rows, step / 4)
        if len(pairs) < len(rows) / 2:
            continue
        # spread of the difference: a constant offset must not influence the delay
        score = statistics.pstdev([sum(r[4]) - sum(s["p"]) for r, s in pairs])
        if best is None or score < best[0]:
            best = (score, step / 4)
    return best[1] if best else None

LIMITS = {"v": (0.5, 0.3), "i": (1.0, 0.02), "p": (1.0, 3.0)}   # accepted difference: percent, or absolute if larger

def compare(samples, rows, phases):
    """Returns (lag, fitted, results). results: (quantity, phase, pairs, P1 mean, oZ mean, difference, %, spread, ok)."""
    fitted = find_lag(samples, rows)
    lag = 1.0 if fitted is None else fitted
    pairs, results = match(samples, rows, lag), []
    if not pairs:
        return lag, fitted is not None, results
    for q, col in (("v", 2), ("i", 3), ("p", 4)):
        for ph in phases:
            ref = [s[q][ph] for _, s in pairs]
            oz = [r[col][ph] for r, _ in pairs]
            mr, mo = statistics.fmean(ref), statistics.fmean(oz)
            diff = mo - mr
            pct = 100 * diff / abs(mr) if mr else float("nan")
            ok = abs(diff) <= max(LIMITS[q][0] / 100 * abs(mr), LIMITS[q][1])
            results.append((q, ph, len(pairs), mr, mo, diff, pct, statistics.pstdev([a - b for a, b in zip(oz, ref)]), ok))
    return lag, fitted is not None, results

def compare_energy(samples, rows, lag):
    """(hours, register Wh, openZmeter Wh) between the first and last register change, None if not usable."""
    reg = [(t - lag, s["energy"]) for t, s in samples if s["energy"] is not None]
    edges = [reg[n] for n in range(1, len(reg)) if reg[n][1] != reg[n - 1][1]]   # exact moments: 1 Wh steps
    if len(edges) < 2:
        return None
    (t0, e0), (t1, e1) = edges[0], edges[-1]
    wh = covered = 0.0
    for start, end, _, _, p in rows:
        part = min(end, t1) - max(start, t0)
        if part > 0:
            wh += sum(p) * part / 3600
            covered += part
    if covered < 0.98 * (t1 - t0):
        return None
    return (t1 - t0) / 3600, e1 - e0, wh

def report(samples, rows, phases, analyzer):
    lag, fitted, results = compare(samples, rows, phases)
    if not results:
        print("No overlapping measurements found. Is the S1 device enabled, and is the clock synchronized?")
        return 1
    print("\nAnalyzer %s, %d periods of 3 s, P1 delay %.2f s (%s)\n" % (analyzer, results[0][2], lag,
          "fitted" if fitted else "assumed, load too constant"))
    part = 3 * results[0][2] / max(samples[-1][0] - samples[0][0], 1)
    if part < 0.95:
        print("openZmeter only has data for %.0f %% of the recording (restarted, or another analyzer selected meanwhile).\n" % (100 * part))
    print("%-10s %10s %10s %10s %8s %9s" % ("", "P1 (ref)", "openZmeter", "diff", "diff %", "spread"))
    names = {"v": ("V", "%.2f"), "i": ("I", "%.3f"), "p": ("P", "%.1f")}
    for q, ph, _, mr, mo, diff, pct, sd, ok in results:
        f = names[q][1]
        print("%-10s %10s %10s %10s %8s %9s  %s" % (names[q][0] + " L%d" % (ph + 1), f % mr, f % mo,
              ("%+" + f[1:]) % diff, "%+.2f" % pct, f % sd, "ok" if ok else "CHECK"))
    tr, to = (sum(r[k] for r in results if r[0] == "p") for k in (3, 4))
    print("%-10s %10.1f %10.1f %+10.1f %+8.2f" % ("P total", tr, to, to - tr, 100 * (to - tr) / abs(tr) if tr else 0))
    e = compare_energy(samples, rows, lag)
    if e and abs(e[1]) >= 20:
        print("\nEnergy over %.2f h: meter registers %.0f Wh, openZmeter %.1f Wh (%+.2f %%, +-%.2f %% from the 1 Wh steps)"
              % (e[0], e[1], e[2], 100 * (e[2] - e[1]) / abs(e[1]), 100 / abs(e[1])))
    else:
        print("\nEnergy: not enough change in the registers (or gaps) for a useful comparison, use a longer -t")
    print("P1 resolution: 0.1 V, 0.01 A, 1 W, 1 Wh. 'spread' is the standard deviation of the difference per period.")
    return 0 if all(r[8] for r in results) else 1

# --- Self test --------------------------------------------------------------------------------------------------------
def fake_telegram(v, i, p, wh, clock="260101120000W"):
    lines = ["/FLU5\\000000000_A", "", "0-0:96.1.4(50217)", "0-0:96.1.1(3153494D30303030303030303031)",
             "0-0:1.0.0(%s)" % clock, "1-0:1.8.1(%010.3f*kWh)" % (wh / 1000), "1-0:1.8.2(000001.000*kWh)",
             "1-0:2.8.1(000000.500*kWh)", "1-0:2.8.2(000000.000*kWh)",
             "0-0:98.1.0(1)(1-0:1.6.0)(1-0:1.6.0)(251101000000W)(251025181500S)(04.188*kW)",
             "1-0:1.7.0(%06.3f*kW)" % (max(sum(p), 0) / 1000), "1-0:2.7.0(%06.3f*kW)" % (max(-sum(p), 0) / 1000)]
    for n in range(3):
        lines += ["1-0:%d1.7.0(%06.3f*kW)" % (2 * n + 2, max(p[n], 0) / 1000), "1-0:%d2.7.0(%06.3f*kW)" % (2 * n + 2, max(-p[n], 0) / 1000),
                  "1-0:%d2.7.0(%05.1f*V)" % (2 * n + 3, v[n]), "1-0:%d1.7.0(%06.2f*A)" % (2 * n + 3, i[n])]
    body = ("\r\n".join(lines) + "\r\n!").encode()
    return body + b"%04X\r\n" % crc16(body)

def selftest():
    import random
    import pty
    checks = fails = 0
    def check(ok, what):
        nonlocal checks, fails
        checks += 1
        if not ok:
            fails += 1
            print("FAIL: " + what)
    check(crc16(b"123456789") == 0xBB3D, "CRC16 check value")
    t = fake_telegram([230.1, 231.2, 229.3], [1.5, 0.2, 7.25], [300, -45, 1500], 9170435)
    p = Parser()
    out = p.push(t, 10.0)
    check(len(out) == 1 and p.bad == 0, "valid telegram")
    d = out[0][1]
    check(d["v"] == [230.1, 231.2, 229.3] and d["i"] == [1.5, 0.2, 7.25], "voltage and current")
    check([round(x) for x in d["p"]] == [300, -45, 1500] and round(d["total"]) == 1755, "power with export")
    check(round(d["energy"]) == 9170435 + 1000 - 500 and d["id"] == "1SIM0000000001" and d["phases"] == [0, 1, 2], "registers and ID")
    stream = t[50:] + t * 5
    for size in (1, 7, 64, 100000):
        p = Parser()
        n = sum(len(p.push(stream[k:k + size], 0.0)) for k in range(0, len(stream), size))
        check(n == 5 and p.bad == 0, "stream in pieces of %d bytes" % size)
    for pos in range(0, len(t) - 7):                        # any single damaged byte
        x = bytearray(t)
        x[pos] ^= 0x04
        check(Parser().push(bytes(x), 0.0) == [], "damaged byte %d" % pos)
    p = Parser()
    cut = t[:400] + t                                       # interrupted telegram followed by a complete one
    check(len(p.push(cut, 0.0)) == 1, "interrupted telegram")
    # comparison: known errors and delay must be found back
    rnd = random.Random(1)
    load, level = [], 500.0
    for n in range(6200):                                   # 0.1 s steps
        if rnd.random() < 0.01:
            level = rnd.uniform(100, 2500)
        load.append(level)
    mean = lambda a, b: sum(load[a:b]) / (b - a)
    samples, wh = [], 5000.0
    for s in range(1, 610):
        w = mean(s * 10 - 10, s * 10)
        wh += w / 3600
        samples.append((1000 + s + 0.75, {"v": [230.0] * 3, "i": [w / 230, 0, 0], "p": [round(w), 0, 0], "energy": math.floor(wh)}))
    def rows(gain):
        return [(1000 + k / 10, 1003 + k / 10, [230.5] * 3, [mean(k, k + 30) / 230, 0, 0], [mean(k, k + 30) * gain, 0, 0]) for k in range(30, 6000, 30)]
    lag, fitted, res = compare(samples, rows(1.02), [0])
    by = {r[0]: r for r in res}
    check(fitted and abs(lag - 1.25) <= 0.75, "delay (%.2f)" % lag)
    check(abs(by["p"][6] - 2.0) < 0.3 and not by["p"][8], "2 %% power error (%.2f)" % by["p"][6])
    check(abs(by["v"][5] - 0.5) < 1e-6 and by["v"][8], "voltage difference within limit")
    e = compare_energy(samples, rows(1.02), lag)
    check(e is not None and abs(100 * (e[2] / e[1] - 1) - 2.0) < 1.5, "2 % energy error")
    check(compare([(t, dict(s, p=[300, 0, 0])) for t, s in samples], [(a, b, v, i, [300, 0, 0]) for a, b, v, i, _ in rows(1)], [0])[1] is False,
          "constant load: delay can not be fitted")
    # through a serial device
    master, slave = pty.openpty()
    tty.setraw(slave)
    fd, p = open_port(os.ttyname(slave)), Parser()
    os.write(master, t * 3)
    got = sum(1 for _ in telegrams(fd, p, 1.0, lambda: None))
    check(got == 3, "serial device (%d)" % got)
    print("%d checks, %d failures" % (checks, fails))
    return 1 if fails else 0

# --- Main -------------------------------------------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("port", nargs="?", help="serial port of the P1 cable (default: the only USB adapter that is not RS-422)")
    ap.add_argument("-t", type=float, metavar="SECONDS", help="stop after SECONDS (default: until Ctrl-C, --compare: 300)")
    ap.add_argument("--compare", action="store_true", help="compare with the openZmeter measurements of the same period")
    ap.add_argument("--analyzer", metavar="NAME", help="openZmeter analyzer (default: the S1 analyzer with the newest data)")
    ap.add_argument("--psql", default="sudo -u postgres psql -d openzmeter", metavar="CMD", help="command to query the openZmeter database (default: %(default)s)")
    ap.add_argument("--selftest", action="store_true", help="test this program, no meter needed")
    args = ap.parse_args()
    sys.stdout.reconfigure(line_buffering=True)
    if args.selftest:
        return selftest()
    if args.analyzer and not re.fullmatch(r"\w+", args.analyzer):
        ap.error("invalid analyzer name")
    port = args.port or find_port()
    try:
        fd = open_port(port)
    except OSError as e:
        extra = " (add your user to the 'dialout' group)" if e.errno == errno.EACCES else ""
        sys.exit("Can not open %s: %s%s" % (port, os.strerror(e.errno), extra))
    seconds = args.t if args.t is not None else (300 if args.compare else 0)
    print("Listening on %s at 115200 baud. %s" % (port, "Recording %.0f s, Ctrl-C to stop earlier." % seconds if seconds else "Ctrl-C to stop."))
    parser, samples, start = Parser(), [], time.monotonic()
    try:
        for arrival, s in telegrams(fd, parser, seconds, Hint(parser)):
            if not samples:
                print("Meter ID '%s', %d phase%s" % (s["id"], max(len(s["phases"]), 1), "s" if len(s["phases"]) > 1 else ""))
                if not args.compare:
                    print(" ".join(["%-8s" % "clock"] + ["%7s" % ("%s%d" % (q, n + 1)) for q in "VIP" for n in s["phases"]] + ["%8s" % "P total", "%13s" % "net Wh"]))
            samples.append((arrival, s))
            if not args.compare:
                c = s["clock"]
                print(" ".join(["%s:%s:%s" % (c[6:8], c[8:10], c[10:12])] + ["%7.1f" % s["v"][n] for n in s["phases"]] + ["%7.2f" % s["i"][n] for n in s["phases"]]
                               + ["%7.0f" % s["p"][n] for n in s["phases"]] + ["%8.0f" % s["total"], "%13.0f" % (s["energy"] or 0)]))
            elif len(samples) % 60 == 0:
                print("%d telegrams..." % len(samples))
    except KeyboardInterrupt:
        pass
    finally:
        os.close(fd)
    print("\nSummary: %.0f s, %d valid telegrams, %d errors" % (time.monotonic() - start, parser.ok, parser.bad))
    if not samples:
        return 1
    if args.compare:
        time.sleep(4)                                       # the last openZmeter row is stored up to 3 s later
        analyzer, rows = read_rows(args.psql, args.analyzer, samples[0][0] - 4, samples[-1][0] + 4)
        return report(samples, rows, samples[0][1]["phases"] or [0], analyzer)
    return 0

if __name__ == "__main__":
    sys.exit(main())
