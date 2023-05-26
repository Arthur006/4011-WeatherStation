#ifndef _BT_STATION_H_
#define _BT_STATION_H_

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

/* Custom Service Variables */
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

#define BT_UUID_CUSTOM_CHAR_VAL \
	BT_UUID_128_ENCODE(0x45825688, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

extern struct bt_uuid_128 primary_service_uuid;

extern struct bt_uuid_128 req_uuid;

void start_scan(void);
void setup_bt();


#endif