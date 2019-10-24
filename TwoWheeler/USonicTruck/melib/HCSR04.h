#ifndef MECLUB_HCSR04_H
#define MECLUB_HCSR04_H
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

#endif//ndef MECLUB_HCSR04_H
