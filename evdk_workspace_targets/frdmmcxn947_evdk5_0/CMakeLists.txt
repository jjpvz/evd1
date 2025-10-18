# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

if (DEFINED ENV{SdkRootDirPath})
	SET(SdkRootDirPath "$ENV{SdkRootDirPath}")
endif()

if (NOT DEFINED SdkRootDirPath)
	MESSAGE(FATAL_ERROR "SDK path could not be determined. Please update SdkRootDirPath value from your CMake presets file(s).")
endif()

# RESET FLAG VALUES
SET(CMAKE_ASM_FLAGS_DEBUG "")
SET(CMAKE_ASM_FLAGS_RELEASE "")
SET(CMAKE_C_FLAGS_DEBUG "")
SET(CMAKE_C_FLAGS_RELEASE "")
SET(CMAKE_CXX_FLAGS_DEBUG "")
SET(CMAKE_CXX_FLAGS_RELEASE "")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "")

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# CURRENT DIRECTORY
SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

set(CMAKE_USER_MAKE_RULES_OVERRIDE "user_override.cmake")

project(frdmmcxn947_evdk5_0)
set(MCUX_SDK_PROJECT_NAME frdmmcxn947_evdk5_0.elf)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

include(${ProjDirPath}/flags.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME} "${ProjDirPath}/CMSIS/tz_context.h"
"${ProjDirPath}/CMSIS/mpu_armv8.h"
"${ProjDirPath}/CMSIS/core_cm33.h"
"${ProjDirPath}/CMSIS/cmsis_version.h"
"${ProjDirPath}/CMSIS/cmsis_gcc.h"
"${ProjDirPath}/CMSIS/cmsis_compiler.h"
"${ProjDirPath}/CMSIS/m-profile/cmsis_gcc_m.h"
"${ProjDirPath}/CMSIS/m-profile/armv8m_mpu.h"
"${ProjDirPath}/board/pin_mux.h"
"${ProjDirPath}/board/pin_mux.c"
"${ProjDirPath}/board/clock_config.h"
"${ProjDirPath}/board/clock_config.c"
"${ProjDirPath}/board/board.h"
"${ProjDirPath}/board/board.c"
"${ProjDirPath}/component/uart/fsl_adapter_uart.h"
"${ProjDirPath}/component/uart/fsl_adapter_lpuart.c"
"${ProjDirPath}/component/lists/fsl_component_generic_list.h"
"${ProjDirPath}/component/lists/fsl_component_generic_list.c"
"${ProjDirPath}/component/serial_manager/fsl_component_serial_port_uart.h"
"${ProjDirPath}/component/serial_manager/fsl_component_serial_port_uart.c"
"${ProjDirPath}/component/serial_manager/fsl_component_serial_port_internal.h"
"${ProjDirPath}/component/serial_manager/fsl_component_serial_manager.h"
"${ProjDirPath}/component/serial_manager/fsl_component_serial_manager.c"
"${ProjDirPath}/component/osa/fsl_os_abstraction_config.h"
"${ProjDirPath}/component/osa/fsl_os_abstraction_bm.h"
"${ProjDirPath}/component/osa/fsl_os_abstraction_bm.c"
"${ProjDirPath}/component/osa/fsl_os_abstraction.h"
"${ProjDirPath}/device/system_MCXN947_cm33_core0.h"
"${ProjDirPath}/device/system_MCXN947_cm33_core0.c"
"${ProjDirPath}/device/MCXN947_cm33_core0_features.h"
"${ProjDirPath}/device/MCXN947_cm33_core0_COMMON.h"
"${ProjDirPath}/device/MCXN947_cm33_core0.h"
"${ProjDirPath}/device/fsl_device_registers.h"
"${ProjDirPath}/device/periph/PERI_WWDT.h"
"${ProjDirPath}/device/periph/PERI_WUU.h"
"${ProjDirPath}/device/periph/PERI_VREF.h"
"${ProjDirPath}/device/periph/PERI_VBAT.h"
"${ProjDirPath}/device/periph/PERI_UTICK.h"
"${ProjDirPath}/device/periph/PERI_USDHC.h"
"${ProjDirPath}/device/periph/PERI_USBPHY.h"
"${ProjDirPath}/device/periph/PERI_USBNC.h"
"${ProjDirPath}/device/periph/PERI_USBHSDCD.h"
"${ProjDirPath}/device/periph/PERI_USBHS.h"
"${ProjDirPath}/device/periph/PERI_USBDCD.h"
"${ProjDirPath}/device/periph/PERI_USB.h"
"${ProjDirPath}/device/periph/PERI_TSI.h"
"${ProjDirPath}/device/periph/PERI_TRDC.h"
"${ProjDirPath}/device/periph/PERI_SYSPM.h"
"${ProjDirPath}/device/periph/PERI_SYSCON.h"
"${ProjDirPath}/device/periph/PERI_SPC.h"
"${ProjDirPath}/device/periph/PERI_SMARTDMA.h"
"${ProjDirPath}/device/periph/PERI_SINC.h"
"${ProjDirPath}/device/periph/PERI_SEMA42.h"
"${ProjDirPath}/device/periph/PERI_SCT.h"
"${ProjDirPath}/device/periph/PERI_SCG.h"
"${ProjDirPath}/device/periph/PERI_S50.h"
"${ProjDirPath}/device/periph/PERI_RTC.h"
"${ProjDirPath}/device/periph/PERI_QDC.h"
"${ProjDirPath}/device/periph/PERI_PWM.h"
"${ProjDirPath}/device/periph/PERI_PUF.h"
"${ProjDirPath}/device/periph/PERI_POWERQUAD.h"
"${ProjDirPath}/device/periph/PERI_PORT.h"
"${ProjDirPath}/device/periph/PERI_PLU.h"
"${ProjDirPath}/device/periph/PERI_PKC.h"
"${ProjDirPath}/device/periph/PERI_PINT.h"
"${ProjDirPath}/device/periph/PERI_PDM.h"
"${ProjDirPath}/device/periph/PERI_OTPC.h"
"${ProjDirPath}/device/periph/PERI_OSTIMER.h"
"${ProjDirPath}/device/periph/PERI_OPAMP.h"
"${ProjDirPath}/device/periph/PERI_NPX.h"
"${ProjDirPath}/device/periph/PERI_MRT.h"
"${ProjDirPath}/device/periph/PERI_MAILBOX.h"
"${ProjDirPath}/device/periph/PERI_LP_FLEXCOMM.h"
"${ProjDirPath}/device/periph/PERI_LPUART.h"
"${ProjDirPath}/device/periph/PERI_LPTMR.h"
"${ProjDirPath}/device/periph/PERI_LPSPI.h"
"${ProjDirPath}/device/periph/PERI_LPI2C.h"
"${ProjDirPath}/device/periph/PERI_LPDAC.h"
"${ProjDirPath}/device/periph/PERI_LPCMP.h"
"${ProjDirPath}/device/periph/PERI_ITRC.h"
"${ProjDirPath}/device/periph/PERI_INTM.h"
"${ProjDirPath}/device/periph/PERI_INPUTMUX.h"
"${ProjDirPath}/device/periph/PERI_I3C.h"
"${ProjDirPath}/device/periph/PERI_I2S.h"
"${ProjDirPath}/device/periph/PERI_HPDAC.h"
"${ProjDirPath}/device/periph/PERI_GPIO.h"
"${ProjDirPath}/device/periph/PERI_GDET.h"
"${ProjDirPath}/device/periph/PERI_FREQME.h"
"${ProjDirPath}/device/periph/PERI_FMUTEST.h"
"${ProjDirPath}/device/periph/PERI_FMU.h"
"${ProjDirPath}/device/periph/PERI_FLEXSPI.h"
"${ProjDirPath}/device/periph/PERI_FLEXIO.h"
"${ProjDirPath}/device/periph/PERI_EWM.h"
"${ProjDirPath}/device/periph/PERI_EVTG.h"
"${ProjDirPath}/device/periph/PERI_ERM.h"
"${ProjDirPath}/device/periph/PERI_ENET.h"
"${ProjDirPath}/device/periph/PERI_EMVSIM.h"
"${ProjDirPath}/device/periph/PERI_EIM.h"
"${ProjDirPath}/device/periph/PERI_DMA.h"
"${ProjDirPath}/device/periph/PERI_DM.h"
"${ProjDirPath}/device/periph/PERI_DIGTMP.h"
"${ProjDirPath}/device/periph/PERI_CTIMER.h"
"${ProjDirPath}/device/periph/PERI_CRC.h"
"${ProjDirPath}/device/periph/PERI_CMC.h"
"${ProjDirPath}/device/periph/PERI_CDOG.h"
"${ProjDirPath}/device/periph/PERI_CAN.h"
"${ProjDirPath}/device/periph/PERI_CACHE64_POLSEL.h"
"${ProjDirPath}/device/periph/PERI_CACHE64_CTRL.h"
"${ProjDirPath}/device/periph/PERI_BSP32.h"
"${ProjDirPath}/device/periph/PERI_AHBSC.h"
"${ProjDirPath}/device/periph/PERI_ADC.h"
"${ProjDirPath}/drivers/fsl_spc.h"
"${ProjDirPath}/drivers/fsl_spc.c"
"${ProjDirPath}/drivers/fsl_smartdma_mcxn.h"
"${ProjDirPath}/drivers/fsl_smartdma_mcxn.c"
"${ProjDirPath}/drivers/fsl_smartdma.h"
"${ProjDirPath}/drivers/fsl_smartdma.c"
"${ProjDirPath}/drivers/fsl_reset.h"
"${ProjDirPath}/drivers/fsl_reset.c"
"${ProjDirPath}/drivers/fsl_port.h"
"${ProjDirPath}/drivers/fsl_lpuart.h"
"${ProjDirPath}/drivers/fsl_lpuart.c"
"${ProjDirPath}/drivers/fsl_lpi2c.h"
"${ProjDirPath}/drivers/fsl_lpi2c.c"
"${ProjDirPath}/drivers/fsl_lpflexcomm.h"
"${ProjDirPath}/drivers/fsl_lpflexcomm.c"
"${ProjDirPath}/drivers/fsl_inputmux_connections.h"
"${ProjDirPath}/drivers/fsl_inputmux.h"
"${ProjDirPath}/drivers/fsl_inputmux.c"
"${ProjDirPath}/drivers/fsl_gpio.h"
"${ProjDirPath}/drivers/fsl_gpio.c"
"${ProjDirPath}/drivers/fsl_common_arm.h"
"${ProjDirPath}/drivers/fsl_common_arm.c"
"${ProjDirPath}/drivers/fsl_common.h"
"${ProjDirPath}/drivers/fsl_common.c"
"${ProjDirPath}/drivers/fsl_clock.h"
"${ProjDirPath}/drivers/fsl_clock.c"
"${ProjDirPath}/source/uvc_camera.h"
"${ProjDirPath}/source/uvc_camera.c"
"${ProjDirPath}/source/usb_device_descriptor.h"
"${ProjDirPath}/source/usb_device_descriptor.c"
"${ProjDirPath}/source/usb_device_config.h"
"${ProjDirPath}/source/semihost_hardfault.c"
"${ProjDirPath}/source/rotate180_cm33.s"
"${ProjDirPath}/source/main.c"
"${ProjDirPath}/source/ssd1306.c"
"${ProjDirPath}/source/lpi2c3_controller_polling.c"
"${ProjDirPath}/source/bitmaps.c"
"${ProjDirPath}/source/gpio_output.h"
"${ProjDirPath}/source/gpio_output.c"
"${ProjDirPath}/source/gpio_input.h"
"${ProjDirPath}/source/gpio_input.c"
"${ProjDirPath}/source/convertuyvytouint8_cm33.s"
"${ProjDirPath}/source/clearuint8image_cm33.s"
"${ProjDirPath}/startup/startup_mcxn947_cm33_core0.c"
"${ProjDirPath}/startup/boot_multicore_slave.h"
"${ProjDirPath}/startup/boot_multicore_slave.c"
"${ProjDirPath}/usb/phy/usb_phy.h"
"${ProjDirPath}/usb/phy/usb_phy.c"
"${ProjDirPath}/usb/include/usb_spec.h"
"${ProjDirPath}/usb/include/usb_misc.h"
"${ProjDirPath}/usb/include/usb.h"
"${ProjDirPath}/usb/device/usb_device.h"
"${ProjDirPath}/usb/device/source/usb_device_dci.h"
"${ProjDirPath}/usb/device/source/usb_device_dci.c"
"${ProjDirPath}/usb/device/source/usb_device_ch9.h"
"${ProjDirPath}/usb/device/source/usb_device_ch9.c"
"${ProjDirPath}/usb/device/include/usb_device.h"
"${ProjDirPath}/usb/device/class/usb_device_class.h"
"${ProjDirPath}/usb/device/class/usb_device_class.c"
"${ProjDirPath}/usb/device/source/khci/usb_device_khci.h"
"${ProjDirPath}/usb/device/source/khci/usb_device_khci.c"
"${ProjDirPath}/usb/device/source/ehci/usb_device_ehci.h"
"${ProjDirPath}/usb/device/source/ehci/usb_device_ehci.c"
"${ProjDirPath}/usb/device/class/video/usb_device_video.h"
"${ProjDirPath}/usb/device/class/video/usb_device_video.c"
"${ProjDirPath}/utilities/fsl_memcpy.S"
"${ProjDirPath}/utilities/fsl_debug_console_conf.h"
"${ProjDirPath}/utilities/fsl_assert.h"
"${ProjDirPath}/utilities/fsl_assert.c"
"${ProjDirPath}/utilities/str/fsl_str.h"
"${ProjDirPath}/utilities/str/fsl_str.c"
"${ProjDirPath}/utilities/debug_console/fsl_debug_console.h"
"${ProjDirPath}/utilities/debug_console/fsl_debug_console.c"
"${ProjDirPath}/video/fsl_video_common.h"
"${ProjDirPath}/video/fsl_video_common.c"
"${ProjDirPath}/video/fsl_sccb.h"
"${ProjDirPath}/video/fsl_sccb.c"
"${ProjDirPath}/video/fsl_ov7670.h"
"${ProjDirPath}/video/fsl_ov7670.c"
"${ProjDirPath}/video/fsl_camera_device.h"
"${ProjDirPath}/video/fsl_camera.h"
"${ProjDirPath}/../../evdk_operators/coding_and_compression.c"
"${ProjDirPath}/../../evdk_operators/coding_and_compression.h"
"${ProjDirPath}/../../evdk_operators/fonts.c"
"${ProjDirPath}/../../evdk_operators/fonts.h"
"${ProjDirPath}/../../evdk_operators/graphics_algorithms.c"
"${ProjDirPath}/../../evdk_operators/graphics_algorithms.h"
"${ProjDirPath}/../../evdk_operators/histogram_operations.c"
"${ProjDirPath}/../../evdk_operators/histogram_operations.h"
"${ProjDirPath}/../../evdk_operators/image.h"
"${ProjDirPath}/../../evdk_operators/image_fundamentals.c"
"${ProjDirPath}/../../evdk_operators/image_fundamentals.h"
"${ProjDirPath}/../../evdk_operators/mensuration.c"
"${ProjDirPath}/../../evdk_operators/mensuration.h"
"${ProjDirPath}/../../evdk_operators/morphological_filters.c"
"${ProjDirPath}/../../evdk_operators/morphological_filters.h"
"${ProjDirPath}/../../evdk_operators/noise.c"
"${ProjDirPath}/../../evdk_operators/noise.h"
"${ProjDirPath}/../../evdk_operators/nonlinear_filters.c"
"${ProjDirPath}/../../evdk_operators/nonlinear_filters.h"
"${ProjDirPath}/../../evdk_operators/operators.h"
"${ProjDirPath}/../../evdk_operators/segmentation.c"
"${ProjDirPath}/../../evdk_operators/segmentation.h"
"${ProjDirPath}/../../evdk_operators/spatial_filters.c"
"${ProjDirPath}/../../evdk_operators/spatial_filters.h"
"${ProjDirPath}/../../evdk_operators/spatial_frequency_filters.c"
"${ProjDirPath}/../../evdk_operators/spatial_frequency_filters.h"
"${ProjDirPath}/../../evdk_operators/transforms.c"
"${ProjDirPath}/../../evdk_operators/transforms.h"
)




