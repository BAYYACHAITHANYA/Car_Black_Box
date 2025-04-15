/*
 * File:   main.c
 * Author: chaithanya
 *
 * Created on 3 February, 2025, 12:42 PM
 */


#include "black_box.h"
#include "clcd.h"
#include "matrix_keypad.h"
#include "adc.h"
#include "i2c.h"
#include "ds1307.h"
#include "uart.h"

State_t state;

// Initialize the system configuration
void init_config()
{  
    state = e_dashboard;  // Set the initial state to dashboard
    init_clcd();          // Initialize the character LCD display
    init_matrix_keypad(); // Initialize the matrix keypad
    init_adc();           // Initialize the ADC (Analog-to-Digital Converter)
    init_i2c();           // Initialize the I2C communication protocol
    init_ds1307();        // Initialize the DS1307 RTC (Real-Time Clock)
    init_uart();          // Initialize the UART (Universal Asynchronous Receiver-Transmitter)
}

// Get the current time from the DS1307 RTC and store it in 'time' array
static void get_time(void)
{
    clock_reg[0] = read_ds1307(HOUR_ADDR);  // Read the hour register from DS1307
    clock_reg[1] = read_ds1307(MIN_ADDR);   // Read the minute register from DS1307
    clock_reg[2] = read_ds1307(SEC_ADDR);   // Read the second register from DS1307

    // Check if the 12-hour format is used (bit 6 of the hour register)
    if (clock_reg[0] & 0x40)
    {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);  // Extract the hour tens place (12-hour format)
        time[1] = '0' + (clock_reg[0] & 0x0F);          // Extract the hour ones place
    }
    else
    {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);  // Extract the hour tens place (24-hour format)
        time[1] = '0' + (clock_reg[0] & 0x0F);          // Extract the hour ones place
    }
    time[2] = ':';  // Add colon separator
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);  // Extract the minute tens place
    time[4] = '0' + (clock_reg[1] & 0x0F);          // Extract the minute ones place
    time[5] = ':';  // Add colon separator
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);  // Extract the second tens place
    time[7] = '0' + (clock_reg[2] & 0x0F);          // Extract the second ones place
    time[8] = '\0';  // Null-terminate the string
}

void main(void) 
{
    init_config();

    while(1)
    {
        get_time();
        // Detect key press
          key = read_switches(STATE_CHANGE);
        switch (state)
        {
            case e_dashboard:
                // Display dashboard
                view_dashboard();
                break;
            
            case e_main_menu:
                // Display dashboard
                display_main_menu();
                break;
            
            case e_view_log:
                // Display dashboard
                view_log();
                break;
                 
            case e_download_log:
                download_log();
                break;
                
            case e_clear_log:
               clear_log();
                break;
                
                      
            case e_set_time:
               set_time();
                break;
                
        }
        
    }
    
}
