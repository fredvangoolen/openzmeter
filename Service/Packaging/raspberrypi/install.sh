#!/usr/bin/env bash
# ----------------------------------------------------------------------------------------------------------------------
# openZmeter installer for Raspberry Pi 4 (Raspberry Pi OS / Debian, 64 bit recommended).
#
# Builds openZmeter natively, sets up a local PostgreSQL database, a dedicated 'openzmeter' system user and a systemd
# service. Safe to run again: it rebuilds/reinstalls the binary and keeps the existing configuration and database.
#
# Usage (from a clone of the repository, as a normal user with sudo rights):
#   sudo Service/Packaging/raspberrypi/install.sh [options]
#
# Options:
#   --skip-deps        Do not install apt packages
#   --skip-build       Do not build, install the already built binary (Service/Tmp/raspberrypi4/openZmeter)
#   --jobs N           Parallel compile jobs (default 2, keeps memory use low on 2 GB boards)
#   --s1-port PATH     Serial port of the S1 RS-422 adapter written to a new config (default: auto detect)
#   --http-port N      Port of the web interface (default 80, or the port of an existing config). Use another port
#                      (e.g. 8080) when port 80 is already used, for example by Pi-hole.
# ----------------------------------------------------------------------------------------------------------------------
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVICE_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
CONFIG_NAME="raspberrypi4"
BINARY_SRC="${SERVICE_DIR}/Tmp/${CONFIG_NAME}/openZmeter"
BINARY_DST="/usr/local/sbin/openZmeter"
CONF_DIR="/etc/openzmeter"
CONF_FILE="${CONF_DIR}/openZmeter.conf"
UNIT_DST="/etc/systemd/system/openzmeter.service"
SVC_USER="openzmeter"
DB_NAME="openzmeter"
DB_USER="openzmeter"

SKIP_DEPS=0
SKIP_BUILD=0
JOBS=2
S1_PORT=""
HTTP_PORT=""

info()  { printf '\033[1;32m==>\033[0m %s\n' "$*"; }
warn()  { printf '\033[1;33mWARNING:\033[0m %s\n' "$*" >&2; }
die()   { printf '\033[1;31mERROR:\033[0m %s\n' "$*" >&2; exit 1; }

while [ $# -gt 0 ]; do
  case "$1" in
    --skip-deps)  SKIP_DEPS=1 ;;
    --skip-build) SKIP_BUILD=1 ;;
    --jobs)       shift; JOBS="${1:?--jobs needs a value}" ;;
    --s1-port)    shift; S1_PORT="${1:?--s1-port needs a value}" ;;
    --http-port)  shift; HTTP_PORT="${1:?--http-port needs a value}" ;;
    -h|--help)    sed -n '2,19p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *)            die "Unknown option '$1' (use --help)" ;;
  esac
  shift
done

# --- Checks -----------------------------------------------------------------------------------------------------------
[ "$(id -u)" -eq 0 ] || die "Run with sudo: sudo $0 $*"
BUILD_USER="${SUDO_USER:-root}"
case "$(uname -m)" in
  aarch64|armv7l|armv8l) ;;
  *) die "This installer targets the Raspberry Pi 4 (ARM). For other machines use Configs/linux_native." ;;
esac
if ! grep -qi "raspberry pi 4\|raspberry pi compute module 4" /proc/device-tree/model 2>/dev/null; then
  warn "This does not look like a Raspberry Pi 4 ($(tr -d '\0' < /proc/device-tree/model 2>/dev/null || echo unknown)). Continuing anyway."
fi
if [ -n "${HTTP_PORT}" ] && ! { [[ "${HTTP_PORT}" =~ ^[0-9]+$ ]] && [ "${HTTP_PORT}" -ge 1 ] && [ "${HTTP_PORT}" -le 65535 ]; }; then
  die "--http-port must be a number between 1 and 65535"
fi
# Effective web port: option, else existing config, else 80. Fail early if another program already listens on it.
if [ -z "${HTTP_PORT}" ] && [ -f "${CONF_FILE}" ]; then
  HTTP_PORT=$(python3 -I -c 'import json,sys; print(json.load(open(sys.argv[1])).get("HTTPServer",{}).get("Port",""))' "${CONF_FILE}" 2>/dev/null || true)
