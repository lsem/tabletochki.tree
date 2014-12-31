#pragma once

#include "Utils.h"


enum IOMode
{
    IM__BEGIN,

    IM_INPUT = IM__BEGIN,
    IM_OUTPUT,
    IM_INPUTPULLUP,

    IM__END,
};

class IInputOutputController
{
public:
    virtual void InputOutputController_ConfigurePin(uint8_t pinNumber, IOMode mode) = 0;
    virtual void InputOutputController_ReadPinData(uint8_t pinNumber, uint8_t &out_value) = 0;
    virtual void InputOutputController_WritePinData(uint8_t pinNumber, uint8_t value) = 0;
    virtual void InputOutputController_ReadAnalog(uint8_t pinNumber, uint16_t &out_value) = 0;
    virtual void InputOutputController_ReadPulse(uint8_t pinNumber, uint8_t valueToRead, uint16_t &out_value, unsigned timeout) = 0;
};

#ifndef TESTS
class ArduinoStdIOController : public IInputOutputController
{
public:
    virtual void InputOutputController_ConfigurePin(uint8_t pinNumber, IOMode mode);
    virtual void InputOutputController_ReadPinData(uint8_t pinNumber, uint8_t &out_value);
    virtual void InputOutputController_WritePinData(uint8_t pinNumber, uint8_t value);
    virtual void InputOutputController_ReadAnalog(uint8_t pinNumber, uint16_t &out_value);
    virtual void InputOutputController_ReadPulse(uint8_t pinNumber, uint8_t valueToRead, uint16_t &out_value, unsigned timeout);
};
#else
class IInputOutputControllerMock : public IInputOutputController
{
public:
    MOCK_METHOD2(InputOutputController_ConfigurePin, void(uint8_t pinNumber, IOMode mode));
    MOCK_METHOD2(InputOutputController_ReadPinData, void(uint8_t pinNumber, uint8_t &out_value));
    MOCK_METHOD2(InputOutputController_WritePinData, void(uint8_t pinNumber, uint8_t value));
    MOCK_METHOD2(InputOutputController_ReadAnalog, void(uint8_t pinNumber, uint16_t &out_value));
    MOCK_METHOD4(InputOutputController_ReadPulse, void(uint8_t pinNumber, uint8_t valueToRead, uint16_t &out_value, unsigned timeout));
};

#endif // #ifndef TESTS
