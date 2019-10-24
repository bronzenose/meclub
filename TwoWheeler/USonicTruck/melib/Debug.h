#ifndef MECLUB_DEBUG_H
#define MECLUB_DEBUG_H

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


#endif//ndef MECLUB_DEBUG_H
