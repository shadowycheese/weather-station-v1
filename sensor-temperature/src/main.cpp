#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup()
{
    Serial.begin(115200);

    while (!Serial)
        ;
}

void loop()
{
    delay(1000);
    Serial.println("hello");
    // put your main code here, to run repeatedly:
}
