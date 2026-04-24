/*======================================================================
 * Project: Vehicle Fault Detection & Logging System (HMI ECU)
 * Author : Mohamed Baiomy
 * MCU    : ATmega32 @ 8 MHz
 * ---------------------------------------------------------------------
 * Description:
 *   - Handles all Human–Machine Interface tasks.
 *   - Displays menus and real-time data on LCD.
 *   - Sends commands to Control ECU via UART.
 *   - Receives live sensor data and fault codes from Control ECU.
 *   - Uses Timer1 interrupts for periodic delays.
 *   - Interacts with user through a 4x4 Keypad.
 *======================================================================*/

#include "keypad.h"
#include "lcd.h"
#include "timer.h"
#include "uart.h"
#include "avr/io.h"
#include <util/delay.h>
#include <avr/interrupt.h>

/* -------------------- System Delays -------------------- */
#define DELAY       2   // Delay in seconds used for display timing

/* -------------------- Commands to Control ECU -------------------- */
#define START_CAR   1
#define DISPLAY     2
#define FAULTS      3
#define STOP_CAR    4
#define EXIT        5
#define CONT        5   // Used as "continue" signal in communication

/* -------------------- Fault Codes -------------------- */
#define P001_CODE   1   // Distance Too Close
#define P002_CODE   2   // Engine Overheat

/* -------------------- Window States -------------------- */
#define CLOSE       0
#define OPEN        1

#define FALSE_START 200 // Sent by Control ECU if car not started

/* -------------------- Function Prototypes -------------------- */
void home_display(void);

/* -------------------- Global Variables -------------------- */
uint8 g_car_state = 0;   // 1 → car ON, 0 → car OFF

/*====================================================================
 * MAIN FUNCTION
 * -------------------------------------------------------------------
 * - Initializes LCD, UART, and Timer1.
 * - Displays Main Menu.
 * - Handles user key inputs (1–4).
 *====================================================================*/
