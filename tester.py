#!/usr/bin/env python

from __future__ import print_function

import serial
import sys
import time
import re
import binascii

from scapy.config import conf
from scapy.packet import Raw
from scapy.layers.l2 import Ether
from scapy.layers.inet6 import *
from scapy.utils6 import *
from scapy.contrib.coap import *

# Scapy config
conf.use_pcap = False

# Tests config
VERBOSE=0
VERDICT_OK=0x00
VERDICT_NOK=0x01

# Serial config
SERIAL_PORT="/dev/tty.usbmodemFA1311"
SERIAL_BAUDRATE=9600

# Serial operations
serial_obj=None

def serial_init():
	global serial_obj
	serial_obj = serial.Serial(SERIAL_PORT, SERIAL_BAUDRATE, timeout=0)

def serial_send(pkt):
	global serial_obj
	if (serial_obj == None):
		return
	tosend=("P: %s" % binascii.hexlify(bytearray(pkt.build())).upper())
	serial_obj.write(tosend)
	serial_obj.write('\n')
	if VERBOSE:
		print("> %s" % tosend)

def serial_signal(test):
	global serial_obj
	if (serial_obj == None):
		return
	tosend=("T: %s" % str(test))
	serial_obj.write(tosend)
	serial_obj.write('\n')
	if VERBOSE:
		print("> %s" % tosend)

def serial_wait_for(type, timeout):
	line = ""
	while True:
		while (serial_obj.in_waiting < 1):
			time.sleep(0.001)
			timeout -= 0.001
			if (timeout <= 0):
				return None

		chr = serial_obj.read(1)
		if chr != '\n':
			line = line + chr
			continue

		# End of line, print and process
		if (type == 'P'):
			m = re.match(r'P:\s*([0-9A-F]+)', line)
			if (m != None):
				if VERBOSE:
					print("<*%s" % line)
				#return bytearray.fromhex(m.group(1))
				return binascii.unhexlify(m.group(1))
		elif (type == 'T'):
			m = re.match(r'T:\s*([0-9]+)', line)
			if (m != None):
				if VERBOSE:
					print("<*%s" % line)
				return int(m.group(1))

		if VERBOSE:
			print("< %s" % line)
		line = ""

	return None

def serial_recv(timeout):
	global serial_obj
	if (serial_obj == None):
		return None
	return serial_wait_for('P', timeout)

def serial_wait_for_signal(timeout):
	global serial_obj
	if (serial_obj == None):
		return None
	return serial_wait_for('T', timeout)

def serial_flush(timeout):
	global serial_obj
	if (serial_obj == None):
		return None
	return serial_wait_for('D', timeout)


#
# MAC tests
#

def test_mac_recv_nodata():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x9000)
	if VERBOSE:
		eth.show2()
	serial_send(eth)
	return VERDICT_OK

def test_mac_recv_data_ucast():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x9000)/Raw("test")
	if VERBOSE:
		eth.show2()
	serial_send(eth)
	return VERDICT_OK

def test_mac_recv_data_mcast():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="33:33:00:00:00:01",type=0x9000)/Raw("test")
	if VERBOSE:
		eth.show2()
	serial_send(eth)
	return VERDICT_OK

def test_mac_recv_badethtype():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x0800)/Raw("test")
	if VERBOSE:
		eth.show2()
	serial_send(eth)
	return VERDICT_OK

def test_mac_recv_baddst():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:60",type=0x0800)/Raw("test")
	if VERBOSE:
		eth.show2()
	serial_send(eth)
	return VERDICT_OK

def test_mac_send_nodata():
	if VERBOSE:
		eth.show2()
	serial_send(eth)
	return VERDICT_OK

def test_mac_send_nodata():
	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[Ether].src != "10:22:33:44:55:66") or
	    (eth[Ether].dst != "76:88:99:aa:bb:cc") or
	    (eth[Ether].type != 0x9000)):
		return VERDICT_NOK

	return VERDICT_OK

def test_mac_send_data():
	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[Ether].src != "10:22:33:44:55:66") or
	    (eth[Ether].dst != "76:88:99:aa:bb:cc") or
	    (eth[Ether].type != 0x9000) or
	    (eth[Ether].load != "test")):
		return VERDICT_NOK

	return VERDICT_OK


