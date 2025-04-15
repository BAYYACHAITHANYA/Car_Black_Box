/*
 * File:   black_box.c
 * Author: chaithanya
 *
 * Created on 3 February, 2025, 12:52 PM
 */


#include <xc.h>
#include "black_box.h"
#include "clcd.h"
#include "matrix_keypad.h"
#include "adc.h"
#include "uart.h"
#include "i2c.h"
#include "ds1307.h"
#include "external_EEPROM.h"

// 2D array to store events with their respective identifiers
unsigned char Ev[10][3] = {"ON", "GN", "G1", "G2", "G3", "G4", "G5", "GR", "C_"}; 

// Variables for speed, index, event count, and other states
unsigned int speed = 0, i = 0, index = 0, event_count = 0, count = 0, star = 0, vflag = 0; 
unsigned char add = 0, add1 = 0, hash = '0', add2 = 0, add3 = 0;
unsigned char *Opr[] = {"VIEW LOG       ", "CLEAR LOG      ", "DOWNLOAD LOG   ", "SET TIME       "};
unsigned char buffer[10][16];
unsigned int j, toggle, l, m;

// Function to view the dashboard and handle events via keypad
void view_dashboard(void) {

    if (key == MK_SW2) { // If key 2 is pressed, increment the event index
        if (index == 8) {
            index = 1;  // Reset index to 1 if it reaches 8
            store_event(time, Ev[index], speed);//store event to EEPROM
        } else if (index < 7) {
            index++;  // Otherwise, increment the index
            store_event(time, Ev[index], speed);//store event to EEPROM
        }
    }

    if (key == MK_SW3 && index > 1) { // If key 3 is pressed, decrement the event index
        if (index == 8) {
            index = 1;  // Reset index to 1 if it reaches 8
            store_event(time, Ev[index], speed);//store event to EEPROM
        } else if (index > 1) {
            index--;  // Decrement the index
            store_event(time, Ev[index], speed);
        }
    }

    if (key == MK_SW1) { // If key 1 is pressed, set index to 8
        if (index != 8) {
            index = 8;
            store_event(time, Ev[index], speed);//store event to EEPROM
        }
    }

    if (key == MK_SW11) { // If key 11 is pressed, return to main menu
        state = e_main_menu;
        CLEAR_DISP_SCREEN;
        return;
    }

    speed = read_adc(CHANNEL4) / 10.23; // Read ADC value for speed
    clcd_print("TIME   EV  SP", LINE1(2)); // Display time, event, and speed on LCD

    clcd_print(Ev[index], LINE2(9)); // Display the current event
    clcd_print(time, LINE2(0)); // Display the current time
    clcd_putch((speed % 10) + 48, LINE2(13)); // Display the last digit of speed
    clcd_putch((speed / 10) + 48, LINE2(14)); // Display the tens digit of speed
}

// Function to store event information in external EEPROM
void store_event(unsigned char time[], unsigned char ev[], unsigned int speed) {
    event_count++;  // Increment the event count
    if (event_count > 10) { // If event count exceeds 10, shift events
        add2 = 12;
        add3 = 0;
        event_count = 10;
        for (l = 0; l < 9; l++) { // Loop to shift stored events in EEPROM
            for (m = 0; m < 12; m++) {
                write_external_EEPROM(add3++, read_external_EEPROM(add2++)); // Shift event data
            }
        }
        add = 108; // Reset the add pointer
    }

    // Store current event information
    for (i = 0; i < 8; i++) {
        write_external_EEPROM(add++, time[i]); // Store time in EEPROM
    }
    for (i = 0; i < 2; i++) {
        write_external_EEPROM(add++, ev[i]); // Store event name in EEPROM
    }
    write_external_EEPROM(add++, (speed % 10) + 48); // Store the ones digit of speed
    write_external_EEPROM(add++, (speed / 10) + 48); // Store the tens digit of speed
}

