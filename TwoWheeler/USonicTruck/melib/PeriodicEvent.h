#ifndef MECLUB_PERIODICEVENT_H
#define MECLUB_PERIODICEVENT_H

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

#endif//ndef MECLUB_PERIODICEVENT_H
