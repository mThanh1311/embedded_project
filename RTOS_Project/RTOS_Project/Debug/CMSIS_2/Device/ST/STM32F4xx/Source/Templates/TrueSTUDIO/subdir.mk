################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../CMSIS_2/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.s 

OBJS += \
./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.o 

S_DEPS += \
./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
CMSIS_2/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/%.o: ../CMSIS_2/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/%.s CMSIS_2/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-CMSIS_2-2f-Device-2f-ST-2f-STM32F4xx-2f-Source-2f-Templates-2f-TrueSTUDIO

clean-CMSIS_2-2f-Device-2f-ST-2f-STM32F4xx-2f-Source-2f-Templates-2f-TrueSTUDIO:
	-$(RM) ./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.d ./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.o

.PHONY: clean-CMSIS_2-2f-Device-2f-ST-2f-STM32F4xx-2f-Source-2f-Templates-2f-TrueSTUDIO

