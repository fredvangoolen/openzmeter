#!/bin/sh
if [ -z ${BUILD_DATE} ]; then BUILD_DATE=$(date +%Y%m%d.%H%M); fi
if [ -z ${BUILD_NAME} ]; then BUILD_NAME=${BUILD_NAME:-custom}; fi
echo "Building openWRT package for version '${BUILD_DATE}_${BUILD_NAME}'"
mkdir -p Package
mkdir -p Package/usr/sbin
mkdir -p Package/CONTROL
cp openZmeter Package/usr/sbin/
cat > Package/CONTROL/control << EOF
Package: openZmeter-${BUILD_NAME}
Version: ${BUILD_DATE}
Section: base
Priority: optional
Architecture: arm_cortex-a7_neon-vfpv4
Depends: fftw3f (>= 3.3.7), libmicrohttpd-no-ssl (>= 0.9.44), libusb-1.0 (>= 1.0.22), libpq (>= 9.6.6), libcurl (>= 7.60.0), bluez-daemon (>= 5.49)
Maintainer: Eduardo Viciana <eviciana@zred.es>
Description: Power quality/mettering and analysis tool.
Source: https://gitlab.com/zredalmeria/openzmeter
EOF
ipkg-build -o root -g root Package .
