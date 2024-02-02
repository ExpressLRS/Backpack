/*
 * ________________________________________________________________________________________________________
 * Copyright (c) 2017 InvenSense Inc. All rights reserved.
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

/** @defgroup Driver Driver
 *  @brief High-level functions to drive the device
 *  @{
 */

/** @file inv_imu_driver.h */

#ifndef _INV_IMU_DRIVER_H_
#define _INV_IMU_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "imu/inv_imu_defs.h"
#include "imu/inv_imu_transport.h"

#include "Invn/InvError.h"

#include <stdint.h>
#include <string.h>

/** Max FSR values for accel */
#define ACCEL_CONFIG0_FS_SEL_MAX ACCEL_CONFIG0_FS_SEL_16g

/** Max FSR values for gyro */
#define GYRO_CONFIG0_FS_SEL_MAX GYRO_CONFIG0_FS_SEL_2000dps

/** Max user offset value for accel (mg) */
#define ACCEL_OFFUSER_MAX_MG 1000

/** Max user offset value for gyro (dps) */
#define GYRO_OFFUSER_MAX_DPS 64

/** Max buffer size mirrored from FIFO at polling time */
#define FIFO_MIRRORING_SIZE 16 * 258 // packet size * max_count = 4kB

/** Accel start-up time */
#define ACC_STARTUP_TIME_US 10000

/** Gyro start-up time */
#define GYR_STARTUP_TIME_US 70000

/** Gyro power-off to power-on duration */
#define GYR_POWER_OFF_DUR_US 20000

/** Sensor identifier for UI control function */
enum inv_imu_sensor {
	INV_SENSOR_ACCEL, /**< Accelerometer */
	INV_SENSOR_GYRO, /**< Gyroscope */
	INV_SENSOR_FSYNC_EVENT, /**< FSYNC */
	INV_SENSOR_TEMPERATURE, /**< Chip temperature */
	INV_SENSOR_DMP_PEDOMETER_EVENT, /**< Pedometer: step detected */
	INV_SENSOR_DMP_PEDOMETER_COUNT, /**< Pedometer: step counter */
	INV_SENSOR_DMP_TILT, /**< Tilt */
	INV_SENSOR_DMP_FF, /**< FreeFall */
	INV_SENSOR_DMP_LOWG, /**< Low G */
	INV_SENSOR_DMP_SMD, /**< Significant Motion Detection */
	INV_SENSOR_MAX
};

/** Configure Fifo usage */
typedef enum {
	INV_IMU_FIFO_DISABLED = 0, /**< Fifo is disabled and data source is sensors registers */
	INV_IMU_FIFO_ENABLED  = 1, /**< Fifo is used as data source */
} INV_IMU_FIFO_CONFIG_t;

/** Sensor event structure definition */
typedef struct {
	int      sensor_mask;
	uint16_t timestamp_fsync;
	int16_t  accel[3];
	int16_t  gyro[3];
	int16_t  temperature;
	int8_t   accel_high_res[3];
	int8_t   gyro_high_res[3];
} inv_imu_sensor_event_t;

/** IMU driver states definition */
struct inv_imu_device {
	/** Transport layer. 
	 *  @warning Must be the first one of struct inv_imu_device 
	 */
	struct inv_imu_transport transport;

	/** callback executed by:
	 *  * inv_imu_get_data_from_fifo (if FIFO is used).
	 *  * inv_imu_get_data_from_registers (if FIFO isn't used).
	 *  May be NULL if above API are not used by application 
	 */
	void (*sensor_event_cb)(inv_imu_sensor_event_t *event);

	
	uint8_t fifo_data[FIFO_MIRRORING_SIZE]; /**< FIFO mirroring memory area */
	uint8_t dmp_is_on; /**< DMP started status */
	uint8_t endianness_data; /**< Data endianness configuration */
	uint8_t fifo_highres_enabled; /**< Highres mode configuration */
	INV_IMU_FIFO_CONFIG_t fifo_is_used; /**< FIFO configuration */
    uint64_t gyro_start_time_us; /**< Gyro start time used to discard first samples */
	uint64_t accel_start_time_us; /**< Accel start time used to discard first samples */
	uint64_t gyro_power_off_tmst; /**< Gyro power off time */
};

/** Interrupt enum state for INT1, INT2, and IBI */
typedef enum {
	INV_IMU_DISABLE = 0, 
	INV_IMU_ENABLE 
} inv_imu_interrupt_value;

