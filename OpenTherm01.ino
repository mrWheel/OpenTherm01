#define _FW_VERSION "v1.0.0 (19-05-2020)"

#include <FS.h>
#include <LittleFS.h>

#define _HOSTNAME   "OpenTherm01"
#include "OpenTherm01.h"

#include <opentherm.h>
#include "timing.h"
// Wemos D1 on 1of!-Wemos board
#define THERMOSTAT_IN   16
#define THERMOSTAT_OUT   4
#define BOILER_IN        5
#define BOILER_OUT      14

#define MODE_LISTEN_MASTER  0
#define MODE_LISTEN_SLAVE   1

OpenthermData message;

int mode = 0;

// WiFi Server object and parameters
WiFiServer server(80);

//=====================================================================
void setup()
{
  Serial.begin(115200);
  while(!Serial) { /* wait a bit */ }

  lastReset     = ESP.getResetReason();

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
  startNTP();

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

  
} // setup()

/**
 * Loop will act as man in the middle connected between Opentherm boiler and Opentherm thermostat.
 * It will listen for requests from thermostat, forward them to boiler and then wait for response from boiler and forward it to thermostat.
 * Requests and response are logged to Serial on the way through the gateway.
*/

//=====================================================================
void loop()
{
  handleNTP();

  
  httpServer.handleClient();
  MDNS.update();

  /*** vanaf hier mag je het helemaal zelf bedenken ;-) ***/
  
  myOpenthermLoop();

} // loop()


