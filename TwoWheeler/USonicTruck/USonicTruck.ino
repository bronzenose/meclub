// following guide at https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html
// install Links2004 websocket library from https://github.com/Links2004/arduinoWebSockets

// ADD FILE UPLOAD TOOL TO IDE: download esp8266fs.jar and install (copy to) [arduino]\tools\ESP8266FS\tool\esp8266fs.jar
// jar is found here: https://github.com/esp8266/arduino-esp8266fs-plugin/releases/latest
// (link found here: https://github.com/esp8266/arduino-esp8266fs-plugin)

// foo.local (mDNS) isn't supported by Android )-:
// the esp8266 wifi server's default IP is
// https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html
// "... default IP address of 192.168.4.1 ..."

#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <FS.h> // SPIFFS
#include <WebSocketsServer.h>
#include "ClubMembers.h"

//#define SERIAL_DEBUG_WIFI
//#define SERIAL_DEBUG_WEBSERVER
//#define SERIAL_DEBUG_MOTORS
#define SERIAL_DEBUG
static const bool g_bEnableMDNS = true;
static const bool g_bEnableWebSockets = true;
static const bool g_bEnableOTA = true;

#define PIN_PWM_LEFT 5 /* D1 */
#define PIN_PWM_RIGHT 4 /* D2 */
#define PIN_DIRECTION_LEFT 0 /* D3 */
#define PIN_DIRECTION_RIGHT 2 /* D4 */
static const bool g_bReverseLeft = 0;
static const bool g_bReverseRight = 1;
static unsigned long g_cmsStop = millis();
static float g_rSpeedRight = 0.0;
static float g_rSpeedLeft = 0.0;
static bool g_bDirRight = false;
static bool g_bDirLeft = false;

//ESP8266WiFiMulti wifiMulti;       // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);       // Create a webserver object that listens for HTTP request on port 80
WebSocketsServer webSocket(81, "", "RCTruck"); // port, origin, protocol

#ifdef SERIAL_DEBUG
void displayMessage(const int n){
	Serial.println(n);
}

void displayMessage(const char* const psz1, const char* const psz2 = NULL){
	if(psz1)
		Serial.println(psz1);
	if(psz2)
		Serial.println(psz2);
}
void displayProgress(float rProgress){
	Serial.print("progress ");
	Serial.print((int)(100.0*rProgress));
	Serial.println('%');
}

#else // ifdef SERIAL_DEBUG
void displayMessage(const int n){}
void displayMessage(const char* const psz1, const char* const psz2 = NULL){}
void displayProgress(float rProgress){}
#endif // def SERIAL_DEBUG

void displayMessageWSDBG(const char* const psz1) {
#ifdef SERIAL_DEBUG_WEBSERVER
	displayMessage(psz1);
#endif //def SERIAL_DEBUG_WEBSERVER
}

void displayMessageMotors(const char* const psz1) {
#ifdef SERIAL_DEBUG_MOTORS
	displayMessage(psz1);
#endif
}

void displayMessageMotors(const int n) {
#ifdef SERIAL_DEBUG_MOTORS
	displayMessage(n);
#endif
}

void vPrintChipId() {
	Serial.printf("%08x\n", ESP.getChipId());
}

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
	if (bytes < 1024) {
		return String(bytes) + "B";
	} else if (bytes < (1024 * 1024)) {
		return String(bytes / 1024.0) + "KB";
	} else if (bytes < (1024 * 1024 * 1024)) {
		return String(bytes / 1024.0 / 1024.0) + "MB";
	}
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
	if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

bool vHandleFileRead(String path) { // send the right file to the client (if it exists)
	Serial.println("handleFileRead: " + path);
	if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
	String contentType = getContentType(path);             // Get the MIME type
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
		if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
			path += ".gz";                                         // Use the compressed verion
		File file = SPIFFS.open(path, "r");                    // Open the file
		size_t sent = server.streamFile(file, contentType);    // Send it to the client
		file.close();                                          // Close the file again
		Serial.println(String("\tSent file: ") + path);
		return true;
	}
	Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
	return false;
}

void vHandleNotFound(){ // if the requested file or page doesn't exist, return a 404 not found error
	displayMessageWSDBG("vHandleNotFound");
	if(!vHandleFileRead(server.uri())){          // check if the file exists in the flash memory (SPIFFS), if so, send it
		server.send(404, "text/plain", "404: File Not Found");
	}
}

void vHandleRoot(){
	const ClubMember* const pmbr = pmbrOwnerOfThisESP8266();
	displayMessageWSDBG("vHandleRoot");
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text/html", "<!DOCTYPE html><html>");
	server.sendContent("<body>");
	server.sendContent("<h1>Hello ");
	server.sendContent(pmbr->pszMemberName());
	server.sendContent("!</h1>");
	server.sendContent("<bold>Hello microelectronics club!</bold>");
	server.sendContent("</body>");
	server.sendContent("</html>");
	server.sendContent(""); // this might end the sending like https://community.platformio.org/t/how-to-split-up-a-long-html-page/3633
}