/** Interrupt definition */
typedef struct {
	inv_imu_interrupt_value INV_UI_FSYNC;
	inv_imu_interrupt_value INV_UI_DRDY;
	inv_imu_interrupt_value INV_FIFO_THS;
	inv_imu_interrupt_value INV_FIFO_FULL;
	inv_imu_interrupt_value INV_SMD;
	inv_imu_interrupt_value INV_WOM_X;
	inv_imu_interrupt_value INV_WOM_Y;
	inv_imu_interrupt_value INV_WOM_Z;
	inv_imu_interrupt_value INV_FF;
	inv_imu_interrupt_value INV_LOWG;
	inv_imu_interrupt_value INV_STEP_DET;
	inv_imu_interrupt_value INV_STEP_CNT_OVFL;
	inv_imu_interrupt_value INV_TILT_DET;
} inv_imu_interrupt_parameter_t;

/** @brief Initializes device.
 *  @param[in] s                Pointer to device.
 *  @param[in] serif            Pointer on serial interface structure.
 *  @param[in] sensor_event_cb  Callback executed when a new sensor event is available. *
 *  @return                     0 on success, negative value on error.
 */
int inv_imu_init(struct inv_imu_device *s, struct inv_imu_serif *serif,
                 void (*sensor_event_cb)(inv_imu_sensor_event_t *event));

/** @brief Reset device by reloading OTPs.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_device_reset(struct inv_imu_device *s);

/** @brief return WHOAMI value.
 *  @param[in] s          Pointer to device.
 *  @param[out] who_am_i  WHOAMI for device.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_get_who_am_i(struct inv_imu_device *s, uint8_t *who_am_i);

/** @brief Enable/put accel in low power mode.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_enable_accel_low_power_mode(struct inv_imu_device *s);

/** @brief Enable/put accel in low noise mode.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_enable_accel_low_noise_mode(struct inv_imu_device *s);

/** @brief Disable all 3 axes of accel.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_disable_accel(struct inv_imu_device *s);

/** @brief Enable/put gyro in low noise mode.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_enable_gyro_low_noise_mode(struct inv_imu_device *s);

/** @brief Disable all 3 axes of gyro.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_disable_gyro(struct inv_imu_device *s);

/** @brief Enable fsync tagging functionality.
 *         * Enables fsync.
 *         * Enables timestamp to registers. Once fsync is enabled fsync counter is pushed to 
 *           fifo instead of timestamp. So timestamp is made available in registers. Note that 
 *           this increase power consumption.
 *         * Enables fsync related interrupt.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_enable_fsync(struct inv_imu_device *s);

/** @brief Disable fsync tagging functionality.
 *         * Disables fsync.
 *         * Disables timestamp to registers. Once fsync is disabled  timestamp is pushed to fifo 
 *           instead of fsync counter. So in order to decrease power consumption, timestamp is no 
 *           more available in registers.
 *         * Disables fsync related interrupt.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_disable_fsync(struct inv_imu_device *s);

/** @brief Configure which interrupt source can trigger INT1.
 *  @param[in] s                       Pointer to device.
 *  @param[in] interrupt_to_configure  Structure with the corresponding state to manage INT1.
 *  @return                            0 on success, negative value on error.
 */
int inv_imu_set_config_int1(struct inv_imu_device *        s,
                            inv_imu_interrupt_parameter_t *interrupt_to_configure);

/** @brief Retrieve interrupts configuration.
 *  @param[in] s                       Pointer to device.
 *  @param[in] interrupt_to_configure  Structure with the corresponding state to manage INT1.
 *  @return                            0 on success, negative value on error.
 */
int inv_imu_get_config_int1(struct inv_imu_device *        s,
                            inv_imu_interrupt_parameter_t *interrupt_to_configure);

/** @brief  Configure which interrupt source can trigger INT2.
 *  @param[in] s                       Pointer to device.
 *  @param[in] interrupt_to_configure  Structure with the corresponding state to INT2.
 *  @return                            0 on success, negative value on error.
 */
int inv_imu_set_config_int2(struct inv_imu_device *        s,
                            inv_imu_interrupt_parameter_t *interrupt_to_configure);

/** @brief  Retrieve interrupts configuration.
 *  @param[in] s                       Pointer to device.
 *  @param[in] interrupt_to_configure  Structure with the corresponding state to manage INT2.
 *  @return                            0 on success, negative value on error.
 */
int inv_imu_get_config_int2(struct inv_imu_device *        s,
                            inv_imu_interrupt_parameter_t *interrupt_to_configure);