if(${LIBRARY_TYPE} STREQUAL "REDLIB")
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE $ENV{ARMGCC_DIR}/redlib/include)
endif()

set(c_include_directories "${ProjDirPath}/usb/device"
"${ProjDirPath}/usb/phy"
"${ProjDirPath}/drivers"
"${ProjDirPath}/utilities"
"${ProjDirPath}/utilities/debug_console"
"${ProjDirPath}/component/uart"
"${ProjDirPath}/component/serial_manager"
"${ProjDirPath}/component/lists"
"${ProjDirPath}/usb/include"
"${ProjDirPath}/CMSIS"
"${ProjDirPath}/CMSIS/m-profile"
"${ProjDirPath}/device"
"${ProjDirPath}/video"
"${ProjDirPath}/device/periph"
"${ProjDirPath}/component/osa/config"
"${ProjDirPath}/component/osa"
"${ProjDirPath}/utilities/str"
"${ProjDirPath}/source/config/device/ehci"
"${ProjDirPath}/source/config/device/khci"
"${ProjDirPath}/../../evdk_operators"
"${ProjDirPath}/usb/device/include"
"${ProjDirPath}/usb/device/source"
"${ProjDirPath}/usb/device/source/ehci"
"${ProjDirPath}/usb/device/source/khci"
"${ProjDirPath}/startup"
"${ProjDirPath}/source"
"${ProjDirPath}/board"
"${ProjDirPath}/usb/device/class"
"${ProjDirPath}/usb/device/class/video"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	list(APPEND c_include_directories "${ProjDirPath}/evdk_operators"
	)
endif()
target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:C>:${c_include_directories}>)

set(cpp_include_directories "")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${cpp_include_directories}>)

set(include_files "")


target_precompile_headers(${MCUX_SDK_PROJECT_NAME} PRIVATE ${include_files})

set(link_directories "")


set(link_libraries "")


target_link_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE ${link_directories})
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES} ${link_libraries})
target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)





