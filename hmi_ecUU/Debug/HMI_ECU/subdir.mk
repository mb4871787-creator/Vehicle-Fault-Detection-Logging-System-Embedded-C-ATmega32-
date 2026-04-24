################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HMI_ECU/gpio.c \
../HMI_ECU/hmi_ecu.c \
../HMI_ECU/keypad.c \
../HMI_ECU/lcd.c \
../HMI_ECU/timer.c \
../HMI_ECU/uart.c 

OBJS += \
./HMI_ECU/gpio.o \
./HMI_ECU/hmi_ecu.o \
./HMI_ECU/keypad.o \
./HMI_ECU/lcd.o \
./HMI_ECU/timer.o \
./HMI_ECU/uart.o 

C_DEPS += \
./HMI_ECU/gpio.d \
./HMI_ECU/hmi_ecu.d \
./HMI_ECU/keypad.d \
./HMI_ECU/lcd.d \
./HMI_ECU/timer.d \
./HMI_ECU/uart.d 


# Each subdirectory must supply rules for building sources it contributes
HMI_ECU/%.o: ../HMI_ECU/%.c HMI_ECU/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega16 -DF_CPU=1000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


