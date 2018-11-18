
#include "deepSleep.h"

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  900        /* Time ESP32 will go to sleep (in seconds) */

void deepSleep()
{
	// sleep, rtc and power mangement
	esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);
	esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
	Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
	Serial.println("Configured all RTC Peripherals to be powered on in sleep");

	Serial.println("Going to sleep now");
	Serial.flush();
	esp_deep_sleep_start();
	Serial.println("This will never be printed");
}