#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f401xx.o \
	${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40_41xxx.o \
	${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40xx.o \
	${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427_437xx.o \
	${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427x.o \
	${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f429_439xx.o \
	${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.o \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/misc.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cec.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dfsdm.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dsi.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash_ramfunc.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmpi2c.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_lptim.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_qspi.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sai.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spdifrx.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.o \
	${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_core.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_init.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_int.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_mem.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_regs.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_sil.o \
	${OBJECTDIR}/SysHandlers.o \
	${OBJECTDIR}/USB_CDC.o \
	${OBJECTDIR}/usb_istr.o \
	${OBJECTDIR}/usb_pwr.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hardwarev2

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hardwarev2: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hardwarev2 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f401xx.o: CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f401xx.s
	${MKDIR} -p ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7
	$(AS) $(ASFLAGS) -o ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f401xx.o CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f401xx.s

${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40_41xxx.o: CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40_41xxx.s
	${MKDIR} -p ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7
	$(AS) $(ASFLAGS) -o ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40_41xxx.o CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40_41xxx.s

${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40xx.o: CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40xx.s
	${MKDIR} -p ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7
	$(AS) $(ASFLAGS) -o ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40xx.o CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f40xx.s

${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427_437xx.o: CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427_437xx.s
	${MKDIR} -p ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7
	$(AS) $(ASFLAGS) -o ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427_437xx.o CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427_437xx.s

${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427x.o: CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427x.s
	${MKDIR} -p ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7
	$(AS) $(ASFLAGS) -o ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427x.o CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f427x.s

${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f429_439xx.o: CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f429_439xx.s
	${MKDIR} -p ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7
	$(AS) $(ASFLAGS) -o ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f429_439xx.o CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc_ride7/startup_stm32f429_439xx.s

${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.o: CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
	${MKDIR} -p ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.o CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c

${OBJECTDIR}/Main.o: Main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Main.o Main.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/misc.o: STM32F4xx_StdPeriph_Driver/src/misc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/misc.o STM32F4xx_StdPeriph_Driver/src/misc.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_adc.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_can.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cec.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cec.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cec.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cec.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_crc.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_aes.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_des.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_cryp_tdes.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dac.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dbgmcu.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dcmi.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dfsdm.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dfsdm.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dfsdm.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dfsdm.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dsi.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dsi.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dsi.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dsi.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash_ramfunc.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash_ramfunc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash_ramfunc.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash_ramfunc.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmpi2c.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmpi2c.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmpi2c.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmpi2c.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_md5.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_hash_sha1.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_i2c.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_iwdg.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_lptim.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_lptim.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_lptim.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_lptim.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_pwr.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_qspi.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_qspi.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_qspi.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_qspi.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rng.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rtc.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sai.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sai.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sai.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sai.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spdifrx.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spdifrx.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spdifrx.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spdifrx.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_spi.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c

${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.o: STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c
	${MKDIR} -p ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.o STM32F4xx_StdPeriph_Driver/src/stm32f4xx_wwdg.c

${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_core.o: STM32_USB-FS-Device_Driver/src/usb_core.c
	${MKDIR} -p ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_core.o STM32_USB-FS-Device_Driver/src/usb_core.c

${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_init.o: STM32_USB-FS-Device_Driver/src/usb_init.c
	${MKDIR} -p ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_init.o STM32_USB-FS-Device_Driver/src/usb_init.c

${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_int.o: STM32_USB-FS-Device_Driver/src/usb_int.c
	${MKDIR} -p ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_int.o STM32_USB-FS-Device_Driver/src/usb_int.c

${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_mem.o: STM32_USB-FS-Device_Driver/src/usb_mem.c
	${MKDIR} -p ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_mem.o STM32_USB-FS-Device_Driver/src/usb_mem.c

${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_regs.o: STM32_USB-FS-Device_Driver/src/usb_regs.c
	${MKDIR} -p ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_regs.o STM32_USB-FS-Device_Driver/src/usb_regs.c

${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_sil.o: STM32_USB-FS-Device_Driver/src/usb_sil.c
	${MKDIR} -p ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_sil.o STM32_USB-FS-Device_Driver/src/usb_sil.c

${OBJECTDIR}/SysHandlers.o: SysHandlers.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SysHandlers.o SysHandlers.c

${OBJECTDIR}/USB_CDC.o: USB_CDC.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/USB_CDC.o USB_CDC.c

${OBJECTDIR}/usb_istr.o: usb_istr.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/usb_istr.o usb_istr.c

${OBJECTDIR}/usb_pwr.o: usb_pwr.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/usb_pwr.o usb_pwr.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
