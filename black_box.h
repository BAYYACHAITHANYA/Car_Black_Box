/* 
 * File:   black_box.h
 * Author: chait
 *
 * Created on 3 February, 2025, 12:43 PM
 */

#include <xc.h>

/* Enum for maintaining the app state */
typedef enum {
    e_dashboard, e_main_menu, e_view_log, e_clear_log,e_download_log,e_set_time
} State_t;

unsigned char key;
extern State_t state; // App state
unsigned char clock_reg[3];
unsigned char time[9];
//Function declarations

//Dashboard function declaration
void view_dashboard(void);

//Password function declaration
void password(void);

//main menu function declaration
void display_main_menu(void);

//View log function declaration
void view_log(void);

//Reading events function declaration
void event_reader(void);

//Change password function declaration
void change_password(void);

//Set time function declaration
void set_time(void);

//Download log function _decleration
void download_log(void);

//Clear log function declaration
void clear_log(void);

//storing event function declecration
void store_event(unsigned char time[], unsigned char ev[], unsigned int speed);