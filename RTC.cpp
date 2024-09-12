#include "RTCmillis.h"

static int8_t _sqw;
static int8_t _sda, _scl;
static int8_t _type;

#define _DS_I2C_ADDRESS 0x68

uint8_t decToBcd(uint8_t val) {
  return ((val / 10 * 16) + (val % 10));
}

uint8_t bcdToDec(uint8_t val) {
  return ((val / 16 * 10) + (val % 16));
}

RTC::RTC(int8_t rtc_type) {
  _type = rtc_type;
  _scl = -1;
  _sda = -1;
  _sqw = -1;
}

RTC::RTC(int8_t rtc_type, int8_t millis_pin) {
  _type = rtc_type;
  _scl = -1;
  _sda = -1;
  _sqw = millis_pin;
}

RTC::RTC(int8_t rtc_type, int8_t sda, int8_t scl) {
  _type = rtc_type;
  _scl = scl;
  _sda = sda;
  _sqw = -1;
}

RTC::RTC(int8_t rtc_type, int8_t sda, int8_t scl, int8_t millis_pin) {
  _type = rtc_type;
  _scl = scl;
  _sda = sda;
  _sqw = millis_pin;
}

uint8_t _rCB(bool cb) {
  Wire.beginTransmission(_DS_I2C_ADDRESS);
  if (cb) {
    Wire.write(15);
  } else {
    Wire.write(14);
  }
  Wire.endTransmission();
  Wire.requestFrom(_DS_I2C_ADDRESS, 1);
  return Wire.read();
}

void _wCB(uint8_t ctrl, bool cb) {
  Wire.beginTransmission(_DS_I2C_ADDRESS);
  if (cb) {
    Wire.write(0x0f);
  } else {
    Wire.write(0x0e);
  }
  Wire.write(ctrl);
  Wire.endTransmission();
}

void _set_sqw() {
  if (_type == DS3231) {
    uint8_t tmp = _rCB(0) & 0b11100111;
    tmp = tmp | 0b01000000;
    tmp = tmp & 0b01111011;
    tmp = tmp | 8;
    _wCB(tmp, 0);
  }

  if (_type == DS1307) {
    Wire.beginTransmission(_DS_I2C_ADDRESS);
    Wire.write(7);
    Wire.write(16);
    Wire.endTransmission();
  }
}

volatile bool count;
static uint32_t _msec;

void _isr(){
  count = 1;
}

void RTC::begin() {
  if (_sda == -1 && _scl == -1) {
    Wire.begin();
  } else {

    #ifndef AVR
    Wire.begin(_sda, _scl);
    #else
    Wire.begin();
    #endif
  }

  if (_sqw != -1) {
    _set_sqw();
		pinMode(_sqw, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(2), _isr, FALLING);
		count = 0;
  }

}

void _set(uint8_t ss, uint8_t mm, uint8_t hh, uint8_t ww, uint8_t d, uint8_t m, uint16_t y) {
  Wire.beginTransmission(_DS_I2C_ADDRESS);

  Wire.write(0);

  Wire.write(decToBcd(ss));
  Wire.write(decToBcd(mm));
  Wire.write(decToBcd(hh));

  Wire.write(decToBcd(ww));

  Wire.write(decToBcd(d));
  Wire.write(decToBcd(m));
  Wire.write(decToBcd(y));

  Wire.endTransmission();
}

void RTC::set(struct tm * tmp) {
  uint8_t wday = tmp -> tm_wday;
  if (wday == 0) {
    wday = 7;
  }
  _set(tmp -> tm_sec, tmp -> tm_min, tmp -> tm_hour, wday, tmp -> tm_mday, tmp -> tm_mon + 1, tmp -> tm_year - 100);
}

void RTC::now_r(struct tm * tmp) {
	uint8_t second, minute, hour, dayOfWeek, dayOfMonth, month;
	uint16_t year;

  Wire.beginTransmission(_DS_I2C_ADDRESS);

  Wire.write(0);

  Wire.endTransmission();

  Wire.requestFrom(_DS_I2C_ADDRESS, 7);

  second = bcdToDec(Wire.read() & 0x7f);
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read() & 0x3f);

  dayOfWeek = bcdToDec(Wire.read());

  dayOfMonth = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());

  if (hour > 23) {
    second = 0;
    minute = 0;
    hour = 0;

    dayOfWeek = 6;

    dayOfMonth = 1;
    month = 1;
    year = 0;
  }

  #define _LEAP_YEAR(Y)(!(((Y)) % 4) && ((((Y)) % 100) || !(((Y)) % 400)))

  if (_LEAP_YEAR(year + 2000) == 0 && dayOfMonth == 29 && month == 2) {
    _set(second, minute, hour, dayOfWeek, 1, 3, year);
  }

  if (year == 0) {
    _set(second, minute, hour, dayOfWeek, dayOfMonth, month, 100);
  }

  uint8_t wday = dayOfWeek;

  if (wday == 7) {
    wday = 0;
  }

  tmp -> tm_sec = second;
  tmp -> tm_min = minute;
  tmp -> tm_hour = hour;

  tmp -> tm_wday = wday;

  tmp -> tm_mday = dayOfMonth;
  tmp -> tm_mon = month - 1;
  tmp -> tm_year = year + 100;
}

struct tm _return;

struct tm * RTC::now() {
  RTC::now_r( & _return);
  return & _return;
}

double RTC::temperature() {
  double tempr;
  if (_type == DS3231) {

    uint8_t tMSB, tLSB;

    Wire.beginTransmission(_DS_I2C_ADDRESS);
    Wire.write(0x11);
    Wire.endTransmission();
    Wire.requestFrom(_DS_I2C_ADDRESS, 2);

    if (Wire.available()) {
      tMSB = Wire.read();
      tLSB = Wire.read();

      int16_t itemp = (tMSB << 8 | (tLSB & 0xC0));
      tempr = ((double) itemp / 256.0);
    } else {
      tempr = 0.00;
    }
  }

  if (_type == DS1307) {
    tempr = 0.00;
  }
  return tempr;
}

uint32_t RTC::millis() {
  if (_sqw == -1) {
    _msec = 0;
  } else {
		if(count){
    	_msec = 0;
    	count = 0;
  	} else {
    	static uint32_t prev;
    	if(micros() - prev >= 1000){
      	_msec++;
      	prev = micros();
    	}
  	}
  }
  return _msec;
}
