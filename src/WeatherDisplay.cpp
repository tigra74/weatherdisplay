#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <SPIFFS.h>
#include <FS.h>
#include <Wire.h>

#define FILESYSTEM SPIFFS

#include "common.h"

#define DEBUG_ON 1

#ifdef DEBUG_ON
#define PRINTLN(...) Serial.println(__VA_ARGS__);
#define PRINT(...) Serial.print(__VA_ARGS__);
#else
#define PRINTLN(...) ;
#define PRINT(...) ;
#endif

#include "time.h"
#include "config.h"
#include "api/openweathermap/api_functions.h"
// #include "api/accuweather/api_functions.h"
#include "draw_functions.h"
#include "system.h"

void begin_sleep();
bool update_local_time();
bool setup_time();

void setup()
{
  StartTime = millis();
  #ifdef DEBUG_ON
  Serial.begin(115200);
  #endif
  PRINTLN("INFO: Serial is started");
  setup_pins();
  
  led_on();
  enable_flash_memory();
  display_init(); // Give screen time to display_init by getting weather data!
  Config::read();
  if (start_wifi() == WL_CONNECTED)
  {
    if (setup_time() == true)
    {
      //if ((CurrentHour >= WAKEUP_TIME_HR && CurrentHour <= SLEEP_TIME_HR)) {
      
      byte Attempts = 1;
      bool RxWeather = false, RxForecast = false;
      WiFiClient client; // wifi client object
      while ((!RxWeather || !RxForecast) && Attempts <= 5)
      { // Try up-to 2 time for Weather and Forecast data
        if (!RxWeather)
          RxWeather = API::obtain_wx_data(client, WEATHER);
        if (!RxForecast)
          RxForecast = API::obtain_wx_data(client, FORECAST);
        Attempts++;
      }
      if (RxWeather && RxForecast)
      {              // Only if received both Weather or Forecast proceed
        stop_wifi(); // Reduces power consumption
        update_local_time();
        draw_weather();
      } else {
        draw_error_msg("Failed to obtain weather data");  
      }
    } else {
      draw_error_msg("Failed to setup time");
    }
    //}
  } else {
    draw_error_msg("Can't connect to Wi-Fi");
  }
  display_update();
  led_off();
  disable_flash_memory();

  begin_sleep();
}

void loop()
{ // this will never run!
}