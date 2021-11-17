#ifndef COMMON_H_
#define COMMON_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "common.h"

namespace API
{
  
  String request_type_to_string(const RequestType type)
  {
    return type == WEATHER ? "weather" : "forecast";
  }

  void convert_to_imperial()
  {
    WxConditions[0].Pressure = hPa_to_inHg(WxConditions[0].Pressure);
    WxForecast[1].Rainfall = mm_to_inches(WxForecast[1].Rainfall);
    WxForecast[1].Snowfall = mm_to_inches(WxForecast[1].Snowfall);
  }

  // Problems with stucturing JSON decodes, see here: https://arduinojson.org/assistant/
  bool decode_weather(WiFiClient &json, const RequestType type)
  {
    // allocate the JsonDocument
    DynamicJsonDocument doc(35 * 1024);
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, json);
    // Test if parsing succeeds.
    if (error)
    {
      PRINT(F("ERROR: JSON deserialization failed: "));
      PRINTLN(error.c_str());
      return false;
    }
    // convert it to a JsonObject
    JsonObject root = doc.as<JsonObject>();
    PRINTLN("INFO: Decoded [" + request_type_to_string(type) + "] data:");
    if (type == WEATHER)
    {
      // All PRINTLN statements are for diagnostic purposes and not required, remove if not needed
      WxConditions[0].Icon = root["weather"][0]["icon"].as<char *>();
      PRINTLN("Icon: " + String(WxConditions[0].Icon));
      WxConditions[0].Temperature = root["main"]["temp"].as<float>();
      PRINTLN("Temp: " + String(WxConditions[0].Temperature));
      WxConditions[0].Pressure = root["main"]["pressure"].as<float>();
      PRINTLN("Pres: " + String(WxConditions[0].Pressure));
      WxConditions[0].Humidity = root["main"]["humidity"].as<float>();
      PRINTLN("Humi: " + String(WxConditions[0].Humidity));
      WxConditions[0].Low = root["main"]["temp_min"].as<float>();
      PRINTLN("TLow: " + String(WxConditions[0].Low));
      WxConditions[0].High = root["main"]["temp_max"].as<float>();
      PRINTLN("THig: " + String(WxConditions[0].High));
      WxConditions[0].Windspeed = root["wind"]["speed"].as<float>();
      PRINTLN("WSpd: " + String(WxConditions[0].Windspeed));
      WxConditions[0].Winddir = root["wind"]["deg"].as<float>();
      PRINTLN("WDir: " + String(WxConditions[0].Winddir));
      WxConditions[0].Cloudcover = root["clouds"]["all"].as<int>();
      PRINTLN("CCov: " + String(WxConditions[0].Cloudcover)); // in % of cloud cover
      WxConditions[0].Rainfall = root["rain"]["1h"].as<float>();
      PRINTLN("Rain: " + String(WxConditions[0].Rainfall));
      WxConditions[0].Snowfall = root["snow"]["1h"].as<float>();
      PRINTLN("Snow: " + String(WxConditions[0].Snowfall));
    }
    if (type == FORECAST)
    {
      //PRINTLN(json);
      PRINT(F("INFO: Receiving Forecast period - ")); //------------------------------------------------
      JsonArray list = root["list"];
      for (byte r = 0; r < MAX_READINGS; r++)
      {
        PRINTLN("\nPeriod-" + String(r));
        WxForecast[r].Dt = list[r]["dt"].as<int>();
        PRINTLN("DTim: " + String(WxForecast[r].Dt));
        WxForecast[r].Temperature = list[r]["main"]["temp"].as<float>();
        PRINTLN("Temp: " + String(WxForecast[r].Temperature));
        WxForecast[r].Low = list[r]["main"]["temp_min"].as<float>();
        PRINTLN("TLow: " + String(WxForecast[r].Low));
        WxForecast[r].High = list[r]["main"]["temp_max"].as<float>();
        PRINTLN("THig: " + String(WxForecast[r].High));
        WxForecast[r].Pressure = list[r]["main"]["pressure"].as<float>();
        PRINTLN("Pres: " + String(WxForecast[r].Pressure));
        WxForecast[r].Humidity = list[r]["main"]["humidity"].as<float>();
        PRINTLN("Humi: " + String(WxForecast[r].Humidity));
        WxForecast[r].Icon = list[r]["weather"][0]["icon"].as<char *>();
        PRINTLN("Icon: " + String(WxForecast[r].Icon));
        WxForecast[r].Description = list[r]["weather"][0]["description"].as<char *>();
        PRINTLN("Desc: " + String(WxForecast[r].Description));
        WxForecast[r].Cloudcover = list[r]["clouds"]["all"].as<int>();
        PRINTLN("CCov: " + String(WxForecast[r].Cloudcover)); // in % of cloud cover
        WxForecast[r].Windspeed = list[r]["wind"]["speed"].as<float>();
        PRINTLN("WSpd: " + String(WxForecast[r].Windspeed));
        WxForecast[r].Winddir = list[r]["wind"]["deg"].as<float>();
        PRINTLN("WDir: " + String(WxForecast[r].Winddir));
        WxForecast[r].Rainfall = list[r]["rain"]["3h"].as<float>();
        PRINTLN("Rain: " + String(WxForecast[r].Rainfall));
        WxForecast[r].Snowfall = list[r]["snow"]["3h"].as<float>();
        PRINTLN("Snow: " + String(WxForecast[r].Snowfall));
        WxForecast[r].Pop = list[r]["pop"].as<float>();
        PRINTLN("Pop:  " + String(WxForecast[r].Pop));
        WxForecast[r].Period = list[r]["dt_txt"].as<char *>();
        PRINTLN("Peri: " + String(WxForecast[r].Period));
      }
      //------------------------------------------
      float pressure_trend = WxForecast[0].Pressure - WxForecast[2].Pressure; // Measure pressure slope between ~now and later
      pressure_trend = ((int)(pressure_trend * 10)) / 10.0;                   // Remove any small variations less than 0.1
      WxConditions[0].Trend = "0";
      if (pressure_trend > 0)
        WxConditions[0].Trend = "+";
      if (pressure_trend < 0)
        WxConditions[0].Trend = "-";
      if (pressure_trend == 0)
        WxConditions[0].Trend = "0";

      if (!Config::isMetric())
        convert_to_imperial();
    }
    return true;
  }

  String build_location()
  {
    return Config::getLatitude() == "null" || Config::getLatitude() == "null"
               ? "?q=" + Config::getCity() + "," + Config::getCountry()
               : "?lat=" + Config::getLatitude() + "&lon=" + Config::getLongitude();
  }

  String build_request_url(const RequestType requestType)
  {
    const String units = (Config::isMetric() ? "metric" : "imperial");
    return "/data/2.5/" + request_type_to_string(requestType) + build_location() + "&APPID=" + Config::getApiKey() + "&mode=json&units=" + units + "&lang=" + Config::getLanguage();
  }

  bool obtain_wx_data(WiFiClient &client, const RequestType requestType)
  {
    client.stop(); // close connection before sending a new request
    HTTPClient http;
    String uri = build_request_url(requestType);
    PRINT("INFO: Request URI: ");
    PRINTLN(uri);
    if (requestType == FORECAST)
    {
      uri += "&cnt=" + String(MAX_READINGS);
    }
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
