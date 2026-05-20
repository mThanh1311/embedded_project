################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/XPT2046/XPT2046.c 

OBJS += \
./Drivers/XPT2046/XPT2046.o 

C_DEPS += \
./Drivers/XPT2046/XPT2046.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/XPT2046/%.o Drivers/XPT2046/%.su Drivers/XPT2046/%.cyclo: ../Drivers/XPT2046/%.c Drivers/XPT2046/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F405xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/EMBEDDED/RTOS_Project/RTOS_Project/Drivers/XPT2046" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-XPT2046

clean-Drivers-2f-XPT2046:
	-$(RM) ./Drivers/XPT2046/XPT2046.cyclo ./Drivers/XPT2046/XPT2046.d ./Drivers/XPT2046/XPT2046.o ./Drivers/XPT2046/XPT2046.su

.PHONY: clean-Drivers-2f-XPT2046

