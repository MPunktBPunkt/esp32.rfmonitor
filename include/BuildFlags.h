#pragma once

#ifndef FW_VERSION
#define FW_VERSION "1.0.0"
#endif

#ifndef DEVICE_NAME_DEFAULT
#define DEVICE_NAME_DEFAULT "RF Monitor"
#endif

#ifndef HUB_HOST_DEFAULT
#define HUB_HOST_DEFAULT "192.168.178.113"
#endif

#ifndef HUB_PORT_DEFAULT
#define HUB_PORT_DEFAULT 8093
#endif

#ifndef WIFI_AP_NAME
#define WIFI_AP_NAME "ESP-RFMonitor-Setup"
#endif

#ifndef WIFI_PORTAL_TIMEOUT_S
#define WIFI_PORTAL_TIMEOUT_S 180
#endif

#ifndef RESET_BUTTON_PIN
#define RESET_BUTTON_PIN 0
#endif

#ifndef RESET_HOLD_SEC
#define RESET_HOLD_SEC 3
#endif

#ifndef RFMON_HISTORY_SIZE
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define RFMON_HISTORY_SIZE 180
#else
#define RFMON_HISTORY_SIZE 72
#endif
#endif

#ifndef RFMON_EVENT_SIZE
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define RFMON_EVENT_SIZE 64
#else
#define RFMON_EVENT_SIZE 32
#endif
#endif

#ifndef RFMON_MAX_APS
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define RFMON_MAX_APS 40
#else
#define RFMON_MAX_APS 24
#endif
#endif

#ifndef RFMON_MAX_BLE
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define RFMON_MAX_BLE 40
#else
#define RFMON_MAX_BLE 24
#endif
#endif

#ifndef RFMON_ENABLE_BLE
#define RFMON_ENABLE_BLE 1
#endif

#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define RFMON_BOARD_ID "esp32-s3"
#define RFMON_BOARD_LABEL "ESP32-S3"
#define RFMON_HW_TYPE "esp32s3"
#else
#define RFMON_BOARD_ID "esp32-mini"
#define RFMON_BOARD_LABEL "ESP32 Mini D1"
#define RFMON_HW_TYPE "esp32"
#endif
