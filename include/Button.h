#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button 
{
private:
    const int digitalPin;
public:
    Button(int digitalPin);

    bool isPressed();
};

#endif