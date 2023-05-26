/* NOTE: */
/* Code is based on sample code from:  */
/* zephyrproject/zephyr/samples/sensor/hts221 */
/* zephyrproject/zephyr/samples/sensor/lps22hb */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor/ccs811.h>

static inline float get_sensor_val(struct sensor_value *val);

/* lps22hb, ccs811 and hts221 devices */
const struct device *const lps22hbDev = DEVICE_DT_GET_ONE(st_lps22hb_press);
const struct device *const ccs811Dev = DEVICE_DT_GET_ONE(ams_ccs811);
const struct device *const hts221Dev = DEVICE_DT_GET_ONE(st_hts221);

/**
 * Gets the current temperature reading from the hts221
 * Parameter is the temperature
 */
void sensor_temperature(float *temperature) {

    struct sensor_value temp;

    sensor_sample_fetch(hts221Dev);
    sensor_channel_get(hts221Dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    *temperature = get_sensor_val(&temp);  
}

/**
 * Gets the current pressure reading from the lps22hb
 * Parameter is the air pressure
 */
void sensor_pressure(float *pressure) {

    struct sensor_value press;

    sensor_sample_fetch(lps22hbDev);
    sensor_channel_get(lps22hbDev, SENSOR_CHAN_PRESS, &press);
    *pressure = get_sensor_val(&press);
}

/**
 * Gets the current humidity reading from the hts221
 * Parameter is the humidity
 */
void sensor_humidity(float *humidity) {

    struct sensor_value hum;

    sensor_sample_fetch(hts221Dev);
    sensor_channel_get(hts221Dev, SENSOR_CHAN_HUMIDITY, &hum);
    *humidity = get_sensor_val(&hum);
}

/**
 * Gets the current CO2 and TVOC reading from the ccs811
 * Parameters are the CO2 and TVOC variables
 */
void sensor_air_quality(float *qualityCo2, float *qualityTvoc) {

    struct sensor_value co2;
    struct sensor_value tvoc;

    sensor_sample_fetch(ccs811Dev);
    sensor_channel_get(ccs811Dev, SENSOR_CHAN_CO2, &co2);
    sensor_channel_get(ccs811Dev, SENSOR_CHAN_VOC, &tvoc);
    *qualityCo2 = get_sensor_val(&co2);
    *qualityTvoc = get_sensor_val(&tvoc);
}

/**
 * Helper function for converting and acquiring the sensor readings
 * Parameter is the sensor value to be converted
 * Returns the sensor reading as a float
 */
static inline float get_sensor_val(struct sensor_value *val) {

    return (val->val1 + (float) val->val2 / 1000000);
}
