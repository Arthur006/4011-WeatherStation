# SPDX-License-Identifier: Apache-2.0

set(CONF_FILE usb.conf shell.conf bt.conf)
set(DTC_OVERLAY_FILE dtc_shell.overlay)
set(OSLIB_PATH ../../oslib)

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(base)

target_sources(app PRIVATE
                    src/main.c
                    ${OSLIB_PATH}/bt_base_multi/bt_base_multi.c
                    )

target_include_directories(app PRIVATE ${OSLIB_PATH}/bt_base_multi)