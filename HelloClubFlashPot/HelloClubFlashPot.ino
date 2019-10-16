// "... default IP address of 192.168.4.1 ..."

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <FS.h>

static const int g_pinLedOne = 2;  // GPIO2
static const int g_pinLedTwo = 16; // GPIO16
static const int g_nLedOn = LOW;
static const int g_nLedOff = HIGH;

ESP8266WebServer server(80);

typedef unsigned long ClubMemberESP8266ID;
class ClubMember {
	protected:
		const ClubMemberESP8266ID m_mbrid;
		const char* const m_pszMemberName;
		const char* const m_pszSsid;
		const char* const m_pszPassword;
		const char* const m_pszMdnsName;
		const char* const m_pszOTAName;
		const char* const m_pszOTAPassword;

	public:
		static ClubMemberESP8266ID mbridThisDevice() {
			return (ClubMemberESP8266ID)ESP.getChipId();
		}

		static const ClubMemberESP8266ID 	MEMBER_UNKNOWN = 0x00000000;
		static const ClubMemberESP8266ID 	MEMBER_CLAUDE  = 0x005dd8cc;
		static const ClubMemberESP8266ID 	MEMBER_ALYSSA  = 0x00716bd8;
		static const ClubMemberESP8266ID 	MEMBER_ELI     = 0x00715b1e;
		static const ClubMemberESP8266ID 	MEMBER_EMILY   = 0x00716cce;
		static const ClubMemberESP8266ID 	MEMBER_EUGENIE = 0x00715973;
		static const ClubMemberESP8266ID 	MEMBER_JAYLYN  = 0x001987cb;
		static const ClubMemberESP8266ID 	MEMBER_KEELER  = 0x00fc6a50;
		static const ClubMemberESP8266ID 	MEMBER_LEAH    = 0x007162b3;
		static const ClubMemberESP8266ID 	MEMBER_NATALIE = 0x0071687a;
		static const ClubMemberESP8266ID 	MEMBER_ROSE    = 0x001980ed;

		static const ClubMember* const s_pmbrUnk;
		static const ClubMember* const s_rgpmbr [];
		static const int s_cpmbr;


		static const ClubMember* const pmbrOwnerOfThisESP8266() {
			const ClubMemberESP8266ID mbrid = mbridThisDevice();
			for(int ipmbr=0;ipmbr<s_cpmbr;ipmbr++) {
				const ClubMember* const pmbr = s_rgpmbr[ipmbr];
				if(pmbr->mbrid() == mbrid) {
					return pmbr;
				}
			}
			return s_pmbrUnk;
		}
		ClubMember(
				const ClubMemberESP8266ID mbrid,
				const char* const pszMemberName,
				const char* const pszSsid,
				const char* const pszPassword,
				const char* const pszMdnsName,
				const char* const pszOTAName,
				const char* const pszOTAPassword
				) :
			m_mbrid(mbrid),
			m_pszMemberName(pszMemberName),
			m_pszSsid(pszSsid),
			m_pszPassword(pszPassword),
			m_pszMdnsName(pszMdnsName),
			m_pszOTAName(pszOTAName),
			m_pszOTAPassword(pszOTAPassword)
	{
	}


		ClubMemberESP8266ID mbrid() const { return m_mbrid; }
		const char* const pszMemberName() const { return m_pszMemberName; }
		const char* const pszSsid() const { return m_pszSsid; }
		const char* const pszPassword() const { return m_pszPassword; }
		const char* const pszMdnsName() const { return m_pszMdnsName; }
		const char* const pszOTAName() const { return m_pszOTAName; }
		const char* const pszOTAPassword() const { return m_pszOTAPassword; }

};

const ClubMember* const ClubMember::s_pmbrUnk = new ClubMember(MEMBER_UNKNOWN , "UNKNOWN_ESP8266", "Unknown member name", "Cl02539#aude", "Claude", "ClaudeProgram", "ClOTA1884aude");
const ClubMember* const ClubMember::s_rgpmbr [] = {
	new ClubMember(MEMBER_CLAUDE , "Claude", "Claude", "Cl02539#aude", "Claude", "ClaudeProgram", "ClOTA1884aude"),
	new ClubMember(MEMBER_ALYSSA , "Alyssa", "Alyssa", "Al02539#yssa", "Alyssa", "AlyssaProgram", "AlOTA1884yssa"),
	new ClubMember(MEMBER_ELI    , "Eli", "Eli", "El02539#i", "Eli", "EliProgram", "ElOTA1884i"),
	new ClubMember(MEMBER_EMILY  , "Emily", "Emily", "Em02539#ily", "Emily", "EmilyProgram", "EmOTA1884ily"),
	new ClubMember(MEMBER_EUGENIE, "Eugenie", "Eugenie", "Eu02539#genie", "Eugenie", "EugenieProgram", "EuOTA1884genie"),
	new ClubMember(MEMBER_JAYLYN , "Jaylyn", "Jaylyn", "Ja02539#ylyn", "Jaylyn", "JaylynProgram", "JaOTA1884ylyn"),
	new ClubMember(MEMBER_KEELER , "Keeler", "Keeler", "Ke02539#eler", "Keeler", "KeelerProgram", "KeOTA1884eler"),
	new ClubMember(MEMBER_LEAH   , "Leah", "Leah", "Le02539#ah", "Leah", "LeahProgram", "LeOTA1884ah"),
	new ClubMember(MEMBER_NATALIE, "Natalie", "Natalie", "Na02539#talie", "Natalie", "NatalieProgram", "NaOTA1884talie"),
	new ClubMember(MEMBER_ROSE   , "Rose", "Rose", "Ro02539#se", "Rose", "RoseProgram", "RoOTA1884se"),
};

