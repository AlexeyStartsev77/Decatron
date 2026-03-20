// быстрый digitalWrite
void setPin(uint8_t pin, uint8_t x) {
  switch (pin) { // откл pwm
      case 5:
        bitClear(TCCR0A, COM0B1);
        break;
      case 6:
        bitClear(TCCR0A, COM0A1);
        break;
      case 10:
        bitClear(TCCR1A, COM1B1);
        break;
      case 9:
        bitClear(TCCR1A, COM1A1);
        break;
      case 3:
        bitClear(TCCR2A, COM2B1);
        break;
      case 11:
        bitClear(TCCR2A, COM2A1);
        break;
  }

  if (pin < 8) bitWrite(PORTD, pin, x);
  else if (pin < 14) bitWrite(PORTB, (pin - 8), x);
  else if (pin < 20) bitWrite(PORTC, (pin - 14), x);
  else return;
}

// быстрый digitalRead
boolean readPin(uint8_t pin) { 	
	if (pin < 8) return bitRead(PIND, pin);
	else if (pin < 14) return bitRead(PINB, pin - 8);
	else if (pin < 20) return bitRead(PINC, pin - 14);	
	else return false;
}

// быстрый analogWrite
void setPWM(uint8_t pin, uint16_t duty) {
  if (duty == 0) setPin(pin, LOW);
  else {
    switch (pin) {
      case 5: // Timer0
        bitSet(TCCR0A, COM0B1);
        OCR0B = duty;
        break;
      case 6: // Timer0
        bitSet(TCCR0A, COM0A1);
        OCR0A = duty;
        break;
      case 10: // Timer1
        bitSet(TCCR1A, COM1B1);
        OCR1B = duty;
        break;
      case 9: // Timer1
        bitSet(TCCR1A, COM1A1);
        OCR1A = duty;
        break;
      case 3: // Timer2
        bitSet(TCCR2A, COM2B1);
        OCR2B = duty;
        break;
      case 11: // Timer2
        bitSet(TCCR2A, COM2A1);
        OCR2A = duty;
        break;
      default:
        break;
    }
  }
}

// ***************************** PWM mode *****************************
void setPWMmode(byte pin, byte mode) {		// 0 - FastPWM, 1 - Phase-correct PWM
// оператор ИЛИ: x |= y; // equivalent to x = x | y;
// оператор  И:  x &= y; // equivalent to x = x & y;
	if (pin == 5 || pin == 6) {
    bitClear(TCCR0A, 0);
		if (mode) bitClear(TCCR0A, 1);
		else bitSet(TCCR0A, 1);
	} else
	if (pin == 9 || pin == 10) { //а тут не как у людей - в регистре TCCR1B скомкано =(
		if (mode) bitClear(TCCR1B, 3); //TCCR1B &= ~_BV(WGM12); // убираем из байта 0b00001000
		else bitSet(TCCR1B, 3); //TCCR1B |= _BV(WGM12); // объединяем с байтом 0b00001000
	} else
	if (pin == 3 || pin == 11) {
    bitClear(TCCR2A, 0);
		if (mode) bitClear(TCCR2A, 1);
		else bitSet(TCCR2A, 1);
	} else {
		return;
	}
}

// ***************************** PWM freq *****************************
float _fixMultiplier = 1.0;		// множитель для функций delayFix и millisFix, по умолч. 1

void setPWMprescaler(uint8_t pin, uint16_t mode) {
  byte prescale;
  if (pin == 5 || pin == 6) {
    switch (mode) {
      case 1: prescale = 0x01; _fixMultiplier = 64; break;
      case 2: prescale = 0x02; _fixMultiplier = 8; break;
      case 3: prescale = 0x03; _fixMultiplier = 1; break;
      case 4: prescale = 0x04; _fixMultiplier = 0.25; break;
      case 5: prescale = 0x05; _fixMultiplier = 0.0625; break;
      default: return;
    }
  } else if (pin == 9 || pin == 10) {
	  switch (mode) {
      case 1: prescale = 0x01; break;
      case 2: prescale = 0x02; break;
      case 3: prescale = 0x03; break;
      case 4: prescale = 0x04; break;
      case 5: prescale = 0x05; break;
      default: return;
    }
  } else if (pin == 3 || pin == 11) {
    switch (mode) {
      case 1: prescale = 0x01; break;
      case 2: prescale = 0x02; break;
      case 3: prescale = 0x03; break;
      case 4: prescale = 0x04; break;
      case 5: prescale = 0x05; break;
      case 6: prescale = 0x06; break;
      case 7: prescale = 0x07; break;
      default: return;
    }
  }
  if (pin == 5 || pin == 6) {
	TCCR0B = 0;
    TCCR0B = TCCR0B & 0xF8 | prescale;
  } else if (pin == 9 || pin == 10) {
	TCCR1B = 0;
    TCCR1B = TCCR1B & 0xF8 | prescale;
  } else if (pin == 3 || pin == 11) {
	TCCR2B = 0;
    TCCR2B = TCCR2B & 0xF8 | prescale;
  }
}

void delayFix(uint32_t delayTime) {
	delay((float) delayTime * _fixMultiplier);
}

void delayMicrosecondsFix(uint32_t delayTime) {
	delayMicroseconds((float) delayTime * _fixMultiplier);
}

uint32_t millisFix() {
	return (float) millis() / _fixMultiplier;
}
uint32_t microsFix() {
	return (float) micros() / _fixMultiplier;
}


/*
Default: delay(1000) or 1000 millis() ~ 1 second

0x01: delay(64000) or 64000 millis() ~ 1 second x64
0x02: delay(8000) or 8000 millis() ~ 1 second x8
0x03: is the default
0x04: delay(250) or 250 millis() ~ 1 second x0.25
0x05: delay(62) or 62 millis() ~ 1 second x0.0625

(Or 63 if you need to round up.  The number is actually 62.5)
*/

// ***************************** PWM resolution *****************************

void set8bitPWM() {
	TCCR1A = TCCR1A & 0xE0 | 0x01;
}
void set10bitPWM() {
	TCCR1A = TCCR1A & 0xE0 | 0x03;
}

// **************************************************************************