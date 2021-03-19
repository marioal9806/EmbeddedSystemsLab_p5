#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

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

uint8_t  initial = I2C_BEGIN;
uint16_t clk=2500;
uint8_t slave;
uint32_t len;
uint8_t  mode2=MODE_WRITE;

//comparse

char buf[MAX_LEN];
char buf2[MAX_LEN];
char buf3[MAX_LEN];
char buf4[MAX_LEN];
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

void PacketeSensor(){
    slave = 77;
    len = 1;
    mode2=MODE_WRITE;
    memset(wbuf, 0, sizeof(wbuf));

    createpack(initial, clk, slave, len, mode2);

    mode2=MODE_READ;
    createpack(initial, clk, slave, len, mode2);
}

void PacketeReloj(){
    slave = 104;
    len = 1;
    mode2=MODE_WRITE;
    memset(wbuf, 0, sizeof(wbuf));
    createpack(initial, clk, slave, len, mode2);

    mode2=MODE_READ;
    len = 7;
    createpack(initial, clk, slave, len, mode2);
}

void printTiempo(char bufa[MAX_LEN]){
  fprintf(f, "Record %d: ", x);
  fprintf(f, "%x/", bufa[4]); //dia
  fprintf(f, "%x/", bufa[5]); //mes
  fprintf(f, "%x", bufa[6]); //año
  //Dia de la semana
  char *weekdays[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  fprintf(f, " %s ", weekdays[bufa[3]]);
  fprintf(f, "%x", bufa[2] & 31); //horas 
  fprintf(f, ":%x:", bufa[1]); //minutos
  fprintf(f, "%x", bufa[0]); //segundos
  // PM o AM -> bit 5 de buf [2]
  int AM_PM = ((bufa[2] << 2) >> 7);
  fprintf(f, " %s\n", AM_PM ? "PM" : "AM");
  x++; //para el siguiente record
}

void printTemp(){
  fprintf(f, "Tempreture: %d°C\n", buf[0]); 
}

int main(int argc, char **argv) {

    printf("Running ... \n");
    int contadorS = 0;
    int Z=0;
    
    while(Z<=30){
      sleep(1);
      if (contadorS >= 10) {
       //PacketeSensor();
       //printTemp();
       PacketeReloj();
       //printTiempo();
       for(int d=0; d<7; d++){
          buf4[d]=buf3[d];
          buf3[d]=buf2[d];
          buf2[d]=buf[d];
       }
       contadorS = 0;
      }
      contadorS ++;
      PacketeSensor();
      if (buf[0]>=30){
        PacketeReloj();
        //printTiempo();
        for(int d=0; d<7; d++){
          buf4[d]=buf3[d];
          buf3[d]=buf2[d];
          buf2[d]=buf[d];
        }
      }
      Z++;
    }

    f=fopen("log.txt", "w");
    PacketeSensor();
    printTemp();

    printTiempo(buf4);
    printTiempo(buf3);
    printTiempo(buf2);
    
    fclose(f);   
    printf("... done!\n");
    return 0;
}

