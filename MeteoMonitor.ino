#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>        // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#include "fMon2038.h"
#include "kirill16.h"

#include <ArduinoJson.h>
#include "unixTimeConv.h"

#define TIME_ZONE_UTC 5

const char *Password = "12341234";
const char *Ssid = "Router81";
const long Time_request = 10 * 60 * 1000;

enum e_sensor
{
  SEN1,
  SEN2,
  SEN3,
  SEN_MAX
};

struct geo_sensor_s
{
  enum e_sensor sensor;
  String name;
  String sensor_id;
  int screen_duration;
  String status_line;
  double value;
  double value_old;
  String date_time;
  String date_time_old;
  String time;
  int trend;
  String sym_trend;
  String symbol;
};

SSD1306Wire display(0x3c, D2, D1);

struct geo_sensor_s sensors[SEN_MAX] = {
    {
        .sensor = SEN1,
        .name = "_Wajtanka",
        .sensor_id = "5598",
        .screen_duration = 8000,
        .status_line = "",
        .value = 0.0,
        .value_old = 0.0,
        .date_time = "",
        .date_time_old = "",
        .time = "",
        .trend = 0,
        .sym_trend = "",
        .symbol = "/",
    },
    {
        .sensor = SEN2,
        .name = "_Wajtanka",
        .sensor_id = "14794",
        .screen_duration = 3000,
        .status_line = "",
        .value = 0.0,
        .value_old = 0.0,
        .date_time = "",
        .date_time_old = "",
        .time = "",
        .trend = 0,
        .sym_trend = "",
        .symbol = "?",
    },
    {
        .sensor = SEN3,
        .name = "_N.Tagil",
        .sensor_id = "18137",
        .screen_duration = 3000,
        .status_line = "",
        .value = 0.0,
        .value_old = 0.0,
        .date_time = "",
        .date_time_old = "",
        .time = "",
        .trend = 0,
        .sym_trend = "",
        .symbol = "/",
    },
};

String status_line_wifi = "";
size_t show_sensor = SEN1;
long time_request_curr = millis();
long time_show_sensor = millis();
long time_duration = sensors[show_sensor].screen_duration;

void get_data_sensor(struct geo_sensor_s *sensor)
{

  sensor->status_line = "";
  HTTPClient http;

  /* Пример запроса JSON(POST):
  {"cmd":"sensorsValues","sensors":[1],"uuid":"UUID","api_key":"API_KEY"} */
  String req1 = "{\"cmd\":\"sensorsValues\",\"sensors\":[";
  String req2 = "],\"uuid\":\"66aeb0aaa005c2bb6e764b95790f7223\",\"api_key\":\"h6ZfZ2S2rQE7Y\"}";
  String request = req1 + sensor->sensor_id + req2;
  http.begin("http://narodmon.ru/api");
  int http_response_code = http.POST(request);
  if (http_response_code <= 0)
  {
    sensor->status_line = "Error on POST request";
    http.end();
    return;
  }

  String json = http.getString();

  /*Close connection*/
  http.end();

  /* Deserialize the JSON document */
  StaticJsonDocument<600> doc;
  DeserializationError error = deserializeJson(doc, json);

  /* Test if parsing succeeds. */
  if (error)
  {
    sensor->status_line = "Json - failed ";
    return;
  }

  sensor->date_time = unixTimeConv(doc["sensors"][0]["time"], TIME_ZONE_UTC);

  sensor->value = doc["sensors"][0]["value"];

  if (sensor->date_time_old == sensor->date_time)
  {
    sensor->status_line = "Outdated data";
    return;
  }

  sensor->date_time_old = sensor->date_time;
  sensor->time = (sensor->date_time).substring(0, 5);

  if (sensor->value > sensor->value_old)
  {
    if (sensor->trend < 0)
    {
      sensor->trend = 0;
    }
    else
    {
      if (0 <= sensor->trend && sensor->trend < 5)
      {
        ++(sensor->trend);
      }
    }
  }

  if (sensor->value < sensor->value_old)
  {
    if (sensor->trend > 0)
    {
      sensor->trend = 0;
    }
    else
    {
      if (-5 < sensor->trend && sensor->trend <= 0)
      {
        --(sensor->trend);
      }
    }
  }

  sensor->value_old = sensor->value;

  if (sensor->trend == -5)
  {
    sensor->sym_trend = ">"; /* min min */
  }
  else if (-5 < sensor->trend && sensor->trend <= -2)
  {
    sensor->sym_trend = "="; /* min */
  }
  else if (2 <= sensor->trend && sensor->trend < 5)
  {
    sensor->sym_trend = ";"; /* max */
  }
  else if (sensor->trend == 5)
  {
    sensor->sym_trend = ":"; /* max max */
  }
  else
  {
    sensor->sym_trend = ""; /* --- */
  }

  sensor->status_line = "S" + sensor->sensor_id;
}

void get_meteo_data(void)
{
  status_line_wifi = "";
  /* Connecting WiFi */;
  WiFi.begin(Ssid, Password);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    i++;
    if (i > 12)
      break;
  }

  /* Check WiFi connection status */
  if (WiFi.status() != WL_CONNECTED)
  {
    /* Error in WiFi connection */
    status_line_wifi = "Error in WiFi connection";
    WiFi.disconnect();
    return;
  }

  for (size_t s = SEN1; s < SEN_MAX; s++)
  {
    get_data_sensor(&sensors[s]);
  }

  WiFi.disconnect();
}

void screen_fill(struct geo_sensor_s *sensor)
{
  /* name */
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(kirill16);
  display.drawString(0, 0, sensor->name);

  /* time */
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(128, 0, sensor->time);

  /* data */
  String value = String(sensor->value, 1);
  String data = value + sensor->symbol;

  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(fMon2038);
  display.drawString(128, 18, data);
  /*
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(fMon2038);
    display.drawString(0, 18, sensor->sym_trend);
  */
}

void display_show(void)
{
  display.clear();
  screen_fill(&sensors[show_sensor]);

  /* status line */
  String status_line = status_line_wifi + sensors[show_sensor].status_line;
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 54, status_line);

  /*
   display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(128, 48, "t"+String(sensors[show_sensor].trend));
  */

  /* write the buffer to the display */
  display.display();
}

void setup()
{
  /*
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  */

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(kirill16);
  display.drawString(10, 8, "Zapusk");
  display.drawString(10, 24, "poluqenie");
  display.drawString(10, 40, "dann{h...");
  display.display();

  get_meteo_data();

  display_show();
}

void loop()
{

  if ((millis() - time_show_sensor) > sensors[show_sensor].screen_duration)
  {
    ++show_sensor;
    if (show_sensor >= SEN_MAX)
    {
      show_sensor = SEN1;
    }
    display_show();
    time_show_sensor = millis();
  }

  if ((millis() - time_request_curr) > Time_request)
  {
    get_meteo_data();
    time_request_curr = millis();
  }

  delay(10);
}
