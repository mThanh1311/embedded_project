################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CMSIS_2/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c 

OBJS += \
./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.o 

C_DEPS += \
./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
CMSIS_2/Device/ST/STM32F4xx/Source/Templates/%.o CMSIS_2/Device/ST/STM32F4xx/Source/Templates/%.su CMSIS_2/Device/ST/STM32F4xx/Source/Templates/%.cyclo: ../CMSIS_2/Device/ST/STM32F4xx/Source/Templates/%.c CMSIS_2/Device/ST/STM32F4xx/Source/Templates/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/do_an/th_htn/test1/RTOS_Project/CMSIS_2/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-CMSIS_2-2f-Device-2f-ST-2f-STM32F4xx-2f-Source-2f-Templates

clean-CMSIS_2-2f-Device-2f-ST-2f-STM32F4xx-2f-Source-2f-Templates:
	-$(RM) ./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.cyclo ./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.d ./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.o ./CMSIS_2/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.su

.PHONY: clean-CMSIS_2-2f-Device-2f-ST-2f-STM32F4xx-2f-Source-2f-Templates

