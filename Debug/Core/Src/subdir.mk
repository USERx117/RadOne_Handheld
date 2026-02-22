################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/app_threadx.c \
../Core/Src/display.c \
../Core/Src/gfx_fonts.c \
../Core/Src/gpio.c \
../Core/Src/icache.c \
../Core/Src/main.c \
../Core/Src/peripherals.c \
../Core/Src/processing.c \
../Core/Src/screen_main.c \
../Core/Src/screen_manager.c \
../Core/Src/screen_menu.c \
../Core/Src/screen_splash.c \
../Core/Src/sensor.c \
../Core/Src/spi.c \
../Core/Src/st7789.c \
../Core/Src/stm32u3xx_hal_msp.c \
../Core/Src/stm32u3xx_hal_timebase_tim.c \
../Core/Src/stm32u3xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32u3xx.c \
../Core/Src/tim.c \
../Core/Src/ui_helpers.c \
../Core/Src/usart.c \
../Core/Src/usb.c 

S_UPPER_SRCS += \
../Core/Src/tx_initialize_low_level.S 

OBJS += \
./Core/Src/adc.o \
./Core/Src/app_threadx.o \
./Core/Src/display.o \
./Core/Src/gfx_fonts.o \
./Core/Src/gpio.o \
./Core/Src/icache.o \
./Core/Src/main.o \
./Core/Src/peripherals.o \
./Core/Src/processing.o \
./Core/Src/screen_main.o \
./Core/Src/screen_manager.o \
./Core/Src/screen_menu.o \
./Core/Src/screen_splash.o \
./Core/Src/sensor.o \
./Core/Src/spi.o \
./Core/Src/st7789.o \
./Core/Src/stm32u3xx_hal_msp.o \
./Core/Src/stm32u3xx_hal_timebase_tim.o \
./Core/Src/stm32u3xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32u3xx.o \
./Core/Src/tim.o \
./Core/Src/tx_initialize_low_level.o \
./Core/Src/ui_helpers.o \
./Core/Src/usart.o \
./Core/Src/usb.o 

