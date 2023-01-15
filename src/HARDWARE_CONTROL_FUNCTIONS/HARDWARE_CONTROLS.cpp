

#include "HARDWARE_CONTROLS.h"
#include "Tone32.h"
#include <Arduino.h>




#define EEPROM_SIZE     4096
#define DATA_PIN        34   //keyboard data
#define CLK_PIN         4    //keyboard clock
#define testLed         19
#define SDA_1           21
#define SCL_1           22
#define BUZZER_PIN      23
#define BUZZER_CHANNEL  0






void HARDWARE_CONTROLS::beep(int pitch, int  dur)
{

        tone(BUZZER_PIN, pitch);
        delay(dur);
        noTone(BUZZER_PIN);
    
}