/** @brief Read all registers containing data (temperature, accelerometer and gyroscope). 
 *         Then it calls sensor_event_cb function passed at init for each packet.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_get_data_from_registers(struct inv_imu_device *s);

/** @brief Read all available packets from the FIFO. 
 *         For each packet function builds a sensor event containing packet data 
 *         and validity information. Then it calls sensor_event_cb funtion passed 
 *         at init for each packet.
 *  @param[in] s  Pointer to device.
 *  @return       Number of valid packets read on success, negative value on error.
 */
int inv_imu_get_data_from_fifo(struct inv_imu_device *s);

/** @brief Converts ACCEL_CONFIG0_ODR_t or GYRO_CONFIG0_ODR_t enums to period expressed in us.
 *  @param[in] odr_bitfield An ACCEL_CONFIG0_ODR_t or GYRO_CONFIG0_ODR_t enum.
 *  @return    The corresponding period expressed in us.
 */
uint32_t inv_imu_convert_odr_bitfield_to_us(uint32_t odr_bitfield);

/** @brief Configure accel Output Data Rate.
 *  @param[in] s          Pointer to device.
 *  @param[in] frequency  The requested frequency.
 *  @return               0 on success, negative value on error.
 */
int inv_imu_set_accel_frequency(struct inv_imu_device *s, const ACCEL_CONFIG0_ODR_t frequency);

/** @brief Configure gyro Output Data Rate.
 *  @param[in] s          Pointer to device.
 *  @param[in] frequency  The requested frequency.
 *  @return               0 on success, negative value on error.
 */
int inv_imu_set_gyro_frequency(struct inv_imu_device *s, const GYRO_CONFIG0_ODR_t frequency);

/** @brief Set accel full scale range.
 *  @param[in] s            Pointer to device.
 *  @param[in] accel_fsr_g  Requested full scale range.
 *  @return                 0 on success, negative value on error.
 */
int inv_imu_set_accel_fsr(struct inv_imu_device *s, ACCEL_CONFIG0_FS_SEL_t accel_fsr_g);

/** @brief Access accel full scale range.
 *  @param[in] s             Pointer to device.
 *  @param[out] accel_fsr_g  Current full scale range.
 *  @return 0 on success, negative value on error.
 */
int inv_imu_get_accel_fsr(struct inv_imu_device *s, ACCEL_CONFIG0_FS_SEL_t *accel_fsr_g);

/** @brief Set gyro full scale range.
 *  @param[in] s             Pointer to device.
 *  @param[in] gyro_fsr_dps  Requested full scale range.
*   @return                  0 on success, negative value on error.
 */
int inv_imu_set_gyro_fsr(struct inv_imu_device *s, GYRO_CONFIG0_FS_SEL_t gyro_fsr_dps);

/** @brief Access gyro full scale range.
 *  @param[in] s              Pointer to device.
 *  @param[out] gyro_fsr_dps  Current full scale range.
 *  @return                   0 on success, negative value on error.
 */
int inv_imu_get_gyro_fsr(struct inv_imu_device *s, GYRO_CONFIG0_FS_SEL_t *gyro_fsr_dps);

/** @brief Set accel Low-Power averaging value.
 *  @param[in] s        Pointer to device.
 *  @param[in] acc_avg  Requested averaging value.
 *  @return             0 on success, negative value on error.
 */
int inv_imu_set_accel_lp_avg(struct inv_imu_device *s, ACCEL_CONFIG1_ACCEL_FILT_AVG_t acc_avg);

/** @brief Set accel Low-Noise bandwidth value.
 *  @param[in] s       Pointer to device.
 *  @param[in] acc_bw  Requested averaging value.
 *  @return            0 on success, negative value on error.
 */
int inv_imu_set_accel_ln_bw(struct inv_imu_device *s, ACCEL_CONFIG1_ACCEL_FILT_BW_t acc_bw);

/** @brief Set gyro Low-Noise bandwidth value.
 *  @param[in] s       Pointer to device.
 *  @param[in] gyr_bw  Requested averaging value.
 *  @return            0 on success, negative value on error.
 */
int inv_imu_set_gyro_ln_bw(struct inv_imu_device *s, GYRO_CONFIG1_GYRO_FILT_BW_t gyr_bw);

/** @brief Set timestamp resolution.
 *  @param[in] s                Pointer to device.
 *  @param[in] timestamp_resol  Requested timestamp resolution.
 *  @return                     0 on success, negative value on error.
 */