fi
HTTP_PORT="${HTTP_PORT:-80}"
# The running service is recognized by PID: its process name is the one of its main thread ("Main")
OWN_PID=$(systemctl show -p MainPID --value openzmeter.service 2>/dev/null || true)
PORT_USERS=$(ss -Hltnp "sport = :${HTTP_PORT}" 2>/dev/null | grep -oE '"[^"]+",pid=[0-9]+' | grep -v ",pid=${OWN_PID:-0}$" | cut -d'"' -f2 | sort -u | paste -sd, - || true)
if [ -n "${PORT_USERS}" ]; then
  die "Port ${HTTP_PORT} is already used by: ${PORT_USERS}. Choose another web port, e.g. --http-port 8080"
fi
[ "${BUILD_USER}" = "root" ] && warn "Building as root. Prefer running this script with sudo from your normal user."

MEM_KB=$(awk '/MemTotal/ {m=$2} /SwapTotal/ {s=$2} END {print m+s}' /proc/meminfo)
if [ "${SKIP_BUILD}" -eq 0 ] && [ "${MEM_KB}" -lt 3000000 ]; then
  warn "RAM + swap is below 3 GB. The optimized (LTO) link and the web build may run out of memory."
  warn "If the build gets killed, enlarge swap (e.g. CONF_SWAPSIZE=2048 in /etc/dphys-swapfile, then"
  warn "'sudo systemctl restart dphys-swapfile') or use --jobs 1."
fi

# --- Dependencies -----------------------------------------------------------------------------------------------------
if [ "${SKIP_DEPS}" -eq 0 ]; then
  info "Installing packages"
  apt-get update
  DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential git python3 curl \
    libpq-dev libfftw3-dev libusb-1.0-0-dev uuid-dev libssl-dev zlib1g-dev libdbus-1-dev \
    nodejs npm \
    postgresql
fi

