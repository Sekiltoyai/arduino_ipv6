/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config.h"
#include "platform.h"

#include <SPI.h>

// TODO: Put in argument
#define SPI_PIN 10


void msleep(uint16_t time_ms)
{
	delay(time_ms);
}

#ifdef USE_SPI

void spi_init()
{
	SPI.begin();
	pinMode(SPI_PIN, OUTPUT);
	digitalWrite(SPI_PIN, HIGH);
}

void spi_destroy()
{
	SPI.end();
}

void spi_start_transaction()
{
	SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
}

void spi_stop_transaction()
{
	SPI.endTransaction();
}

void spi_start_transfer()
{
	digitalWrite(10, LOW);
}

void spi_stop_transfer()
{
	digitalWrite(10, HIGH);
}

uint8_t spi_read_byte()
{
	return SPI.transfer(0x00);
}

void spi_read(uint8_t *buffer, uint16_t buflen)
{
	SPI.transfer(buffer, buflen);
}

void spi_write(uint8_t *buffer, uint16_t buflen)
{
	SPI.transfer(buffer, buflen);
}

#else

void spi_init() {}
void spi_destroy() {}
void spi_start_transaction() {}
void spi_stop_transaction() {}
void spi_start_transfer() {}
void spi_stop_transfer() {}
uint8_t spi_read_byte() { return 0; }
void spi_read(uint8_t *buffer, uint16_t buflen) {}
void spi_write(uint8_t *buffer, uint16_t buflen) {}

#endif

#ifdef USE_SERIAL

#define PACKET_TIMEOUT 500

void serial_init()
{
	Serial.begin(9600);
}

void serial_debug_beg()
{
	uint8_t value;

	value = 'D';
	Serial.write(&value, 1);

	value = ':';
	Serial.write(&value, 1);

	value = ' ';
	Serial.write(&value, 1);
}

void serial_debug_end()
{
	uint8_t value;

	value = '\n';
	Serial.write(&value, 1);

	Serial.flush();
}

void serial_debug(const char * const message)
{
	serial_debug_beg();
	Serial.write(message, strlen(message));
	serial_debug_end();
}

void serial_signal(uint8_t signal)
{
	uint16_t i = 0;
	uint8_t value;
	uint8_t high;
	uint8_t low;

	value = 'T';
	Serial.write(&value, 1);

	value = ':';
	Serial.write(&value, 1);

	value = ' ';
	Serial.write(&value, 1);

	Serial.print(signal);

	value = '\n';
	Serial.write(&value, 1);

	Serial.flush();
}

uint16_t serial_read(uint8_t *buffer, uint16_t buflen)
{
	uint8_t state = 0;
	uint16_t i = 0;
	uint8_t chr = 0;
	uint8_t current_byte = 0;
	uint16_t elapsed = 0;

	while (i < buflen) {
		if (Serial.available() <= 0) {
			delay(1);
			elapsed++;
			if (elapsed < PACKET_TIMEOUT) {
				continue;
			} else {
				return 0;
			}
		}

		chr = Serial.read();
		if (chr == '\n') {
			break;
		}

		switch (state) {
		// First character of the line
		case 0:
			if (chr == 'P') {
				state = 1;
			} else {
				state = 255;
			}
			break;

		// Matched 'P'acket mode
		case 1:
			if (chr == ':') {
				state = 2;
			} else {
				state = 255;
			}
			break;

		// Matched ':'
		case 2:
			if (chr == ' ') {
				state = 2;
			} else if (((chr >= '0') && (chr <= '9')) || ((chr >= 'A') && (chr <= 'F'))) {
				state = 3;
				current_byte = (((chr >= '0') && (chr <= '9')) ? (chr - '0') : (chr - 'A') + 10) << 4;
			} else {
				state = 255;
			}
			break;

		// Byte hexa low
		case 3:
			if (((chr >= '0') && (chr <= '9')) || ((chr >= 'A') && (chr <= 'F'))) {
				state = 4;
				current_byte += (((chr >= '0') && (chr <= '9')) ? (chr - '0') : (chr - 'A') + 10);
				buffer[i] = current_byte;
				i++;
			} else {
				state = 255;
			}
			break;

		// Byte hexa high
		case 4:
			if (((chr >= '0') && (chr <= '9')) || ((chr >= 'A') && (chr <= 'F'))) {
				state = 3;
				current_byte = (((chr >= '0') && (chr <= '9')) ? (chr - '0') : (chr - 'A') + 10) << 4;
			} else {
				state = 255;
			}
			break;

		// Skip the rest of the line
		case 255:
			break;

		default:
			break;
		}
	}

	return i;
}

uint8_t serial_wait_for_signal(uint16_t timeout)
{
	uint8_t state = 0;
	uint8_t chr = 0;
	uint8_t current_byte = 0;
	uint16_t elapsed = 0;

	while (current_byte < 26) {
		if (Serial.available() <= 0) {
			delay(1);
			elapsed++;
			if (elapsed < timeout) {
				continue;
			} else {
				return 0;
			}
		}

		chr = Serial.read();
		if (chr == '\n') {
			break;
		}

		switch (state) {
		// First character of the line
		case 0:
			if (chr == 'T') {
				state = 1;
			} else {
				state = 255;
			}
			break;

		// Matched 'T'est frame
		case 1:
			if (chr == ':') {
				state = 2;
			} else {
				state = 255;
			}
			break;

		// Matched ':'
		case 2:
			if (chr == ' ') {
				state = 2;
			} else if ((chr >= '0') && (chr <= '9')) {
				state = 3;
				current_byte = (chr - '0');
			} else {
				state = 255;
			}
			break;

		// Next digit
		case 3:
			if ((chr >= '0') && (chr <= '9')) {
				state = 3;
				current_byte = (current_byte * 10) + (chr - '0');
			} else {
				state = 255;
			}
			break;

		// Skip the rest of the line
		case 255:
			break;

		default:
			break;
		}
	}

	return current_byte;
}

uint16_t serial_write(uint8_t *buffer, uint16_t buflen)
{
	uint16_t i = 0;
	uint8_t value;
	uint8_t high;
	uint8_t low;

	value = 'P';
	Serial.write(&value, 1);

	value = ':';
	Serial.write(&value, 1);

	value = ' ';
	Serial.write(&value, 1);

	while (i < buflen) {
		high = (buffer[i] & 0xF0) >> 4;
		low = (buffer[i] & 0x0F);

		value = ((high < 10) ? high + '0' : (high - 10) + 'A');
		Serial.write(&value, 1);

		value = ((low < 10) ? low + '0' : (low - 10) + 'A');
		Serial.write(&value, 1);

		i++;
	}

	value = '\n';
	Serial.write(&value, 1);

	Serial.flush();

	return i;
}


#else

void serial_init() {}
void serial_debug_beg() {}
void serial_debug_end() {}
void serial_debug(const char * const message) {}
void serial_signal(uint8_t signal) {}
uint16_t serial_read(uint8_t *buffer, uint16_t buflen) { return 0; }
uint8_t serial_wait_for_signal(uint16_t timeout) { return 0; }
uint16_t serial_write(uint8_t *buffer, uint16_t buflen) {}

#endif
