/*
 *
 * ------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2022 InvenSense, Inc All rights reserved.
 *
 * This software, related documentation and any modifications thereto (collectively "Software") is subject
 * to InvenSense, Inc and its licencors' intellectual property rights under U.S. and international copyright
 * and other intellectual property rights laws.
 *
 * InvenSense, Inc and its licencors retain all intellectual property and proprietary rights in and to the Software
 * and any use, reproduction, disclosure or distribution of the Software without an express license agreement
 * from InvenSense, Inc is strictly prohibited.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE SOFTWARE IS
 * PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * InvenSense, Inc BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTUOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 *
 * ------------------------------------------------------------------------------------------------------------
 */

#include "Arduino.h"
#include "ICM42670P.h"

static int i2c_write(struct inv_imu_serif * serif, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen);
static int i2c_read(struct inv_imu_serif * serif, uint8_t reg, uint8_t * rbuffer, uint32_t rlen);
static int spi_write(struct inv_imu_serif * serif, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen);
static int spi_read(struct inv_imu_serif * serif, uint8_t reg, uint8_t * rbuffer, uint32_t rlen);
static void event_cb(inv_imu_sensor_event_t *event);

// i2c and SPI interfaces are used from C driver callbacks, without any knowledge of the object
// As they are declared as static, they will be overriden each time a new ICM42670P object is created
// i2c
uint8_t i2c_address = 0;
static TwoWire *i2c = NULL;
#define ICM42670P_I2C_SPEED 1000000
#define ICM42670P_I2C_ADDRESS 0x68
// spi
static SPIClass *spi = NULL;
static uint8_t chip_select_id = 0;
bool _useSPI = false;
#define SPI_READ 0x80
#define SPI_CLOCK 16000000

// This is used by the event callback (not object aware), declared static
static inv_imu_sensor_event_t* event;

// This is used by the getDataFromFifo callback (not object aware), declared static
static struct inv_imu_device *icm_driver_ptr = NULL;

// ICM42670P constructor for I2c interface
ICM42670P::ICM42670P(TwoWire &i2c_ref,bool lsb) {
  i2c = &i2c_ref;
  i2c_address = ICM42670P_I2C_ADDRESS | (lsb ? 0x1 : 0);
}

// ICM42670P constructor for spi interface
ICM42670P::ICM42670P(SPIClass &spi_ref,uint8_t cs_id) {
  spi = &spi_ref;
  chip_select_id = cs_id;
}

/* starts communication with the ICM42670P */
int ICM42670P::begin() {
  struct inv_imu_serif icm_serif;
  int rc = 0;
  uint8_t who_am_i;

  if (i2c != NULL) {
    i2c->begin();
    i2c->setClock(ICM42670P_I2C_SPEED);
    icm_serif.serif_type = UI_I2C;
    icm_serif.read_reg  = i2c_read;
    icm_serif.write_reg = i2c_write;
  } else {
    spi->begin();
    pinMode(chip_select_id,OUTPUT);
    digitalWrite(chip_select_id,HIGH);
    icm_serif.serif_type = UI_SPI4;
    icm_serif.read_reg  = spi_read;
    icm_serif.write_reg = spi_write;
  }
  /* Initialize serial interface between MCU and Icm43xxx */
  icm_serif.context   = 0;        /* no need */
  icm_serif.max_read  = 2048; /* maximum number of bytes allowed per serial read */
  icm_serif.max_write = 2048; /* maximum number of bytes allowed per serial write */
  rc = inv_imu_init(&icm_driver, &icm_serif, NULL);
  if (rc != INV_ERROR_SUCCESS) {
    return rc;
  }
  icm_driver.sensor_event_cb = event_cb;

  /* Check WHOAMI */
  rc = inv_imu_get_who_am_i(&icm_driver, &who_am_i);
  if(rc != 0) {
    return -2;
  }
  if ((who_am_i != ICM42607P_WHOAMI) && (who_am_i != ICM42607C_WHOAMI) && (who_am_i != ICM42670P_WHOAMI) && (who_am_i != ICM42670S_WHOAMI) && (who_am_i != ICM42670T_WHOAMI)) {
    return -3;
  }

  // successful init, return 0
  return 0;
}

int ICM42670P::startAccel(uint16_t odr, uint16_t fsr) {
  int rc = 0;
  rc |= inv_imu_set_accel_fsr(&icm_driver, accel_fsr_g_to_param(fsr));
  rc |= inv_imu_set_accel_frequency(&icm_driver, accel_freq_to_param(odr));
  rc |= inv_imu_enable_accel_low_noise_mode(&icm_driver);
  return rc;
}