# --- Build ------------------------------------------------------------------------------------------------------------
if [ "${SKIP_BUILD}" -eq 0 ]; then
  info "Building openZmeter as '${BUILD_USER}' (this takes a while on a Pi)"
  NODE_OPTS="--max-old-space-size=1536"
  NODE_MAJOR=$(node -p 'process.versions.node.split(".")[0]' 2>/dev/null || echo 0)
  [ "${NODE_MAJOR}" -ge 17 ] && NODE_OPTS="${NODE_OPTS} --openssl-legacy-provider"   # webpack 4 uses md4
  BRANCH=$(sudo -u "${BUILD_USER}" git -C "${SERVICE_DIR}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo custom)
  sudo -u "${BUILD_USER}" env NODE_OPTIONS="${NODE_OPTS}" \
    make -C "${SERVICE_DIR}" CONFIG="Configs/${CONFIG_NAME}" BUILD_BRANCH="${BRANCH}" -j"${JOBS}"
fi
[ -x "${BINARY_SRC}" ] || die "Binary not found at ${BINARY_SRC}. Build failed or was skipped."

# --- User -------------------------------------------------------------------------------------------------------------
if ! id "${SVC_USER}" >/dev/null 2>&1; then
  info "Creating system user '${SVC_USER}'"
  useradd --system --no-create-home --home-dir /nonexistent --shell /usr/sbin/nologin "${SVC_USER}"
fi
usermod -aG dialout "${SVC_USER}"

# --- Binary -----------------------------------------------------------------------------------------------------------
info "Installing ${BINARY_DST}"
systemctl stop openzmeter.service 2>/dev/null || true
install -m 0755 "${BINARY_SRC}" "${BINARY_DST}"

# --- Database ---------------------------------------------------------------------------------------------------------
info "Setting up PostgreSQL database '${DB_NAME}'"
systemctl enable --now postgresql.service
for _ in $(seq 1 30); do runuser -u postgres -- psql -tAc "SELECT 1" >/dev/null 2>&1 && break; sleep 1; done
DB_PASS=""
if [ -f "${CONF_FILE}" ]; then
  DB_PASS=$(python3 -I -c 'import json,sys; print(json.load(open(sys.argv[1])).get("Database",{}).get("Password",""))' "${CONF_FILE}" 2>/dev/null || true)
fi
[ -n "${DB_PASS}" ] || DB_PASS=$(head -c 16 /dev/urandom | od -An -tx1 | tr -d ' \n')
if [ "$(runuser -u postgres -- psql -tAc "SELECT 1 FROM pg_roles WHERE rolname='${DB_USER}'")" = "1" ]; then
  runuser -u postgres -- psql -q -c "ALTER ROLE ${DB_USER} WITH LOGIN PASSWORD '${DB_PASS}'"
else
  runuser -u postgres -- psql -q -c "CREATE ROLE ${DB_USER} WITH LOGIN PASSWORD '${DB_PASS}'"
fi
if [ "$(runuser -u postgres -- psql -tAc "SELECT 1 FROM pg_database WHERE datname='${DB_NAME}'")" != "1" ]; then
  runuser -u postgres -- createdb -O "${DB_USER}" "${DB_NAME}"
fi

# --- Configuration ----------------------------------------------------------------------------------------------------
install -d -m 0750 -o "${SVC_USER}" -g "${SVC_USER}" "${CONF_DIR}"
if [ ! -f "${CONF_FILE}" ]; then
  if [ -z "${S1_PORT}" ]; then
    mapfile -t PORTS < <(ls /dev/serial/by-id/* 2>/dev/null || true)
    if [ "${#PORTS[@]}" -eq 1 ]; then
      S1_PORT="${PORTS[0]}"
      info "Detected serial adapter ${S1_PORT}"
    else
      S1_PORT="/dev/ttyUSB0"
      [ "${#PORTS[@]}" -gt 1 ] && warn "Several serial adapters found, using ${S1_PORT}. Change it in Settings » Devices » S1."
    fi
  fi
  info "Writing ${CONF_FILE}"
  python3 -I - "${CONF_FILE}" "${DB_NAME}" "${DB_USER}" "${DB_PASS}" "${S1_PORT}" "${HTTP_PORT}" <<'EOF'
import json, sys
path, db, user, password, port, http_port = sys.argv[1:]
config = {
  "Database":   { "Hostname": "localhost", "Port": 5432, "Database": db, "Username": user, "Password": password },
  "PidFile":    "/run/openzmeter/openZmeter.pid",
  "Log":        { "File": "", "Level": "INFO" },          # log to journald only (saves SD card writes)
  "HTTPServer": { "Port": int(http_port) },
  "Devices":    { "S1": { "Port": port, "Mode": "AUTO", "Enabled": False, "Analyzer": {} } }
}
with open(path, "w") as f:
  json.dump(config, f, indent=2)
EOF
else
  info "Keeping existing ${CONF_FILE} (web port ${HTTP_PORT})"
  python3 -I - "${CONF_FILE}" "${HTTP_PORT}" <<'EOF'
import json, sys
path, http_port = sys.argv[1], int(sys.argv[2])
config = json.load(open(path))
if config.setdefault("HTTPServer", {}).get("Port") != http_port:
  config["HTTPServer"]["Port"] = http_port
  with open(path, "w") as f:
    json.dump(config, f, indent=2)
EOF
fi
chown "${SVC_USER}:${SVC_USER}" "${CONF_FILE}"
chmod 0600 "${CONF_FILE}"

# --- Service ----------------------------------------------------------------------------------------------------------
info "Installing systemd service"
install -m 0644 "${SCRIPT_DIR}/openzmeter.service" "${UNIT_DST}"
install -m 0755 "${SCRIPT_DIR}/openzmeter-watchdog.sh" /usr/local/sbin/openzmeter-watchdog
install -m 0644 "${SCRIPT_DIR}/openzmeter-watchdog.service" /etc/systemd/system/openzmeter-watchdog.service
systemctl daemon-reload
systemctl enable openzmeter.service
systemctl restart openzmeter.service
systemctl enable openzmeter-watchdog.service
systemctl restart openzmeter-watchdog.service
sleep 5
if systemctl is-active --quiet openzmeter.service; then
  IP=$(hostname -I 2>/dev/null | awk '{print $1}')
  URL_PORT=""
  [ "${HTTP_PORT}" != "80" ] && URL_PORT=":${HTTP_PORT}"
  info "openZmeter is running: http://${IP:-<pi-address>}${URL_PORT}/"
  cat <<EOF

Next steps:
  1. Log in as admin / admin and change the password right away.
  2. Ask Fluvius to activate the S1 port of your meter, connect the RS-422 adapter
     (RJ12 pin 3 = GND, 4 = BUS-, 5 = BUS+, 120 ohm termination on the adapter side).
  3. Settings » Devices » S1: check the serial port, set Enabled = Yes and save.
     The analyzer appears as soon as valid frames are received.

Logs: journalctl -u openzmeter -f
EOF
else
  warn "Service did not start. Check: journalctl -u openzmeter -n 100"
  exit 1
fi