const int ClubMember::s_cpmbr = sizeof(ClubMember::s_rgpmbr) / sizeof(ClubMember::s_rgpmbr[0]);
void vHandleRoot(){
	const ClubMember* const pmbr = ClubMember::pmbrOwnerOfThisESP8266();
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.send(200, "text/html", "<!DOCTYPE html><html>");
	server.sendContent("<body>");
	server.sendContent("<h1>Hello ");
	server.sendContent(pmbr->pszMemberName());
	server.sendContent("!</h1>");
	server.sendContent("<bold>Hello microelectronics club!</bold>");
	server.sendContent("<p>Copy your code from <a href=\"HelloClub.ino\">HelloClub.ino</a></p>");
	server.sendContent("</body>");
	server.sendContent("</html>");
	server.sendContent(""); // this might end the sending like https://community.platformio.org/t/how-to-split-up-a-long-html-page/3633
}

class PeriodicEvent {
	protected:
		unsigned long m_cmsNextEvent;
		const unsigned long m_cmsInterval;
	public:
		PeriodicEvent(const unsigned long cmsInterval) : m_cmsInterval(cmsInterval), m_cmsNextEvent(millis()) {
		}

		bool bEventFired() {
			long cmsNow = millis();
			if(cmsNow >= m_cmsNextEvent){
				// Deal with overflow.
				if(m_cmsNextEvent < m_cmsInterval && cmsNow > (ULONG_MAX-m_cmsInterval)) {
					Serial.println("millis overflow");
				} else {
					m_cmsNextEvent += m_cmsInterval; // offset from previous event, not from 'now'
					return true;
				}
			}
			return false;
		}
};

float rAnalogReadA0() {
	static PeriodicEvent s_tickA0(50);
	static float s_rA0; // will be initialized on first read
	if(s_tickA0.bEventFired()){
		const long nA0 = analogRead(A0);
		s_rA0 = ((float)nA0)/1023.0;
	}
	return s_rA0;
}

void vHandleHello(){
	server.send(200, "text/html", "<h1>Hello</h1><p>microelectronics club!</p>");
}

void startWiFi() {
	const ClubMember* const pmbr = ClubMember::pmbrOwnerOfThisESP8266();
	const char* const pszSsid = pmbr->pszSsid();
	const char* const pszPwd = pmbr->pszPassword();
	WiFi.setAutoConnect(1);
	WiFi.mode(WIFI_AP);
	WiFi.softAP(pszSsid, pszPwd);
}

void vHandleNotFound(){
	if(!handleFileRead(server.uri())){
		server.send(404, "text/plain", "404: File Not Found");
	}
}

void startWebServer() {
	server.on("/", vHandleRoot);
	server.on("/hello", vHandleHello);
	server.onNotFound(vHandleNotFound);
	server.begin();
}

void startOTA() {
	const ClubMember* const pmbr = ClubMember::pmbrOwnerOfThisESP8266();
	ArduinoOTA.setHostname(pmbr->pszOTAName());
	ArduinoOTA.setPassword(pmbr->pszOTAPassword());
	ArduinoOTA.begin();
}

void startSPIFFS() {
	SPIFFS.begin();
}

String getContentType(String filename) {
	if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	else if (filename.endsWith(".ino")) return "text/plain";
	return "text/plain";
}

bool handleFileRead(String path) {
	Serial.println("handleFileRead: " + path);
	if (path.endsWith("/")) path += "index.html";
	String contentType = getContentType(path);
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz))
			path += ".gz";
		File file = SPIFFS.open(path, "r");
		size_t sent = server.streamFile(file, contentType);
		file.close();
		Serial.println(String("\tSent file: ") + path);
		return true;
	}
	Serial.println(String("\tFile Not Found: ") + path);
	return false;
}

void setup() {
	Serial.begin(115200);
	startWiFi();   // 1st
	startSPIFFS(); // 1.5th
	startOTA();    // 2nd
	startWebServer(); // 6th (last)
	pinMode(A0, INPUT);
	pinMode(g_pinLedOne, OUTPUT);
	pinMode(g_pinLedTwo, OUTPUT);
}

void loop() {
	server.handleClient();
	ArduinoOTA.handle();
	long cmsOneBar = (long)(500.0 * rAnalogReadA0());
	const long cmsMin = 10;
	if(cmsMin > cmsOneBar) {
		cmsOneBar = cmsMin;
	}
	long cmsNow = millis();
	const int iQuarterNote = (cmsNow / cmsOneBar) % 4;
	int nLedOneState = g_nLedOn;
	int nLedTwoState = g_nLedOn;
	if(1 < iQuarterNote) {
		nLedOneState = g_nLedOff;
	}
	if(0 == iQuarterNote || 2 == iQuarterNote) {
		nLedTwoState = g_nLedOff;
	}
	digitalWrite(g_pinLedOne, nLedOneState);
	digitalWrite(g_pinLedTwo, nLedTwoState);
}