void vHandleHello(){
	displayMessageWSDBG("vHandleHello");
	server.send(200, "text/plain", "Hello microelectronics club!");
}

void vHandleHtml(){
	displayMessageWSDBG("vHandleHtml");
	server.send(200, "text/html", "<h1>Hello</h1><p>microelectronics club!</p>");
}

void dbg_broadcast(const char* psz) {
	webSocket.broadcastTXT(psz, strlen(psz));
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
	switch (type) {
		case WStype_DISCONNECTED:
			Serial.printf("[%u] Disconnected!\n", num);
			break;
		case WStype_CONNECTED: {
								   IPAddress ip = webSocket.remoteIP(num);
								   Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
							   }
							   break;
		case WStype_TEXT:
							   //Serial.printf("[%u] get Text: %s\n", num, payload);
							   if (payload[0] == 'G') {
								   dbg_broadcast("you said START");
								   uint32_t nAll = (uint32_t) strtol((const char *) &payload[1], NULL, 10);
								   const int nR = nAll % 100;
								   nAll = nAll / 100; // integer division returns the floor, not a rounding.
								   const int nL  = nAll % 100;
								   nAll = nAll / 100;
								   const int nD  = nAll % 100;
								   g_rSpeedLeft = ((float)nL)/99.0;
								   g_rSpeedRight = ((float)nR)/99.0;
								   g_cmsStop = millis() + (nD * 100);
								   Serial.printf("start left at speed %d, right at %d for duration %d", nL, nR, nD);
							   } else if (payload[0] == 'J') {
								   // a Joystick command. Format is "J+n.n3 +n.n3"
								   float rX = 0.0;
								   float rY = 0.0;
								   const int cScanned = sscanf((const char*)&payload[1], "%f %f", &rX, &rY);
								   if(cScanned != 2) {
									   Serial.println("sscanf failure");
								   }

								   g_bDirRight = rX >0.0;
								   g_bDirLeft = rY >0.0;
								   // abs() returns an int. ugh.
								   rX = (g_bDirRight ? 2.0 : -2.0) * rX; // range from client is -0.5 to +0.5 so adjust to 0 to 1.0
								   rY = (g_bDirLeft ? 2.0 : -2.0) * rY;
								   static const float rDeadzone = 0.5; // motors have a high minimum
								   if(rX < rDeadzone) rX = 0.0;
								   if(rY < rDeadzone) rY = 0.0;
								   g_rSpeedRight = rX;
								   g_rSpeedLeft = rY;
								   g_cmsStop = millis() + 2000;
								   /*
									  Serial.print(rX);
									  Serial.print(',');
									  Serial.print(rY);
									  Serial.print(':');
									  Serial.print(g_rSpeedRight);
									  Serial.print(',');
									  Serial.println(g_rSpeedLeft);
									*/
							   } else if (payload[0] == '!') {
								   dbg_broadcast("you said STOP");
								   g_rSpeedLeft = 0;
								   g_rSpeedRight = 0;
								   g_cmsStop = millis();
							   }				   
							   break;
	}
}

void startSPIFFS() {
	SPIFFS.begin();
	Serial.println("SPIFFS started. Contents:");
	{
		Dir dir = SPIFFS.openDir("/");
		while (dir.next()) {
			String fileName = dir.fileName();
			size_t fileSize = dir.fileSize();
			Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
		}
		Serial.printf("\n");
	}
}

void startWiFi() {
	const ClubMember* const pmbr = pmbrOwnerOfThisESP8266();
	const char* const pszSsid = pmbr->pszSsid();
	const char* const pszPwd = pmbr->pszPassword();
	WiFi.softAP(pszSsid, pszPwd);
	Serial.print("Access Point \"");
	Serial.print(pszSsid);
	Serial.print(pszPwd);
	Serial.println("\" started\r\n");
	WiFi.printDiag(Serial);

	Serial.println("Connecting");
	while(WiFi.softAPgetStationNum() < 1) {
		delay(250);
		Serial.println('.');
	}
	Serial.println("");
#ifdef SERIAL_DEBUG_WIFI
	displayMessage("after startWiFi() we find...");
	// per https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html#use-printdiag
	Serial.setDebugOutput(true);
	WiFi.printDiag(Serial);
#endif
	WiFi.printDiag(Serial);
}

void startWebSocket() { // Start a WebSocket server
	webSocket.begin();                          // start the websocket server
	webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
	Serial.println("WebSocket server started.");
}

