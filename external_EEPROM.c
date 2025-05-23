/*
 * File:   external_EEPROM.c
 * Author: chaithanya
 *
 * Created on 12 February, 2025, 2:16 PM
 */
#include "i2c.h"
#include "external_EEPROM.h"


 void write_external_EEPROM(unsigned char address1,  unsigned char data)

{
	i2c_start();
	i2c_write(SLAVE_WRITE_E);
	i2c_write(address1);
	i2c_write(data);
	i2c_stop();
    
    for(unsigned int i = 3000;i--;);
}

unsigned char read_external_EEPROM(unsigned char address1)
{
	unsigned char data;

	i2c_start();
	i2c_write(SLAVE_WRITE_E);
	i2c_write(address1);
	i2c_rep_start();
	i2c_write(SLAVE_READ_E);
	data = i2c_read();
	i2c_stop();

	return data;
}
