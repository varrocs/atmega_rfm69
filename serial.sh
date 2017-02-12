#!/bin/sh
SERIAL_PORT=/dev/ttyUSB0
BAUD=9600
screen -L $SERIAL_PORT $BAUD
