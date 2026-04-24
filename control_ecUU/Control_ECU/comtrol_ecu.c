/*==============================================================
 * Project: Vehicle Fault Detection & Logging System (Control ECU)
 * Author : Mohamed Baiomy
 * MCU    : ATmega32 @ 8 MHz
 * -------------------------------------------------------------
 * Description:
 *   - Reads distance (Ultrasonic) and temperature (LM35)
 *   - Detects vehicle faults:
 *        P001 → Distance < 10 cm (Too Close)
 *        P002 → Temperature > 90°C (Overheat)
 *   - Logs fault codes in External EEPROM
 *   - Controls window motors using push buttons
 *   - Communicates with HMI ECU via UART
 *==============================================================*/

#include "DC_motor.h"
#include "external_eeprom.h"
#include "lm35_sensor.h"
#include "adc.h"
#include "uart.h"
#include "pwm_t0.h"
#include "avr/io.h"
#include "Ultrasonic.h"
#include "twi.h"
#include <avr/interrupt.h>
#include "buttons.h"
#include "util/delay.h"

/* -------------------- Command IDs -------------------- */
#define START_CAR   1
#define DISPLAY     2
#define FAULTS      3
#define STOP_CAR    4
#define EXIT        5

/* -------------------- Window States -------------------- */
#define CLOSE       0
#define OPEN        1

/* -------------------- Fault Codes -------------------- */
#define P001_CODE   1   // Distance < 10 cm
#define P002_CODE   2   // Temperature > 90°C

#define FALSE_START 200  // Marker for car not started

/* -------------------- Global Variables -------------------- */
volatile uint8 g_car_state = 0;   // 1 → car running, 0 → stopped
uint8 window1_state = CLOSE, window2_state = CLOSE;
uint8 fault_det;                  // temporary variable for EEPROM reading

/* -------------------- Function Prototypes -------------------- */
void fault_detect(uint8 distance, uint8 temperature);
void window_check(void);

/* ===============================================================
 * MAIN FUNCTION
 * ---------------------------------------------------------------
 * Initializes all drivers and continuously handles:
 *   - UART command reception
 *   - Fault detection
 *   - Window control
 *==============================================================*/
int main(void)
{
    /* Enable global interrupts */
    SREG |= (1 << 7);

    /* Initialize I2C (TWI) for EEPROM communication */
    TWI_init();

    /* ADC configuration for LM35 sensor */
    ADC_ConfigType adc_config;
    adc_config.ref_volt  = INTERNAL_2_56V;
    adc_config.prescaler = ADC_PRESCALER_64;

    /* UART configuration for communication with HMI ECU */
    UART_ConfigType uart_config;
    uart_config.baud_rate = 9600;
    uart_config.bit_data  = EIGHT_BIT;
    uart_config.parity    = UART_PARITY_OFF;
    uart_config.stop_bit  = TWO_BIT;
    UART_init(&uart_config);

    /* Local variables */
    uint8 code1, code2, display_state;
    uint8 command;
    uint8 temperature = 0;
    uint16 distance = 100;

    /* Clear EEPROM memory (first 2 bytes used for logging) */
    EEPROM_writeByte(0x0000, 0xFF);
    _delay_ms(10);
    EEPROM_writeByte(0x0001, 0xFF);
    _delay_ms(10);

    /* -------------------- Infinite Super Loop -------------------- */
    while (1)
    {
        /* Always perform background tasks */
        fault_detect(distance, temperature);
        window_check();

        /* Wait for command from HMI ECU */
        command = UART_recieveByte();

        /* =================== START CAR =================== */
        if (command == START_CAR)
        {
            g_car_state = 1;

            /* Initialize subsystems */
            buttons_INIT();
            DcMotor_Init();
            Ultrasonic_init();
            ADC_init(&adc_config);

            /* Initial readings */
            temperature = LM35_getTemperature();
            distance = Ultrasonic_readDistance();

            /* Perform initial checks */
            window_check();
            fault_detect(distance, temperature);
        }

        /* =================== DISPLAY MODE =================== */
        else if (command == DISPLAY)
        {
            /* If system is running */
            if (g_car_state == 1)
            {
                display_state = 1;
                while (display_state)
                {
                    window_check();
                    temperature = LM35_getTemperature();
                    distance = Ultrasonic_readDistance();
                    fault_detect(distance, temperature);

                    /* Send frame to HMI ECU:
                     * 0xAA [Temp] [Dist] [Win1] [Win2]
                     */
                    UART_sendByte(0xAA);
                    UART_sendByte(temperature);
                    UART_sendByte((uint8)distance);
                    UART_sendByte(window1_state);
                    UART_sendByte(window2_state);

                    /* Wait for continue / exit command */
                    display_state = UART_recieveByte();
                }
            }
            else
            {
                /* If user requested DISPLAY but car not started */
                UART_sendByte(0xAA);
                UART_sendByte(FALSE_START);
                UART_sendByte(0x55);
            }
        }

        /* =================== READ FAULTS =================== */
        else if (command == FAULTS)
        {
            display_state = 1;
            while (display_state)
            {
                /* Read stored faults from EEPROM (2 bytes only) */
                EEPROM_readByte(0x0000, &code1);
                EEPROM_readByte(0x0001, &code2);

                /* Send frame: 0xAA [code1] [code2] 0x55 */
                UART_sendByte(0xAA);
                UART_sendByte(code1);
                UART_sendByte(code2);
                UART_sendByte(0x55);

                display_state = UART_recieveByte();
            }
        }

        /* =================== STOP CAR =================== */
        else if (command == STOP_CAR)
        {
            g_car_state = 0;
            /* Turn off all actuators */
            DcMotor_DeInit();
        }

        /* =================== DEFAULT (Background Tasks) =================== */
        else
        {
            if (g_car_state)
            {
                fault_detect(distance, temperature);
                window_check();
            }
            /* else: car stopped → do nothing */
        }
    }
}

