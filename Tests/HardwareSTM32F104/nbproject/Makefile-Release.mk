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
	${OBJECTDIR}/CMSIS/core_cm3.o \
	${OBJECTDIR}/CMSIS/startup_stm32f10x_md.o \
	${OBJECTDIR}/CMSIS/system_stm32f10x.o \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/misc.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_bkp.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_can.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_cec.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_crc.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_dac.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_fsmc.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_pwr.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_rtc.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_sdio.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.o \
	${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_wwdg.o \
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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hardwarestm32f104

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hardwarestm32f104: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hardwarestm32f104 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/CMSIS/core_cm3.o: CMSIS/core_cm3.c
	${MKDIR} -p ${OBJECTDIR}/CMSIS
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CMSIS/core_cm3.o CMSIS/core_cm3.c

${OBJECTDIR}/CMSIS/startup_stm32f10x_md.o: CMSIS/startup_stm32f10x_md.c
	${MKDIR} -p ${OBJECTDIR}/CMSIS
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CMSIS/startup_stm32f10x_md.o CMSIS/startup_stm32f10x_md.c

${OBJECTDIR}/CMSIS/system_stm32f10x.o: CMSIS/system_stm32f10x.c
	${MKDIR} -p ${OBJECTDIR}/CMSIS
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CMSIS/system_stm32f10x.o CMSIS/system_stm32f10x.c

${OBJECTDIR}/Main.o: Main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Main.o Main.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/misc.o: STM32F10x_StdPeriph_Driver/src/misc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/misc.o STM32F10x_StdPeriph_Driver/src/misc.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.o STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_bkp.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_bkp.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_bkp.o STM32F10x_StdPeriph_Driver/src/stm32f10x_bkp.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_can.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_can.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_can.o STM32F10x_StdPeriph_Driver/src/stm32f10x_can.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_cec.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_cec.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_cec.o STM32F10x_StdPeriph_Driver/src/stm32f10x_cec.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_crc.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_crc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_crc.o STM32F10x_StdPeriph_Driver/src/stm32f10x_crc.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_dac.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_dac.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_dac.o STM32F10x_StdPeriph_Driver/src/stm32f10x_dac.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.o STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.o STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.o STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.o STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_fsmc.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_fsmc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_fsmc.o STM32F10x_StdPeriph_Driver/src/stm32f10x_fsmc.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.o STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.o STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.o STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_pwr.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_pwr.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_pwr.o STM32F10x_StdPeriph_Driver/src/stm32f10x_pwr.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.o STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_rtc.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_rtc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_rtc.o STM32F10x_StdPeriph_Driver/src/stm32f10x_rtc.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_sdio.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_sdio.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_sdio.o STM32F10x_StdPeriph_Driver/src/stm32f10x_sdio.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.o STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.o STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.o STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c

${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_wwdg.o: STM32F10x_StdPeriph_Driver/src/stm32f10x_wwdg.c
	${MKDIR} -p ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F10x_StdPeriph_Driver/src/stm32f10x_wwdg.o STM32F10x_StdPeriph_Driver/src/stm32f10x_wwdg.c

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
