#ifndef RTCmillis
#define RTCmillis "v1.0.0"

#if ARDUINO >= 100
#include <Arduino.h> 
#else
#include <WProgram.h> 
#endif

#include <time.h>
#include <Wire.h>

#define DS1307 0x00
#define DS3231 0x01

class RTC {
public:
  RTC(int8_t rtc_type);

  RTC(int8_t rtc_type, int8_t millis_pin);

  RTC(int8_t rtc_type, int8_t sda, int8_t scl);

  RTC(int8_t rtc_type, int8_t sda, int8_t scl, int8_t millis_pin);

  void begin();

  void set(struct tm *tmp);

  void now_r(struct tm *tmp);

  struct tm *now();

  double temperature();

  uint32_t millis();
};

#endif