S_UPPER_DEPS += \
./Core/Src/tx_initialize_low_level.d 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/app_threadx.d \
./Core/Src/display.d \
./Core/Src/gfx_fonts.d \
./Core/Src/gpio.d \
./Core/Src/icache.d \
./Core/Src/main.d \
./Core/Src/peripherals.d \
./Core/Src/processing.d \
./Core/Src/screen_main.d \
./Core/Src/screen_manager.d \
./Core/Src/screen_menu.d \
./Core/Src/screen_splash.d \
./Core/Src/sensor.d \
./Core/Src/spi.d \
./Core/Src/st7789.d \
./Core/Src/stm32u3xx_hal_msp.d \
./Core/Src/stm32u3xx_hal_timebase_tim.d \
./Core/Src/stm32u3xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32u3xx.d \
./Core/Src/tim.d \
./Core/Src/ui_helpers.d \
./Core/Src/usart.d \
./Core/Src/usb.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DUX_INCLUDE_USER_DEFINE_FILE -DUSE_HAL_DRIVER -DSTM32U385xx -c -I../Core/Inc -I../AZURE_RTOS/App -I../USBX/App -I../USBX/Target -I../Drivers/STM32U3xx_HAL_Driver/Inc -I../Drivers/STM32U3xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/threadx/common/inc -I../Drivers/CMSIS/Device/ST/STM32U3xx/Include -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../Middlewares/ST/usbx/common/core/inc -I../Middlewares/ST/usbx/ports/generic/inc -I../Middlewares/ST/usbx/common/usbx_stm32_device_controllers -I../Middlewares/ST/usbx/common/usbx_device_classes/inc -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/display.o: ../Core/Src/display.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DTX_INCLUDE_USER_DEFINE_FILE -DTX_SINGLE_MODE_NON_SECURE=1 -DUX_INCLUDE_USER_DEFINE_FILE -DUSE_HAL_DRIVER -DSTM32U385xx -c -I../Core/Inc -I../AZURE_RTOS/App -I../USBX/App -I../USBX/Target -I../Drivers/STM32U3xx_HAL_Driver/Inc -I../Drivers/STM32U3xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/threadx/common/inc -I../Drivers/CMSIS/Device/ST/STM32U3xx/Include -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../Middlewares/ST/usbx/common/core/inc -I../Middlewares/ST/usbx/ports/generic/inc -I../Middlewares/ST/usbx/common/usbx_stm32_device_controllers -I../Middlewares/ST/usbx/common/usbx_device_classes/inc -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -u _printf_float -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/%.o: ../Core/Src/%.S Core/Src/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m33 -g3 -DDEBUG -DTX_SINGLE_MODE_NON_SECURE=1 -c -I../Core/Inc -I../AZURE_RTOS/App -I../USBX/App -I../USBX/Target -I../Drivers/STM32U3xx_HAL_Driver/Inc -I../Drivers/STM32U3xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/threadx/common/inc -I../Drivers/CMSIS/Device/ST/STM32U3xx/Include -I../Middlewares/ST/threadx/ports/cortex_m33/gnu/inc -I../Middlewares/ST/usbx/common/core/inc -I../Middlewares/ST/usbx/ports/generic/inc -I../Middlewares/ST/usbx/common/usbx_stm32_device_controllers -I../Middlewares/ST/usbx/common/usbx_device_classes/inc -I../Drivers/CMSIS/Include -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/app_threadx.cyclo ./Core/Src/app_threadx.d ./Core/Src/app_threadx.o ./Core/Src/app_threadx.su ./Core/Src/display.cyclo ./Core/Src/display.d ./Core/Src/display.o ./Core/Src/display.su ./Core/Src/gfx_fonts.cyclo ./Core/Src/gfx_fonts.d ./Core/Src/gfx_fonts.o ./Core/Src/gfx_fonts.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/icache.cyclo ./Core/Src/icache.d ./Core/Src/icache.o ./Core/Src/icache.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/peripherals.cyclo ./Core/Src/peripherals.d ./Core/Src/peripherals.o ./Core/Src/peripherals.su ./Core/Src/processing.cyclo ./Core/Src/processing.d ./Core/Src/processing.o ./Core/Src/processing.su ./Core/Src/screen_main.cyclo ./Core/Src/screen_main.d ./Core/Src/screen_main.o ./Core/Src/screen_main.su ./Core/Src/screen_manager.cyclo ./Core/Src/screen_manager.d ./Core/Src/screen_manager.o ./Core/Src/screen_manager.su ./Core/Src/screen_menu.cyclo ./Core/Src/screen_menu.d ./Core/Src/screen_menu.o ./Core/Src/screen_menu.su ./Core/Src/screen_splash.cyclo ./Core/Src/screen_splash.d ./Core/Src/screen_splash.o ./Core/Src/screen_splash.su ./Core/Src/sensor.cyclo ./Core/Src/sensor.d ./Core/Src/sensor.o ./Core/Src/sensor.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/st7789.cyclo ./Core/Src/st7789.d ./Core/Src/st7789.o ./Core/Src/st7789.su ./Core/Src/stm32u3xx_hal_msp.cyclo ./Core/Src/stm32u3xx_hal_msp.d ./Core/Src/stm32u3xx_hal_msp.o ./Core/Src/stm32u3xx_hal_msp.su ./Core/Src/stm32u3xx_hal_timebase_tim.cyclo ./Core/Src/stm32u3xx_hal_timebase_tim.d ./Core/Src/stm32u3xx_hal_timebase_tim.o ./Core/Src/stm32u3xx_hal_timebase_tim.su ./Core/Src/stm32u3xx_it.cyclo ./Core/Src/stm32u3xx_it.d ./Core/Src/stm32u3xx_it.o ./Core/Src/stm32u3xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32u3xx.cyclo ./Core/Src/system_stm32u3xx.d ./Core/Src/system_stm32u3xx.o ./Core/Src/system_stm32u3xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/tx_initialize_low_level.d ./Core/Src/tx_initialize_low_level.o ./Core/Src/ui_helpers.cyclo ./Core/Src/ui_helpers.d ./Core/Src/ui_helpers.o ./Core/Src/ui_helpers.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su ./Core/Src/usb.cyclo ./Core/Src/usb.d ./Core/Src/usb.o ./Core/Src/usb.su

.PHONY: clean-Core-2f-Src

