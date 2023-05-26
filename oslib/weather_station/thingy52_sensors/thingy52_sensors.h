#ifndef THINGY52_SENSORS_H
#define THINGY52_SENSORS_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor/ccs811.h>

void sensor_temperature(float *temperature);
void sensor_pressure(float *pressure);
void sensor_humidity(float *humidity);
void sensor_air_quality(float *qualityCo2, float *qualityTvoc);

#endif