#
# IP6 data tests
#

def test_ip6_recv_nodata():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=59)
	pkt = eth/ipv6
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_ip6_recv_data():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=253)/Raw("test")
	pkt = eth/ipv6
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_ip6_recv_badnh():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=6)
	pkt = eth/ipv6
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_ip6_recv_badsrc():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:d:c:b:a",dst="2001:1:2:3:f:e:d:c",nh=59)
	pkt = eth/ipv6
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_ip6_recv_baddst():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:c:d:e:f",nh=59)
	pkt = eth/ipv6
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_ip6_recv_badlen():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:c:d:e:f",nh=253,plen=1550)/Raw("test")
	pkt = eth/ipv6
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_ip6_send_nodata():
	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[IPv6].src != "2001:1:2:3:f:e:d:c") or
	    (eth[IPv6].dst != "2001:1:2:3:a:b:c:d") or
	    (eth[IPv6].plen != 0) or
	    (eth[IPv6].nh != 59)):
		return VERDICT_NOK

	return VERDICT_OK

def test_ip6_send_data():
	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[IPv6].src != "2001:1:2:3:f:e:d:c") or
	    (eth[IPv6].dst != "2001:1:2:3:a:b:c:d") or
	    (eth[IPv6].plen != 4) or
	    (eth[IPv6].nh != 253) or
	    (eth[IPv6].load != "test")):
		return VERDICT_NOK

	return VERDICT_OK


#
# ICMPv6 NDP NS/NA tests
#

def test_ip6_icmpv6_nsna_recv_uc():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="33:33:00:00:00:01",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=58)
	icmpv6 = ICMPv6ND_NS(tgt="2001:1:2:3:f:e:d:c")
	icmpv6ndopt = ICMPv6NDOptSrcLLAddr(lladdr="76:88:99:AA:BB:CC")
	pkt = eth/ipv6/icmpv6/icmpv6ndopt
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[IPv6].dst != "2001:1:2:3:a:b:c:d") or
	    (eth[IPv6].nh != 58) or
	    (eth[ICMPv6ND_NA].type != 136) or
	    (eth[ICMPv6ND_NA].code != 0) or
	    (eth[ICMPv6ND_NA].R != 0) or
	    (eth[ICMPv6ND_NA].S != 1) or
	    (eth[ICMPv6ND_NA].O != 1) or
	    (eth[ICMPv6ND_NA].res != 0) or
	    (eth[ICMPv6ND_NA].tgt != "2001:1:2:3:f:e:d:c") or
	    (eth[ICMPv6NDOptDstLLAddr].type != 2) or
	    (eth[ICMPv6NDOptDstLLAddr].len != 1) or
	    (eth[ICMPv6NDOptDstLLAddr].lladdr != "10:22:33:44:55:66")):
		return VERDICT_NOK

	checksum_orig = eth[ICMPv6ND_NA].cksum
	eth[ICMPv6ND_NA].cksum = 0
	checksum_comp = in6_chksum(eth[IPv6].nh, eth[IPv6], raw(eth)[54:])
	if (checksum_orig != checksum_comp):
		if VERBOSE:
			print("checksum: orig=%x, comp=%x" % (checksum_orig, checksum_comp))
		return VERDICT_NOK

	return VERDICT_OK

