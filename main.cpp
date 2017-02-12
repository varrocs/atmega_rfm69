#include "constants.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "uart.h"
#include "RFM69.h"
#include "RFM69registers.h"

#define FREQUENCY     RF69_433MHZ
#define NODEID        2
#define OTHERID      1
#define NETWORKID     42

RFM69 radio;
const char buffer[] ="X";

void flash() {
			PORTB ^= 0xFF; // invert all the pins
			delay(100); // wait some time
			PORTB ^= 0xFF; // invert all the pins
			delay(100); // wait some time
}

int main(void) {
	// Arduino init
	init();
	// uart
	//uart_init();
    	//stdout = &uart_output;
        //stdin  = &uart_input;
	Serial.begin(9600);

	// LEDS
	DDRB = 0xFF; // PORTB is output, all pins
	PORTB = 0x00; // Make pins low to start

	flash();
	Serial.println("Starting Preparations");

	radio.initialize(FREQUENCY, NODEID, NETWORKID);
	Serial.println("Radio init done");
	radio.setPowerLevel(31);
	Serial.println("Radio level set");
	radio.readAllRegs();
//	radio.setMode(RF69_MODE_RX);
//	radio.promiscuous(true);
	radio.rcCalibration();
	Serial.println("RC calibration done");
	radio.setFrequency(433200000);
	Serial.println("Freq set done");

	uint8_t powerLevel=15;
	radio.setPowerLevel(powerLevel);

	flash();
	Serial.println("Preparations done");

//	while (true) {
//		radio.readAllRegs();
//		delay(10000);
//	}
	radio.writeReg(REG_DATAMODUL,
			RF_DATAMODUL_DATAMODE_PACKET |
			RF_DATAMODUL_MODULATIONTYPE_OOK |
			RF_DATAMODUL_MODULATIONSHAPING_00);


	for (;;) {
#if 1
		bool ok = radio.sendWithRetry(OTHERID, buffer, sizeof buffer);
		flash();
		if (ok) flash();
#else
		bool done = radio.receiveDone();
		Serial.print("Done "); Serial.print(done);
		Serial.print(", DATALEN: "); Serial.print(RFM69::DATALEN);
		Serial.print(", SENDERID: "); Serial.print(RFM69::SENDERID);
		RFM69::DATA[RFM69::DATALEN] = '\0';
		Serial.println((char*)RFM69::DATA);
		if (done || RFM69::DATALEN || RFM69::SENDERID) {
			flash();
		}
#endif
		delay(300);

		if (serialEventRun) serialEventRun();
	}
	return 0;
}
