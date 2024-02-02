/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2015-2015 InvenSense Inc. All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively "Software") is subject
 * to InvenSense and its licensors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense and its licensors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * INVENSENSE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 * ________________________________________________________________________________________________________
 */

#include "imu/inv_imu_extfunc.h"
#include "imu/inv_imu_transport.h"
#include "imu/inv_imu_regmap.h"

#include "Invn/InvError.h"

/* Function definition */
static uint8_t *get_register_cache_addr(struct inv_imu_device *s, uint32_t reg);
static int      write_sreg(struct inv_imu_device *s, uint8_t reg, uint32_t len, const uint8_t *buf);
static int      read_sreg(struct inv_imu_device *s, uint8_t reg, uint32_t len, uint8_t *buf);
static int      write_mclk_reg(struct inv_imu_device *s, uint16_t regaddr, uint8_t wr_cnt,
                               const uint8_t *buf);
static int read_mclk_reg(struct inv_imu_device *s, uint16_t regaddr, uint8_t rd_cnt, uint8_t *buf);

int inv_imu_init_transport(struct inv_imu_device *s)
{
	int                       status = 0;
	struct inv_imu_transport *t      = (struct inv_imu_transport *)s;

	status |= read_sreg(s, (uint8_t)PWR_MGMT0, 1, &(t->register_cache.pwr_mgmt0_reg));
	status |= read_sreg(s, (uint8_t)GYRO_CONFIG0, 1, &(t->register_cache.gyro_config0_reg));
	status |= read_sreg(s, (uint8_t)ACCEL_CONFIG0, 1, &(t->register_cache.accel_config0_reg));

	status |=
	    read_mclk_reg(s, (TMST_CONFIG1_MREG1 & 0xFFFF), 1, &(t->register_cache.tmst_config1_reg));

	t->need_mclk_cnt = 0;

	return status;
}

int inv_imu_read_reg(struct inv_imu_device *s, uint32_t reg, uint32_t len, uint8_t *buf)
{
	uint32_t i;
	int      rc = 0;

	for (i = 0; i < len; i++) {
		uint8_t *cache_addr = get_register_cache_addr(s, reg + i);

		if (cache_addr) {
			buf[i] = *cache_addr;
		} else {
			if (!(reg & 0x10000)) {
				rc |= read_mclk_reg(s, ((reg + i) & 0xFFFF), 1, &buf[i]);
			} else {
				rc |= read_sreg(s, (uint8_t)reg + i, len - i, &buf[i]);
				break;
			}
		}
	}

	return rc;
}

int inv_imu_write_reg(struct inv_imu_device *s, uint32_t reg, uint32_t len, const uint8_t *buf)
{
	uint32_t i;
	int      rc = 0;

	for (i = 0; i < len; i++) {
		uint8_t *cache_addr = get_register_cache_addr(s, reg + i);

		if (cache_addr)
			*cache_addr = buf[i];

		if (!(reg & 0x10000))
			rc |= write_mclk_reg(s, ((reg + i) & 0xFFFF), 1, &buf[i]);
	}

	if (reg & 0x10000)
		rc |= write_sreg(s, (uint8_t)reg, len, buf);

	return rc;
}

int inv_imu_switch_on_mclk(struct inv_imu_device *s)
{
	int                       status = 0;
	uint8_t                   data;
	struct inv_imu_transport *t = (struct inv_imu_transport *)s;
	uint64_t timeout_us = 1000000; /* 1 sec */
	uint64_t start;
	uint64_t current;

	/* set IDLE bit only if it is not set yet */
	if (t->need_mclk_cnt == 0) {
		status |= inv_imu_read_reg(s, PWR_MGMT0, 1, &data);
		data |= PWR_MGMT0_IDLE_MASK;
		status |= inv_imu_write_reg(s, PWR_MGMT0, 1, &data);

		if (status)
			return status;

		/* Check if MCLK is ready */
		start = inv_imu_get_time_us();
		do {
			status = inv_imu_read_reg(s, MCLK_RDY, 1, &data);
			
			if (status)
				return status;

			/* Timeout */
			current = inv_imu_get_time_us();
			if (current - start > timeout_us)
				return INV_ERROR_TIMEOUT;

		} while (!(data & MCLK_RDY_MCLK_RDY_MASK));
	} else {
		/* Make sure it is already on */
		status |= inv_imu_read_reg(s, PWR_MGMT0, 1, &data);
		if (0 == (data &= PWR_MGMT0_IDLE_MASK))
			status |= INV_ERROR;
	}

	/* Increment the counter to keep track of number of MCLK requesters */
	t->need_mclk_cnt++;

	return status;
}

