/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef __SENSOR_GYRO_H__
#define __SENSOR_GYRO_H__

#include "os/os.h"
#include "os/os_dev.h"
#include "sensor/sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Data representing a singular read from a gyroscope
 * All values are in degress per sec
 */
struct sensor_gyro_data {
    float sgd_x;
    float sgd_y;
    float sgd_z;
    /* Validity */
    uint8_t sgd_x_is_valid:1;
    uint8_t sgd_y_is_valid:1;
    uint8_t sgd_z_is_valid:1;
} __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_GYRO_H__ */
