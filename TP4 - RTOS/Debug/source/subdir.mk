################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/App.c \
../source/Encoder.c \
../source/Queue.c \
../source/Queue2.c \
../source/cardReader.c \
../source/display.c \
../source/fsm.c \
../source/gateway.c \
../source/gpio.c \
../source/led.c \
../source/main.c \
../source/threads.c \
../source/timer.c \
../source/uart.c 

OBJS += \
./source/App.o \
./source/Encoder.o \
./source/Queue.o \
./source/Queue2.o \
./source/cardReader.o \
./source/display.o \
./source/fsm.o \
./source/gateway.o \
./source/gpio.o \
./source/led.o \
./source/main.o \
./source/threads.o \
./source/timer.o \
./source/uart.o 

C_DEPS += \
./source/App.d \
./source/Encoder.d \
./source/Queue.d \
./source/Queue2.d \
./source/cardReader.d \
./source/display.d \
./source/fsm.d \
./source/gateway.d \
./source/gpio.d \
./source/led.d \
./source/main.d \
./source/threads.d \
./source/timer.d \
./source/uart.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MK64FN1M0VLL12 -D__USE_CMSIS -DDEBUG -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\ucosiii_config" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU\ARM-Cortex-M4\GNU" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-CPU" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uC-LIB" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Ports\ARM-Cortex-M4\Generic\GNU" -I"C:\labo\TPs_LaboDeMicros\ucosiii_labo_de_micros_project\source\rtos\uCOSIII\src\uCOS-III\Source" -I../source -I../ -I../SDK/CMSIS -I../SDK/startup -O0 -fno-common -g3 -Wall -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


