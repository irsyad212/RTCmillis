#include <RTCmillis.h>

RTC rtc(DS3231, 2);
//RTC rtc(DS1307, 2);

void setup() {
  rtc.begin();

  Serial.begin(9600);
}

void loop() {
  struct tm* now = rtc.now();

  /*

  struct tm now;

  rtc.now_r(&now);

  */
  
  if (rtc.millis() == 0) {
    Serial.println(asctime(now));
    //Serial.println(asctime(&now));
  }
}
