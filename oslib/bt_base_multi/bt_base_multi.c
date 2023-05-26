
#include "bt_base_multi.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

#define BT_UUID_CUSTOM_CHAR_VAL \
	BT_UUID_128_ENCODE(0x45825688, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static struct bt_uuid_128 primary_service_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_SERVICE_VAL);

static struct bt_uuid_128 req_uuid = BT_UUID_INIT_128(BT_UUID_CUSTOM_CHAR_VAL);

static const char *DEV_NAME1 = "STATION1";
static const char *DEV_NAME2 = "STATION2";
static struct bt_conn *conn1;
static struct bt_conn *conn2;

struct k_msgq bt_data_msgq;

K_MSGQ_DEFINE(bt_data_msgq, sizeof(struct WeatherData), 4, 4);


struct bt_gatt_attr ble_data_read_write;


static bool parse_adv(struct bt_data *data, void *user_data) {
	char buf[30];
	memset(buf, 0, 30);
	memcpy(buf, data->data, data->data_len);

	int *dev_id = (int *)user_data;

	if (data->type == BT_DATA_NAME_COMPLETE) {
		if (!strcmp(buf, DEV_NAME1)) {
            printk("Got complete name1: %s\n", buf);
			*dev_id = 1;
            return false;
		} else if (!strcmp(buf, DEV_NAME2)) {
			printk("Got complete name2: %s\n", buf);
			*dev_id = 2;
			return false;
		}
    }
	return true;
}

static void advertising_data_received(const bt_addr_le_t *addr, int8_t rssi,
                             uint8_t adv_type, struct net_buf_simple *buf)
{
	int dev_id = 0;
	bt_data_parse(buf, parse_adv, &dev_id);

	if (!dev_id)
		return;

	if (bt_le_scan_stop()) {
		return;
	}
	
	int err = 0;
	if (dev_id == 1) {
		if (conn1 != NULL) {
			start_scan();
			return;
		}
		err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
				BT_LE_CONN_PARAM_DEFAULT, &conn1);
	} else if (dev_id == 2) {
		if (conn2 != NULL) {
			start_scan();
			return;
		}
		err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
				BT_LE_CONN_PARAM_DEFAULT, &conn2);
	}

	if (err) {
		printk("Create conn failed (%u)\n", err);
		start_scan();
	}
}

void start_scan(void)
{
	int err;

    err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, advertising_data_received);
	if (err) {
		printk("Scanning failed to start (err %d)\n", err);
		return;
	}

	printk("Scanning successfully started\n");
}

struct bt_gatt_write_params write_params = {
    .func = NULL,
};

uint8_t read_func(struct bt_conn *conn, uint8_t err, struct bt_gatt_read_params *params, const void *data, uint16_t length) {
	// printk("Read: %d\n", length);

	struct StationData station_data;
	memcpy(&station_data, data, length);
	
	// printk("Temp: %u\n", station_data.temp);
	// printk("Hum: %u\n", station_data.hum);
	// printk("Press: %u\n", station_data.air_press);
	// printk("c02: %u\n", station_data.c02);
	// printk("tvoc: %u\n", station_data.tvoc);

	uint8_t stationId = 0;
	if (conn == conn1) 
		stationId = 1;
	else if (conn == conn2)
		stationId = 2;

	struct WeatherData weather_data = {.stationID = stationId, .temp = station_data.temp, 
			.hum = station_data.hum, .air_press = station_data.air_press, .tvoc = station_data.tvoc,
			.c02 = station_data.c02};

	k_msgq_put(&bt_data_msgq, &weather_data, K_NO_WAIT);

	return BT_GATT_ITER_STOP;
}

struct bt_gatt_read_params read_params = {
    .func = read_func,
	.handle_count = 0,
	.by_uuid.uuid = &req_uuid.uuid,
	.by_uuid.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE,
	.by_uuid.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE,
};

void read_weather_data() {
	if (conn1 != NULL) {
		if (bt_gatt_read(conn1, &read_params))
			printk("Error!!!!\n");
	}
	if (conn2 != NULL) {
		if (bt_gatt_read(conn2, &read_params))
			printk("Error!!!!\n");
	}
}

// static struct bt_gatt_discover_params params = {
//     // .uuid = BT_UUID_TYPE_16,
//     .func = discover,
//     .start_handle = 0x0001,
//     .end_handle = 0xffff,
//     .type = BT_GATT_DISCOVER_PRIMARY,
// };

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		printk("Failed to connect to %s (%u)\n", addr, err);

		if (conn == conn1) {
			bt_conn_unref(conn1);
			conn1 = NULL;
		} else if (conn == conn2) {
			bt_conn_unref(conn2);
			conn2 = NULL;
		}

		start_scan();
		return;
	}

	if (conn != conn1 && conn != conn2) {
		start_scan();
		return;
	}

    // bt_gatt_discover(conn, &params);

	printk("Connected: %s\n", addr);

	if (conn1 == NULL || conn2 == NULL) {
		start_scan();
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (conn != conn1 && conn != conn2) {
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	if (conn == conn1) {
		bt_conn_unref(conn1);
		conn1 = NULL;
	} else if (conn == conn2) {
		bt_conn_unref(conn2);
		conn2 = NULL;
	}

	start_scan();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

void setup_bt()
{
    printk("Setting up some bluetooth!!!\n");
    int err = bt_enable(NULL);

    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return;
    }
}

