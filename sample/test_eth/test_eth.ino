
#include "platform.h"
#include "hw_w5500.h"

//uint8_t buffer[1514];


void print_mac(uint8_t *macaddress)
{
	Serial.print(macaddress[0], HEX);
	Serial.print(":");
	Serial.print(macaddress[1], HEX);
	Serial.print(":");
	Serial.print(macaddress[2], HEX);
	Serial.print(":");
	Serial.print(macaddress[3], HEX);
	Serial.print(":");
	Serial.print(macaddress[4], HEX);
	Serial.print(":");
	Serial.print(macaddress[5], HEX);
}

void setup() {
	serial_init();
	serial_debug("Initializing W5500 test");

	pinMode(53, OUTPUT);
	pinMode(4, OUTPUT);
	digitalWrite(4, HIGH);

	//hw_w5500_init();
}

void loop() {
	uint8_t version = 0;
	bool up = false;
	uint8_t speed = 0;
	uint16_t framelen = 0;
	uint16_t framecnt = 0;

	uint8_t macaddress[6];
	macaddress[0] = 0x00;
	macaddress[1] = 0x11;
	macaddress[2] = 0x22;
	macaddress[3] = 0x33;
	macaddress[4] = 0x44;
	macaddress[5] = 0x55;

	serial_debug("hw_w5500_read_version()");
	hw_w5500_read_version(&version);
	serial_debug_beg();
	Serial.print("Chip version: ");
	Serial.print(version);
	serial_debug_end();

	serial_debug("hw_w5500_set_phycfg()");
	//hw_w5500_set_phycfg(true, true, HW_SPEED_100MBPS_FD);
	hw_w5500_set_phycfg(true, false, HW_SPEED_100MBPS_HD);

	delay(2000);
	hw_w5500_get_phycfg(&up, &speed);
	serial_debug_beg();
	Serial.print("Link: ");
	if (up) {
		Serial.print("UP");
	} else {
		Serial.print("DOWN");
	}
	serial_debug_end();
	serial_debug_beg();
	Serial.print("Speed: ");
	switch (speed) {
	case HW_SPEED_10MBPS_HD:
		Serial.print("10Mbps Half-Duplex");
		break;
	case HW_SPEED_10MBPS_FD:
		Serial.print("10Mbps Full-Duplex");
		break;
	case HW_SPEED_100MBPS_HD:
		Serial.print("100Mbps Half-Duplex");
		break;
	case HW_SPEED_100MBPS_FD:
		Serial.print("100Mbps Full-Duplex");
		break;
	case HW_SPEED_NONE:
	default:
		Serial.print("N/A");
		break;
	}
	serial_debug_end();

	serial_debug("hw_w5500_get_macaddress()");
	hw_w5500_get_macaddress(macaddress);
	serial_debug_beg();
	Serial.print("MAC Address: ");
	print_mac(macaddress);
	serial_debug_end();

	macaddress[0] = 0x66;
	macaddress[1] = 0x77;
	macaddress[2] = 0x88;
	macaddress[3] = 0x99;
	macaddress[4] = 0xAA;
	macaddress[5] = 0xBB;

	serial_debug("hw_w5500_set_macaddress()");
	hw_w5500_set_macaddress(macaddress);

	macaddress[0] = 0xF0;
	macaddress[1] = 0xE1;
	macaddress[2] = 0xD2;
	macaddress[3] = 0xC3;
	macaddress[4] = 0xB4;
	macaddress[5] = 0xA5;

	serial_debug("hw_w5500_get_macaddress()");
	hw_w5500_get_macaddress(macaddress);
	serial_debug_beg();
	Serial.print("MAC Address: ");
	print_mac(macaddress);
	serial_debug_end();

	do {
		serial_debug("Waiting link to be up");
		delay(2000);
		hw_w5500_get_phycfg(&up, &speed);
	} while (!up);

	if (hw_w5500_open(NULL)) {
		serial_debug("Socket 0 opened");
	} else {
		serial_debug("Failed opening socket 0");
	}

	while (true) {
#if 0
		framelen = hw_w5500_recv(NULL, buffer, 1514);
		if (framelen == 0) {
			delay(10);
		} else if ((framecnt % 1000) == 0) {
			serial_debug("Frame on s0");
			serial_debug_beg();
			Serial.print("Cnt: ");
			Serial.print(framecnt);
			serial_debug_end();
			serial_debug_beg();
			Serial.print("Len: ");
			Serial.print(framelen);
			serial_debug_end();
			serial_debug_beg();
			Serial.print("Eth: ");
			Serial.print("src=");
			print_mac(&(buffer[6]));
			Serial.print(" dst=");
			print_mac(&(buffer[0]));
			Serial.print(" ethtype=");
			Serial.print(buffer[12], HEX);
			Serial.print(":");
			Serial.print(buffer[13], HEX);
			serial_debug_end();
			serial_debug_beg();
			Serial.print("Seq: ");
			Serial.print((uint16_t) ((buffer[60] << 8) + buffer[61]), DEC);
			serial_debug_end();
			framecnt++;
		} else {
			framecnt++;
		}
#endif
	}

	//hw_w5500_destroy(NULL);
}