def test_ip6_icmpv6_nsna_recv_lla():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="33:33:00:00:00:01",type=0x86DD)
	ipv6 = IPv6(src="fe80::a:b:c:d",dst="fe80::f:e:d:c",nh=58)
	icmpv6 = ICMPv6ND_NS(tgt="fe80::f:e:d:c")
 	icmpv6ndopt = ICMPv6NDOptSrcLLAddr(lladdr="76:88:99:AA:BB:CC")
	pkt = eth/ipv6/icmpv6/icmpv6ndopt
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[IPv6].dst != "fe80::a:b:c:d") or
	    (eth[IPv6].nh != 58) or
	    (eth[ICMPv6ND_NA].type != 136) or
	    (eth[ICMPv6ND_NA].code != 0) or
	    (eth[ICMPv6ND_NA].R != 0) or
	    (eth[ICMPv6ND_NA].S != 1) or
	    (eth[ICMPv6ND_NA].O != 1) or
	    (eth[ICMPv6ND_NA].res != 0) or
	    (eth[ICMPv6ND_NA].tgt != "fe80::f:e:d:c") or
	    (eth[ICMPv6NDOptDstLLAddr].type != 2) or
	    (eth[ICMPv6NDOptDstLLAddr].len != 1) or
	    (eth[ICMPv6NDOptDstLLAddr].lladdr != "10:22:33:44:55:66")):
		return VERDICT_NOK

	checksum_orig = eth[ICMPv6ND_NA].cksum
	eth[ICMPv6ND_NA].cksum = 0
	checksum_comp = in6_chksum(eth[IPv6].nh, eth[IPv6], raw(eth)[54:])
	if (checksum_orig != checksum_comp):
		if VERBOSE:
			print("checksum: orig=%x, comp=%x" % (checksum_orig, checksum_comp))
		return VERDICT_NOK

	return VERDICT_OK

def test_ip6_icmpv6_nsna_recv_mcsn():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="33:33:ff:0d:00:0c",type=0x86DD)
	ipv6 = IPv6(src="fe80::a:b:c:d",dst="ff02::1:ff0d:c",nh=58)
	icmpv6 = ICMPv6ND_NS(tgt="2001:1:2:3:f:e:d:c")
 	icmpv6ndopt = ICMPv6NDOptSrcLLAddr(lladdr="76:88:99:AA:BB:CC")
	pkt = eth/ipv6/icmpv6/icmpv6ndopt
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[IPv6].dst != "fe80::a:b:c:d") or
	    (eth[IPv6].nh != 58) or
	    (eth[ICMPv6ND_NA].type != 136) or
	    (eth[ICMPv6ND_NA].code != 0) or
	    (eth[ICMPv6ND_NA].R != 0) or
	    (eth[ICMPv6ND_NA].S != 1) or
	    (eth[ICMPv6ND_NA].O != 1) or
	    (eth[ICMPv6ND_NA].res != 0) or
	    (eth[ICMPv6ND_NA].tgt != "2001:1:2:3:f:e:d:c") or
	    (eth[ICMPv6NDOptDstLLAddr].type != 2) or
	    (eth[ICMPv6NDOptDstLLAddr].len != 1) or
	    (eth[ICMPv6NDOptDstLLAddr].lladdr != "10:22:33:44:55:66")):
		return VERDICT_NOK

	checksum_orig = eth[ICMPv6ND_NA].cksum
	eth[ICMPv6ND_NA].cksum = 0
	checksum_comp = in6_chksum(eth[IPv6].nh, eth[IPv6], raw(eth)[54:])
	if (checksum_orig != checksum_comp):
		if VERBOSE:
			print("checksum: orig=%x, comp=%x" % (checksum_orig, checksum_comp))
		return VERDICT_NOK

	return VERDICT_OK

def test_ip6_icmpv6_nsna_recv_dad():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="33:33:00:00:00:01",type=0x86DD)
	ipv6 = IPv6(src="::",dst="2001:1:2:3:f:e:d:c",nh=58)
	icmpv6 = ICMPv6ND_NS(tgt="2001:1:2:3:f:e:d:c")
 	icmpv6ndopt = ICMPv6NDOptSrcLLAddr(lladdr="76:88:99:AA:BB:CC")
	pkt = eth/ipv6/icmpv6/icmpv6ndopt
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[IPv6].dst != "ff02::1") or
	    (eth[IPv6].nh != 58) or
	    (eth[ICMPv6ND_NA].type != 136) or
	    (eth[ICMPv6ND_NA].code != 0) or
	    (eth[ICMPv6ND_NA].R != 0) or
	    (eth[ICMPv6ND_NA].S != 1) or
	    (eth[ICMPv6ND_NA].O != 1) or
	    (eth[ICMPv6ND_NA].res != 0) or
	    (eth[ICMPv6ND_NA].tgt != "2001:1:2:3:f:e:d:c") or
	    (eth[ICMPv6NDOptDstLLAddr].type != 2) or
	    (eth[ICMPv6NDOptDstLLAddr].len != 1) or
	    (eth[ICMPv6NDOptDstLLAddr].lladdr != "10:22:33:44:55:66")):
		return VERDICT_NOK

	checksum_orig = eth[ICMPv6ND_NA].cksum
	eth[ICMPv6ND_NA].cksum = 0
	checksum_comp = in6_chksum(eth[IPv6].nh, eth[IPv6], raw(eth)[54:])
	if (checksum_orig != checksum_comp):
		if VERBOSE:
			print("checksum: orig=%x, comp=%x" % (checksum_orig, checksum_comp))
		return VERDICT_NOK

	return VERDICT_OK