int main(void)
{
    /* Enable global interrupts */
    SREG |= (1 << 7);

    /* Initialize LCD (4x16) */
    LCD_init();

    /* UART setup for communication with Control ECU */
    UART_ConfigType uart_config;
    uart_config.baud_rate = 9600;
    uart_config.bit_data  = EIGHT_BIT;
    uart_config.parity    = UART_PARITY_OFF;
    uart_config.stop_bit  = TWO_BIT;
    UART_init(&uart_config);

    /* Configure Timer1 for 1-second interrupt using compare mode */
    Timer_ConfigType timer_config;
    timer_config.timer_ID = TIMER1_ID;
    timer_config.timer_InitialValue = 0;
    timer_config.timer_clock = TMR_PRESCALER_1024;
    timer_config.timer_compare_MatchValue = 7812; // For 1 sec @ 8 MHz
    timer_config.timer_mode = TMR_MODE_COMPARE;
    Timer_init(&timer_config);

    /* Show main menu on startup */
    home_display();

    /* Local variables */
    uint8 key = 0, temp, distance, window1_state, window2_state;
    uint8 code1, code2;

    /* -------------------- Main Super Loop -------------------- */
    while (1)
    {
        /* Read user input from keypad */
        key = KEYPAD_getPressedKey();

        switch (key)
        {
        /* ------------------------------------------------------
         * CASE 1: START CAR
         * ------------------------------------------------------*/
        case 1:
            UART_sendByte(START_CAR);      // Send start command
            g_car_state = 1;               // Update system state

            LCD_clearScreen();
            LCD_moveCursor(1,0);
            LCD_displayString("Operation Started");
            LCD_moveCursor(2,0);
            LCD_displayString("Monitoring Active");

            /* Wait a short period before returning */
            g_sec = 0;
            while (g_sec < DELAY);
            home_display();
            key = 0;
            break;

        /* ------------------------------------------------------
         * CASE 2: DISPLAY LIVE DATA
         * ------------------------------------------------------*/
        case 2:
            LCD_clearScreen();
            UART_sendByte(DISPLAY);  // Request live sensor data

            do {
                g_sec = 0;
                while (g_sec < DELAY)
                {
                    /* Wait for start frame 0xAA */
                    while (UART_recieveByte() != 0xAA);

                    /* Read first byte (temperature or FALSE_START) */
                    uint8 x = UART_recieveByte();

                    /* ----------- Valid live data received ----------- */
                    if (x != FALSE_START)
                    {
                        temp = x;                            // Temperature
                        distance = UART_recieveByte();        // Distance
                        window1_state = UART_recieveByte();   // Window 1 state
                        window2_state = UART_recieveByte();   // Window 2 state

                        /* Display on LCD */
                        LCD_moveCursor(0,0);
                        LCD_displayString("Temp= ");
                        LCD_intgerToString(temp);
                        LCD_displayString(" C ");

                        LCD_moveCursor(1,0);
                        LCD_displayString("Distance= ");
                        LCD_intgerToString(distance);
                        LCD_displayString(" cm ");

                        LCD_moveCursor(2,0);
                        LCD_displayString("Win1: ");
                        if (window1_state == OPEN)
                            LCD_displayString("OPEN ");
                        else
                            LCD_displayString("CLOSE");

                        LCD_moveCursor(3,0);
                        LCD_displayString("Win2: ");
                        if (window2_state == OPEN)
                            LCD_displayString("OPEN ");
                        else
                            LCD_displayString("CLOSE");

                        /* Ask Control ECU for next frame */
                        UART_sendByte(CONT);
                    }

                    /* ----------- Car not started yet ----------- */
                    else {
                        LCD_moveCursor(1,0);
                        LCD_displayString("Please ");
                        LCD_moveCursor(2,0);
                        LCD_displayString("Start car first ");
                        while (g_sec < DELAY);
                        break;
                    }
                }

                /* End of one display cycle */
                UART_sendByte(0);

                /* Ask user if they want to display again */
                if (g_car_state)
                {
                    LCD_clearScreen();
                    LCD_moveCursor(0,0);
                    LCD_displayString("Display again?");
                    LCD_moveCursor(1,0);
                    LCD_displayString("Press 2=Yes");
                    LCD_moveCursor(2,0);
                    LCD_displayString("Menu=Other key");

                    key = KEYPAD_getPressedKey();

                    if (key != 2)
                    {
                        home_display();
                        key = 0;
                        break;
                    }
                    else
                    {
                        LCD_clearScreen();
                        UART_sendByte(DISPLAY); // Request again
                    }
                }
            } while (key == 2);

            break;

        /* ------------------------------------------------------
         * CASE 3: DISPLAY LOGGED FAULTS
         * ------------------------------------------------------*/
        case 3:
            UART_sendByte(FAULTS);     // Request stored faults
            LCD_clearScreen();
            LCD_moveCursor(0,0);
            LCD_displayString("Logged Faults:");

            g_sec = 0;
            while (g_sec <= DELAY)
            {
                /* Wait for start of frame */
                while (UART_recieveByte() != 0xAA);
                code1 = UART_recieveByte(); // First fault code
                code2 = UART_recieveByte(); // Second fault code

                /* Check end of frame */
                if (UART_recieveByte() != 0x55)
                    continue;

                /* Clear lines and show results */
                LCD_moveCursor(1,0);

                if (code1 == P001_CODE)
                {
                    LCD_displayString("P001: TooClose");
                    LCD_moveCursor(2,0);
                    if (code2 == P002_CODE)
                        LCD_displayString("P002: Overheat");
                    LCD_moveCursor(3,0);
                    LCD_displayString("--End of List--");
                }
                else if (code1 == P002_CODE)
                {
                    LCD_displayString("P002: Overheat");
                    LCD_moveCursor(2,0);
                    if (code2 == P001_CODE)
                        LCD_displayString("P001: TooClose");
                    LCD_moveCursor(3,0);
                    LCD_displayString("--End of List--");
                }
                else
                {
                    LCD_displayString("NO ERRORS");
                }

                UART_sendByte(CONT);  // Ask for next frame (if any)
            }

            UART_sendByte(0);
            home_display();
            key = 0;
            break;

        /* ------------------------------------------------------
         * CASE 4: STOP CAR
         * ------------------------------------------------------*/
        case 4:
            UART_sendByte(STOP_CAR);
            g_car_state = 0;
            LCD_clearScreen();
            LCD_displayString("System Stopped!");
            g_sec = 0;
            while (g_sec < DELAY);
            home_display();
            key = 0;
            break;

        /* ------------------------------------------------------
         * DEFAULT CASE: no key / invalid input
         * ------------------------------------------------------*/
        default:
            UART_sendByte(CONT);
            break;
        }
    }
}

/*====================================================================
 * FUNCTION: home_display
 * -------------------------------------------------------------------
 * Displays the main menu on the LCD:
 *   1. Start
 *   2. Display
 *   3. Faults
 *   4. Stop
 *====================================================================*/
void home_display(void)
{
    LCD_clearScreen();
    LCD_moveCursor(0,0);
    LCD_displayString("1.Start");
    LCD_moveCursor(1,0);
    LCD_displayString("2.Display");
    LCD_moveCursor(2,0);
    LCD_displayString("3.Faults");
    LCD_moveCursor(3,0);
    LCD_displayString("4.Stop");
}
