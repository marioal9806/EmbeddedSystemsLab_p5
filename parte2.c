#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MODE_READ 0
#define MODE_WRITE 1

#define MAX_LEN 32

char wbuf[MAX_LEN];

typedef enum {
    NO_ACTION,
    I2C_BEGIN,
    I2C_END
} i2c_init;

uint8_t  init = NO_ACTION;
uint16_t clk_div = BCM2835_I2C_CLOCK_DIVIDER_148;
uint8_t slave_address = 0x00;
uint32_t len = 0;
uint8_t  mode = MODE_READ;

//comparse

char buf[MAX_LEN];
int i;
uint8_t data;
FILE *f;
int x=1;

int createpack (uint8_t  init, uint16_t clk_div, uint8_t slave_address, 
                    uint32_t len, uint8_t  mode){

    if (!bcm2835_init())
    {
      printf("bcm2835_init failed. Are you running as root??\n");
      return 1;
    }
      
    // I2C begin if specified    
    if (init == I2C_BEGIN)
    {
      if (!bcm2835_i2c_begin())
      {
        printf("bcm2835_i2c_begin failed. Are you running as root??\n");
        return 1;
      }
    }
	  
    bcm2835_i2c_setSlaveAddress(slave_address);
    bcm2835_i2c_setClockDivider(clk_div);
    //fprintf(stderr, "Clock divider set to: %d\n", clk_div);
    //fprintf(stderr, "len set to: %d\n", len);
    //fprintf(stderr, "Slave address set to: %d\n", slave_address);   
    
    if (mode == MODE_READ) {
    	for (i=0; i<MAX_LEN; i++) buf[i] = 'n';
    	data = bcm2835_i2c_read(buf, len);
    	//printf("Read Result = %d\n", data);   
    	for (i=0; i<MAX_LEN; i++) {
    		if(buf[i] != 'n'){
			 //printf("Read Buf[%d] = %d\n", i, buf[i]);
		} 
	}
	    if(slave_address == 104){
            fprintf(f, "Record %d: ", x);
            fprintf(f, "%x/", buf[4]); //dia
            fprintf(f, "%x/", buf[5]); //mes
            fprintf(f, "%x", buf[6]); //año
            //Dia de la semana
            char *weekdays[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
            fprintf(f, " %s ", weekdays[buf[3]]);
            fprintf(f, "%x", buf[2] & 31); //horas 
            fprintf(f, ":%x:", buf[1]); //minutos
            fprintf(f, "%x", buf[0]); //segundos
            // PM o AM -> bit 5 de buf [2]
            int AM_PM = ((buf[2] << 2) >> 7);
            fprintf(f, " %s\n", AM_PM ? "PM" : "AM");
            x++; //para el siguiente record
	    }else{
        	fprintf(f, "Tempreture: %d°C\n", buf[0]); 
    	}
    }

    if (mode == MODE_WRITE) {
    	data = bcm2835_i2c_write(wbuf, len);
    	//printf("Write Result = %d\n", data);
    }
   

    // This I2C end is done after a transfer if specified
    if (init == I2C_END) bcm2835_i2c_end();
    bcm2835_close();
    return 1;
}

int main(int argc, char **argv) {

    printf("Running ... \n");
    
    // parse the command line
    //if (comparse(argc, argv) == EXIT_FAILURE) return showusage (EXIT_FAILURE);
    f=fopen("log.txt", "w");
    
    uint8_t  initial = I2C_BEGIN;
    uint16_t clk=2500;
    uint8_t slave=77;
    uint32_t len=1;
    uint8_t  mode2=MODE_WRITE;

    memset(wbuf, 0, sizeof(wbuf));

    createpack(initial, clk, slave, len, mode2);

    mode2=MODE_READ;

    createpack(initial, clk, slave, len, mode2);

    slave = 104;
    mode2=MODE_WRITE;
    memset(wbuf, 0, sizeof(wbuf));
    
    createpack(initial, clk, slave, len, mode2);

    mode2=MODE_READ;
    len = 7;
    createpack(initial, clk, slave, len, mode2);

    fclose(f);   
    printf("... done!\n");
    return 0;
}

