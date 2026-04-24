################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../DC_motor.c \
../Ultrasonic.c \
../adc.c \
../buttons.c \
../comtrol_ecu.c \
../external_eeprom.c \
../gpio.c \
../icu.c \
../lm35_sensor.c \
../pwm_t0.c \
../twi.c \
../uart.c 

OBJS += \
./DC_motor.o \
./Ultrasonic.o \
./adc.o \
./buttons.o \
./comtrol_ecu.o \
./external_eeprom.o \
./gpio.o \
./icu.o \
./lm35_sensor.o \
./pwm_t0.o \
./twi.o \
./uart.o 

C_DEPS += \
./DC_motor.d \
./Ultrasonic.d \
./adc.d \
./buttons.d \
./comtrol_ecu.d \
./external_eeprom.d \
./gpio.d \
./icu.d \
./lm35_sensor.d \
./pwm_t0.d \
./twi.d \
./uart.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega32 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


