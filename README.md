# ESP32-CAM-FreeRTOS
A combination of the excellent ESP32 MJPEG Multiclient Streaming Server by arkhipenko ([arkhipenko/esp32-cam-mjpeg-multiclient](https://github.com/arkhipenko/esp32-cam-mjpeg-multiclient)), the default ESP32-CAM Web Server Example sans the Face Detection, and including WebOTA ([scottchiefbaker/ESP-WebOTA](https://github.com/scottchiefbaker/ESP-WebOTA)). Settings are stored in NVS via the Preferences library.

## Handlers

Handler | URL | NOte
------------ | ------------- | -------------
Stream | `/mjpeg/1`
Capture | `/jpg`
Set a variable | `/set?var=<var>&val=<val>`
Get the values of all variables | `/get`
Activate WebOTA | `/activatewebota` | sets a flag that changes the FreeRTOS-delay to webota.delay(...)
Restart | `/restart`
Factory defaults | `/reset`
WebOTA | `:8080/webota`

## Instructions

Rename `home_wifi_multi_template.h` to `home_wifi_multi.h` and add your SSID and WiFi Password.

## Board settings for Arduino IDE:

* Board: ESP32 Dev Module
* CPU Frequency: 240MHz
* Flash Frequency: 40MHz
* Flash Mode: QIO
* Flash Size: 4MB
* Partition Scheme: Minimal SPIFFS
* PSRAM: Enabled