def test_ip6_icmpv6_nsna_recv_badtgt():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="33:33:00:00:00:01",type=0x86DD)
	ipv6 = IPv6(src="fe80::a:b:c:d",dst="ff02::1",nh=58)
	icmpv6 = ICMPv6ND_NS(tgt="2001:1:2:3:c:d:e:f")
 	icmpv6ndopt = ICMPv6NDOptSrcLLAddr(lladdr="AA:BB:CC:DD:EE:FF")
	pkt = eth/ipv6/icmpv6/icmpv6ndopt
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	rep = serial_recv(0.5)
	if (rep != None):
		if VERBOSE:
			eth = Ether(rep)
			eth.show()
		return VERDICT_NOK

	return VERDICT_OK


#
# UDP tests
#

def test_udp_recv_nodata():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5678, dport=1234, len=8)
	pkt=eth/ipv6/udp
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_udp_recv_data():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5678, dport=1234, len=12)
	pload = Raw("test")
	pkt=eth/ipv6/udp/pload
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_udp_recv_badsrc():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5670, dport=1234, len=12)
	pload = Raw("test")
	pkt=eth/ipv6/udp/pload
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_udp_recv_baddst():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5678, dport=1230, len=12)
	pload = Raw("test")
	pkt=eth/ipv6/udp/pload
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_udp_recv_badlen():
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5678, dport=1234, len=1500)
	pload = Raw("test")
	pkt=eth/ipv6/udp/pload
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)
	return VERDICT_OK

def test_udp_send_nodata():
	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[UDP].sport != 1234) or
	    (eth[UDP].dport != 5678) or
	    (eth[UDP].len != 8)):
		return VERDICT_NOK

	checksum_orig = eth[UDP].chksum
	eth[UDP].chksum = 0
	checksum_comp = in6_chksum(eth[IPv6].nh, eth[IPv6], raw(eth)[54:])
	if (checksum_orig != checksum_comp):
		if VERBOSE:
			print("checksum: orig=%x, comp=%x" % (checksum_orig, checksum_comp))
		return VERDICT_NOK

	return VERDICT_OK

def test_udp_send_data():
	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[UDP].sport != 1234) or
	    (eth[UDP].dport != 5678) or
	    (eth[UDP].len != 12) or
	    (eth[UDP].load != "test")):
		return VERDICT_NOK

	checksum_orig = eth[UDP].chksum
	eth[UDP].chksum = 0
	checksum_comp = in6_chksum(eth[IPv6].nh, eth[IPv6], raw(eth)[54:])
	if (checksum_orig != checksum_comp):
		if VERBOSE:
			print("checksum: orig=%x, comp=%x" % (checksum_orig, checksum_comp))
		return VERDICT_NOK

	return VERDICT_OK


def test_coap_noncf_send_nodata():
	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[UDP].len != 13) or
	    (eth[CoAP].ver != 1) or
	    (eth[CoAP].type != 1) or
	    (eth[CoAP].tkl != 1) or
	    (eth[CoAP].code != 2) or
	    (eth[CoAP].token != '\x12') or
	    (eth[CoAP].paymark != '')):
		return VERDICT_NOK

	return VERDICT_OK

def test_coap_noncf_send_data():
	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[UDP].len != 28) or
	    (eth[CoAP].ver != 1) or
	    (eth[CoAP].type != 1) or
	    (eth[CoAP].tkl != 1) or
	    (eth[CoAP].code != 2) or
	    (eth[CoAP].token != '\x34') or
	    (eth[CoAP].paymark != '\xff') or
	    (eth[CoAP].load != 'test')):
		return VERDICT_NOK

	return VERDICT_OK

