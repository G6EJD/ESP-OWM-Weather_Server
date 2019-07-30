const char* ssid     = "your-SSID";
const char* password = "your-PASSWORD";

// Use your own API key by signing up for a free developer account at https://openweathermap.org/
String apikey = "your_OWM-API-Key";           // See: https://openweathermap.org/
const char wxserver[] = "api.openweathermap.org";
//Set your location according to OWM locations
String City             = "MELKSHAM";                      // Your home city See: http://bulk.openweathermap.org/sample/
String Country          = "GB";                            // Your _ISO-3166-1_two-letter_country_code country code, on OWM find your nearest city and the country code is displayed
                                                           // https://en.wikipedia.org/wiki/List_of_ISO_3166_country_codes
String Language         = "EN";                            // NOTE: Only the weather description is translated by OWM
                                                           // Examples: Arabic (AR) Czech (CZ) English (EN) Greek (EL) Persian(Farsi) (FA) Galician (GL) Hungarian (HU) Japanese (JA)
                                                           // Korean (KR) Latvian (LA) Lithuanian (LT) Macedonian (MK) Slovak (SK) Slovenian (SL) Vietnamese (VI)
String Units            = "M";                             // Use 'M' for Metric or I for Imperial
String Mode             = "F";                             // Use 'F' to highlight Foreground colours (Text) or 'B' for the Background colours
int    PageWidth        = 1024;                            // Adjust for desired webpage width
int    ForecastPeriods  = 9;                               // Adjust number of forecast periods to display, maximum 96hrs (96/3 / 8 = 4-days
const char* Timezone    = "GMT0BST,M3.5.0/01,M10.5.0/02";  // Choose your time zone from: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv 
                                                           // See below for examples
const char* ntpServer   = "pool.ntp.org";                  // Or, choose a time server close to you, but in most cases it's best to use pool.ntp.org to find an NTP server
                                                           // then the NTP system decides e.g. 0.pool.ntp.org, 1.pool.ntp.org as the NTP syem tries to find  the closest available servers
                                                           // EU "0.europe.pool.ntp.org"
                                                           // US "0.north-america.pool.ntp.org"
                                                           // See: https://www.ntppool.org/en/   
                                                           
