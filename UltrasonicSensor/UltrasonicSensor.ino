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

class HCSR04 {
	protected:
		static const unsigned long s_cusTrigger = 10;
		static const unsigned long s_cusEchoTimeout = 8062; // about 2m (2000*2.0155*2)
		static const float s_ruspmmRoundTrip;
		const int m_pinTrigger, m_pinEcho;

	public:
		HCSR04(const int pinTrigger, const int pinEcho) : m_pinTrigger(pinTrigger), m_pinEcho(pinEcho) {
			pinMode(m_pinTrigger, OUTPUT);
			pinMode(m_pinEcho, INPUT);
			digitalWrite(m_pinTrigger, LOW);
			delayMicroseconds(2);
		}
		float rmmMeasure() {
			digitalWrite(m_pinTrigger, HIGH);
			delayMicroseconds(10);
			digitalWrite(m_pinTrigger, LOW);
			const long cusEcho = pulseIn(m_pinEcho, HIGH, s_cusEchoTimeout);
			return ((float)cusEcho) / s_ruspmmRoundTrip;
		}
};
const float HCSR04::s_ruspmmRoundTrip = 2.9155*2.0; // sound travels 343 m per second

HCSR04 g_echoLeft(g_pinTrigger0, g_pinEcho0);
HCSR04 g_echoRight(g_pinTrigger1, g_pinEcho1);

void setup() {
	Serial.begin(114200);
}

void loop() {
	Serial.print(g_echoLeft.rmmMeasure());
	Serial.print("mm ");
	delay(25);
	Serial.print(g_echoRight.rmmMeasure());
	Serial.println("mm");
	delay(25);
}

