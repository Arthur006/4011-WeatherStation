/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <string.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <stdio.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <ctype.h>
#include <zephyr/sys/slist.h>

#include "bt_base_multi.h"

extern struct k_msgq bt_data_msgq;

/* Thread definitions */
#define PC_SERIAL_STACK_SIZE 4000
#define PC_SERIAL_PRIORITY 6

/* UART sending and receiving definitions */
#define REQ_ADD 0x01
#define REQ_REM 0x02
#define RESP 0x03
#define PREAMBLE 0xAA
#define SEND_LENGTH 24

/* Thread definitions */
K_THREAD_STACK_DEFINE(pcSerialStackArea, PC_SERIAL_STACK_SIZE);
struct k_thread pcSerialData;

const struct device *shellDev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

void pc_serial_thread(void *, void *, void *);
void print_uart(char *msg);

/*
 * Sample app to init USB, and demonstrate shell functionality
 */
int main(void) {

    /* Create serial PC connection thread */
    k_thread_create(&pcSerialData, pcSerialStackArea,
            K_THREAD_STACK_SIZEOF(pcSerialStackArea),
            pc_serial_thread,
            NULL, NULL, NULL,
            PC_SERIAL_PRIORITY, 0, K_NO_WAIT);

    /* Setup DTR */
    uint32_t dtr = 0;

    if (usb_enable(NULL))
        return 0;

    /* Wait on DTR - 'Data Terminal Ready */
    while (!dtr) {
        uart_line_ctrl_get(shellDev, UART_LINE_CTRL_DTR, &dtr);
        k_msleep(100);
    } 

    /* DTR - OK, Continue */
    return 0;
}

void pc_serial_thread(void *a, void *b, void *c) {

    setup_bt();
    start_scan();

    while (1) {

        read_weather_data();

        struct WeatherData weather_data;

        while (!k_msgq_get(&bt_data_msgq, &weather_data, K_NO_WAIT)) {

            /* Used to convert sensor int readings into 4 chars */
            char cTemp[sizeof(int)];
            char cPress[sizeof(int)];
            char cHum[sizeof(int)];
            char cCo2[sizeof(int)];
            char cTvoc[sizeof(int)];

            int val;
            val = weather_data.temp * 10;
            memcpy(cTemp, &val, sizeof(int));
            val = weather_data.air_press * 10;
            memcpy(cPress, &val, sizeof(int));
            val = weather_data.hum * 10;
            memcpy(cHum, &val, sizeof(int));
            val = weather_data.c02 * 10;
            memcpy(cCo2, &val, sizeof(int));
            val = weather_data.tvoc * 10 + 1;
            memcpy(cTvoc, &val, sizeof(int));

            /* String to be sent as a response to the AHU */
            char baseData[SEND_LENGTH] = {PREAMBLE, weather_data.stationID,
                    cTemp[0], cTemp[1], cTemp[2], cTemp[3],
                    cPress[0], cPress[1], cPress[2], cPress[3],
                    cHum[0], cHum[1], cHum[2], cHum[3],
                    cCo2[0], cCo2[1], cCo2[2], cCo2[3],
                    cTvoc[0], cTvoc[1], cTvoc[2], cTvoc[3], '\n'};

            /* Send data over serial */
            print_uart(baseData);
        }

        // printk("Done\n");

        /* Sleep */
        k_msleep(2000);
    }
}

/**
 * Print a string to UART
 * Parameter is the string to be printed
 */
void print_uart(char *msg) {

    for (int i = 0; i < SEND_LENGTH; i++) {

        uart_poll_out(shellDev, msg[i]);
    }
}