int inv_imu_switch_off_mclk(struct inv_imu_device *s)
{
	int                       status = 0;
	uint8_t                   data;
	struct inv_imu_transport *t = (struct inv_imu_transport *)s;

	/* Reset the IDLE but only if there is one requester left */
	if (t->need_mclk_cnt == 1) {
		status |= inv_imu_read_reg(s, PWR_MGMT0, 1, &data);
		data &= ~PWR_MGMT0_IDLE_MASK;
		status |= inv_imu_write_reg(s, PWR_MGMT0, 1, &data);
	} else {
		/* Make sure it is still on */
		status |= inv_imu_read_reg(s, PWR_MGMT0, 1, &data);
		if (0 == (data &= PWR_MGMT0_IDLE_MASK))
			status |= INV_ERROR;
	}

	/* Decrement the counter */
	t->need_mclk_cnt--;

	return status;
}

/* Static function */

static uint8_t *get_register_cache_addr(struct inv_imu_device *s, uint32_t reg)
{
	struct inv_imu_transport *t = (struct inv_imu_transport *)s;

	switch (reg) {
	case PWR_MGMT0:
		return &(t->register_cache.pwr_mgmt0_reg);
	case GYRO_CONFIG0:
		return &(t->register_cache.gyro_config0_reg);
	case ACCEL_CONFIG0:
		return &(t->register_cache.accel_config0_reg);
	case TMST_CONFIG1_MREG1:
		return &(t->register_cache.tmst_config1_reg);
	default:
		return (uint8_t *)0; // Not found
	}
}

static int read_sreg(struct inv_imu_device *s, uint8_t reg, uint32_t len, uint8_t *buf)
{
	struct inv_imu_serif *serif = (struct inv_imu_serif *)s;

	if (len > serif->max_read)
		return INV_ERROR_SIZE;

	if (serif->read_reg(serif, reg, buf, len) != 0)
		return INV_ERROR_TRANSPORT;

	return 0;
}

static int write_sreg(struct inv_imu_device *s, uint8_t reg, uint32_t len, const uint8_t *buf)
{
	struct inv_imu_serif *serif = (struct inv_imu_serif *)s;

	if (len > serif->max_write)
		return INV_ERROR_SIZE;

	if (serif->write_reg(serif, reg, buf, len) != 0)
		return INV_ERROR_TRANSPORT;

	return 0;
}

static int read_mclk_reg(struct inv_imu_device *s, uint16_t regaddr, uint8_t rd_cnt, uint8_t *buf)
{
	uint8_t data;
	uint8_t blk_sel = (regaddr & 0xFF00) >> 8;
	int     status  = 0;

	// Have IMU not in IDLE mode to access MCLK domain
	status |= inv_imu_switch_on_mclk(s);

	// optimize by changing BLK_SEL only if not NULL
	if (blk_sel)
		status |= write_sreg(s, (uint8_t)BLK_SEL_R & 0xff, 1, &blk_sel);

	data = (regaddr & 0x00FF);
	status |= write_sreg(s, (uint8_t)MADDR_R, 1, &data);
	inv_imu_sleep_us(10);
	status |= read_sreg(s, (uint8_t)M_R, rd_cnt, buf);
	inv_imu_sleep_us(10);

	if (blk_sel) {
		data = 0;
		status |= write_sreg(s, (uint8_t)BLK_SEL_R, 1, &data);
	}

	// switch OFF MCLK if needed
	status |= inv_imu_switch_off_mclk(s);

	return status;
}

static int write_mclk_reg(struct inv_imu_device *s, uint16_t regaddr, uint8_t wr_cnt,
                          const uint8_t *buf)
{
	uint8_t data;
	uint8_t blk_sel = (regaddr & 0xFF00) >> 8;
	int     status  = 0;

	// Have IMU not in IDLE mode to access MCLK domain
	status |= inv_imu_switch_on_mclk(s);

	// optimize by changing BLK_SEL only if not NULL
	if (blk_sel)
		status |= write_sreg(s, (uint8_t)BLK_SEL_W, 1, &blk_sel);

	data = (regaddr & 0x00FF);
	status |= write_sreg(s, (uint8_t)MADDR_W, 1, &data);
	for (uint8_t i = 0; i < wr_cnt; i++) {
		status |= write_sreg(s, (uint8_t)M_W, 1, &buf[i]);
		inv_imu_sleep_us(10);
	}

	if (blk_sel) {
		data   = 0;
		status = write_sreg(s, (uint8_t)BLK_SEL_W, 1, &data);
	}

	status |= inv_imu_switch_off_mclk(s);

	return status;
}
