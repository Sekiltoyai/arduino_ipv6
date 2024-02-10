/*-
 * Copyright (c) 2024 Emmanuel Thierry
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config.h"
#include "hw_w5500.h"
#include "net_utils.h"
#include "platform.h"



/* Address phase defines */

#define CRB_ADDR_MR          0x00,0x00
#define CRB_ADDR_SHAR0       0x00,0x09
#define CRB_ADDR_VERSIONR    0x00,0x39
#define CRB_ADDR_PHYCFGR     0x00,0x2E

#define SRB_ADDR_SNMR        0x00,0x00
#define SRB_ADDR_SNCR        0x00,0x01
#define SRB_ADDR_SNIR        0x00,0x02
#define SRB_ADDR_SNSR        0x00,0x03
#define SRB_ADDR_SNRXBUFSIZE 0x00,0x1E
#define SRB_ADDR_SNTXBUFSIZE 0x00,0x1F
#define SRB_ADDR_SNTXFSR     0x00,0x20
#define SRB_ADDR_SNTXRD      0x00,0x22
#define SRB_ADDR_SNTXWR      0x00,0x24
#define SRB_ADDR_SNRXRSR     0x00,0x26
#define SRB_ADDR_SNRXRD      0x00,0x28
#define SRB_ADDR_SNRXWR      0x00,0x2A
#define SRB_ADDR_SNIMR       0x00,0x2C

#define ADDR_SPLIT(address) \
	((uint8_t) ((address & 0xFF00) >> 8)), \
	((uint8_t) (address & 0x00FF))

/* Control phase defines */

#define BSB_COMMON_REG            0x00
#define BSB_SOCKET0_REG           0x08
#define BSB_SOCKET0_TXB           0x10
#define BSB_SOCKET0_RXB           0x18

#define RWB_READ                  0x00
#define RWB_WRITE                 0x04

#define OM_VDM                    0x00
#define OM_FDM_1                  0x01
#define OM_FDM_2                  0x02
#define OM_FDM_4                  0x03


/* Data phase defines (Common registers) */

#define MR_RST                    0x80

#define PHYCFG_RST                0x80
#define PHYCFG_OMPD_PIN           0x00
#define PHYCFG_OMPD_REG           0x40
#define PHYCFG_OMPDC_10BT_HD      0x00
#define PHYCFG_OMPDC_10BT_FD      0x08
#define PHYCFG_OMPDC_100BT_HD     0x10
#define PHYCFG_OMPDC_100BT_FD     0x18
#define PHYCFG_OMPDC_100BT_HD_AN  0x20
#define PHYCFG_OMPDC_POWER_DOWN   0x30
#define PHYCFG_OMPDC_ALL          0x38
#define PHYCFG_DPX_HD             0x00
#define PHYCFG_DPX_FD             0x04
#define PHYCFG_SPD_10MBPS         0x00
#define PHYCFG_SPD_100MBPS        0x02
#define PHYCFG_LNK_DOWN           0x00
#define PHYCFG_LNK_UP             0x01


/* Data phase defines (Socket registers) */

#define SNMR_MFEN_FLAG            0x80
#define SNMR_BCASTB_FLAG          0x40
#define SNMR_MMB_FLAG             0x20
#define SNMR_MIP6B_FLAG           0x10
#define SNMR_PROTO_MACRAW         0x04

#define SNCR_OPEN                 0x01
#define SNCR_CLOSE                0x10
#define SNCR_SEND                 0x20
#define SNCR_RECV                 0x40

#define SNSR_SOCK_CLOSED          0x00
#define SNSR_SOCK_MACRAW          0x42




static void _hw_w5500_spi_do_command(uint8_t addr_h, uint8_t addr_l, uint8_t control,
                                  uint8_t *buffer, uint16_t buflen)
{
	uint8_t spi_command[3] = { addr_h, addr_l, control };

	spi_start_transfer();
	spi_write(spi_command, 3);
	spi_read(buffer, buflen);
	spi_stop_transfer();
}

static void _hw_w5500_spi_command(uint8_t addr_h, uint8_t addr_l, uint8_t control,
                                  uint8_t *buffer, uint16_t buflen)
{
	spi_start_transaction();
	_hw_w5500_spi_do_command(addr_h, addr_l, control, buffer, buflen);
	spi_stop_transaction();
}

