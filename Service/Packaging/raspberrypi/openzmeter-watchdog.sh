#!/usr/bin/env bash
# ----------------------------------------------------------------------------------------------------------------------
# Restarts openZmeter when its web interface stops answering. systemd only notices when the process ends, not when it
# hangs. Nothing is done while the service is stopped or (re)starting, shortly after its start, or without database.
# Runs as openzmeter-watchdog.service. Messages: journalctl -u openzmeter-watchdog
# ----------------------------------------------------------------------------------------------------------------------
set -u
INTERVAL="${INTERVAL:-60}"        # seconds between checks
FAILURES="${FAILURES:-3}"         # failed checks in a row before a restart
GRACE="${GRACE:-180}"             # seconds after a start without checks
MAXTIME="${MAXTIME:-15}"          # seconds to wait for an answer
CONF="${CONF:-/etc/openzmeter/openZmeter.conf}"
UNIT="openzmeter.service"

web_url() {
  python3 -I - "${CONF}" <<'PY' 2>/dev/null || echo "http://127.0.0.1:80/"
import json, sys
c = json.load(open(sys.argv[1])).get("HTTPServer", {})
print("http://127.0.0.1:%d%s" % (c.get("Port", 80), c.get("RelativePath", "/") or "/"))
PY
}
running_for() {                   # seconds since the service became active
  local since now
  since=$(systemctl show -p ActiveEnterTimestampMonotonic --value "${UNIT}")
  now=$(cut -d. -f1 /proc/uptime)
  echo $(( now - ${since:-0} / 1000000 ))
}

failed=0
while sleep "${INTERVAL}"; do
  if [ "$(systemctl is-active "${UNIT}")" != "active" ] || ! systemctl is-active --quiet postgresql.service || [ "$(running_for)" -lt "${GRACE}" ]; then
    failed=0
    continue
  fi
  if curl -s -f -o /dev/null --max-time "${MAXTIME}" "$(web_url)"; then
    [ "${failed}" -gt 0 ] && echo "openZmeter answers again"
    failed=0
    continue
  fi
  failed=$(( failed + 1 ))
  echo "openZmeter does not answer (${failed} of ${FAILURES})"
  if [ "${failed}" -ge "${FAILURES}" ]; then
    echo "Restarting ${UNIT}"
    systemctl restart "${UNIT}"
    failed=0
  fi
done
