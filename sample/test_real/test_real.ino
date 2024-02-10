
#include "config.h"
#include "platform.h"
#include "hw_w5500.h"
#include "proto_mac.h"
#include "proto_ip6.h"
#include "proto_udp.h"
#include "proto_coap.h"

struct net_coap_ctx coap;
struct net_udp_ctx udp;
struct net_ip6_ctx ip6;
struct net_mac_ctx mac;
struct hw_w5500_ctx w5500;

uint8_t buffer[1514];

uint8_t src_addr[16] = {0xfd,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x01,
                        0x00,0x02,
                        0x00,0x03,
                        0x00,0x0f};
uint8_t dst_addr[16] = {0xfd,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x00,
                        0x00,0x01,
                        0x00,0x02,
                        0x00,0x03,
                        0x00,0x04};

uint8_t src_l2addr[6] = {0x00, 0xd0, 0x12, 0xa2, 0xf3, 0x2f};
uint8_t dst_l2addr[6] = {0x00, 0xd0, 0x12, 0xa2, 0xf3, 0x21};
net_mac_mcsuffix_t mcsuffixes[] = NET_IP6_L2_MCSUFFIXES(src_addr);


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
	serial_debug("Init. real test");

	pinMode(53, OUTPUT);
	pinMode(4, OUTPUT);
	digitalWrite(4, HIGH);

	hw_w5500_init();


	coap.lower = &udp;
	udp.lower = &ip6;
	ip6.lower = &mac;
	mac.lower = &w5500;
}

void loop() {
	int8_t errno = 0;
	bool up = false;
	uint8_t speed = 0;
	uint16_t framelen = 0;
	uint16_t framecnt = 0;
	uint16_t datalen = 0;
	uint16_t dataoffset = 0;
	uint8_t payload[] = "test";

	serial_debug("Setting hw");
	hw_w5500_set_macaddress(src_l2addr);
	//hw_w5500_set_phycfg(true, true, HW_SPEED_100MBPS_FD);
	hw_w5500_set_phycfg(true, false, HW_SPEED_100MBPS_HD);

	do {
		delay(1000);
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

	} while (!up);

	hw_w5500_open(&w5500);

	serial_debug("Setting mac");
	net_mac_set_source_addr(&mac, src_l2addr);
	net_mac_set_destination_addr(&mac, dst_l2addr);
	net_mac_set_ethertype(&mac, NET_MAC_ETHERTYPE_IPV6);
	net_mac_set_ip6mcast(&mac, NET_IP6_L2_MCSUFFIX_CNT, mcsuffixes);

	serial_debug("Setting ip6");
	net_ip6_set_source_addr(&ip6, src_addr);
	net_ip6_set_destination_addr(&ip6, dst_addr);
	net_ip6_set_nexthdr(&ip6, NET_IP6_NH_UDP);

	serial_debug("Setting udp");
	net_udp_set_source_port(&udp, 1234);
	net_udp_set_destination_port(&udp, 5678);

	for (int i=0; i<100; i++) {
		serial_debug("Send. UDP");
		net_udp_connect(&udp);
		dataoffset = net_udp_pload_pos(&udp);
		memcpy(&(buffer[dataoffset]), payload, 4);
		errno = net_udp_send(&udp, buffer, 1514, dataoffset, 4);
		if (errno != NET_STATUS_OK) {
			serial_debug_beg();
			Serial.print("UDP Error: ");
			Serial.print(errno);
			serial_debug_end();
		}

		for (int wait=0; wait<10000; wait++) {
			delay(1);
			if (net_udp_recv(&udp, buffer, 1514, &dataoffset, &datalen) == NET_STATUS_OK) {
				serial_debug_beg();
				Serial.print("Recv. data: ");
				for (int d=0; d<datalen; d++) {
					Serial.print(buffer[dataoffset+d], HEX);
				}
				serial_debug_end();
			}
		}
	}

	//hw_w5500_destroy(NULL);
}