static bool _hw_w5500_spi_wait(uint8_t addr_h, uint8_t addr_l, uint8_t control,
                               uint8_t expectedval, uint8_t expectedmask,
                               uint8_t timeout)
{
	uint8_t spi_command[3] = { addr_h, addr_l, control };
	uint8_t value = 0;

	do {
		/* Do request the value */
		spi_start_transfer();
		spi_write(spi_command, 3);
		value = spi_read_byte();
		spi_stop_transfer();

		/* The expected value were found */
		if ((value & expectedmask) == expectedval) {
			return true;
		}
		msleep(1);

	} while ((--timeout) > 0);

	return false;
}


void hw_w5500_init()
{
	uint8_t reset = 0x80;
	uint8_t mode = 0x00;
	uint8_t spi_command[3];

	msleep(1000);
	spi_init();
	msleep(500);


	/* Get the SPI port */
	spi_start_transaction();

	/* Reset the W5500 */
	_hw_w5500_spi_do_command(CRB_ADDR_MR, BSB_COMMON_REG | RWB_WRITE | OM_VDM,
	                         &reset, 1);

	/* Wait for reset to complete */
	_hw_w5500_spi_wait(CRB_ADDR_MR, BSB_COMMON_REG | RWB_READ | OM_VDM,
	                   0x00, MR_RST, 50);

	/* Set global W5500 mode */
	_hw_w5500_spi_do_command(CRB_ADDR_MR, BSB_COMMON_REG | RWB_WRITE | OM_VDM,
	                         &mode, 1);

	/* Release the SPI port */
	spi_stop_transaction();
}

void hw_w5500_destroy()
{
	spi_destroy();
}

void hw_w5500_read_version(uint8_t *version)
{
	/* Read the W5500 chipset version (=4) */
	_hw_w5500_spi_command(CRB_ADDR_VERSIONR, BSB_COMMON_REG | RWB_READ | OM_VDM,
	                      version, 1);
}

void hw_w5500_set_phycfg(bool up, bool autoneg, uint8_t speed)
{
	uint8_t reset = 0x00;
	uint8_t mode = PHYCFG_RST | PHYCFG_OMPD_REG;

	/* Get the SPI port */
	spi_start_transaction();

	/* Reset the W5500 PHY */
	_hw_w5500_spi_do_command(CRB_ADDR_PHYCFGR, BSB_COMMON_REG | RWB_WRITE | OM_VDM,
	                         &reset, 1);

	/* Wait for the W5500 PHY to complete */
	_hw_w5500_spi_wait(CRB_ADDR_PHYCFGR, BSB_COMMON_REG | RWB_READ | OM_VDM,
	                   PHYCFG_RST, PHYCFG_RST, 200);

	/* Build PHY configuration */
	if (!up) {
		mode |= PHYCFG_OMPDC_POWER_DOWN;
	} else if (autoneg) {
		switch (speed) {
		case HW_SPEED_100MBPS_HD:
			mode |= PHYCFG_OMPDC_100BT_HD_AN;
			break;
		case HW_SPEED_100MBPS_FD:
		case HW_SPEED_NONE:
			mode |= PHYCFG_OMPDC_ALL;
			break;
		default:
			return;
		}
	} else {
		switch (speed) {
		case HW_SPEED_NONE:
			mode |= PHYCFG_OMPDC_POWER_DOWN;
			break;
		case HW_SPEED_10MBPS_HD:
			mode |= PHYCFG_OMPDC_10BT_HD;
			break;
		case HW_SPEED_10MBPS_FD:
			mode |= PHYCFG_OMPDC_10BT_FD;
			break;
		case HW_SPEED_100MBPS_HD:
			mode |= PHYCFG_OMPDC_100BT_HD;
			break;
		case HW_SPEED_100MBPS_FD:
			mode |= PHYCFG_OMPDC_100BT_FD;
			break;
		default:
			return;
		}
	}

	/* Set the W5500 PHY config */
	_hw_w5500_spi_do_command(CRB_ADDR_PHYCFGR, BSB_COMMON_REG | RWB_WRITE | OM_VDM,
	                         &mode, 1);

	/* Release the SPI port */
	spi_stop_transaction();
}

