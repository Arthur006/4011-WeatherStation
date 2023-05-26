# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(OSLIB_PATH ../../oslib)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(scu)

include_directories(
                        ${OSLIB_PATH}/weather_station/thingy52_sensors/
                        ${OSLIB_PATH}/bt_station/
                        )

target_sources(app PRIVATE
                    src/main.c
                    ${OSLIB_PATH}/weather_station/thingy52_sensors/thingy52_sensors.c
                    ${OSLIB_PATH}/bt_station/bt_station.c
                    )