// Function to display the main menu
void display_main_menu(void) {
    clcd_print(Opr[count], LINE1(2)); // Display the first option in the main menu
    clcd_print(Opr[count + 1], LINE2(2)); // Display the second option

    if (key == MK_SW1) { // Handle key press for SW1 to navigate the menu
        if (star == 1) star = 1; // If star is already set, stay at the same position
        else if (count > 0) count--; // Otherwise, move up in the menu
        star = 0;
    }
    
    if (key == MK_SW2) { // Handle key press for SW2 to navigate the menu
        if (star == 0) star = 0; // If star is not set, stay at the same position
        else if (count < 2) count++; // Otherwise, move down in the menu
        star = 1;
    }

    // Display the '*' character to highlight the selected option
    if (star == 1) {
        clcd_putch('*', LINE2(0));
        clcd_putch(' ', LINE1(0));
    } else {
        clcd_putch('*', LINE1(0));
        clcd_putch(' ', LINE2(0));
    }

    if (key == MK_SW11) { // If key 11 is pressed, go to the selected menu option
        CLEAR_DISP_SCREEN;
        state = star + count + 2;
        star = count = 0;
    }

    if (key == MK_SW12) { // If key 12 is pressed, go to the dashboard
        CLEAR_DISP_SCREEN;
        state = e_dashboard;
        CLEAR_DISP_SCREEN;
    }
}

// Function to read events from external EEPROM and store them in the buffer
void event_reader(void) {
    add1 = 0x00; // Reset address pointer
    for (j = 0; j < event_count; j++) { // Loop through the events
        for (i = 0; i < 14; i++) {
            if (i == 8 || i == 11) // store spaces if this condition is true
                buffer[j][i] = ' ';
            else
                buffer[j][i] = read_external_EEPROM(add1++); // Read event data from EEPROM
        }
        buffer[j][14] = '\0'; // Null-terminate the string
    }
}

// Function to view the event log
void view_log(void) {
    static int flag, index2;
    if (event_count == 0) { // If no events are stored, display "NO LOGS"
        CLEAR_DISP_SCREEN;
        clcd_print("NO LOGS ARE", LINE1(3));
        clcd_print("AVAILABLE", LINE2(4));
        for (unsigned long int wait = 500000; wait--;);
        CLEAR_DISP_SCREEN;
        state = e_main_menu;
    }
    if (flag == 0) { // If it's the first time, read events
        flag = 1;//make flag as 1
        event_reader();
    }

    // Handle key presses to navigate through the logs
    if (key == MK_SW1 && index2 > 0) { 
        index2--;
        hash--;
    }
    if (key == MK_SW2 && index2 < event_count - 1) { 
        index2++;
        hash++;
    }

    clcd_print("# TIME   EV  SP", LINE1(0)); // Display header
    clcd_putch(hash, LINE2(0)); // Display the log number
    clcd_print(buffer[index2], LINE2(2)); // Display the current log entry

    if (key == MK_SW12) { // If key 12 is pressed, return to the main menu
        CLEAR_DISP_SCREEN;//and reset all values
        flag = 0;
        index2 = 0;
        hash = '0';
        add1 = 0x00;
        clcd_print("EXSITING..", LINE1(0));
        for (unsigned long int wait = 500000; wait--;);//wait some time
        i = count = 0;
        CLEAR_DISP_SCREEN;
        state = e_main_menu;
        return;
    }
}

// Function to clear all event logs
void clear_log(void) {
    CLEAR_DISP_SCREEN;
    event_count = 0; // Reset event count
    add = 0x00; // Reset address pointer
    add1 = 0x00; // Reset address pointer for reading
    clcd_print("Clearing logs...", LINE1(0)); // Display clearing message
    clcd_print("Wait sometime   ", LINE2(0));
    for (unsigned long int wait = 500000; wait--;);
    CLEAR_DISP_SCREEN;
    state = e_main_menu;
}

// Function to download event logs
void download_log(void) {
    if (event_count == 0) { // If no events, display "NO LOGS"
        CLEAR_DISP_SCREEN;
        clcd_print("NO LOGS....", LINE1(0));
        clcd_print("TO DOWNLOAD ", LINE2(2));
        for (unsigned long int d = 500000; d--;);
        CLEAR_DISP_SCREEN;
        puts("NO LOGS TO DOWNLOAD.\n\r");
        state = e_main_menu;
    } else {
        CLEAR_DISP_SCREEN;
        puts("TIME     EV SP\n\r"); // Print headers for logs

        clcd_print("Downloading..", LINE1(0)); // Show download message
        clcd_print("open teraterm ", LINE2(0)); // Prompt to open terminal

        event_reader(); // Read the events
        for (int i = 0; i < event_count; i++) {
            puts(buffer[i]); // Print all logs
            puts("\n\r");
        }
        state = e_main_menu;//return to main menu;
        for (unsigned long int k = 500000; k--;);//wait some time
        CLEAR_DISP_SCREEN;
    }
}