int inv_imu_set_timestamp_resolution(struct inv_imu_device *    s,
                                     const TMST_CONFIG1_RESOL_t timestamp_resol);

/** @brief Reset IMU fifo.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_reset_fifo(struct inv_imu_device *s);

/** @brief Enable 20 bits raw acc and raw gyr data in fifo.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_enable_high_resolution_fifo(struct inv_imu_device *s);

/** @brief Disable 20 bits raw acc and raw gyr data in fifo.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_disable_high_resolution_fifo(struct inv_imu_device *s);

/** @brief Configure Fifo.
  *  @param[in] s            Pointer to device.
  *  @param[in] fifo_config  Fifo configuration method.
  *                          Enabled: data are pushed to FIFO and FIFO THS interrupt is set.
  *                          Disabled: data are not pused to FIFO and DRDY interrupt is set.
  *  @return                 0 on success, negative value on error.
  */
int inv_imu_configure_fifo(struct inv_imu_device *s, INV_IMU_FIFO_CONFIG_t fifo_config);

/** @brief Get timestamp resolution
 *  @param[in] s  Pointer to device.
 *  @return       The timestamp resolution in us or 0 in case of error
 */
uint32_t inv_imu_get_timestamp_resolution_us(struct inv_imu_device *s);

/** @brief  Enable Wake On Motion.
 *  @param[in] s         Pointer to device.
 *  @param[in] wom_x_th  Threshold value for the Wake on Motion Interrupt for X-axis accel.
 *  @param[in] wom_y_th  Threshold value for the Wake on Motion Interrupt for Y-axis accel.
 *  @param[in] wom_z_th  Threshold value for the Wake on Motion Interrupt for Z-axis accel.
 *  @param[in] wom_int   Select which mode between AND/OR is used to generate interrupt.
 *  @param[in] wom_dur   Select the number of overthreshold event to wait 
 *                       before generating interrupt.
 *  @return              0 on success, negative value on error.
 */
int inv_imu_configure_wom(struct inv_imu_device *s, const uint8_t wom_x_th, const uint8_t wom_y_th,
                          const uint8_t wom_z_th, WOM_CONFIG_WOM_INT_MODE_t wom_int,
                          WOM_CONFIG_WOM_INT_DUR_t wom_dur);

/** @brief Enable Wake On Motion.
 *         WoM requests to have the accelerometer enabled to work. 
 *         As a consequence Fifo water-mark interrupt is disabled to only trigger WoM interrupts.
 *         To have good performance, it's recommended to set accel ODR to 20ms 
 *         and in Low Power Mode. 
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_enable_wom(struct inv_imu_device *s);

/** @brief Disable Wake On Motion.
 *         Fifo water-mark interrupt is re-enabled when WoM is disabled.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_disable_wom(struct inv_imu_device *s);

/** @brief Start DMP for APEX algorithms and selftest.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_start_dmp(struct inv_imu_device *s);

/** @brief Reset DMP for APEX algorithms and selftest.
 *  @param[in] s           Pointer to device.
 *  @param[in] sram_reset  Reset mode for the SRAM.
 *  @return                0 on success, negative value on error.
 */
int inv_imu_reset_dmp(struct inv_imu_device *s, const APEX_CONFIG0_DMP_MEM_RESET_t sram_reset);

/** @brief Set the UI endianness and set the inv_device endianness field.
 *  @param[in] s           Pointer to device.
 *  @param[in] endianness  Endianness to be set.
 *  @return                0 on success, negative value on error.
 */
int inv_imu_set_endianness(struct inv_imu_device *s, INTF_CONFIG0_DATA_ENDIAN_t endianness);

/** @brief Read the UI endianness and set the inv_device endianness field.
 *  @param[in] s  Pointer to device.
 *  @return       0 on success, negative value on error.
 */
int inv_imu_get_endianness(struct inv_imu_device *s);

/** @brief Configure Fifo decimation.
 *  @param[in] s           Pointer to device.
 *  @param[in] dec_factor  Requested decimation factor value from 2 to 256.
 *  @return                0 on success, negative value on error.
 */
int inv_imu_configure_fifo_data_rate(struct inv_imu_device *s, FDR_CONFIG_FDR_SEL_t dec_factor);

/** @brief Return driver version x.y.z-suffix as a char array
 *  @return driver version a char array "x.y.z-suffix"
 */
const char *inv_imu_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* _INV_IMU_DRIVER_H_ */

/** @} */
