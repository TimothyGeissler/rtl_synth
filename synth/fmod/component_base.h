/**
 * @file component_base.h
 * @brief Base interface for all 74xx series components
 */

#ifndef COMPONENT_BASE_H
#define COMPONENT_BASE_H

#include <cassert>

class Component {
public:
    enum LogicLevel {
        LOW = 0,
        HIGH = 1,
        FLOATING = -1
    };

    virtual ~Component() = default;
    virtual void setPin(int pin, LogicLevel level) = 0;
    virtual LogicLevel getPin(int pin) const = 0;
    virtual void setPower(bool on) = 0;
    virtual bool isPowerOn() const = 0;
    virtual double getPropagationDelay() const = 0;
};

#endif // COMPONENT_BASE_H