int ICM42670P::startGyro(uint16_t odr, uint16_t fsr) {
  int rc = 0;
  rc |= inv_imu_set_gyro_fsr(&icm_driver, gyro_fsr_dps_to_param(fsr));
  rc |= inv_imu_set_gyro_frequency(&icm_driver, gyro_freq_to_param(odr));
  rc |= inv_imu_enable_gyro_low_noise_mode(&icm_driver);
  return rc;
}

int ICM42670P::getDataFromRegisters(inv_imu_sensor_event_t* evt) {
  if(evt != NULL) {
    // Set event buffer to be used by the callback
    event = evt;
    return inv_imu_get_data_from_registers(&icm_driver);
  } else {
    return -1;
  }
}

int ICM42670P::enableFifoInterrupt(uint8_t intpin, ICM42670P_irq_handler handler, uint8_t fifo_watermark) {
  int rc = 0;
  uint8_t data;

  if(handler == NULL) {
    return -1;
  }
  pinMode(intpin, INPUT);
  attachInterrupt(intpin, handler, RISING);
  rc |= inv_imu_configure_fifo(&icm_driver, INV_IMU_FIFO_ENABLED);
  rc |= inv_imu_write_reg(&icm_driver, FIFO_CONFIG2, 1, &fifo_watermark);
  // Set fifo_wm_int_w generating condition : fifo_wm_int_w generated when counter == threshold
  rc |= inv_imu_read_reg(&icm_driver, FIFO_CONFIG5_MREG1, 1, &data);
  data &= (uint8_t)~FIFO_CONFIG5_WM_GT_TH_EN;
  rc |= inv_imu_write_reg(&icm_driver, FIFO_CONFIG5_MREG1, 1, &data);
  // Disable APEX to use 2.25kB of fifo for raw data
  data = SENSOR_CONFIG3_APEX_DISABLE_MASK;
  rc |= inv_imu_write_reg(&icm_driver, SENSOR_CONFIG3_MREG1, 1, &data);
  return rc;
}

int ICM42670P::enableDataInterrupt(uint8_t intpin, ICM42670P_irq_handler handler) {
  int rc = 0;
  uint8_t data;

  if(handler == NULL) {
    return -1;
  }
  pinMode(intpin, INPUT);
  attachInterrupt(intpin, handler, RISING);
  rc |= inv_imu_configure_fifo(&icm_driver, INV_IMU_FIFO_DISABLED);
  return rc;
}

int ICM42670P::getDataFromFifo(ICM42670P_sensor_event_cb event_cb) {
  if(event_cb == NULL) {
    return -1;
  }
  icm_driver.sensor_event_cb = event_cb;
  return inv_imu_get_data_from_fifo(&icm_driver);
}

bool ICM42670P::isAccelDataValid(inv_imu_sensor_event_t *evt) {
  return (evt->sensor_mask & (1<<INV_SENSOR_ACCEL));
}

bool ICM42670P::isGyroDataValid(inv_imu_sensor_event_t *evt) {
  return (evt->sensor_mask & (1<<INV_SENSOR_GYRO));
}

static int i2c_write(struct inv_imu_serif * serif, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen) {
  i2c->beginTransmission(i2c_address);
  i2c->write(reg);
  for(uint8_t i = 0; i < wlen; i++) {
    i2c->write(wbuffer[i]);
  }
  i2c->endTransmission();
  return 0;
}

static int i2c_read(struct inv_imu_serif * serif, uint8_t reg, uint8_t * rbuffer, uint32_t rlen) {
  uint16_t rx_bytes = 0;

  i2c->beginTransmission(i2c_address);
  i2c->write(reg);
  i2c->endTransmission(false);
  rx_bytes = i2c->requestFrom(i2c_address, rlen);
  if (rlen == rx_bytes) {
    for(uint8_t i = 0; i < rx_bytes; i++) {
      rbuffer[i] = i2c->read();
    }
    return 0;
  } else {
    return -1;
  }
}

static int spi_write(struct inv_imu_serif * serif, uint8_t reg, const uint8_t * wbuffer, uint32_t wlen) {
  spi->beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE1));
  digitalWrite(chip_select_id,LOW);
  spi->transfer(reg);
  for(uint8_t i = 0; i < wlen; i++) {
    spi->transfer(wbuffer[i]);
  }
  digitalWrite(chip_select_id,HIGH);
  spi->endTransaction();
  return 0;
}

