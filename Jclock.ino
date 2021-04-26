#include <ArduinoJson.h>
#include <WiFi.h>
#include "time.h"

// E-paper includes
#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include <stdlib.h>

UBYTE *ImageCache;

const char* host = "www.hebcal.com";
const int httpPort = 80;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

//Function prototypes
DynamicJsonDocument GetHebcalInfo(char *URL);

void setup()
{
  SetupEpaper();

  ConnectWiFi("***", "***");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);   // Init and get the time
}

void loop()
{
  char URL[200];

  if (!ConnectWiFi("FeingoldWiFi IoT", "Edgware2016"))
  {
    delay(5000);
    return;
  }

  // Get the time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  else
  {
    Serial.println("Time retrieved successfully");
  }

  // Make hebcal message
  char sLastUpdated[100];
  strftime(sLastUpdated, 100, "Updated:%d.%m.%y %H:%M", &timeinfo);

  // Get current time
  char sCurrentTime[10];
  strftime(sCurrentTime, 10, "%H:%M", &timeinfo);
  String ssCurrentTime = String(sCurrentTime);
  Serial.print("ssCurrentTime = ");
  Serial.println(ssCurrentTime);


  // Hebcal Shabbat
  sprintf(URL, "/shabbat?cfg=json&geonameid=2643743&M=on&leyning=off");
  DynamicJsonDocument HebcalDocShabbat = GetHebcalInfo(URL);

  //check data HebcalDocShabbat is valid
  if (HebcalDocShabbat == NULL)
  {
    Serial.println("Invalid response from HebcalDocShabbat.");
    delay(1000);
    return;
  }

  // Get shabbat information
  char sCandles[50], sHavdalah[50];
  for (int i = 0; HebcalDocShabbat["items"][i]["title"]; i++)
  {
    String sTitle = HebcalDocShabbat["items"][i]["title"];
    String sCategory = HebcalDocShabbat["items"][i]["category"];

    if (sCategory == "candles")
    {
      sTitle.toCharArray(sCandles,  sizeof(sCandles));
    }
    else if (sCategory == "havdalah")
    {
      sTitle.toCharArray(sHavdalah,  sizeof(sHavdalah));
    }
  }

  // Get candles time part
  String ssHavdalah = String(sHavdalah);
  String ssHavdalahTime = ssHavdalah.substring(10);
  Serial.print("sHavdalah = ");
  Serial.println(sHavdalah);
  Serial.print("ssHavdalah = ");
  Serial.println(ssHavdalah);
  Serial.print("ssHavdalahTime = ");
  Serial.println(ssHavdalahTime);



  // Day or night
  char *sSunset;
  if (ssHavdalahTime > ssCurrentTime)
  {
    Serial.println("Day");
    sSunset = "&gs=off";
  }
  else
  {
    Serial.println("Night");
    sSunset = "&gs=on";
  }

  Serial.println(sCandles);
  Serial.println(sHavdalah);


  // Hebcal Converter
  strftime(URL, 200, "/converter?cfg=json&gy=%Y&gm=%m&gd=%d&g2h=1", &timeinfo);
  sprintf(URL, "%s%s", URL, sSunset);
  DynamicJsonDocument HebcalDocConv = GetHebcalInfo(URL);

  //check data HebcalDocConv is valid
  if (HebcalDocConv == NULL)
  {
    Serial.println("Invalid response from HebcalDocConv.");
    delay(1000);
    return;
  }

  // Making hebrew date
  String sHebrewYear = HebcalDocConv["hy"];
  String sHebrewMonth = HebcalDocConv["hm"];
  String sHebrewDay = HebcalDocConv["hd"];

  char sHebrewDate[100];
  sprintf(sHebrewDate, "%s %s, %s", sHebrewDay, sHebrewMonth, sHebrewYear);

  // EVENTS
  String sEventOne = "";
  String sEventTwo = "";
  String sEventThree = "";
  String sEventFour = "";

  for (int i = 0; HebcalDocConv["events"][i]; i++)
  {
    String sTitle = HebcalDocConv["events"][i];

    if (i == 0)
    {
      sEventOne = sTitle;
    }
    else if (i == 1)
    {
      sEventTwo = sTitle;
    }
    else if (i == 2)
    {
      sEventThree = sTitle;
    }
    else if (i == 3)
    {
      sEventFour = sTitle;
    }
  }

  // Events string to char
  char saEventOne[sEventOne.length() + 1];
  sEventOne.toCharArray(saEventOne, sEventOne.length() + 1);

  char saEventTwo[sEventTwo.length() + 1];
  sEventTwo.toCharArray(saEventTwo, sEventTwo.length() + 1);

  char saEventThree[sEventThree.length() + 1];
  sEventThree.toCharArray(saEventThree, sEventThree.length() + 1);

  char saEventFour[sEventFour.length() + 1];
  sEventFour.toCharArray(saEventFour, sEventFour.length() + 1);

  Serial.println(sHebrewDate);
  Serial.println();

  PrintToEpaper(sHebrewDate, sCandles, sHavdalah, sLastUpdated, saEventOne, saEventTwo, saEventThree, saEventFour);

  Serial.println("Waiting");
  delay(1000 * 60 * 15);
}

