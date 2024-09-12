#include <RTCmillis.h>

RTC rtc(DS3231);

void setup() {
  rtc.begin();
  Serial.begin(9600);
}

void loop() {
  struct tm *now = rtc.now();

  Serial.print(asctime(now));
  Serial.print(" Temperature: ");
  Serial.print(rtc.temperature());
  Serial.println(" C");

  delay(1000);
}
