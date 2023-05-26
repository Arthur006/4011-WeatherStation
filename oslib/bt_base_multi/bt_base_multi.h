#ifndef _BT_BASE_MULTI_H_
#define _BT_BASE_MULTI_H_

#include <stdint.h>

struct WeatherData {
    uint8_t stationID;
	uint16_t temp;
	uint8_t hum;
	uint16_t air_press;
	uint16_t tvoc;
	uint16_t c02;
};

struct StationData {
	uint16_t temp;
	uint8_t hum;
	uint16_t air_press;
	uint16_t tvoc;
	uint16_t c02;
};

void start_scan(void);
void setup_bt();
void read_weather_data();



#endif