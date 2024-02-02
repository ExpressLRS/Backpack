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

/** @defgroup ExtFunc ExtFunc
 *  @brief Extern functions (to be implemented in application) required by the driver.
 *  @{
 */

/** @file  inv_imu_extfunc.h */

#ifndef _INV_IMU_EXTFUNC_H_
#define _INV_IMU_EXTFUNC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Sleep function.
 *  @param[in] us  Sleep duration in microseconds (us).
 */
extern void inv_imu_sleep_us(uint32_t us);

/** @brief Get time function. Value is expected to be on 64bit with a 1 us resolution.
 *  @return  The current time in us.
 */
extern uint64_t inv_imu_get_time_us(void);

#ifdef __cplusplus
}
#endif

#endif /* _INV_IMU_EXTFUNC_H_ */

/** @} */