void hw_w5500_get_phycfg(bool *up, uint8_t *speed)
{
	uint8_t mode = 0;

	/* Get the W5500 PHY config */
	_hw_w5500_spi_command(CRB_ADDR_PHYCFGR, BSB_COMMON_REG | RWB_READ | OM_VDM,
	                      &mode, 1);
	if (mode & PHYCFG_LNK_UP) {
		*up = true;
		switch (mode & (PHYCFG_DPX_FD | PHYCFG_SPD_100MBPS)) {
		case (PHYCFG_DPX_HD | PHYCFG_SPD_10MBPS):
			*speed = HW_SPEED_10MBPS_HD;
			break;
		case (PHYCFG_DPX_FD | PHYCFG_SPD_10MBPS):
			*speed = HW_SPEED_10MBPS_FD;
			break;
		case (PHYCFG_DPX_HD | PHYCFG_SPD_100MBPS):
			*speed = HW_SPEED_100MBPS_FD;
			break;
		case (PHYCFG_DPX_FD | PHYCFG_SPD_100MBPS):
			*speed = HW_SPEED_100MBPS_FD;
			break;
		default:
			*speed = HW_SPEED_NONE;
			break;
		}
	} else {
		*up = false;
		*speed = HW_SPEED_NONE;
	}
}

void hw_w5500_set_macaddress(uint8_t *macaddress)
{
	uint8_t macaddress_cpy[6] = {
		macaddress[0],
		macaddress[1],
		macaddress[2],
		macaddress[3],
		macaddress[4],
		macaddress[5]
	};

	/* Set the MAC Address to the W5500 */
	_hw_w5500_spi_command(CRB_ADDR_SHAR0, BSB_COMMON_REG | RWB_WRITE | OM_VDM,
	                      macaddress_cpy, 6);
}

void hw_w5500_get_macaddress(uint8_t *macaddress)
{
	/* Get the MAC Address from the W5500 */
	_hw_w5500_spi_command(CRB_ADDR_SHAR0, BSB_COMMON_REG | RWB_READ | OM_VDM,
	                      macaddress, 6);
}


bool hw_w5500_open(struct hw_w5500_ctx *w5500)
{
	uint8_t mode = SNMR_PROTO_MACRAW;
	uint8_t command = SNCR_OPEN;
	bool status;

	/* Get the SPI port */
	spi_start_transaction();

	/* Set the socket 0 mode to MACRAW  */
	_hw_w5500_spi_do_command(SRB_ADDR_SNMR, BSB_SOCKET0_REG | RWB_WRITE | OM_VDM,
			      &mode, 1);

	/* Open the socket 0 */
	_hw_w5500_spi_do_command(SRB_ADDR_SNCR, BSB_SOCKET0_REG | RWB_WRITE | OM_VDM,
	                      &command, 1);

	/* Wait the socket 0 to be in SOCK_MACRAW status */
	status = _hw_w5500_spi_wait(SRB_ADDR_SNSR, BSB_SOCKET0_REG | RWB_READ | OM_VDM,
	                            SNSR_SOCK_MACRAW, 0xFF, 50);

	/* Release the SPI port */
	spi_stop_transaction();

	return status;
}

