#include <RTCmillis.h>

RTC rtc(DS3231);
//RTC rtc(DS1307);

void setup() {
  rtc.begin();

  Serial.begin(9600);
}

void loop() {
  struct tm *now = rtc.now();

  Serial.println(asctime(now));

  /*
  struct tm now;

  rtc.now_r(&now);

  Serial.println(asctime(&now));
  */

  delay(1000);
}
