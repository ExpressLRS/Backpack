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

/** @defgroup Transport Transport
 *  @brief    Abstraction layer to access device's registers
 *  @{
 */

/** @file  inv_imu_transport.h */

#ifndef _INV_IMU_TRANSPORT_H_
#define _INV_IMU_TRANSPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* forward declaration */
struct inv_imu_device;

/** Available serial interface type. */
typedef enum { 
	UI_I2C, 
	UI_SPI4, 
	UI_SPI3 
} SERIAL_IF_TYPE_t;

/** Serial interface definition */
struct inv_imu_serif {
	void *context;
	int (*read_reg)(struct inv_imu_serif *serif, uint8_t reg, uint8_t *buf, uint32_t len);
	int (*write_reg)(struct inv_imu_serif *serif, uint8_t reg, const uint8_t *buf, uint32_t len);
	uint32_t         max_read;
	uint32_t         max_write;
	SERIAL_IF_TYPE_t serif_type;
};

/** Transport interface definition. */
struct inv_imu_transport {
	/** Serial interface object. 
	 *  @warning Must be the first object in this structure. 
	 */
	struct inv_imu_serif serif;

	/** Contains mirrored values of some IP registers. */
	struct register_cache {
		uint8_t pwr_mgmt0_reg;
		uint8_t gyro_config0_reg;
		uint8_t accel_config0_reg;
		uint8_t tmst_config1_reg;
	} register_cache;

	/** Internal counter for MCLK requests. */
	uint8_t need_mclk_cnt;
};

/** @brief Init cache variable.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_init_transport(struct inv_imu_device *s);

/** @brief Reads data from a register on IMU.
 *  @param[in] s     Pointer to device.
 *  @param[in] reg   Register address to be read.
 *  @param[in] len   Number of byte to be read.
 *  @param[out] buf  Output data from the register.
 *  @return          0 on success, negative value on error.
 */
int inv_imu_read_reg(struct inv_imu_device *s, uint32_t reg, uint32_t len, uint8_t *buf);

/** @brief Writes data to a register on IMU.
 *  @param[in] s    Pointer to device.
 *  @param[in] reg  Register address to be written.
 *  @param[in] len  Number of byte to be written.
 *  @param[in] buf  Input data to write.
 *  @return         0 on success, negative value on error.
 */
int inv_imu_write_reg(struct inv_imu_device *s, uint32_t reg, uint32_t len, const uint8_t *buf);

/** @brief Enable MCLK.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_switch_on_mclk(struct inv_imu_device *s);

/** @brief Disable MCLK.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_switch_off_mclk(struct inv_imu_device *s);

#ifdef __cplusplus
}
#endif

#endif /* _INV_IMU_TRANSPORT_H_ */

/** @} */
