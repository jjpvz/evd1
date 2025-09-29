if(NOT DEFINED FPU)
	set(FPU "-mfpu=fpv5-sp-d16 -mfloat-abi=hard")
endif()

if(${LIBRARY_TYPE} STREQUAL "REDLIB")
	set(SPECS "-specs=redlib.specs")
elseif(${LIBRARY_TYPE} STREQUAL "NEWLIB_NANO")
	set(SPECS "--specs=nano.specs")
endif()

if(NOT DEFINED DEBUG_CONSOLE_CONFIG)
	set(DEBUG_CONSOLE_CONFIG "-DSDK_DEBUGCONSOLE=1")
endif()

set(CMAKE_ASM_FLAGS_DEBUG " \
    ${CMAKE_ASM_FLAGS_DEBUG} \
    ${FPU} \
    -mcpu=cortex-m33 \
    -mthumb \
")

set(CMAKE_C_FLAGS_DEBUG " \
    ${CMAKE_C_FLAGS_DEBUG} \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
    -std=gnu99 \
    -DCPU_MCXN947VDF \
    -DCPU_MCXN947VDF_cm33 \
    -DCPU_MCXN947VDF_cm33_core0 \
    -D_DEBUG=1 \
    -DUSB_STACK_BM \
    -DFSL_OSA_BM_TASK_ENABLE=0 \
    -DFSL_OSA_BM_TIMER_CONFIG=0 \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_BAREMETAL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DCR_INTEGER_PRINTF \
    -DPRINTF_FLOAT_ENABLE=1 \
    -D__MCUXPRESSO \
    -D__USE_CMSIS \
    -DDEBUG \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DUSE_RTOS=0 \
    -Og \
    -fno-common \
    -fmerge-constants \
    -g3 \
    -gdwarf-4 \
    -mcpu=cortex-m33 -ffunction-sections -fdata-sections  -fno-builtin \
    -fstack-usage \
    -mcpu=cortex-m33 \
    -mthumb \
")

set(CMAKE_CXX_FLAGS_DEBUG " \
    ${CMAKE_CXX_FLAGS_DEBUG} \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
    -O0 \
    -fno-common \
    -fmerge-constants \
    -g3 \
    -gdwarf-4 \
    -Wall \
    -fstack-usage \
    -mcpu=cortex-m33 \
    -mthumb \
")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG " \
    ${CMAKE_EXE_LINKER_FLAGS_DEBUG} \
    ${FPU} \
    ${SPECS} \
    -nostdlib \
    -Xlinker \
    -no-warn-rwx-segments \
    -Xlinker \
    -Map=output.map \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -print-memory-usage \
    -Xlinker \
    --sort-section=alignment \
    -Xlinker \
    --cref \
    -mcpu=cortex-m33 \
    -mthumb \
    -T\"${ProjDirPath}/frdmmcxn947_evdk5_0_Debug.ld\" \
")

set(CMAKE_ASM_FLAGS_RELEASE " \
    ${CMAKE_ASM_FLAGS_RELEASE} \
    ${FPU} \
    -mcpu=cortex-m33 \
    -mthumb \
")

set(CMAKE_C_FLAGS_RELEASE " \
    ${CMAKE_C_FLAGS_RELEASE} \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
    -std=gnu99 \
    -DCPU_MCXN947VDF \
    -DCPU_MCXN947VDF_cm33 \
    -DCPU_MCXN947VDF_cm33_core0 \
    -D_DEBUG=1 \
    -DUSB_STACK_BM \
    -DFSL_OSA_BM_TASK_ENABLE=0 \
    -DFSL_OSA_BM_TIMER_CONFIG=0 \
    -DMCUXPRESSO_SDK \
    -DSDK_OS_BAREMETAL \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DCR_INTEGER_PRINTF \
    -DPRINTF_FLOAT_ENABLE=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -D__MCUXPRESSO \
    -D__USE_CMSIS \
    -DNDEBUG \
    -DSDK_I2C_BASED_COMPONENT_USED=1 \
    -DUSE_RTOS=0 \
    -O3 \
    -fno-common \
    -fmerge-constants \
    -gdwarf-4 \
    -mcpu=cortex-m33 -ffunction-sections -fdata-sections  -fno-builtin \
    -fstack-usage \
    -mcpu=cortex-m33 \
    -mthumb \
")

set(CMAKE_CXX_FLAGS_RELEASE " \
    ${CMAKE_CXX_FLAGS_RELEASE} \
    ${FPU} \
    ${DEBUG_CONSOLE_CONFIG} \
    -fno-common \
    -fmerge-constants \
    -gdwarf-4 \
    -Wall \
    -fstack-usage \
    -mcpu=cortex-m33 \
    -mthumb \
")

set(CMAKE_EXE_LINKER_FLAGS_RELEASE " \
    ${CMAKE_EXE_LINKER_FLAGS_RELEASE} \
    ${FPU} \
    ${SPECS} \
    -nostdlib \
    -Xlinker \
    -no-warn-rwx-segments \
    -Xlinker \
    -Map=output.map \
    -Xlinker \
    --gc-sections \
    -Xlinker \
    -print-memory-usage \
    -Xlinker \
    --sort-section=alignment \
    -Xlinker \
    --cref \
    -mcpu=cortex-m33 \
    -mthumb \
    -T\"${ProjDirPath}/frdmmcxn947_evdk5_0_Release.ld\" \
")
