#ifndef MECLUB_ANALOGREAD_H
#define MECLUB_ANALOGREAD_H
#include "PeriodicEvent.h"

float rAnalogReadA0() {
	static PeriodicEvent s_tickA0(50);
	static float s_rA0; // will be initialized on first read
	if(s_tickA0.bEventFired()){
		const long nA0 = analogRead(A0);
		s_rA0 = ((float)nA0)/1023.0;
	}
	return s_rA0;
}

#endif//ndef MECLUB_ANALOGREAD_H