def test_coap_noncf_send_data_resp():
	recv = serial_recv(0.5)
	if (recv == None):
		return VERDICT_NOK

	eth = Ether(recv)
	if VERBOSE:
		eth.show()

	if ((eth[UDP].len != 28) or
	    (eth[CoAP].ver != 1) or
	    (eth[CoAP].type != 1) or
	    (eth[CoAP].tkl != 1) or
	    (eth[CoAP].code != 2) or
	    (eth[CoAP].token != '\x56') or
	    (eth[CoAP].paymark != '\xff') or
	    (eth[CoAP].load != 'test')):
		return VERDICT_NOK

	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5683, dport=1234)
	coap = CoAP(type=1, tkl=1, code=0x41, token='\x56')
	pkt=eth/ipv6/udp/coap
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	return VERDICT_OK

def test_coap_cf_send_nodata():
	recv = serial_recv(0.5)
	if (recv == None):
		return VERDICT_NOK

	eth = Ether(recv)
	if VERBOSE:
		eth.show()

	if ((eth[UDP].len != 23) or
	    (eth[CoAP].ver != 1) or
	    (eth[CoAP].type != 0) or
	    (eth[CoAP].tkl != 1) or
	    (eth[CoAP].code != 2) or
	    (eth[CoAP].token != '\x78') or
	    (eth[CoAP].paymark != '')):
		return VERDICT_NOK

	msg_id = eth[CoAP].msg_id
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5683, dport=1234)
	coap = CoAP(type=2, tkl=1, code=0x00, msg_id=msg_id, token='\x78')
	pkt=eth/ipv6/udp/coap
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	return VERDICT_OK

def test_coap_cf_send_data():
	rep = serial_recv(0.5)
	if (rep == None):
		return VERDICT_NOK

	eth = Ether(rep)
	if VERBOSE:
		eth.show()

	if ((eth[UDP].len != 28) or
	    (eth[CoAP].ver != 1) or
	    (eth[CoAP].type != 0) or
	    (eth[CoAP].tkl != 1) or
	    (eth[CoAP].code != 2) or
	    (eth[CoAP].token != '\x9a') or
	    (eth[CoAP].paymark != '\xff') or
	    (eth[CoAP].load != 'test')):
		return VERDICT_NOK

	msg_id = eth[CoAP].msg_id
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5683, dport=1234)
	coap = CoAP(type=2, tkl=1, code=0x00, msg_id=msg_id, token='\x9a')
	pkt=eth/ipv6/udp/coap
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	return VERDICT_OK

def test_coap_cf_send_data_ackresp():
	recv = serial_recv(0.5)
	if (recv == None):
		return VERDICT_NOK

	eth = Ether(recv)
	if VERBOSE:
		eth.show()

	if ((eth[UDP].len != 28) or
	    (eth[CoAP].ver != 1) or
	    (eth[CoAP].type != 0) or
	    (eth[CoAP].tkl != 1) or
	    (eth[CoAP].code != 2) or
	    (eth[CoAP].token != '\xbc') or
	    (eth[CoAP].paymark != '\xff') or
	    (eth[CoAP].load != 'test')):
		return VERDICT_NOK

	msg_id = eth[CoAP].msg_id
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5683, dport=1234)
	coap = CoAP(type=2, tkl=1, code=0x00, msg_id=msg_id, token='\xbc')
	pkt=eth/ipv6/udp/coap
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5683, dport=1234)
	coap = CoAP(type=0, tkl=1, code=0x41, token='\xbc')
	pkt=eth/ipv6/udp/coap
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	# TODO: ACK

	return VERDICT_OK

def test_coap_cf_send_data_piggybacked():
	recv = serial_recv(0.5)
	if (recv == None):
		return VERDICT_NOK

	eth = Ether(recv)
	if VERBOSE:
		eth.show()

	if ((eth[UDP].len != 28) or
	    (eth[CoAP].ver != 1) or
	    (eth[CoAP].type != 0) or
	    (eth[CoAP].tkl != 1) or
	    (eth[CoAP].code != 2) or
	    (eth[CoAP].token != '\xde') or
	    (eth[CoAP].paymark != '\xff') or
	    (eth[CoAP].load != 'test')):
		return VERDICT_NOK

	msg_id = eth[CoAP].msg_id
	eth = Ether(src="76:88:99:AA:BB:CC",dst="10:22:33:44:55:66",type=0x86DD)
	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
	udp = UDP(sport=5683, dport=1234)
	coap = CoAP(type=2, tkl=1, code=0x41, msg_id=msg_id, token='\xde')
	pkt=eth/ipv6/udp/coap
	if VERBOSE:
		pkt.show2()
	serial_send(pkt)

	return VERDICT_OK