void startMDNS() { // Start the mDNS responder
	const ClubMember* const pmbr = pmbrOwnerOfThisESP8266();
	MDNS.begin(pmbr->pszMdnsName());                        // start the multicast domain name server
	Serial.print("mDNS responder started: http://");
	Serial.print(pmbr->pszMdnsName());
	Serial.println(".local (note that mDNS is currently NOT supported by ANDROID; try ip 192.168.4.1 instead)");
}

void startWebServer() {
	// we only have one page, we'll just handle it with onNotFound.
	/* server.on("/edit.html",  HTTP_GET, []() {
	   server.send(200, "text/plain", ""); 
	   }, handleFileUpload);
	 */

	server.on("/", vHandleRoot);
	server.on("/hello.html", vHandleHtml);
	server.on("/hello", vHandleHello);
	server.onNotFound(vHandleNotFound);
	displayMessageWSDBG("handling /hello.html and /hello directly; all other paths using SPIFFS");

	server.begin();
	displayMessageWSDBG("HTTP server started.");
}

void startOTA() {
	const ClubMember* const pmbr = pmbrOwnerOfThisESP8266();
	ArduinoOTA.setHostname(pmbr->pszOTAName());
	ArduinoOTA.setPassword(pmbr->pszOTAPassword());

	ArduinoOTA.onStart([]() {
			Serial.println("Start");
			});
	ArduinoOTA.onEnd([]() {
			Serial.println("\r\nEnd");
			});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
			});
	ArduinoOTA.onError([](ota_error_t error) {
			Serial.printf("Error[%u]: ", error);
			if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
			else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
			else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
			else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
			else if (error == OTA_END_ERROR) Serial.println("End Failed");
			});
	ArduinoOTA.begin();
	Serial.println("OTA ready\r\n");
}

const size_t g_cchStatusMessage = 32;
char g_rgchzStatusMessage[g_cchStatusMessage];
unsigned long g_nStatus = 0;
static const unsigned long g_cmsUpdatePeriod = 50000;
static unsigned long g_cmsNext = 0;

void vClubSetup() {
#ifdef SERIAL_DEBUG
	vPrintChipId();
#endif //def SERIAL_DEBUG
	startWiFi();   // 1st
	if(g_bEnableOTA){
		startOTA();    // 2nd
	}
	startSPIFFS(); // 3rd
	if(g_bEnableWebSockets){
		startWebSocket();  // 4th
	}
	if(g_bEnableMDNS){
		startMDNS(); // 5th
	}
	startWebServer(); // 6th (last)
#ifdef SERIAL_DEBUG
	vPrintChipId();
#endif //def SERIAL_DEBUG
}

void vClubLoop() {
	if(g_bEnableWebSockets){
		displayMessageWSDBG("handleWebSocket");
		webSocket.loop();
	}
	//displayMessageWSDBG("handleClient");
	server.handleClient();
	//displayMessageWSDBG("handleOTA");
	if(g_bEnableOTA){
		ArduinoOTA.handle();
	}
#ifdef SERIAL_DEBUG_WIFI
	WiFi.printDiag(Serial);
#endif
}

void vDelay(unsigned long cms) {
	vClubLoop();
	const unsigned long cmsStart = millis();
	const unsigned long cmsStop = cmsStart + cms;
	// in the event of our delay continuing past the millis() overflow, just delay in one chunk.
	if(cmsStop < cmsStart) {
		delay(cms);
		return;
	}
	while(cms > 5) {
		delay(5);
		cms -= 5;
		vClubLoop();
	}
	delay(cms);
}

void setup() {
	Serial.begin(115200);        // Start the Serial communication to send messages to the computer
	delay(10);
	Serial.println("Hello from ESP8266");

	pinMode(PIN_PWM_RIGHT, OUTPUT);
	pinMode(PIN_PWM_LEFT, OUTPUT);
	pinMode(PIN_DIRECTION_RIGHT, OUTPUT);
	pinMode(PIN_DIRECTION_LEFT, OUTPUT);

	vClubSetup();
}

void loop() {
	vClubLoop();
	if(millis() > g_cmsStop) {
		g_rSpeedRight = 0.0;
		g_rSpeedLeft = 0.0;
	}
	// see http://arduino.esp8266.com/Arduino/versions/2.0.0/doc/reference.html#analog-output
	// consider using analogWriteRange(new_range);
	int nSpeedRight = g_rSpeedRight * 1023.0;
	int nSpeedLeft = g_rSpeedLeft * 1023.0;
	displayMessageMotors(nSpeedLeft);
	displayMessageMotors(nSpeedRight);
	analogWrite(PIN_PWM_RIGHT, nSpeedRight);
	analogWrite(PIN_PWM_LEFT, nSpeedLeft);
	digitalWrite(PIN_DIRECTION_RIGHT, g_bDirRight ^ g_bReverseRight);
	digitalWrite(PIN_DIRECTION_LEFT, g_bDirLeft ^ g_bReverseLeft);
}

