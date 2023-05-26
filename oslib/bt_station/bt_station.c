
#include "bt_station.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

// struct k_msgq bt_data_msgq;

// struct station_data {
// 	int temp;
// 	int num;
// 	int air_press;
// 	int tvoc;
// 	int c02;
// };

// K_MSGQ_DEFINE(bt_data_msgq, sizeof(struct station_data), 5, 4);

/* Notification state */
volatile bool notify_enable;

// static ssize_t bt_recv(struct bt_conn *conn,
// 		    const struct bt_gatt_attr *attr, const void *buf,
// 		    uint16_t len, uint16_t offset, uint8_t flags);

// static ssize_t bt_write(struct bt_conn *conn, 
// 			const struct bt_gatt_attr *attr, void *buf, 
// 			uint16_t len, uint16_t offset);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),
};

void setup_bt() {
 	printk("Setting up some bluetooth!!!\n");

	int err;

    // Initialize Bluetooth
    err = bt_enable(NULL);
	
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Failed to start advertising\n");
		return;
	}
}