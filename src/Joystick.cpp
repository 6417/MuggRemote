#include "Joystick.h"

double mapF(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

Joystick::Joystick(int analogPinX, int analogPinY, int analogMax) : analogPinX(analogPinX), analogPinY(analogPinY), analogMax(analogMax)
{
    pinMode(analogPinX, INPUT);
    pinMode(analogPinY, INPUT);
}

void Joystick::normalize(double &x, double &y)
{
    double length = sqrt(x * x + y * y);
    if (length > 1)
    {
        x /= length;
        y /= length;
    }
}

Joystick::Position Joystick::getPosition()
{
    double x = limit(analogPinX);
    double y = limit(analogPinY);
    normalize(x, y);
    applyDeadzone(x, y);
    return Joystick::Position{x, y};
}

void Joystick::applyDeadzone(double &x, double &y)
{
    double length = sqrt(x * x + y * y);
    if (length < deadzone)
    {
        x = 0;
        y = 0;
    }
    else
    {
        double mapedLength = mapF(length, deadzone, 1, 0, 1);
        x *= mapedLength / length;
        y *= mapedLength / length;
    }
}

Joystick::Joystick(int analogPinX, int analogPinY, int analogMax, double deadzone) : analogPinX(analogPinX), analogPinY(analogPinY), analogMax(analogMax), deadzone(deadzone)
{
    pinMode(analogPinX, INPUT);
    pinMode(analogPinY, INPUT);
}

double Joystick::limit(int analogPin)
{
    const double min = -1;
    const double max = 1;
    double x = mapF(analogRead(analogPin), 0, analogMax, min, max);
    return constrain(x, min, max);
}

double Joystick::getX()
{
    return getPosition().x;
}

double Joystick::getY()
{
    return getPosition().y;
}

int Joystick::getXraw()
{
    return analogRead(analogPinX);
}

int Joystick::getYraw()
{
    return analogRead(analogPinY);
}