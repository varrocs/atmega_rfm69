#include "constants.h"
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include "uart.h"
#include "RFM69.h"
#include "RFM69registers.h"
#include "LowPower.h"
#include "dht.h"

#define SERIAL_DEBUG 0
#define FAST_SLEEP   0

#define FREQUENCY     		RF69_433MHZ
#define NODEID        		2
#define OTHERID       		1
#define NETWORKID     		42
#define WAIT_INTERVAL 		1
#define PIN_LED       		9
#define PIN_RADIO_RESET 	8
#define PIN_DHT			7

// Upper threshold of voltage, more would kill the radio unit
#define UPPER_TRESHOLD 3400
// Experimental lower threshold for radio usage. Less is not enogh for radio usage
#define LOWER_TRESHOLD 2900

RFM69 radio;
char txBuffer[32];
int counter = 0;    // Transmission counter
dht DHT;

volatile bool adcDone;

ISR(ADC_vect) { adcDone = true; }

static void flash() {
    digitalWrite(PIN_LED, HIGH);
    delay(100); // wait some time
    digitalWrite(PIN_LED, LOW);
    delay(100); // wait some time
}

#if SERIAL_DEBUG
static inline const char* debugGetResetSource() {
	byte resetSrc = MCUSR;
	MCUSR = 0x00;	// Reset for the next read
	if (resetSrc & PORF) return "POWERON";
	else if (resetSrc & EXTRF) return "EXTERNAL";
	else if (resetSrc & BORF) return "BOD";
	else if (resetSrc & WDRF) return "WDT";
	else return "?";

}
#endif

static inline const void debugLogResetSource() {
#if SERIAL_DEBUG
	Serial.print("Reset source: "); Serial.println(debugGetResetSource());
#endif
}

static inline void debugLog(const char* message) {
#if SERIAL_DEBUG
	Serial.begin(9600);
        Serial.println(message);
	Serial.flush();
	Serial.end();
#endif
}

static inline void debugLog(int message) {
#if SERIAL_DEBUG
	Serial.begin(9600);
        Serial.println(message);
	Serial.flush();
	Serial.end();
#endif
}

static inline void debugLogFlush() {
#if SERIAL_DEBUG
        //Serial.flush();
        //delay(10);
#endif
}

static inline void debugFlash(int times = 1) {
#if SERIAL_DEBUG
    for (int i = 0; i<times; ++i) {
	    flash();
    }
#endif
}

// Return vcc * 1000
static int vccRead(byte count=4) {
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

// Voltage musn't go too high to protect the radio chip
static void burnVoltageIfNeeded() {
    _delay_ms(5); // Wait for bandgap voltage stabilize
    bool overVoltage;
    do {
        overVoltage = (vccRead() > UPPER_TRESHOLD);
        if (overVoltage) {
            flash();    // burn some voltage
            debugLog("Voltage exceeded");
        }
    } while (overVoltage);
}

static bool readDHT() {
	int chk = DHT.read11(PIN_DHT);
	return chk == DHTLIB_OK;
}

static void longSleep(const uint16_t minutes) {
    // Put the radio to sleep
    //debugLog("RADIO sleep");
    //radio.sleep();

#if FAST_SLEEP
    uint32_t sleepCycles = minutes;		// For debug purposes
#else
    uint32_t sleepCycles = minutes*60/8;	// minutes to sleep cycles
#endif
    while (sleepCycles > 0) {
        debugLogFlush();
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
        debugLog("SYNC SYNC SYNC");
        sleepCycles--;
        burnVoltageIfNeeded();
#if SERIAL_DEBUG
        // Debug stuff
        debugLog("Sleep cycles left: "); Serial.println(sleepCycles);
        {
            int voltage = vccRead();
            Serial.print("Sleep bandgap voltage: "); Serial.println(voltage);
        }
#endif
    }

    // Wake up the radio
    //debugLog("RADIO wake");
    //radio.receiveDone();
}


void resetRadio() {
    digitalWrite(PIN_RADIO_RESET, HIGH);
    delay(10);
    digitalWrite(PIN_RADIO_RESET, LOW);
    delay(10);
}

void setupRadio() {
    debugLog("Radio init");
    radio.initialize(FREQUENCY, NODEID, NETWORKID);
    debugLog("Radio init done");
    radio.setPowerLevel(31);
    debugLog("Radio level set");
//  radio.readAllRegs();
    radio.rcCalibration();
    debugLog("RC calibration done");
    radio.setFrequency(433200000);
    debugLog("Freq set done");

    const uint8_t powerLevel=15;
    radio.setPowerLevel(powerLevel);

    radio.writeReg(REG_DATAMODUL,
            RF_DATAMODUL_DATAMODE_PACKET |
            RF_DATAMODUL_MODULATIONTYPE_OOK |
            RF_DATAMODUL_MODULATIONSHAPING_00);
}

void transmitWithRadio(char* message, size_t length) {
    setupRadio();

    const uint8_t retryCount = 5;
    const uint8_t waitBetweenRetry = 255;
    debugLog("Sending");
    bool ok = radio.sendWithRetry(OTHERID, txBuffer, length, retryCount, waitBetweenRetry);
    debugLog(ok ? "OK" : "FAIL");

    radio.sleep();

    debugFlash( ok ? 2 : 1 );
}


void setup() {
    debugLog("-------  RESET  --------");
    debugLogResetSource();
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_RADIO_RESET, OUTPUT);
    digitalWrite(PIN_RADIO_RESET, LOW);

    setupRadio();
    radio.sleep();

    debugLog("Preparations done");
}

void cycle() {
    longSleep(WAIT_INTERVAL);
    const int voltage = vccRead();

    debugLog("X Bandgap voltage: ");
    debugLog(voltage);
    debugFlash();

    if (voltage > LOWER_TRESHOLD) {
	bool dhtOk = readDHT();
        int c = snprintf(txBuffer, sizeof txBuffer,
			"%d %d %d %d",
			++counter,
			voltage,
			dhtOk ? (int)(DHT.temperature*10) : -99,
			dhtOk ? (int)(DHT.humidity*10) : -99
		      	);
        transmitWithRadio(txBuffer, c);
        resetRadio();
	setupRadio();
	radio.sleep();
    }
    radio.sleep();
}


int main(void) {
    // Arduino init
    init();

    // Own setup
    setup();

    for (;;) {
	cycle();
        if (serialEventRun) serialEventRun();
    }
    return 0;
}
