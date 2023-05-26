#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <string.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/drivers/uart.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "thingy52_sensors.h"
#include "bt_station.h"

/* BT Setup */

struct bt_uuid_128 primary_service_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_SERVICE_VAL);

struct bt_uuid_128 req_uuid = BT_UUID_INIT_128(BT_UUID_CUSTOM_CHAR_VAL);

struct k_msgq bt_data_msgq;

struct StationData {
	uint16_t temp;
	uint8_t hum;
	uint16_t air_press;
	uint16_t tvoc;
	uint16_t c02;
};

K_MSGQ_DEFINE(bt_data_msgq, sizeof(struct StationData), 1, 8);

static ssize_t bt_recv(struct bt_conn *conn,
		    const struct bt_gatt_attr *attr, const void *buf,
		    uint16_t len, uint16_t offset, uint8_t flags);

static ssize_t bt_write(struct bt_conn *conn, 
			const struct bt_gatt_attr *attr, void *buf, 
			uint16_t len, uint16_t offset);

BT_GATT_SERVICE_DEFINE(ble_svc,
BT_GATT_PRIMARY_SERVICE(&primary_service_uuid),
BT_GATT_CHARACTERISTIC(&req_uuid.uuid,
		       BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
		       BT_GATT_PERM_WRITE | BT_GATT_PERM_READ, bt_write, bt_recv, NULL));

/* Thread definitions */
#define SENSING_STACK_SIZE 500
#define SENSING_PRIORITY 5

/* Thread definitions */
K_THREAD_STACK_DEFINE(senStackArea, SENSING_STACK_SIZE);
struct k_thread senThreadData;

/* DeviceTree get node ID from label */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

/* State */
#define LOW 0
#define HIGH 1
#define RED 0
#define GREEN 1
#define BLUE 2

/* LED0, LED1 and LED2 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

void sensing(void *, void *, void *);
void print_uart(char *msg, int length);
void set_led_colour(int colour);

/* Main function for setting up threads */
int main(void) {

    setup_bt();
    /* Create sensing thread */  
    k_thread_create(&senThreadData, senStackArea,
            K_THREAD_STACK_SIZEOF(senStackArea),
            sensing,
            NULL, NULL, NULL,
            SENSING_PRIORITY, 0, K_NO_WAIT);

    return 0;
}

/**
 * Thread for sensing
 * Parameters are thread presets
 */
void sensing(void *a, void *b, void *c) {
    
    /* Variables for storing sensor readings */
    float tp = 0, pr = 0;
    float hum = 0, co2 = 0, tvoc = 0;

    printk("START\n");  

    /* Configure PINS */
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
    
    /* Set led to red as bluetooth connection has not been established */
    set_led_colour(RED);

    while(1) {

        /* Get sensor values */       
        sensor_temperature(&tp);
        sensor_pressure(&pr);
        sensor_air_quality(&co2, &tvoc);
        sensor_humidity(&hum);

        uint16_t tpx = (uint16_t)(tp * (float)100);
        uint16_t prx = (uint16_t)(pr * (float)100);
        uint8_t humx = (uint8_t)(hum * (float)100);
        uint16_t co2x = (uint16_t)(co2 * (float)100);
        uint16_t tvocx = (uint16_t)(tvoc * (float)100);

        // Send data to queue

        struct StationData station_data;

        station_data.temp = tpx;
        station_data.hum = humx;
        station_data.air_press = prx;
        station_data.tvoc = tvocx;
        station_data.c02 = co2x;

		k_msgq_put(&bt_data_msgq, &station_data, K_NO_WAIT);
    }
}

/**
 * Function for setting the RGB led to either red, green or blue
 * Parameter is the colour the led will be set too
 */
void set_led_colour(int colour) {
    
    if (colour == RED) {
        gpio_pin_set_dt(&led0, HIGH);
        gpio_pin_set_dt(&led1, LOW);
        gpio_pin_set_dt(&led2, LOW);
        
    } else if (colour == GREEN) {
        gpio_pin_set_dt(&led0, LOW);
        gpio_pin_set_dt(&led1, HIGH);
        gpio_pin_set_dt(&led2, LOW);
            
    } else if (colour == BLUE) {
        gpio_pin_set_dt(&led0, LOW);
        gpio_pin_set_dt(&led1, LOW);
        gpio_pin_set_dt(&led2, HIGH);
    }
}

static ssize_t bt_write(struct bt_conn *conn,
		    const struct bt_gatt_attr *attr, void *buf,
		    uint16_t len, uint16_t offset)
{
    // BLUE
    static int blue = 0;

    if (blue)
        set_led_colour(BLUE);
    else
        set_led_colour(GREEN);

    blue = !blue;

	struct StationData station_data;

    k_msgq_get(&bt_data_msgq, &station_data, K_FOREVER);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, (void *)&station_data, sizeof(station_data));
}

static ssize_t bt_recv(struct bt_conn *conn,
		    const struct bt_gatt_attr *attr, const void *buf,
		    uint16_t len, uint16_t offset, uint8_t flags)
{
	char cmd[30];
	memset(cmd, 0, 30);
	memcpy(cmd, buf, len);
	printk("Received!!!\n");
	printk("Data: %s\n", cmd);

	return 0;
}