/* ===============================================================
 * FUNCTION: fault_detect
 * ---------------------------------------------------------------
 * Description:
 *   - Monitors sensors and logs any fault conditions
 *   - Faults logged once to EEPROM at addresses 0x0000–0x0001
 * Inputs:
 *   distance   → value from Ultrasonic sensor (cm)
 *   temperature→ value from LM35 sensor (°C)
 *==============================================================*/
void fault_detect(uint8 distance, uint8 temperature)
{
    /* ------------ Distance Fault Detection ------------ */
    if (distance < 10)
    {
        EEPROM_readByte(0x0000, &fault_det);
        if (fault_det == 0xFF)
        {
            EEPROM_writeByte(0x0000, P001_CODE); // Log first fault
            _delay_ms(10);
        }
        else
        {
            EEPROM_readByte(0x0001, &fault_det);
            if (fault_det == 0xFF)
            {
                EEPROM_writeByte(0x0001, P001_CODE); // Log second fault
                _delay_ms(10);
            }
        }
    }

    /* ------------ Temperature Fault Detection ------------ */
    if (temperature > 90)
    {
        EEPROM_readByte(0x0000, &fault_det);
        if (fault_det == 0xFF)
        {
            EEPROM_writeByte(0x0000, P002_CODE);
            _delay_ms(10);
        }
        else
        {
            EEPROM_readByte(0x0001, &fault_det);
            if (fault_det == 0xFF)
            {
                EEPROM_writeByte(0x0001, P002_CODE);
                _delay_ms(10);
            }
        }
    }
}

/* ===============================================================
 * FUNCTION: window_check
 * ---------------------------------------------------------------
 * Description:
 *   - Reads button inputs
 *   - Controls DC motors for window movement
 *   - Updates window state variables
 *==============================================================*/
void window_check(void)
{
    if (g_car_state)
    {
        /* Read button input and act accordingly */
        switch (button_pressed())
        {
        case 1: // Window1 open
            if (window1_state == CLOSE)
            {
                DcMotor_Rotate(ACW, 100, MOTOR1);
                _delay_ms(1000);
                DcMotor_Rotate(STOP, 100, MOTOR1);
                window1_state = OPEN;
            }
            break;

        case 2: // Window1 close
            if (window1_state == OPEN)
            {
                DcMotor_Rotate(CW, 100, MOTOR1);
                _delay_ms(1000);
                DcMotor_Rotate(STOP, 100, MOTOR1);
                window1_state = CLOSE;
            }
            break;

        case 3: // Window2 open
            if (window2_state == CLOSE)
            {
                DcMotor_Rotate(ACW, 100, MOTOR2);
                _delay_ms(1000);
                DcMotor_Rotate(STOP, 100, MOTOR2);
                window2_state = OPEN;
            }
            break;

        case 4: // Window2 close
            if (window2_state == OPEN)
            {
                DcMotor_Rotate(CW, 100, MOTOR2);
                _delay_ms(1000);
                DcMotor_Rotate(STOP, 100, MOTOR2);
                window2_state = CLOSE;
            }
            break;

        default:
            break;
        }
    }
    else
    {
        /* If car is stopped, stop all motors */
        DcMotor_Rotate(STOP, 0, MOTOR1);
        DcMotor_Rotate(STOP, 0, MOTOR2);
    }
}
