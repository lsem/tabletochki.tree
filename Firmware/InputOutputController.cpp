#include "Global.h"
#include "CoreDefs.h"
#include "InputOutputController.h"
#include "Utils.h"


#ifndef TESTS

STATIC_ASSERT(IM__END <= 8);
STATIC_MAP(IOModeMapping, IOMode, uint8_t, IM__BEGIN, IM__END)
{
    (uint8_t)INPUT,       // IM_INPUT,
    (uint8_t)OUTPUT,      // IM_OUTPUT,
    (uint8_t)INPUT_PULLUP  // IM_INPUTPULLUP,
};

/*virtual */
void ArduinoStdIOController::InputOutputController_ConfigurePin(uint8_t pinNumber, IOMode mode)
{
    const int selectedPinMode = IOModeMapping.GetMappedValue(mode);
    ::pinMode(pinNumber, selectedPinMode);
}

/*virtual */
void ArduinoStdIOController::InputOutputController_ReadPinData(uint8_t pinNumber, uint8_t &out_value)
{
    out_value = ::digitalRead(pinNumber);
}

/*virtual */
void ArduinoStdIOController::InputOutputController_WritePinData(uint8_t pinNumber, uint8_t value)
{
    ::digitalWrite(pinNumber, value);
}

/*virtual */
void ArduinoStdIOController::InputOutputController_ReadAnalog(uint8_t pinNumber, uint16_t &out_value)
{
    int value;
    value = ::analogRead(pinNumber);
    out_value = value;
}

/*virtual */
void ArduinoStdIOController::InputOutputController_ReadPulse(uint8_t pinNumber, uint8_t valueToRead, uint16_t &out_value, unsigned timeout)
{
    out_value = ::pulseIn(pinNumber, valueToRead, timeout);
}

#endif // #ifndef TESTS
