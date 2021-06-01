# ESP32-CAM-FreeRTOS
A combination of the excellent ESP32 MJPEG Multiclient Streaming Server by arkhipenko ([arkhipenko/esp32-cam-mjpeg-multiclient](https://github.com/arkhipenko/esp32-cam-mjpeg-multiclient)) and the default ESP32-CAM Web Server Example sans the Face Detection and including WebOTA ([scottchiefbaker/ESP-WebOTA](https://github.com/scottchiefbaker/ESP-WebOTA))

## Handlers

Handler | URL | NOte
------------ | ------------- | -------------
Stream | `/mjpeg/1`
Still | `/jpg`
Set a variable | `/set?var=<var>&val=<val>`
Get the status of all variables | `/get`
Activate WebOTA | `/activatewebota` | sets a flag that changes the FreeRTOS-delay to webota.delay(...)
Restart | `/restart`
WebOTA | `:8080/webota`

## Instructions

Rename `home_wifi_multi_template.h` to `home_wifi_multi.h` and change SSID and WiFi Password contained int that file.

## Board settings for Arduino IDE:

* Board: ESP32 Dev Module
* CPU Frequency: 240MHz
* Flash Frequency: 40MHz
* Flash Mode: QIO
* Flash Size: 4MB
* Partition Scheme: Minimal SPIFFS
* PSRAM: Enabled
