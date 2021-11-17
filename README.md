# Weather Display
An application for LILYGO TTGO T5 e-ink board to display weather for specified city from openweather.
[alt text](https://github.com/gagauz/weatherdisplay/blob/main/images/img1.jpg?raw=true)
This application will query openweather api for forecast and display it on the e-ink display.
The user specific configuration (e.g. WiFi ssid, password, location etc) are not stored in the codebase. You just need to create simple configuration file wifi_config.txt (see example) and put it into the root of Micro SD card. Before first launch card is inserted into the module. 
Application will read configuration and store it into EEROM, so the SD card can be removed for the next launches.

The files for 3d printed case can be downloaded [here](https://www.thingiverse.com/thing:4670205).

**Check with manufacturer for valid display driver version**
Commonly used drivers are GxGDEH0213B72, GxGDEH0213B73 and GxDEPG0213BN