bool ConnectWiFi(char *ssid, char *sPassword)
{
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, sPassword);

  int i = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    i++;
    if (i > 20)
    {
      Serial.println("WiFi failed to connect");
      return false;
    }
  }

  Serial.println();
  Serial.print("WiFi connected. ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  return true;
}

DynamicJsonDocument GetHebcalInfo(char *sURL)
{
  //Serial.println(sURL);

  DynamicJsonDocument HebcalDoc(5000);

  String sMyResponse = GetResponse(sURL);   //Get the response from hebcal
  //Serial.println(sMyResponse);

  if (sMyResponse != "Error")
  {
    Serial.println("Received response from Hebcal");
    DeserializationError err1 = deserializeJson(HebcalDoc, sMyResponse);
  }
  else
  {
    Serial.println("Error from GetResponse");
  }

  return HebcalDoc;
}

String GetResponse(String sUrl)
{
  WiFiClient client;   // Use WiFiClient class to create TCP connections
  if (!client.connect(host, httpPort))
  {
    Serial.println("GetResponse(): Connect failed");
    return "Error";
  }

  // Send the request to the server
  client.println("GET " + sUrl + " HTTP/1.1");
  client.println((String)"Host: " + host);
  client.println("Connection: close");
  client.println(); // end HTTP request header

  unsigned long timeout = millis();

  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println("Client Timeout!");
      client.stop();
      return "Error";
    }
  }

  // Read all the lines of the reply from server
  String sResponse;
  while (client.available())
  {
    sResponse = client.readStringUntil('\r');
  }

  client.stop();

  return sResponse; //We only care about the final line of the response
}

void SetupEpaper()
{
  DEV_Module_Init();
  EPD_2IN9_V2_Init();
  EPD_2IN9_V2_Clear();

  Serial.println("SetupEpaper()");

  //Allocate ImageCache
  UWORD ImageSize = ((EPD_2IN9_V2_WIDTH % 8 == 0) ? (EPD_2IN9_V2_WIDTH / 8 ) : (EPD_2IN9_V2_WIDTH / 8 + 1)) * EPD_2IN9_V2_HEIGHT;

  if ((ImageCache = (UBYTE *)malloc(ImageSize)) == NULL)
  {
    Serial.println("Failed to allocate ImageCache memory.");
    while (1);
  }

  Paint_NewImage(ImageCache, EPD_2IN9_V2_WIDTH, EPD_2IN9_V2_HEIGHT, 270, WHITE);
  Paint_SelectImage(ImageCache);

  //Welcome message
  Paint_Clear(WHITE);
  Paint_DrawString_EN(10, 10, "J-Clock", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(10, 50, "by Aron Feingold", &Font24, WHITE, BLACK);
  EPD_2IN9_V2_Display_Base(ImageCache);
}

void PrintToEpaper(char * psHebrewDate, char * psCandles, char * psHavadalah, char *psLastUpdated, char *psEventOne, char *psEventTwo, char *psEventThree, char *psEventFour)
{
  int iEventOneLength = strlen(psEventOne);


  // The main message
  Paint_Clear(WHITE);

  Paint_DrawString_EN(5, 1, psHebrewDate, &Font20, WHITE, BLACK);

  Paint_DrawString_EN(5, 43, psEventTwo, &Font12, WHITE, BLACK);
  Paint_DrawString_EN(5, 58, psEventThree, &Font12, WHITE, BLACK);
  Paint_DrawString_EN(5, 64, psEventFour, &Font8, WHITE, BLACK);
  Paint_DrawString_EN(5, 90, psCandles, &Font16, WHITE, BLACK);
  Paint_DrawString_EN(5, 110, psHavadalah, &Font16, WHITE, BLACK);
  Paint_DrawString_EN(235, 1, psLastUpdated, &Font12, WHITE, BLACK);

  if (iEventOneLength > 26)
  {
    Paint_DrawString_EN(5, 25, psEventOne, &Font12, WHITE, BLACK);
  }
  else
  {
    Paint_DrawString_EN(5, 25, psEventOne, &Font16, WHITE, BLACK);
  }

  EPD_2IN9_V2_Display_Base(ImageCache);
}