def test_coap_send_null_tkl():
	return VERDICT_OK

def test_coap_send_max_tkl():
	return VERDICT_OK

def test_coap_send_data_post():
	return VERDICT_OK

def test_coap_send_data_put():
	return VERDICT_OK

def test_coap_send_data_delete():
	return VERDICT_OK

def test_coap_send_data_ack_mismatch():
	return VERDICT_OK

def test_coap_send_data_confirm_mismatch():
	return VERDICT_OK

def test_coap_send_msgid_incr():
	return VERDICT_OK
#	ipv6 = IPv6(src="2001:1:2:3:a:b:c:d",dst="2001:1:2:3:f:e:d:c",nh=17)
#	udp = UDP(sport=5678, dport=1234, len=0)
#	coap = CoAP(type=1, tkl=1, code=0, msg_id=1, token=0x12)
#	pkt=ipv6/udp/coap
#	if VERBOSE:
#		pkt.show2()
#	serial_send(pkt)
#	return VERDICT_OK


tests = {
#	0x1*: test_mac_*
	0x11: test_mac_recv_nodata,
	0x12: test_mac_recv_data_ucast,
	0x13: test_mac_recv_data_mcast,
	0x14: test_mac_recv_badethtype,
	0x15: test_mac_recv_baddst,
	0x16: test_mac_send_nodata,
	0x17: test_mac_send_data,

#	0x2*: test_ip6_*
	0x21: test_ip6_recv_nodata,
	0x22: test_ip6_recv_data,
	0x23: test_ip6_recv_badnh,
	0x24: test_ip6_recv_badsrc,
	0x25: test_ip6_recv_baddst,
	0x26: test_ip6_recv_badlen,
	0x27: test_ip6_send_nodata,
	0x28: test_ip6_send_data,

#	0x3*: test_ip6_icmpv6_nsna_*
	0x31: test_ip6_icmpv6_nsna_recv_uc,
	0x32: test_ip6_icmpv6_nsna_recv_lla,
	0x33: test_ip6_icmpv6_nsna_recv_mcsn,
	0x34: test_ip6_icmpv6_nsna_recv_dad,
	0x35: test_ip6_icmpv6_nsna_recv_badtgt,

#	0x5*: test_udp_*
	0x51: test_udp_recv_nodata,
	0x52: test_udp_recv_data,
	0x53: test_udp_recv_badsrc,
	0x54: test_udp_recv_baddst,
	0x55: test_udp_recv_badlen,
	0x56: test_udp_send_nodata,
	0x57: test_udp_send_data,

#	0x6*: test_coap_*
	0x61: test_coap_noncf_send_nodata,
	0x62: test_coap_noncf_send_data,
	0x63: test_coap_noncf_send_data_resp,
	0x64: test_coap_cf_send_nodata,
	0x65: test_coap_cf_send_data,
	0x66: test_coap_cf_send_data_ackresp,
	0x67: test_coap_cf_send_data_piggybacked,
}

# Run tests, with python as the test controller
def run_tests():
	for i in tests:
		print("test %d: %-40s  " % (i, str(tests[i].__name__)), end="")
		if VERBOSE:
			print("")
		serial_flush(2)
		serial_signal(i)
		local_verdict = tests[i]()
		remote_verdict = serial_wait_for_signal(5)
		if ((local_verdict == VERDICT_OK) and (remote_verdict == VERDICT_OK)):
			print("[OK]")
		elif ((local_verdict == VERDICT_NOK) or (remote_verdict == VERDICT_NOK)):
			print("[NOK]")
		else:
			print("[ERROR]")
		if VERBOSE:
			print("")

def net_console():
	while True:
		pkt = serial_recv(0.5)
		if (pkt != None):
			IPv6(pkt).show2()

serial_init()
run_tests()
#net_console()
