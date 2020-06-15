#define _FW_VERSION "v1.0.1 (14-06-2020)"

#include <FS.h>
#include <LittleFS.h>

#define _HOSTNAME   "OpenTherm01"
#include "OpenTherm01.h"

#include <opentherm.h>
#include "timing.h"
// Wemos D1 on 1of!-Wemos board
#define THERMOSTAT_IN     16  //--- GPIO-16 / PIN-4  / D0
#define THERMOSTAT_OUT     4  //--- GPIO-04 / PIN-5  / D2
#define BOILER_IN          5  //--- GPIO-05 / PIN-6  / D1
#define BOILER_OUT        14  //--- GPIO-14 / PIN-12 / D5

#define KEEP_ALIVE_PIN    13  //--- GPIO-13 / PIN-14 / D7
#define LED_BLUE           2  //--- GPIO-02 / PIN-3  / D4
#define LED_RED_B          0  //--- GPIO-00 / PIN-4  / D3
#define LED_RED_C         12  //--- GPIO-12 / PIN-13 / D6

#define FEEDTIME      2500

#define MODE_LISTEN_MASTER  0
#define MODE_LISTEN_SLAVE   1

OpenthermData message;

int mode = 0;

// WiFi Server object and parameters
WiFiServer server(80);


//=====================================================================
void handleWDTfeed(bool force)
{
  if (force || (millis() > WDTfeedTimer))
  {
    WDTfeedTimer = millis() + FEEDTIME;
    digitalWrite(KEEP_ALIVE_PIN, !digitalRead(KEEP_ALIVE_PIN));
    digitalWrite(LED_BLUE,        digitalRead(KEEP_ALIVE_PIN));
  }
  
} // handleWDTfeed();


//=====================================================================
void setup()
{
  Serial.begin(115200);
  while(!Serial) { /* wait a bit */ }

  lastReset     = ESP.getResetReason();

  pinMode(LED_BUILTIN,    OUTPUT); // This is the same pin as LED_BLUE
  pinMode(KEEP_ALIVE_PIN, OUTPUT);
  pinMode(LED_BLUE,       OUTPUT);
  pinMode(LED_RED_B,      OUTPUT);
  pinMode(LED_RED_C,      OUTPUT);

  startTelnet();
  
  DebugTln("\r\n[OpenTherm01]\r\n");
  DebugTf("Booting....[%s]\r\n\r\n", String(_FW_VERSION).c_str());
  
//================ LittleFS ===========================================
  if (LittleFS.begin()) 
  {
    DebugTln(F("LittleFS Mount succesfull\r"));
    LittleFSmounted = true;
  } else { 
    DebugTln(F("LittleFS Mount failed\r"));   // Serious problem with LittleFS 
    LittleFSmounted = false;
}

  readSettings(true);

  // attempt to connect to Wifi network:
  DebugTln("Attempting to connect to WiFi network\r");
  int t = 0;
  while ((WiFi.status() != WL_CONNECTED) && (t < 25))
  {
    delay(100);
    Debug(".");
    t++;
  }
  Debugln();
  if ( WiFi.status() != WL_CONNECTED) 
  {
    sprintf(cMsg, "Connect to AP '%s' and configure WiFi on  192.168.4.1   ", _HOSTNAME);
    DebugTln(cMsg);
  }
  // Connect to and initialise WiFi network
  digitalWrite(LED_BUILTIN, HIGH);
  startWiFi(_HOSTNAME, 240);  // timeout 4 minuten
  digitalWrite(LED_BUILTIN, LOW);

  startMDNS(settingHostname);

  //--- ezTime initialisation
  setDebug(INFO);  
  waitForSync(); 
  CET.setLocation(F("Europe/Amsterdam"));
  CET.setDefault(); 
  
  Debugln("UTC time: "+ UTC.dateTime());
  Debugln("CET time: "+ CET.dateTime());

  snprintf(cMsg, sizeof(cMsg), "Last reset reason: [%s]\r", ESP.getResetReason().c_str());
  DebugTln(cMsg);

  Debug("\nGebruik 'telnet ");
  Debug (WiFi.localIP());
  Debugln("' voor verdere debugging\r\n");

//================ Start HTTP Server ================================
  setupFSexplorer();
  httpServer.serveStatic("/FSexplorer.png",   LittleFS, "/FSexplorer.png");
  httpServer.on("/",          sendIndexPage);
  httpServer.on("/index",     sendIndexPage);
  httpServer.on("/index.html",sendIndexPage);
  httpServer.serveStatic("/index.css", LittleFS, "/index.css");
  httpServer.serveStatic("/index.js",  LittleFS, "/index.js");
  // all other api calls are catched in FSexplorer onNotFounD!
  httpServer.on("/api", HTTP_GET, processAPI);


  httpServer.begin();
  DebugTln("\nServer started\r");
  
  // Set up first message as the IP address
  sprintf(cMsg, "%03d.%03d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
  DebugTf("\nAssigned IP[%s]\r\n", cMsg);

/*****************************************************************************************
**  OpenTherm Initialisatie
*****************************************************************************************/
  pinMode(THERMOSTAT_IN, INPUT);
  digitalWrite(THERMOSTAT_IN, HIGH); // pull up
  digitalWrite(THERMOSTAT_OUT, HIGH);
  pinMode(THERMOSTAT_OUT, OUTPUT); // low output = high current, high output = low current
  pinMode(BOILER_IN, INPUT);
  digitalWrite(BOILER_IN, HIGH); // pull up
  digitalWrite(BOILER_OUT, LOW);
  pinMode(BOILER_OUT, OUTPUT); // low output = high voltage, high output = low voltage

  for (int b=0; b<20; b++)
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(200);
  }

} // setup()

/**
 * Loop will act as man in the middle connected between Opentherm boiler and Opentherm thermostat.
 * It will listen for requests from thermostat, forward them to boiler and then wait for response from boiler and forward it to thermostat.
 * Requests and response are logged to Serial on the way through the gateway.
*/

//=====================================================================
void loop()
{
  events(); // trigger ezTime update etc.
  handleWDTfeed(false);
  httpServer.handleClient();
  MDNS.update();

  /*** vanaf hier mag je het helemaal zelf bedenken ;-) ***/
  
  myOpenthermLoop();

} // loop()
