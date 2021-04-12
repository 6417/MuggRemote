#include "Button.h"

Button::Button(int digitalPin) : digitalPin(digitalPin)
{
    pinMode(digitalPin, INPUT);
}

bool Button::isPressed()
{
    return digitalRead(digitalPin);
}