#include <RTCmillis.h>

RTC rtc(DS3231);
//RTC rtc(DS1307);

void setup() {
  rtc.begin();

  Serial.begin(9600);

  struct tm temp;
  
  temp.tm_sec = 59;
  temp.tm_min = 59;
  temp.tm_hour = 23;

  temp.tm_wday = 0;

  temp.tm_mday = 31;
  temp.tm_mon = 12 - 1;
  temp.tm_year = 2019 - 1900;

  rtc.set(&temp);
}

void loop() {
}
