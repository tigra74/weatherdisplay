#ifndef COMMON_H_
#define COMMON_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "common.h"

namespace API
{

  String request_type_to_string(RequestType type) {
    return type == WEATHER ? "currentconditions/v1/" : "forecasts/v1/hourly/12hour/";
  }

  String convert_press_trend(String trend) {
    return trend == "F" ? "-" : "+";
  }

  String daylight_icon(bool daylight) {
    return daylight ? "d" : "n";
  }

  String convert_icon(int icon, bool daylight) {
    switch(icon) {
      case 1:
      case 2:
      case 30:
      case 31:
      case 33:
      case 34:
        return "01" + daylight_icon(daylight); // clear sky
      case 3:
      case 4:
      case 21:
      case 35:
        return "02" + daylight_icon(daylight); // few clouds 
      case 5:
      case 6:
      case 20:
      case 36:
        return "03" + daylight_icon(daylight); // scattered clouds 
      case 7:
      case 8:
      case 19:
      case 38:
      case 43:
        return "04" + daylight_icon(daylight); // broken clouds 
      case 12:
      case 13:
      case 14:
      case 39:
      case 40:
        return "09" + daylight_icon(daylight); // Shower rain
      case 18:
      case 26:
        return "10" + daylight_icon(daylight); // Rain
      case 15:
      case 16:
      case 17:
      case 41:
      case 42:
        return "11" + daylight_icon(daylight); // Thunderstorms
      case 22:
      case 23:
      case 24:
      case 25:
      case 29:
      case 44:
        return "13" + daylight_icon(daylight); // Snow
      case 11:
      case 37:
        return "50" + daylight_icon(daylight); // Fog / mist

    }
  }

  // Problems with stucturing JSON decodes, see here: https://arduinojson.org/assistant/
  bool decode_weather(WiFiClient &json, const RequestType type)
  {
    PRINTLN(F("INFO: DEcoding weather JSON"));
    // allocate the JsonDocument
    DynamicJsonDocument doc(35 * 1024);
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, json);
    // Test if parsing succeeds.
    if (error)
    {
      PRINT(F("deserializeJson() failed: "));
      PRINTLN(error.c_str());
      return false;
    }
    // convert it to a JsonObject 
    PRINTLN("INFO: Decoding " + request_type_to_string(type) + " data");
    if (type == WEATHER)
    {
      JsonArray root = doc.as<JsonArray>();
      // All PRINTLN statements are for diagnostic purposes and not required, remove if not needed
      PRINTLN(root);
      WxConditions[0].Icon = convert_icon(root[0]["WeatherIcon"].as<int>(), root[0]["IsDayTime"].as<bool>());
      PRINTLN("Icon: " + String(WxConditions[0].Icon));

      WxConditions[0].Temperature = root[0]["Temperature"]["Metric"]["Value"].as<float>();
      PRINTLN("Temp: " + String(WxConditions[0].Temperature));
      
      WxConditions[0].Pressure = root[0]["Pressure"]["Metric"]["Value"].as<float>();
      PRINTLN("Pres: " + String(WxConditions[0].Pressure));

      WxConditions[0].Trend = convert_press_trend(root[0]["PressureTendency"]["Code"].as<char *>());
      PRINTLN("Pres: " + String(WxConditions[0].Trend));
      
      WxConditions[0].Humidity = root[0]["RelativeHumidity"].as<float>();
      PRINTLN("Humi: " + String(WxConditions[0].Humidity));

      WxConditions[0].Windspeed = root[0]["Wind"]["Speed"]["Metric"]["Value"].as<float>();
      PRINTLN("WSpd: " + String(WxConditions[0].Windspeed));
      
      WxConditions[0].Winddir = root[0]["Wind"]["Direction"]["Degrees"].as<float>();
      PRINTLN("WDir: " + String(WxConditions[0].Winddir));
      
      WxConditions[0].Cloudcover = root[0]["CloudCover"].as<int>();
      PRINTLN("CCov: " + String(WxConditions[0].Cloudcover)); // in % of cloud cover
      
    } else if (type == FORECAST)
    {
      JsonArray list = doc.as<JsonArray>();
      PRINT(F("\nReceiving Forecast period - ")); //------------------------------------------------
      for (byte r = 0; r < MAX_READINGS; r++)
      {
        PRINTLN("\nPeriod-" + String(r) + "--------------");
        WxForecast[r].Dt = list[r]["EpochDateTime"].as<int>();
        PRINTLN("EpochDateTime: " + String(WxForecast[r].Dt));

        WxForecast[r].Period = list[r]["DateTime"].as<char *>();
        PRINTLN("Peri: " + String(WxForecast[r].Period));

        WxForecast[r].Temperature = list[r]["Temperature"]["Value"].as<float>();
        PRINTLN("Temp: " + String(WxForecast[r].Temperature));

        WxForecast[r].Humidity = list[r]["RelativeHumidity"].as<int>();
        PRINTLN("Humi: " + String(WxForecast[r].Humidity));

        WxForecast[r].Icon = convert_icon(list[r]["WeatherIcon"].as<int>(), list[r]["IsDaylight"].as<bool>());
        PRINTLN("Icon: " + String(WxForecast[r].Icon));
        
        WxForecast[r].Description = list[r]["IconPhrase"].as<char *>();
        PRINTLN("Desc: " + String(WxForecast[r].Description));
        
        WxForecast[r].Cloudcover = list[r]["CloudCover"].as<int>();
        PRINTLN("CCov: " + String(WxForecast[r].Cloudcover)); // in % of cloud cover
        
        WxForecast[r].Windspeed = list[r]["Wind"]["Speed"]["Value"].as<float>();
        PRINTLN("WSpd: " + String(WxForecast[r].Windspeed));
        
        WxForecast[r].Winddir = list[r]["Wind"]["Direction"]["Degrees"].as<int>();
        PRINTLN("WDir: " + String(WxForecast[r].Winddir));
        
        WxForecast[r].Rainfall = list[r]["Rain"]["Value"].as<float>();
        PRINTLN("Rain: " + String(WxForecast[r].Rainfall));
        
        WxForecast[r].Snowfall = list[r]["Snow"]["Value"].as<float>();
        PRINTLN("Snow: " + String(WxForecast[r].Snowfall));
      }
    }
    return true;
  }

  String build_location()
  {
    return Config::getLocationId();
  }

  String build_request_url(const RequestType requestType)
  {
    const String metric = (Config::isMetric() ? "true" : "false");
    return "/" + request_type_to_string(requestType) + build_location() + "?apikey=" + Config::getApiKey() + "&metric=" + metric + "&lang=" + Config::getLanguage();
  }

  bool obtain_wx_data(WiFiClient &client, const RequestType requestType)
  {
    client.stop(); // close connection before sending a new request
    HTTPClient http;
    String uri = build_request_url(requestType);
    PRINT("INFO: Request URI: ");
    PRINTLN(uri);
    //http.begin(uri,test_root_ca); //HTTPS example connection
    http.begin(client, Config::getServer(), 80, uri);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      if (!decode_weather(http.getStream(), requestType))
        return false;
      client.stop();
      http.end();
      return true;
    }
    else
    {
      PRINT("ERROR: Connection failed, error: %s");
      PRINTLN(http.errorToString(httpCode).c_str());
      client.stop();
      http.end();
      return false;
    }
    http.end();
    return true;
  }
}
#endif /* ifndef COMMON_H_ */
