/*
  //########################  Webpage Weather Display  #############################
  For an ESP8266 or ESP32
  
  Receives and displays the weather forecast from the Open Weather Map servers
  and a Webserver to display a web page with the weather data, using colour to denote levels of temperatures and humidity

  This software, the ideas and concepts is Copyright (c) David Bird 2019 and beyond.
  All rights to this software are reserved.
  It is prohibited to redistribute or reproduce of any part or all of the software contents in any form other than the following:
  1. You may print or download to a local hard disk extracts for your personal and non-commercial use only.
  2. You may copy the content to individual third parties for their personal use, but only if you acknowledge the author David Bird as the source of the material.
  3. You may not, except with my express written permission, distribute or commercially exploit the content.
  4. You may not transmit it or store it in any other website or other form of electronic retrieval system for commercial purposes.
  5. You MUST include all of this copyright and permission notice ('as annotated') and this shall be included in all copies or substantial portions of the software and where the software use is visible to an end-user.

  THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
  FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//################# LIBRARIES ################
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266HTTPClient.h>
#else
  #include <WiFi.h>
  #include <WebServer.h>
  #include <HTTPClient.h>
#endif
#include <ArduinoJson.h>     // https://github.com/bblanchon/ArduinoJson
#include "owm_credentials.h"

//################ VARIABLES ################
const unsigned long UpdateInterval = 5 * 60 * 1000L; // Delay between updates, in milliseconds, WU allows 500 requests per-day maximum and no more than 2.8 per-minute!
String webpage, Wx_Description, CompassPointer;
bool   RxWeather = false, RxForecast = false;
long   StartTime = 0;

//################ PROGRAM VARIABLES and OBJECTS ################

typedef struct { // For current Day and Day 1, 2, 3, etc
  float    lat;
  float    lon;
  int      Dt;
  String   Period;
  float    Temperature;
  float    Humidity;
  String   Icon;
  float    High;
  float    Low;
  float    Rainfall;
  float    Snowfall;
  float    Pressure;
  int      Cloudcover;
  int      Visibility;
  String   Trend;
  float    Winddir;
  float    Windspeed;
  String   Main0;
  String   Forecast0;
  String   Forecast1;
  String   Forecast2;
  String   Description;
  String   Time;
  int      Dtime;
  int      Timezone;
  int      Sunrise;
  int      Sunset;
  String   Country;
} Forecast_record_type;

#define MaxReadings 9 // 96 Maximum, but practical limit is 12

Forecast_record_type  WxConditions[1];
Forecast_record_type  WxForecast[MaxReadings];

WiFiClient wxclient;

String version = "v1";     // Version of this program
#ifdef ESP8266
  ESP8266WebServer server(80); // Start server on port 80 (default for a web-browser, change to your requirements, e.g. 8080 if your Router uses port 80
#else
  WebServer server(80); // Start server on port 80 (default for a web-browser, change to your requirements, e.g. 8080 if your Router uses port 80
#endif
// To access server from the outside of a WiFi network e.g. ESP8266WebServer server(8080) add a rule on your Router to forwards a connection request on port 8080
// to http://your_network_ip_address:8080 to port 8080 and view your ESP server from anywhere.
// Example http://yourhome.ip:8080 will be directed to http://192.168.0.40:8080 or whatever IP address your router gives to this server
//#########################################################################################
void setup() {
  Serial.begin(115200); // initialize serial communications
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("WiFi connected.."));
  //----------------------------------------------------------------------
  server.begin();
  Serial.println(F("Webserver started..."));                                            // Start the webserver
  Serial.println("Use this URL to connect: http://" + WiFi.localIP().toString() + "/"); // Print the IP address
  server.on("/", homepage);
  GetWxData();
  StartTime = millis();
}
//#########################################################################################
void loop() {
  server.handleClient();
  if ((millis() - StartTime) > UpdateInterval) {
    Serial.println("Updating Wx Data");
    StartTime = millis();
    GetWxData();
  }
}
//#############################################################################################
void GetWxData() {
  WiFiClient client; // wifi client object
  byte Attempts = 1;
  RxWeather  = false;
  RxForecast = false;
  while ((RxWeather == false || RxForecast == false) && Attempts <= 2) { // Try up-to twice for Weather and Forecast data
    if (RxWeather  == false) RxWeather  = obtain_wx_data(client, "weather");
    if (RxForecast == false) RxForecast = obtain_wx_data(client, "forecast");
    Attempts++;
  }
  if (RxWeather || RxForecast) {
    Wx_Description = WxConditions[0].Main0;
    if (WxConditions[0].Forecast1 != "") Wx_Description += ", " + WxConditions[0].Forecast1;
    if (WxConditions[0].Forecast2 != "") Wx_Description += ", "  + WxConditions[0].Forecast2;
  }
}
//#########################################################################################
void homepage() {
  int i = 0;
  append_page_header(true);
  webpage += F("<table align='center' style='width: 100%; height: 200px'>"); // T1 open
  webpage += F("<tr>");
  webpage += F("  <td>");
  webpage += F("   <table style='width:100%'>"); // T2 open
  webpage += F("     <tr>");
  webpage += F("       <td style='border:0px solid #C0C0C0;align:center;'>");
  webpage += "          <img src='http://openweathermap.org/img/w/" + WxConditions[0].Icon + ".png' alt='Weather Symbol' style='width:200px;height:200px;'>";
  webpage += F("       </td>");
  webpage += F("     </tr>");
  webpage += F("   </table>"); // T2 closed
  webpage += F("  </td>");
  webpage += F("  <td>");
  webpage += F("  <table style='width: 100%'>");// T3 open
  webpage += F("    <tr style='font-size:72px'>");
  webpage += "      <th colspan = '3' class='" + TempToClass(WxConditions[0].Temperature) + "'>" + String(WxConditions[0].Temperature, 1) + "&deg;</th>";
  webpage += F("    </tr>");
  webpage += F("    <tr>");
  webpage += F("      <td style='font-size:14px'>Hi</td><td style='font-size:14px'>Lo</td><td style='font-size:14px'>%</td>");
  webpage += F("    </tr>");
  webpage += F("    <tr>");
  webpage += "      <td class='" + TempToClass(WxConditions[0].High) + "'>" + String(WxConditions[0].High, 0) + "&deg;</td>";
  webpage += "      <td class='" + TempToClass(WxConditions[0].Low) + "'>" + String(WxConditions[0].Low, 0)  + "&deg;</td>";
  webpage += "      <td class='" + HumiToClass(WxConditions[0].Humidity) + "'>" + String(WxConditions[0].Humidity, 0)  + "%</td>";
  webpage += F("    </tr>");
  webpage += F("  </table>"); // T3 closed
  webpage += F("  </td>");
  webpage += F("  <td>");
  webpage += F("  <table style='width: 100%'>"); // T4 open
  webpage += F("    <tr>");
  webpage += F("      <td>");
  webpage += F("       <table>");  // T5 open
  webpage += F("         <tr>");
  webpage += "           <td colspan='2' class='style1'>" + WindDegToDirection(WxConditions[0].Winddir)  + " / " + String(WxConditions[0].Winddir, 0) + "&deg;";
  webpage +=             CompassPointer + "</td>";
  webpage += F("         </tr>");
  webpage += F("         <tr>");
  webpage += "           <td class='style2'>" + String(WxConditions[0].Windspeed, 1) + String(Units == "M" ? " KPH" : " MPH") + "</td>";
  webpage += F("         </tr>");
  webpage += F("       </table>");  // T5 closed
  webpage += F("      </td>");
  webpage += F("    </tr>");
  webpage += F("  </table>");  // T4 closed
  webpage += F("  </td>");
  webpage += F("  <td>");
  webpage += F("   <table class = 'style3'>");  // T6 open
  webpage += F("     <tr>");
  webpage += F("       <td>Pressure </td>");
  webpage += "       <td>" + String(WxConditions[0].Pressure * (Units == "M" ? 1 : 0.02953), (Units == "M" ? 0 : 2)) + String(Units == "M" ? " kpa " : " inches ") ;
  if (WxConditions[0].Trend == "0") webpage += "&#x27A1;"; // Steady
  if (WxConditions[0].Trend == "+") webpage += "&#x2197;"; // Rising
  if (WxConditions[0].Trend == "-") webpage += "&#x2198;"; // Falling
  webpage += F("       </td>");
  webpage += F("     </tr>");
  // Don't display windchill unless wind speed is greater than 3 MPH and temperature is less than 14°C or equivalent
  if ( (Units == "M" && WxConditions[0].Temperature < 14 && WxConditions[0].Windspeed > 3) ||
       (Units == "I" && ((WxConditions[0].Temperature - 32) / 1.8) < 14 && WxConditions[0].Windspeed > 3) ) {
    String Windchill = String(WindChill(WxConditions[0].Temperature, WxConditions[0].Windspeed));
    webpage += F("    <tr>");
    webpage += "      <td>Windchill </td><td>" + Windchill + "&deg;" + String(Units == "M" ? "C" : "F") + "</td>";
    webpage += F("    </tr>");
  }
  String Dewpoint;
  if (Units == "M") {
    Dewpoint = String(DewPoint(WxConditions[0].Temperature, WxConditions[0].Humidity));
  }
  else {
    Dewpoint = String(DewPoint((WxConditions[0].Temperature - 32) / 1.8, WxConditions[0].Humidity) * 9.0 / 5.0 + 32, 1);
  }
  webpage += F("    <tr>");
  webpage += "      <td>Dew Point </td><td>" + Dewpoint + "&deg;" + String(Units == "M" ? "C" : "F") + "</td>";
  webpage += F("    </tr>");
  // Don't display heatindex unless temperature is greater than 20°C or equivalent
  if ( (Units == "M" && WxConditions[0].Temperature >= 20) ||
       (Units == "I" && WxConditions[0].Temperature >= 68) ) {
    String Heatindex = String(HeatIndex(WxConditions[0].Temperature, WxConditions[0].Humidity));
    webpage += F("    <tr>");
    webpage += "        <td>Heat Index </td><td>" + Heatindex + "&deg;" + String(Units == "M" ? "C" : "F") + "</td>";
    webpage += F("    </tr>");
  }
  if (WxConditions[0].Rainfall > 0.005) {
    webpage += F("    <tr>");
    webpage += "        <td>Rainfall </td><td>" + String(WxConditions[0].Rainfall, 2) + String(Units == "M" ? " mm" : " inches") + "</td>";
    webpage += F("    </tr>");
  }
  if (WxConditions[0].Snowfall > 0.005) {
    webpage += F("    <tr>");
    webpage += "        <td>Snowfall </td><td>" + String(WxConditions[0].Snowfall, 2) + String(Units == "M" ? " mm" : " inches") + "</td>";
    webpage += F("    </tr>");
  }
  webpage += F("    <tr>");
  webpage += "      <td>Cloudcover </td><td>" + String(WxConditions[0].Cloudcover) + " %</td>";
  webpage += F("    </tr>");
  webpage += F("    <tr>");
  webpage += "      <td>Visibility </td><td>" + String(WxConditions[0].Visibility * (Units == "M" ? 1 : 1.0936133), 0) + String(Units == "M" ? " Metres" : " Yards") + "</td>";
  webpage += F("    </tr>");
  webpage += F("   </table>");  // T6 closed
  webpage += F("  </td>");
  webpage += F(" </tr>");
  //########################################################
  webpage += F(" <table>"); // T7 open
  //########################################################
  webpage += F("  <tr>");
  i = 0; do {
    webpage += "<td colspan='3'>" + DayOfWeek(WxForecast[i].Dt) + "</td>";
    i++;
  } while (i < MaxReadings);
  webpage += F("  </tr>");
  //########################################################
  webpage += F("  <tr>");
  i = 0; do {
    webpage += "<td colspan='3'>" + HourMinute(WxForecast[i].Dt) + "</td>";
    i++;
  } while (i < MaxReadings);
  webpage += F("  </tr>");
  //########################################################
  webpage += F("  <tr>");
  i = 0; do {
    webpage += "<td colspan='3'><img src='http://openweathermap.org/img/w/" + WxForecast[i].Icon + ".png' alt='Forecast Symbol' style='width:75px;height:75px;'></td>"; i++;
  } while (i < MaxReadings);
  webpage += F("  </tr>");
  //########################################################
  webpage += F("  <tr>");
  i = 0; do {
    webpage += "<th colspan = '3' class='" + TempToClass(WxForecast[i].Temperature) + "'>" + String(WxForecast[i].Temperature, 0) + "&deg;</th>"; i++;
  } while (i < MaxReadings);
  webpage += F("  </tr>");
  //########################################################
  webpage += F("  <tr>");
  i = 0; do {
    webpage += "<td style='font-size:14px'>Hi</td><td style='font-size:14px'>Lo</td><td style='font-size:14px'>%</td>"; i++;
  } while (i < MaxReadings);
  webpage += F("  </tr>");
  //########################################################
  webpage += F("  <tr>");
  i = 0; do {
    webpage += "    <td class='" + TempToClass(WxForecast[i].Temperature) + "'>" + String(WxForecast[i].High, 0) + "&deg;</td>";
    webpage += "    <td class='" + TempToClass(WxForecast[i].Temperature) + "'>" + String(WxForecast[i].Low, 0) + "&deg;</td>";
    webpage += "    <td class='" + HumiToClass(WxForecast[i].Humidity)    + "'>" + String(WxForecast[i].Humidity, 0) + "%</td>";
    i++;
  } while (i < MaxReadings);
  webpage += F("  </tr>");
  //########################################################
  webpage += F("</table>");     // T7 closed
  webpage += F("</table><br>"); // T1 closed
  append_page_footer();
  server.send(200, "text/html", webpage);
  webpage = "";
}
//#########################################################################################
void append_page_header(bool Header) {
  webpage  = F("<!DOCTYPE html><html><head>");
  webpage += F("<meta http-equiv='refresh' content='60'>"); // 30-sec refresh time, test needed to prevent auto updates repeating some commands
  webpage += F("<title>Weather Webserver</title><style>");
  webpage += "body {width:" + String(PageWidth) + "px;margin:0 auto;font-family:arial;font-size:14px;text-align:center;color:blue;background-color:#b4b4ff;}";
  webpage += "</style></head><body><h1>ESP Weather Server " + version + "</h1>";
  if (Header) {
    webpage += "<h3>" + City + "</h3>";
    webpage += "<h3>" + ConvertUnixTime(WxConditions[0].Dtime + WxConditions[0].Timezone) + "</h3>";
    webpage += "<h3>*** " + Wx_Description + " ***</h3>";
  }
}
//#########################################################################################
void append_page_footer() { // Saves repeating many lines of code for HTML page footers
  webpage += F("<style>ul{list-style-type:none;margin:0;padding:0;overflow:hidden;background-color:#B4DAFF;font-size:16px;}"); //
  webpage += F("li{float:left;border-right:1px solid #bbb;}last-child {border-right: none;}");
  webpage += F("li a{display: block;padding:3px 10px;text-decoration:none;}");
  webpage += F("li a:hover{background-color:#F8F8F8;}");
  webpage += F("section {font-size:14px;}");
  webpage += F("title {background-color:#E3D1E2}");
  webpage += F("p {background-color:#E3D1E2;font-size:14px;}");
  webpage += F("h1{background-color:#d8d8d8;}");
  webpage += F("h3{color:#fdfd96;font-size:24px; line-height:75%;}");
  webpage += F("table {font-family:arial,sans-serif;font-size:16px;border-collapse:collapse;table-layout:auto;width:100%;text-align:center;}");
  webpage += F("img.imgdisplay {display:block;margin-left:auto;margin-right:auto;}");
  webpage += F(".style1 {text-align:center;font-size:50px; background-color:#D8BFD8;}");
  webpage += F(".style2 {text-align:center;font-size:36px; background-color:#ADD8E6;}");
  webpage += F(".style3 {text-align:left;font-size:14px; background-color:#B0C4DE;width:100%;}");
  webpage += ".tempH  {text-align:center;font-size:18px;font-size:inherit;" + String(Mode == "B" ? "background-color:#FF2400" : "color:#FF0000") + ";}";
  webpage += ".tempM  {text-align:center;font-size:18px;font-size:inherit;" + String(Mode == "B" ? "background-color:#FFA500" : "color:#FFC87C") + ";}";
  webpage += ".tempL  {text-align:center;font-size:18px;font-size:inherit;" + String(Mode == "B" ? "background-color:#87CEEB" : "color:#C3E3FF") + ";}";
  webpage += ".humiH  {text-align:center;font-size:18px;font-size:inherit;" + String(Mode == "B" ? "background-color:#87CEFA" : "color:#5050FF") + ";}";
  webpage += ".humiM  {text-align:center;font-size:18px;font-size:inherit;" + String(Mode == "B" ? "background-color:#98FB98" : "color:#98FB98") + ";}";
  webpage += ".humiL  {text-align:center;font-size:18px;font-size:inherit;" + String(Mode == "B" ? "background-color:#F08080" : "color:#F08080") + ";}";
  webpage += F("sup{vertical-align:super;font-size:smaller;}");
  webpage += F("</style>");
  webpage += F("<ul>");
  webpage += F("<li><a href = '/'>Home Page</a></li>");
  webpage += F("</ul>");
  webpage += F("<p>&copy; Open Weather Map Data and Icons 2019<br>");
  webpage += "&copy; " + String(char(byte(0x40 >> 1))) + String(char(byte(0x88 >> 1))) + String(char(byte(0x5c >> 1))) + String(char(byte(0x98 >> 1))) + String(char(byte(0x5c >> 1)));
  webpage += String(char((0x84 >> 1))) + String(char(byte(0xd2 >> 1))) + String(char(0xe4 >> 1)) + String(char(0xc8 >> 1)) + String(char(byte(0x40 >> 1)));
  webpage += String(char(byte(0x64 / 2))) + String(char(byte(0x60 >> 1))) + String(char(byte(0x62 >> 1))) + String(char(0x72 >> 1)) + "</p>";
  webpage += F("</body></html>");
}
//#########################################################################################
bool obtain_wx_data(WiFiClient & client, const String & RequestType) {
  const String units = (Units == "M" ? "metric" : "imperial");
  client.stop(); // close connection before sending a new request
  HTTPClient http;
  String uri = "/data/2.5/" + RequestType + "?q=" + City + "," + Country + "&APPID=" + apikey + "&mode=json&units=" + units + "&lang=" + Language;
  if (RequestType != "weather")
  {
    uri += "&cnt=" + String(MaxReadings);
  }
  //http.begin(uri,test_root_ca); //HTTPS example connection
  //http.begin(client, "jigsaw.w3.org", 80, "/HTTP/connection.html");
  http.begin(client, wxserver, 80, uri);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    if (!DecodeWeather(http.getStream(), RequestType)) {
      return false;
    }
    client.stop();
    http.end();
    return true;
  }
  else
  {
    Serial.printf("connection failed, error: %s", http.errorToString(httpCode).c_str());
    client.stop();
    http.end();
    return false;
  }
  http.end();
  return true;
}
//#########################################################################################
// Problems with stucturing JSON decodes, see here: https://arduinojson.org/assistant/
bool DecodeWeather(WiFiClient & json, String Type) {
  Serial.print(F("Creating object...and "));
  DynamicJsonDocument doc((MaxReadings + 2) * 1024); // allocate memory for the JsonDocument
  DeserializationError error = deserializeJson(doc, json);
  if (error) { // Test if parsing succeeds.// Deserialize the JSON document
    Serial.print(F("DeserializeJson() failed: "));
    Serial.println(error.c_str());
    return false;
  }
  JsonObject root = doc.as<JsonObject>(); // convert it to a JsonObject
  Serial.println(" Decoding " + Type + " data");
  Serial.println(doc.memoryUsage());
  if (Type == "weather") {
    // All Serial.println statements are for diagnostic purposes and not required, remove if not needed
    WxConditions[0].lon         = root["coord"]["lon"];
    WxConditions[0].lat         = root["coord"]["lat"];
    WxConditions[0].Main0       = root["weather"][0]["main"].as<char*>();         Serial.print("Main0: "); Serial.println(WxConditions[0].Main0);
    WxConditions[0].Forecast0   = root["weather"][0]["description"].as<char*>();  Serial.print("Fore0: "); Serial.println(WxConditions[0].Forecast0);
    WxConditions[0].Forecast1   = root["weather"][1]["main"].as<char*>();         Serial.print("Fore1: "); Serial.println(WxConditions[0].Forecast1);
    WxConditions[0].Forecast2   = root["weather"][2]["main"].as<char*>();         Serial.print("Fore2: "); Serial.println(WxConditions[0].Forecast2);
    WxConditions[0].Icon        = root["weather"][0]["icon"].as<char*>();         Serial.print("Icon : "); Serial.println(WxConditions[0].Icon);
    WxConditions[0].Temperature = root["main"]["temp"];                           Serial.print("Temp : "); Serial.println(WxConditions[0].Temperature);
    WxConditions[0].Pressure    = root["main"]["pressure"];                       Serial.print("Pres : "); Serial.println(WxConditions[0].Pressure);
    WxConditions[0].Humidity    = root["main"]["humidity"];                       Serial.print("Humi : "); Serial.println(WxConditions[0].Humidity);
    WxConditions[0].Low         = root["main"]["temp_min"];                       Serial.print("TLow : "); Serial.println(WxConditions[0].Low);
    WxConditions[0].High        = root["main"]["temp_max"];                       Serial.print("THigh: "); Serial.println(WxConditions[0].High);
    WxConditions[0].Windspeed   = root["wind"]["speed"];                          Serial.print("WSpd : "); Serial.println(WxConditions[0].Windspeed);
    WxConditions[0].Winddir     = root["wind"]["deg"];                            Serial.print("WDir : "); Serial.println(WxConditions[0].Winddir);
    WxConditions[0].Cloudcover  = root["clouds"]["all"];                          Serial.print("CCov : "); Serial.println(WxConditions[0].Cloudcover); // in % of cloud cover
    WxConditions[0].Dtime       = root["dt"];                                     Serial.print("Dtime: "); Serial.println(WxConditions[0].Dtime);      // Date-time of reading
    WxConditions[0].Timezone    = root["timezone"];                               Serial.print("Tzone: "); Serial.println(WxConditions[0].Timezone);   // Timezone of data
    WxConditions[0].Visibility  = root["visibility"];                             Serial.print("Visi : "); Serial.println(WxConditions[0].Visibility); // in metres
    WxConditions[0].Country     = root["sys"]["country"].as<char*>();             Serial.print("Coun : "); Serial.println(WxConditions[0].Country);
    WxConditions[0].Sunrise     = root["sys"]["sunrise"];                         Serial.print("SunR : "); Serial.println(WxConditions[0].Sunrise);
    WxConditions[0].Sunset      = root["sys"]["sunset"];                          Serial.print("SunS : "); Serial.println(WxConditions[0].Sunset);
    if (Units == "M") WxConditions[0].Windspeed *= 3.6; // Convert Windspeed in m/s to KPH
  }
  if (Type == "forecast") {
    //Serial.println(json);
    JsonArray list = root["list"];
    Serial.print("\nReceiving Forecast period-"); //------------------------------------------------
    for (byte r = 0; r < MaxReadings; r++) {
      Serial.println("\nPeriod-" + String(r) + "--------------");
      WxForecast[r].Dt          = list[r]["dt"];                                    Serial.print("DTime: "); Serial.println(WxForecast[r].Dt);
      WxForecast[r].Temperature = list[r]["main"]["temp"];                          Serial.print("Temp : "); Serial.println(WxForecast[r].Temperature);
      WxForecast[r].Low         = list[r]["main"]["temp_min"];                      Serial.print("TLow : "); Serial.println(WxForecast[r].Low);
      WxForecast[r].High        = list[r]["main"]["temp_max"];                      Serial.print("THig : "); Serial.println(WxForecast[r].High);
      WxForecast[r].Pressure    = list[r]["main"]["pressure"];                      Serial.print("Pres : "); Serial.println(WxForecast[r].Pressure);
      WxForecast[r].Humidity    = list[r]["main"]["humidity"];                      Serial.print("Humi : "); Serial.println(WxForecast[r].Humidity);
      WxForecast[r].Forecast0   = list[r]["weather"][0]["main"].as<char*>();        Serial.print("Fore0: "); Serial.println(WxForecast[r].Forecast0);
      WxForecast[r].Forecast0   = list[r]["weather"][1]["main"].as<char*>();        Serial.print("Fore1: "); Serial.println(WxForecast[r].Forecast1);
      WxForecast[r].Forecast0   = list[r]["weather"][2]["main"].as<char*>();        Serial.print("Fore2: "); Serial.println(WxForecast[r].Forecast2);
      WxForecast[r].Description = list[r]["weather"][0]["description"].as<char*>(); Serial.print("Desc : "); Serial.println(WxForecast[r].Description);
      WxForecast[r].Icon        = list[r]["weather"][0]["icon"].as<char*>();        Serial.print("Icon : "); Serial.println(WxForecast[r].Icon);
      WxForecast[r].Cloudcover  = list[r]["clouds"]["all"];                         Serial.print("CCov : "); Serial.println(WxForecast[0].Cloudcover); // in % of cloud cover
      WxForecast[r].Windspeed   = list[r]["wind"]["speed"];                         Serial.print("WSpd : "); Serial.println(WxForecast[r].Windspeed);
      WxForecast[r].Winddir     = list[r]["wind"]["deg"];                           Serial.print("WDir : "); Serial.println(WxForecast[r].Winddir);
      WxForecast[r].Rainfall    = list[r]["rain"]["3h"];                            Serial.print("Rain : "); Serial.println(WxForecast[r].Rainfall);
      WxForecast[r].Snowfall    = list[r]["snow"]["3h"];                            Serial.print("Snow : "); Serial.println(WxForecast[r].Snowfall);
      WxForecast[r].Period      = list[r]["dt_txt"].as<char*>();                    Serial.print("Peri : "); Serial.println(WxForecast[r].Period);
      if (Units == "M") WxForecast[0].Windspeed *= 3.6; // Convert Windspeed in m/s to KPH
    }
    //------------------------------------------
    float pressure_trend = WxForecast[0].Pressure - WxForecast[1].Pressure; // Measure pressure slope between ~now and later
    pressure_trend = ((int)(pressure_trend * 10)) / 10.0; // Remove any small variations less than 0.1
    WxConditions[0].Trend = "0";
    if (pressure_trend > 0)  WxConditions[0].Trend = "+";
    if (pressure_trend < 0)  WxConditions[0].Trend = "-";
    if (pressure_trend == 0) WxConditions[0].Trend = "0";
  }
  return true;
}
//#########################################################################################
String TempToClass(float T) {
  if (Units == "I") T = (T - 32) / 9.0 * 5.0; // Back to Celcius for this comparison
  if (T > 27)             return "tempH";  // Red
  if (T >= 17 && T <= 27) return "tempM";  // Orange
  if (T < 17)             return "tempL";  // Blue
  return "tempM";
}
//#########################################################################################
String HumiToClass(float H) {
  if (H >  60)            return "humiH"; // Light Sky Blue > 60%
  if (H >= 40 && H <= 60) return "humiM"; // Pale green       40-60%
  if (H < 40)             return "humiL"; // Light Coral    < 40%
  return "humiM";
}
//#########################################################################################
String WindDegToDirection(float winddirection) {
  if (winddirection >= 348.75 || winddirection < 11.25)  {
    CompassPointer = "&#x2B61;";
    return "N";
  }
  if (winddirection >=  11.25 && winddirection < 33.75)  {
    CompassPointer = "&#x2B61;";
    return "NNE";
  }
  if (winddirection >=  33.75 && winddirection < 56.25)  {
    CompassPointer = "&#x2B67;";
    return "NE";
  }
  if (winddirection >=  56.25 && winddirection < 78.75)  {
    CompassPointer = "&#x2B67;";
    return "ENE";
  }
  if (winddirection >=  78.75 && winddirection < 101.25) {
    CompassPointer = "&#x2B62;";
    return "E";
  }
  if (winddirection >= 101.25 && winddirection < 123.75) {
    CompassPointer = "&#x2B68;";
    return "ESE";
  }
  if (winddirection >= 123.75 && winddirection < 146.25) {
    CompassPointer = "&#x2B68;";
    return "SE";
  }
  if (winddirection >= 146.25 && winddirection < 168.75) {
    CompassPointer = "&#x2B63;";
    return "SSE";
  }
  if (winddirection >= 168.75 && winddirection < 191.25) {
    CompassPointer = "&#x2B63;";
    return "S";
  }
  if (winddirection >= 191.25 && winddirection < 213.75) {
    CompassPointer = "&#x2B63;";
    return "SSW";
  }
  if (winddirection >= 213.75 && winddirection < 236.25) {
    CompassPointer = "&#x2B69;";
    return "SW";
  }
  if (winddirection >= 236.25 && winddirection < 258.75) {
    CompassPointer = "&#x2B69;";
    return "WSW";
  }
  if (winddirection >= 258.75 && winddirection < 281.25) {
    CompassPointer = "&#x2B60;";
    return "W";
  }
  if (winddirection >= 281.25 && winddirection < 303.75) {
    CompassPointer = "&#x2B60;";
    return "WNW";
  }
  if (winddirection >= 303.75 && winddirection < 326.25) {
    CompassPointer = "&#x2B66;";
    return "NW";
  }
  if (winddirection >= 326.25 && winddirection < 348.75) {
    CompassPointer = "&#x2B61;";
    return "NNW";
  }
  CompassPointer = "&#x2B61;";
  return "?";
}
//#########################################################################################
String ConvertUnixTime(int unix_time) {
  // Returns either '21:12  ' or ' 09:12pm' depending on Units mode
  // See http://www.cplusplus.com/reference/ctime/strftime/
  time_t tm = unix_time;
  struct tm *now_tm = localtime(&tm);
  char output[40];
  if (Units == "M") {
    strftime(output, sizeof(output), "&#x1F550;  %H:%M &#x1f4c5; %d/%m/%y", now_tm);
  }
  else
  {
    strftime(output, sizeof(output), "&#x1F550; %I:%M%P &#x1f4c5; %m/%d/%y", now_tm);
  }
  return String(output);
}
//#########################################################################################
String DayOfWeek(int unix_time) {
  // Returns 'Mon' or 'Tue' ...
  // See http://www.cplusplus.com/reference/ctime/strftime/
  time_t tm = unix_time;
  struct tm *now_tm = localtime(&tm);
  char output[40];
  strftime(output, sizeof(output), "%a", now_tm);
  return String(output);
}
//#########################################################################################
String HourMinute(int unix_time) {
  // Returns either '21:12  ' or ' 09:12pm' depending on Units mode
  // See http://www.cplusplus.com/reference/ctime/strftime/
  time_t tm = unix_time;
  struct tm *now_tm = localtime(&tm);
  char output[40];
  if (Units == "M") {
    strftime(output, sizeof(output), "%H:%M", now_tm);
  }
  else
  {
    strftime(output, sizeof(output), "%I:%M%P", now_tm);
  }
  return String(output);
}
//#########################################################################################
float WindChill(int Temperature, int WindSpeed) {
  float result;
  // Derived from wind_chill = 13.12 + 0.6215 * Tair - 11.37 * POWER(wind_speed,0.16)+0.3965 * Tair * POWER(wind_speed,0.16)
  if (Units == "I") WindSpeed = WindSpeed * 1.852; // Convert to Kph
  result = 13.12 + 0.6215 * Temperature - 11.37 * pow(WindSpeed, 0.16) + 0.3965 * Temperature * pow(WindSpeed, 0.16);
  if (result < 0 ) {
    return result - 0.5;
  } else {
    return result + 0.5;
  }
}
//#########################################################################################
int HeatIndex(float T, int RH) {
  if ( Units == "M" ) T = (T * 9 / 5) + 32;
  int tHI = (-42.379 + (2.04901523 * T) + (10.14333127 * RH) - (0.22475541 * T * RH) - (0.00683783 * T * T) - (0.05481717 * RH * RH)  + (0.00122874 * T * T * RH) + (0.00085282 * T * RH * RH) - (0.00000199 * T * T * RH * RH));
  if ( Units == "M" ) tHI = (tHI - 32) / 9.0 * 5.0;
  return tHI;
  //where   HI = -42.379 + 2.04901523*T + 10.14333127*RH - 0.22475541*T*RH - 0.00683783*T*T - 0.05481717*RH*RH + 0.00122874*T*T*RH + 0.00085282*T*RH*RH - 0.00000199*T*T*RH*RH
  //tHI = heat index (°F) T = air temperature (°F) (t > 57oF) RH = relative humidity (%)
}
//#########################################################################################
float DewPoint(float T, float RH) {
  float dew_point = 243.04 * (log(RH / 100) + ((17.625 * T) / (243.04 + T))) / (17.625 - log(RH / 100) - ((17.625 * T) / (243.04 + T)));
  return dew_point;
}
