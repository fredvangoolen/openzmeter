FROM alpine:3.15

#install packages ----------------------------------------------------------
RUN apk add --no-cache asciidoc bash bc binutils bzip2 cdrkit coreutils diffutils findutils flex g++ gawk gcc gettext git grep intltool libxslt linux-headers make ncurses-dev patch perl python2-dev python3-dev tar unzip  util-linux wget zlib-dev xz npm openssh-client curl rsync

#Download openWRT ----------------------------------------------------------
RUN git config --global http.sslVerify "false"
RUN cd /opt && git clone --progress --branch openwrt-19.07 https://git.openwrt.org/openwrt/openwrt.git openWRT_SDK
WORKDIR /opt/openWRT_SDK
RUN ./scripts/feeds update -a && ./scripts/feeds install -a

#Setup openWRT SDK ---------------------------------------------------------
RUN echo "CONFIG_TARGET_sunxi=y" > .config
RUN echo "CONFIG_TARGET_sunxi_cortexa7=y" >> .config
RUN echo "CONFIG_TARGET_sunxi_cortexa7_DEVICE_sun8i-h3-nanopi-neo=y" >> .config
RUN make defconfig

#Generate required openWRT packages ----------------------------------------
ENV FORCE_UNSAFE_CONFIGURE=1
RUN make tools/install V=sc -j8
RUN make toolchain/install V=sc -j8
RUN make package/usign/host/compile V=sc -j8
RUN make package/zlib/compile V=sc -j8
RUN make package/postgresql/compile V=sc -j8
RUN make package/openssl/compile V=sc -j8
RUN make package/wireless-tools/compile V=sc-j8
RUN make package/dbus/compile V=sc -j8
RUN make package/libusb/compile V=sc -j8
RUN BUILD_VARIANT="single" make package/fftw3/compile V=sc -j8
RUN make package/util-linux/compile V=sc -j8
RUN make package/glib2/compile V=sc -j8
RUN make package/bluez/compile V=sc -j8
RUN make package/uuid/compile V=sc -j8

#Clean intermediate files --------------------------------------------------
#RUN cd /opt/openWRT_SDK && rm -rf build_dir

# OpenWRT SDK --------------------------------------------------------------
ENV SDK_DIR "/opt/openWRT_SDK"
ENV STAGING_DIR "${SDK_DIR}/staging_dir"
ENV TOOLCHAIN "${STAGING_DIR}/toolchain-arm_cortex-a7+neon-vfpv4_gcc-7.5.0_musl_eabi"
ENV CC "arm-openwrt-linux-gcc"
ENV CXX "arm-openwrt-linux-g++"
ENV CMAKE_C_COMPILER "arm-openwrt-linux-gcc"
ENV CMAKE_CXX_COMPILER "arm-openwrt-linux-g++"
ENV STRIP "arm-openwrt-linux-strip"
ENV RANLIB "arm-openwrt-linux-ranlib"
ENV AS "arm-openwrt-linux-as"
ENV AR "arm-openwrt-linux-ar" 
ENV LD "arm-openwrt-linux-ld"
ENV PATH "${TOOLCHAIN}/bin:${SDK_DIR}/scripts:$STAGING_DIR/host/bin:${PATH}"
