// following guide at https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html
// install Links2004 websocket library from https://github.com/Links2004/arduinoWebSockets

// ADD FILE UPLOAD TOOL TO IDE: download esp8266fs.jar and install (copy to) [arduino]\tools\ESP8266FS\tool\esp8266fs.jar
// jar is found here: https://github.com/esp8266/arduino-esp8266fs-plugin/releases/latest
// (link found here: https://github.com/esp8266/arduino-esp8266fs-plugin)

// foo.local (mDNS) isn't supported by Android )-:
// the esp8266 wifi server's default IP is
// https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html
// "... default IP address of 192.168.4.1 ..."

#include "melib/HCSR04.h"
#include "melib/ClubMembers.h"
#include "melib/Debug.h"
#include "melib/ClubWiFi.h"

//#define SERIAL_DEBUG_WIFI
//#define SERIAL_DEBUG_WEBSERVER
//#define SERIAL_DEBUG_MOTORS
#define SERIAL_DEBUG
static const bool g_bEnableMDNS = true;
static const bool g_bEnableWebSockets = true;
static const bool g_bEnableOTA = true;

static const int  g_pinD0 = 16 ; /*     works for pulsing stepper */
static const int  g_pinD1 = 5  ; /*     works for pulsing stepper */
static const int  g_pinD2 = 4  ; /*     works for pulsing stepper */
static const int  g_pinD3 = 0  ; /* !!! connection prevents boot (also labeled "FLASH") */
static const int  g_pinD4 = 2  ; /* !!! NODE_MCU_PIN_D4 is used for programming (also labeled "TXD1" */
static const int  g_pinD5 = 14 ; /* works for pulsing stepper */
static const int  g_pinD6 = 12 ; /* works for pulsing stepper */
static const int  g_pinD7 = 13 ; /* appear not to work as TwoWire SCL but maybe it's only pin D8 that won't play */
// pin D8 (GPIO 15) has internal pulldown: https://arduino.stackexchange.com/questions/55940/input-pullup-not-work-for-d8-in-my-nodemcu-v3
// https://github.com/espressif/esptool/wiki/ESP8266-Boot-Mode-Selection#required-pins
static const int  g_pinD8 = 15 ; /* !!! appear not to work as TwoWire SDA !!! */
// TX and RX can be used as GPIO at the expense of Serial communications.
// https://arduino.stackexchange.com/questions/29938/how-to-i-make-the-tx-and-rx-pins-on-an-esp-8266-01-into-gpio-pins
static const int  g_pinRX = 3  ; /* !!! used for serial communication with PC */
static const int  g_pinTX = 1  ; /* !!! used for serial communication with PC */
static const int  g_pinSD0 = 7;
static const int  g_pinSD1 = 8;
static const int  g_pinSD2 = 9;
static const int  g_pinSD3 = 10;

// HW 588 shield
static const int g_pinPwmLeft = 5; /* D1 */
static const int g_pinPwmRight = 4; /* D2 */
static const int g_pinDirectionLeft = 0; /* D3 */
static const int g_pinDirectionRight = 2; /* D4 */

static const int g_pinTrigger0 = g_pinD0;
static const int g_pinEcho0 = g_pinD5;
static const int g_pinTrigger1 = g_pinD6;
static const int g_pinEcho1 = g_pinD7;

static const bool g_bReverseLeft = 0;
static const bool g_bReverseRight = 1;
static unsigned long g_cmsStop = millis();
static float g_rSpeedRight = 0.0;
static float g_rSpeedLeft = 0.0;
static bool g_bDirRight = false;
static bool g_bDirLeft = false;


HCSR04 g_echoLeft(g_pinTrigger0, g_pinEcho0);
HCSR04 g_echoRight(g_pinTrigger1, g_pinEcho1);

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
		startWebSocket(webSocketEvent);  // 4th
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

