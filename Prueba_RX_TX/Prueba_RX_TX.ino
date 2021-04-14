#include <AX12A.h>

#define DirectionPin  (2u)
#define BaudRate    (1000000ul) // <--- Servo baud rate Default 1000000
                                                 //  Please change Arduino Serial monitor baud rate to 57600
                 
                                                 // If  your Servo baud rate 1000000
                                                 //  Please change Arduino Serial monitor baud rate to 1000000
                                                
#define ID    (4u)

int reg = 0;

void setup()
{
 
  
 ax12a.begin(BaudRate, DirectionPin, &Serial);
       delay(1000);
       Serial.begin(1000000);
       ax12a.setEndless(ID,OFF);
       ax12a.move(ID,100);
      delay(1000);
}

void loop()
{
 reg = ax12a.readPosition(ID);
 Serial.println(reg);
ax12a.move(ID,200);
 delay(1000);
}
