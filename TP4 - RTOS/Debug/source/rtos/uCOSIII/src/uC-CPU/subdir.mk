################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/rtos/uCOSIII/src/uC-CPU/cpu_core.c 

OBJS += \
./source/rtos/uCOSIII/src/uC-CPU/cpu_core.o 

C_DEPS += \
./source/rtos/uCOSIII/src/uC-CPU/cpu_core.d 


# Each subdirectory must supply rules for building sources it contributes
source/rtos/uCOSIII/src/uC-CPU/%.o: ../source/rtos/uCOSIII/src/uC-CPU/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\ucosiii_config" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU\ARM-Cortex-M4\GNU" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-LIB" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Ports\ARM-Cortex-M4\Generic\GNU" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Source" -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


