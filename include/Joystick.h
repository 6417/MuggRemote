#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick
{
    public:
    struct Position
    {
        double x;
        double y; 
    };
    
private:
    const int analogPinX;
    const int analogPinY;
    const int analogMax;
    double limit(int analogPin);
    double deadzone = 0.1;
    void applyDeadzone(double& x, double& y);
    void normalize(double& x, double& y);
public:
    Joystick(int analogPinX, int analogPinY, int analogMax, double deadzone);
    Joystick(int analogPinX, int analogPinY, int analogMax);
    double getX();
    double getY();
    int getXraw();
    int getYraw();
    Position getPosition();
};

#endif