uint16_t hw_w5500_recv(struct hw_w5500_ctx *w5500, uint8_t *buffer, uint16_t buflen)
{
	uint8_t rxrsr[2], rxrd[2], rxlen[2];
	uint32_t rxrd_address = 0;
	uint16_t frame_length = 0;
	uint8_t command = SNCR_RECV;


	/* Get the SPI port */
	spi_start_transaction();

	/**
	 * Read the RX Received Size Register
	 * Note: No need to reliably read the register, as it would necessarily increase
	 */
	_hw_w5500_spi_do_command(SRB_ADDR_SNRXRSR, BSB_SOCKET0_REG | RWB_READ | OM_VDM,
	                      rxrsr, 2);

	/* Check whether there is data to read (i.e. at least 2 bytes) */
	if ((rxrsr[0] == 0) && (rxrsr[1] < 2)) {
		frame_length = 0;
		goto out_zerodata;
	}

	/* Read the RX Read Pointer */
	_hw_w5500_spi_do_command(SRB_ADDR_SNRXRD, BSB_SOCKET0_REG | RWB_READ | OM_VDM,
	                      rxrd, 2);

	/**
	 * Undocumented in W5500 datasheet:
	 * The MAC frame is prepended by the RX length (2 bytes length + frame length)
	 */
	_hw_w5500_spi_do_command(rxrd[0], rxrd[1], BSB_SOCKET0_RXB | RWB_READ | OM_VDM,
	                      rxlen, 2);
	frame_length = (rxlen[0] << 8) + rxlen[1] - 2;

	/* Check that the frame fits into the buffer (the read pointer is let untouched )*/
	if (frame_length > buflen) {
		frame_length = 0;
		goto out_zerodata;
	}

	/* Read the frame */
	rxrd_address = (rxrd[0] << 8) + rxrd[1] + 2;
	_hw_w5500_spi_do_command(ADDR_SPLIT(rxrd_address), BSB_SOCKET0_RXB | RWB_READ | OM_VDM,
	                      buffer, frame_length);

	/* Compute the rxrd pointer after read and write it back to the RX_RD register */
	rxrd_address += frame_length;
	rxrd[0] = (uint8_t) ((rxrd_address & 0x0000FF00) >> 8);
	rxrd[1] = (uint8_t) (rxrd_address & 0x000000FF);
	_hw_w5500_spi_do_command(SRB_ADDR_SNRXRD, BSB_SOCKET0_REG | RWB_WRITE | OM_VDM,
	                      rxrd, 2);

	/* Signal the W5500 that the RX_RD register has been updated */
	_hw_w5500_spi_do_command(SRB_ADDR_SNCR, BSB_SOCKET0_REG | RWB_WRITE | OM_VDM,
	                      &command, 1);

out_zerodata:
	/* Release the SPI port */
	spi_stop_transaction();

	return frame_length;
}

uint16_t hw_w5500_send(struct hw_w5500_ctx *w5500, uint8_t *buffer, uint16_t buflen)
{
	uint8_t txfsr[2], txwr[2], txlen[2];
	uint32_t txwr_address = 0;
	uint16_t write_length = 0;
	uint8_t command = SNCR_SEND;


	/* Get the SPI port */
	spi_start_transaction();

	/**
	 * Read the TX Free Size Register
	 * Note: No need to reliably read the register, as it would necessarily increase
	 */
	_hw_w5500_spi_do_command(SRB_ADDR_SNTXFSR, BSB_SOCKET0_REG | RWB_READ | OM_VDM,
	                         txfsr, 2);

	/* Check whether the buffer fits into the free size (+ 2 bytes of frame length) */
	if (((txfsr[0] << 8) + txfsr[1]) < buflen + 2) {
		buflen = 0;
		goto out_zerodata;
	}

	/* Read the TX Write Pointer */
	_hw_w5500_spi_do_command(SRB_ADDR_SNTXWR, BSB_SOCKET0_REG | RWB_READ | OM_VDM,
	                         txwr, 2);

	/* Write the frame length */
	/*write_length = buflen + 2;
	txlen[0] = (uint8_t) ((write_length & 0xFF00) >> 8);
	txlen[1] = (uint8_t) (write_length & 0x00FF);
	_hw_w5500_spi_command(txwr[0], txwr[1], BSB_SOCKET0_TXB | RWB_WRITE | OM_VDM,
	                      txlen, 2);*/

	/* Write the frame */
	//txwr_address = (txwr[0] << 8) + txwr[1] + 2;
	txwr_address = (txwr[0] << 8) + txwr[1];
	_hw_w5500_spi_do_command(ADDR_SPLIT(txwr_address), BSB_SOCKET0_TXB | RWB_WRITE | OM_VDM,
	                      buffer, buflen);

	/* Compute the txwr pointer after write and write it back to the TX_WR register */
	txwr_address += buflen;
	txwr[0] = (uint8_t) ((txwr_address & 0x0000FF00) >> 8);
	txwr[1] = (uint8_t) (txwr_address & 0x000000FF);
	_hw_w5500_spi_do_command(SRB_ADDR_SNTXWR, BSB_SOCKET0_REG | RWB_WRITE | OM_VDM,
	                         txwr, 2);

	/* Signal the W5500 that the TX_WR register has been updated */
	_hw_w5500_spi_do_command(SRB_ADDR_SNCR, BSB_SOCKET0_REG | RWB_WRITE | OM_VDM,
	                         &command, 1);

out_zerodata:
	/* Release the SPI port */
	spi_stop_transaction();

	return buflen;
}
