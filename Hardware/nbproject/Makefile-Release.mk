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
CC=arm-none-eabi-gcc
CCC=arm-none-eabi-g++
CXX=arm-none-eabi-g++
FC=gfortran
AS=arm-none-eabi-as

# Macros
CND_PLATFORM=arm-none-eabi-Linux
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
	${OBJECTDIR}/ADC_Capture.o \
	${OBJECTDIR}/Buffer.o \
	${OBJECTDIR}/CMSIS/Source/startup_stm32f303xc.o \
	${OBJECTDIR}/CMSIS/Source/system_stm32f30x.o \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_adc.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_can.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_dac.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_dbgmcu.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_dma.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_exti.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_flash.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_fmc.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_gpio.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_hrtim.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_i2c.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_iwdg.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_misc.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_opamp.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_pwr.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_rcc.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_rtc.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_spi.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_syscfg.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_tim.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_usart.o \
	${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_wwdg.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_core.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_init.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_int.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_mem.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_regs.o \
	${OBJECTDIR}/STM32_USB-FS-Device_Driver/src/usb_sil.o \
	${OBJECTDIR}/SysHandlers.o \
	${OBJECTDIR}/USB_Class.o \
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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hardware

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hardware: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/hardware ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/ADC_Capture.o: ADC_Capture.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ADC_Capture.o ADC_Capture.c

${OBJECTDIR}/Buffer.o: Buffer.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Buffer.o Buffer.c

${OBJECTDIR}/CMSIS/Source/startup_stm32f303xc.o: CMSIS/Source/startup_stm32f303xc.s
	${MKDIR} -p ${OBJECTDIR}/CMSIS/Source
	$(AS) $(ASFLAGS) -o ${OBJECTDIR}/CMSIS/Source/startup_stm32f303xc.o CMSIS/Source/startup_stm32f303xc.s

${OBJECTDIR}/CMSIS/Source/system_stm32f30x.o: CMSIS/Source/system_stm32f30x.c
	${MKDIR} -p ${OBJECTDIR}/CMSIS/Source
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CMSIS/Source/system_stm32f30x.o CMSIS/Source/system_stm32f30x.c

${OBJECTDIR}/Main.o: Main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Main.o Main.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_adc.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_adc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_adc.o STM32F30x_StdPeriph_Driver/src/stm32f30x_adc.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_can.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_can.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_can.o STM32F30x_StdPeriph_Driver/src/stm32f30x_can.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_dac.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_dac.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_dac.o STM32F30x_StdPeriph_Driver/src/stm32f30x_dac.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_dbgmcu.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_dbgmcu.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_dbgmcu.o STM32F30x_StdPeriph_Driver/src/stm32f30x_dbgmcu.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_dma.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_dma.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_dma.o STM32F30x_StdPeriph_Driver/src/stm32f30x_dma.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_exti.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_exti.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_exti.o STM32F30x_StdPeriph_Driver/src/stm32f30x_exti.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_flash.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_flash.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_flash.o STM32F30x_StdPeriph_Driver/src/stm32f30x_flash.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_fmc.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_fmc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_fmc.o STM32F30x_StdPeriph_Driver/src/stm32f30x_fmc.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_gpio.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_gpio.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_gpio.o STM32F30x_StdPeriph_Driver/src/stm32f30x_gpio.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_hrtim.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_hrtim.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_hrtim.o STM32F30x_StdPeriph_Driver/src/stm32f30x_hrtim.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_i2c.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_i2c.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_i2c.o STM32F30x_StdPeriph_Driver/src/stm32f30x_i2c.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_iwdg.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_iwdg.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_iwdg.o STM32F30x_StdPeriph_Driver/src/stm32f30x_iwdg.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_misc.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_misc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_misc.o STM32F30x_StdPeriph_Driver/src/stm32f30x_misc.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_opamp.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_opamp.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_opamp.o STM32F30x_StdPeriph_Driver/src/stm32f30x_opamp.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_pwr.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_pwr.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_pwr.o STM32F30x_StdPeriph_Driver/src/stm32f30x_pwr.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_rcc.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_rcc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_rcc.o STM32F30x_StdPeriph_Driver/src/stm32f30x_rcc.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_rtc.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_rtc.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_rtc.o STM32F30x_StdPeriph_Driver/src/stm32f30x_rtc.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_spi.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_spi.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_spi.o STM32F30x_StdPeriph_Driver/src/stm32f30x_spi.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_syscfg.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_syscfg.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_syscfg.o STM32F30x_StdPeriph_Driver/src/stm32f30x_syscfg.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_tim.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_tim.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_tim.o STM32F30x_StdPeriph_Driver/src/stm32f30x_tim.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_usart.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_usart.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_usart.o STM32F30x_StdPeriph_Driver/src/stm32f30x_usart.c

${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_wwdg.o: STM32F30x_StdPeriph_Driver/src/stm32f30x_wwdg.c
	${MKDIR} -p ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/STM32F30x_StdPeriph_Driver/src/stm32f30x_wwdg.o STM32F30x_StdPeriph_Driver/src/stm32f30x_wwdg.c

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

${OBJECTDIR}/USB_Class.o: USB_Class.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/USB_Class.o USB_Class.c

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
