#include "constants.h"
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include "uart.h"
#include "RFM69.h"
#include "RFM69registers.h"
#include "LowPower.h"

#define SERIAL_DEBUG 1

#define FREQUENCY     RF69_433MHZ
#define NODEID        2
#define OTHERID       1
#define NETWORKID     42
#define WAIT_INTERVAL 1

// Upper threshold of voltage
#define UPPER_TRESHOLD 3400

RFM69 radio;
char buffer[32] ;
int counter = 0;

volatile bool adcDone;

ISR(ADC_vect) { adcDone = true; }

static void flash() {
	PORTB ^= 0xFF; // invert all the pins
	delay(100); // wait some time
	PORTB ^= 0xFF; // invert all the pins
	delay(100); // wait some time
}


static int vccRead (byte count =4) {
	set_sleep_mode(SLEEP_MODE_ADC);
	ADMUX = bit(REFS0) | 14; // use VCC and internal bandgap
	bitSet(ADCSRA, ADIE);
	while (count-- > 0) {
		adcDone = false;
		while (!adcDone) sleep_mode();
	}
	bitClear(ADCSRA, ADIE);

	int x = ADC;
	return x ? (1100L*1023) / x : -1;
}

// Voltage musn't go too high to protect the radio
static void burnVolateIfNeeded() {
	_delay_ms(5); // Wait for bandgap voltage stabilize
	int voltage;
check:
	voltage = vccRead();
	if (voltage > UPPER_TRESHOLD) {
		_delay_ms(100); // delay
		//TODO: maybe turn on the LED
#if SERIAL_DEBUG
		Serial.println("Voltage exceeded");
#endif
		goto check;
	}
}

static void longSleep(uint16_t minutes) {
	// Put the radio to sleep
	radio.sleep();

	uint32_t sleepCycles = minutes*60/8;
	while (sleepCycles > 0) {
#if SERIAL_DEBUG
		Serial.flush();
#endif
		LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
		sleepCycles--;
		burnVolateIfNeeded();
#if SERIAL_DEBUG
		// Debug stuff
		flash();
		Serial.print("- Sleep cycles left: "); Serial.println(sleepCycles);
		{
			int voltage = vccRead();
			Serial.print("Bandgap voltage: "); Serial.println(voltage);
		}
#endif
	}

	// Wake up the radio
	radio.receiveDone();
}

int main(void) {
	// Arduino init
	init();
	// uart
	//uart_init();
    	//stdout = &uart_output;
        //stdin  = &uart_input;
#if SERIAL_DEBUG
	Serial.begin(9600);
#endif

	// LEDS
	DDRB = 0xFF; // PORTB is output, all pins
	PORTB = 0x00; // Make pins low to start

	flash();
	Serial.println("Starting Preparations");

	radio.initialize(FREQUENCY, NODEID, NETWORKID);
//	Serial.println("Radio init done");
	radio.setPowerLevel(31);
//	Serial.println("Radio level set");
//	radio.readAllRegs();
	radio.rcCalibration();
//	Serial.println("RC calibration done");
	radio.setFrequency(433200000);
//	Serial.println("Freq set done");

	uint8_t powerLevel=15;
	radio.setPowerLevel(powerLevel);

	flash();
	Serial.println("Preparations done");

	radio.writeReg(REG_DATAMODUL,
			RF_DATAMODUL_DATAMODE_PACKET |
			RF_DATAMODUL_MODULATIONTYPE_OOK |
			RF_DATAMODUL_MODULATIONSHAPING_00);

	int voltage;

	for (;;) {
		voltage = vccRead();
#if SERIAL_DEBUG
		Serial.print("Bandgap voltage: "); Serial.println(voltage);
		Serial.println("Sending");
#endif
		flash();
		int c = sprintf(buffer, "%d %d", ++counter, voltage);
		bool ok = radio.sendWithRetry(OTHERID, buffer, c, 5/*Retries*/, 255/*Wait between attempts*/);
		flash();
		if (ok) flash();
		longSleep(WAIT_INTERVAL);

		if (serialEventRun) serialEventRun();
	}
	return 0;
}
