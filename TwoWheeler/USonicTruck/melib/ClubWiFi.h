#ifndef MECLUB_CLUBWIFI_H
#define MECLUB_CLUBWIFI_H

#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <FS.h> // SPIFFS
#include <WebSocketsServer.h>

//ESP8266WiFiMulti wifiMulti;       // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);       // Create a webserver object that listens for HTTP request on port 80
WebSocketsServer webSocket(81, "", "RCTruck"); // port, origin, protocol

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

	/*
	Serial.println("Connecting");
	while(WiFi.softAPgetStationNum() < 1) {
		delay(250);
		Serial.println('.');
	}
	*/
	Serial.println("");
#ifdef SERIAL_DEBUG_WIFI
	displayMessage("after startWiFi() we find...");
	// per https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html#use-printdiag
	Serial.setDebugOutput(true);
	WiFi.printDiag(Serial);
#endif
	WiFi.printDiag(Serial);
}

void startWebSocket(void (*vEventHandler)(uint8_t, WStype_t, uint8_t*, size_t)) { // Start a WebSocket server
	webSocket.begin();                          // start the websocket server
	webSocket.onEvent(vEventHandler);          // if there's an incomming websocket message, go to function 'webSocketEvent'
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


#endif//ndef MECLUB_CLUBWIFI_H