static int spi_read(struct inv_imu_serif * serif, uint8_t reg, uint8_t * rbuffer, uint32_t rlen) {
  spi->beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE1));
  digitalWrite(chip_select_id,LOW);
  spi->transfer(reg | SPI_READ);
  spi->transfer(rbuffer,rlen);
  digitalWrite(chip_select_id,HIGH);
  spi->endTransaction();
  return 0;
}

ACCEL_CONFIG0_FS_SEL_t ICM42670P::accel_fsr_g_to_param(uint16_t accel_fsr_g) {
  ACCEL_CONFIG0_FS_SEL_t ret = ACCEL_CONFIG0_FS_SEL_16g;

  switch(accel_fsr_g) {
  case 2:  ret = ACCEL_CONFIG0_FS_SEL_2g;  break;
  case 4:  ret = ACCEL_CONFIG0_FS_SEL_4g;  break;
  case 8:  ret = ACCEL_CONFIG0_FS_SEL_8g;  break;
  case 16: ret = ACCEL_CONFIG0_FS_SEL_16g; break;
  default:
    /* Unknown accel FSR. Set to default 16G */
    break;
  }
  return ret;
}

GYRO_CONFIG0_FS_SEL_t ICM42670P::gyro_fsr_dps_to_param(uint16_t gyro_fsr_dps) {
  GYRO_CONFIG0_FS_SEL_t ret = GYRO_CONFIG0_FS_SEL_2000dps;

  switch(gyro_fsr_dps) {
  case 250:  ret = GYRO_CONFIG0_FS_SEL_250dps;  break;
  case 500:  ret = GYRO_CONFIG0_FS_SEL_500dps;  break;
  case 1000: ret = GYRO_CONFIG0_FS_SEL_1000dps; break;
  case 2000: ret = GYRO_CONFIG0_FS_SEL_2000dps; break;
  default:
    /* Unknown gyro FSR. Set to default 2000dps" */
    break;
  }
  return ret;
}

ACCEL_CONFIG0_ODR_t ICM42670P::accel_freq_to_param(uint16_t accel_freq_hz) {
  ACCEL_CONFIG0_ODR_t ret = ACCEL_CONFIG0_ODR_100_HZ;

  switch(accel_freq_hz) {
  case 12:   ret = ACCEL_CONFIG0_ODR_12_5_HZ;  break;
  case 25:   ret = ACCEL_CONFIG0_ODR_25_HZ;  break;
  case 50:   ret = ACCEL_CONFIG0_ODR_50_HZ;  break;
  case 100:  ret = ACCEL_CONFIG0_ODR_100_HZ; break;
  case 200:  ret = ACCEL_CONFIG0_ODR_200_HZ; break;
  case 400:  ret = ACCEL_CONFIG0_ODR_400_HZ; break;
  case 800:  ret = ACCEL_CONFIG0_ODR_800_HZ; break;
  case 1600: ret = ACCEL_CONFIG0_ODR_1600_HZ;  break;
  default:
    /* Unknown accel frequency. Set to default 100Hz */
    break;
  }
  return ret;
}

GYRO_CONFIG0_ODR_t ICM42670P::gyro_freq_to_param(uint16_t gyro_freq_hz) {
  GYRO_CONFIG0_ODR_t ret = GYRO_CONFIG0_ODR_100_HZ;

  switch(gyro_freq_hz) {
  case 12:   ret = GYRO_CONFIG0_ODR_12_5_HZ;  break;
  case 25:   ret = GYRO_CONFIG0_ODR_25_HZ;  break;
  case 50:   ret = GYRO_CONFIG0_ODR_50_HZ;  break;
  case 100:  ret = GYRO_CONFIG0_ODR_100_HZ; break;
  case 200:  ret = GYRO_CONFIG0_ODR_200_HZ; break;
  case 400:  ret = GYRO_CONFIG0_ODR_400_HZ; break;
  case 800:  ret = GYRO_CONFIG0_ODR_800_HZ; break;
  case 1600: ret = GYRO_CONFIG0_ODR_1600_HZ;  break;
  default:
    /* Unknown gyro ODR. Set to default 100Hz */
    break;
  }
  return ret;
}


static void event_cb(inv_imu_sensor_event_t *evt) {
  memcpy(event,evt,sizeof(inv_imu_sensor_event_t));
}
