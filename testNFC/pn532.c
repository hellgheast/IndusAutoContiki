
/*
 * Copyright (c) 2016, HE-Arc, Sébastien Glauser.
 */

/**
 * \file
 *         PN532 MIFARE manager
 * 		   Inspired by https://github.com/adafruit/Adafruit_NFCShield_I2C
 * \author
 *         Sébastien Glauser <seb.glauser@gmail.com>
 */

#include "contiki.h"
#include <pn532.h>
#include "dev/i2cmaster.h"
#include <stdio.h>
#include <string.h>
#include <core/sys/timer.h>

uint8_t pn532ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
uint8_t pn532response_firmwarevers[] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};
#define PN532_PACKBUFFSIZ 64
uint8_t pn532_packetbuffer[PN532_PACKBUFFSIZ];

static uint8_t wirereadstatus(){
 		//(P4IN & 0x01) != 0
		if((P4IN & 0x01) != 0){
		 	return PN532_I2C_BUSY;
		}
		else{
			return PN532_I2C_READY;
		}
}

static void wiresendcommand(uint8_t * cmd, uint8_t cmdlen){
  uint8_t aTemp[PN532_PACKBUFFSIZ];
	uint8_t aPacketDataChecksum;
	aTemp[0]=0x00;
	aTemp[1]=0x00;
	aTemp[2]=0xFF;
	aTemp[3]=cmdlen+2;
	aTemp[4]=0-cmdlen-2;
	aTemp[5]=PN532_HOSTTOPN532;
	aPacketDataChecksum=0-0xD4;
	uint8_t i;
	for(i=0;i<cmdlen;i++){
		aTemp[6+i]=cmd[i];
		aPacketDataChecksum-=cmd[i];
	}
	aTemp[6+i]=aPacketDataChecksum;
	aTemp[7+i]=0x00;
	printf("\nsend:");
	for(i=0;i<8+cmdlen;i++){
		printf(" 0x%02x", aTemp[i]);
	}
 	i2c_transmitinit(PN532_I2C_ADDRESS);
  while(i2c_busy());
  i2c_transmit_n(8+cmdlen,aTemp);
	while(i2c_busy());

}

static void wirereaddata(uint8_t* buff, uint8_t n){
	uint8_t STATUS;
	uint8_t aTemp[PN532_PACKBUFFSIZ]; 
	i2c_enable();
	do{
  	i2c_receiveinit(PN532_I2C_ADDRESS);
  	while(i2c_busy());
		i2c_receive_n(1, &STATUS);
    while(i2c_busy());
	}while((STATUS&0x01)==0x00);
	i2c_receiveinit(PN532_I2C_ADDRESS);
  while(i2c_busy());
	i2c_receive_n(n, aTemp);
  while(i2c_busy());
  uint8_t i;
	printf("\nrecieved:");
	for(i=0;i<n;i++){
		buff[i]=aTemp[i+1];
		printf(" 0x%02x", aTemp[i+1]);
	}
}

uint8_t sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen){
	uint8_t ackbuff[6];	
	wiresendcommand(cmd, cmdlen);
  while(wirereadstatus()==PN532_I2C_BUSY);
  wirereaddata(ackbuff, 6);
	
  return (0 == strncmp((char *)ackbuff, (char *)pn532ack, 6));
}


uint8_t setPassiveActivationRetries(uint8_t maxRetries) {
  pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
  pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
  pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
  pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
  pn532_packetbuffer[4] = maxRetries;

  if (! sendCommandCheckAck(pn532_packetbuffer, 5))
    return 0x0;  // no ACK
  
  return 1;
}

// Generic PN532 functions
uint8_t SAMConfig(void){
	i2c_enable();
  P4DIR&= ~0x01; // set the port as input
  P4SEL &= ~0x01;                  
  P4REN |=  0x01;   
	pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
  pn532_packetbuffer[1] = 0x01; // normal mode;
  pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
  pn532_packetbuffer[3] = 0x01; // use IRQ pin!
  
  if (! sendCommandCheckAck(pn532_packetbuffer, 4))
     return 0x00;
	

  // read data packet
  wirereaddata(pn532_packetbuffer, 8);
  
  return  (pn532_packetbuffer[6] == 0x15);
}

uint32_t getFirmwareVersion(void) {
  uint32_t response;

  pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

  if (! sendCommandCheckAck(pn532_packetbuffer, 1))
    return 0;
	
  // read data packet
  wirereaddata(pn532_packetbuffer, 12);
  
  // check some basic stuff
  if (0 != strncmp((char *)pn532_packetbuffer, (char *)pn532response_firmwarevers, 6)) {
    return 0;
  }
  
  response = pn532_packetbuffer[7];
  response <<= 8;
  response |= pn532_packetbuffer[8];
  response <<= 8;
  response |= pn532_packetbuffer[9];
  response <<= 8;
  response |= pn532_packetbuffer[10];

  return response;
}

uint8_t readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength) {
  pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
  pn532_packetbuffer[2] = cardbaudrate;
  
	sendCommandCheckAck(pn532_packetbuffer, 3);


	while(wirereadstatus()==PN532_I2C_BUSY);

  // read data packet
  wirereaddata(pn532_packetbuffer, 20);
  
  // check some basic stuff
  /* ISO14443A card response should be in the following format:
  
    byte            Description
    -------------   ------------------------------------------
    b0..6           Frame header and preamble
    b7              Tags Found
    b8              Tag Number (only one used in this example)
    b9..10          SENS_RES
    b11             SEL_RES
    b12             NFCID Length
    b13..NFCIDLen   NFCID                                      */
  
  if (pn532_packetbuffer[7] != 1) 
    return 0;
    
  uint16_t sens_res = pn532_packetbuffer[9];
  sens_res <<= 8;
  sens_res |= pn532_packetbuffer[10];

  
  /* Card appears to be Mifare Classic */
  *uidLength = pn532_packetbuffer[12];
	uint8_t i;
  for (i=0; i < pn532_packetbuffer[12]; i++) 
  {
    uid[i] = pn532_packetbuffer[13+i];
  }

  return 1;
}
