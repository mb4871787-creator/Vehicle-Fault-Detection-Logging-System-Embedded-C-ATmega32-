################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Control_ECU/DC_motor.c \
../Control_ECU/Ultrasonic.c \
../Control_ECU/adc.c \
../Control_ECU/buttons.c \
../Control_ECU/comtrol_ecu.c \
../Control_ECU/external_eeprom.c \
../Control_ECU/gpio.c \
../Control_ECU/icu.c \
../Control_ECU/lm35_sensor.c \
../Control_ECU/pwm_t0.c \
../Control_ECU/twi.c \
../Control_ECU/uart.c 

OBJS += \
./Control_ECU/DC_motor.o \
./Control_ECU/Ultrasonic.o \
./Control_ECU/adc.o \
./Control_ECU/buttons.o \
./Control_ECU/comtrol_ecu.o \
./Control_ECU/external_eeprom.o \
./Control_ECU/gpio.o \
./Control_ECU/icu.o \
./Control_ECU/lm35_sensor.o \
./Control_ECU/pwm_t0.o \
./Control_ECU/twi.o \
./Control_ECU/uart.o 

C_DEPS += \
./Control_ECU/DC_motor.d \
./Control_ECU/Ultrasonic.d \
./Control_ECU/adc.d \
./Control_ECU/buttons.d \
./Control_ECU/comtrol_ecu.d \
./Control_ECU/external_eeprom.d \
./Control_ECU/gpio.d \
./Control_ECU/icu.d \
./Control_ECU/lm35_sensor.d \
./Control_ECU/pwm_t0.d \
./Control_ECU/twi.d \
./Control_ECU/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Control_ECU/%.o: ../Control_ECU/%.c Control_ECU/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega16 -DF_CPU=1000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


