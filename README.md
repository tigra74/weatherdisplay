# Weather Display
An application for LILYGO TTGO T5 e-ink board to display weather for specified city from openweather.

![example](https://github.com/gagauz/weatherdisplay/blob/main/images/img1.jpg?raw=true)

The weather data is queried openweather api (you should sign up here and obtain api key, it's free).
There is no need to modify code to provide user specific settings (e.g. WiFi ssid, password, location etc). You just need to create simple configuration file wifi_config.txt (see example in /config_sample) and put it into the root of Micro SD card. During the first launch application will read settings and store it into the EEROM, so the SD card can be removed for the next launches.

The files for 3d printed case can be downloaded [here](https://www.thingiverse.com/thing:4670205).

**Check with manufacturer for valid display driver version**
Commonly used drivers are GxGDEH0213B72, GxGDEH0213B73 and GxDEPG0213BN