// Variables for setting time
unsigned char present_field = 0, Hours, Minutes, Seconds;
unsigned int delay = 0;

// Function to set the time on DS1307
void set_time(void) {
    static int flag;
    if (flag == 0) { // If it's the first time, initialize time variables
        Hours = ((time[0] - 48)*10)+(time[1] - 48);
        Minutes = ((time[3] - 48)*10)+(time[4] - 48);
        Seconds = ((time[6] - 48)*10)+(time[7] - 48);
        flag = 1;
    }

    clcd_print("HH:MM:SS ", LINE1(0)); // Display time format on LCD

    // Toggle the display every 150 cycles
    if (++delay == 150) {
        delay = 0;
        toggle = !toggle;
    }

    // Display the current time and toggle the blinking cursor
    if (present_field == 0) {
        if (toggle == 0) {
            clcd_print("  ", LINE2(0));
        } else {
            clcd_putch((Hours / 10) + '0', LINE2(0));
            clcd_putch((Hours % 10) + '0', LINE2(1));
            clcd_putch(':', LINE2(2));
            clcd_putch((Minutes / 10) + '0', LINE2(3));
            clcd_putch((Minutes % 10) + '0', LINE2(4));
            clcd_putch(':', LINE2(5));
            clcd_putch((Seconds / 10) + '0', LINE2(6));
            clcd_putch((Seconds % 10) + '0', LINE2(7));
        }
    } else if (present_field == 1) {
        if (toggle == 0) {
            clcd_print("  ", LINE2(3));
        } else {
            clcd_putch((Hours / 10) + '0', LINE2(0));
            clcd_putch((Hours % 10) + '0', LINE2(1));
            clcd_putch(':', LINE2(2));
            clcd_putch((Minutes / 10) + '0', LINE2(3));
            clcd_putch((Minutes % 10) + '0', LINE2(4));
            clcd_putch(':', LINE2(5));
            clcd_putch((Seconds / 10) + '0', LINE2(6));
            clcd_putch((Seconds % 10) + '0', LINE2(7));
        }
    } else if (present_field == 2) {
        if (toggle == 0) {
            clcd_print("  ", LINE2(6));
        } else {
            clcd_putch((Hours / 10) + '0', LINE2(0));
            clcd_putch((Hours % 10) + '0', LINE2(1));
            clcd_putch(':', LINE2(2));
            clcd_putch((Minutes / 10) + '0', LINE2(3));
            clcd_putch((Minutes % 10) + '0', LINE2(4));
            clcd_putch((Seconds / 10) + '0', LINE2(6));
            clcd_putch((Seconds % 10) + '0', LINE2(7));
        }
    }

    // Handle key presses to navigate and update time fields
    if (key == MK_SW2) 
    {
        if (present_field < 3) {
            present_field++;
        } else {
            present_field = 0;
        }
    }
    if (present_field == 0) 
    {
        if (key == MK_SW1) {
            if (Hours < 23) {
                Hours++; // Increment hour
            } else {
                Hours = 0; // Reset to 0 if hour exceeds 23
            }
        }
    }
    if (present_field == 1) 
    {
        if (key == MK_SW1) {
            if (Minutes < 59) {
                Minutes++; // Increment minute
            } else {
                Minutes = 0; // Reset to 0 if minute exceeds 59
            }
        }
    }
    if (present_field == 2) 
    {
        if (key == MK_SW1) {
            if (Seconds < 59) {
                Seconds++; // Increment second
            } else {
                Seconds = 0; // Reset to 0 if second exceeds 59
            }
        }
    }

    // Store the updated time in the RTC
    if (key == MK_SW11) 
    {
        write_ds1307(HOUR_ADDR, ((Hours / 10) << 4) | Hours % 10);
        write_ds1307(MIN_ADDR, ((Minutes / 10) << 4) | Minutes % 10);
        write_ds1307(SEC_ADDR, ((Seconds / 10) << 4) | Seconds % 10);
        delay = 0;
        present_field = 0;
        flag = 0; 
        toggle = 0;
        CLEAR_DISP_SCREEN;//clear display
        state = e_dashboard; // Go back to dashboard
    }
    if(key == MK_SW12)
    {
       //make all values to 0
        delay = 0;
        present_field = 0;
        flag = 0;
        toggle = 0;
        CLEAR_DISP_SCREEN;//clear display
        state = e_dashboard;// Go back to dashboard
        
    }
}
