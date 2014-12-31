#include "CoreDefs.h"
#include "SerialInterface.h"


#ifndef TESTS

/*virtual */
size_t ArduinoSerialInterface::SerialInterface_BytesAvailable()
{
    return ::Serial.available();
}

/*virtual */
void ArduinoSerialInterface::SerialInterface_Initialize()
{
    ::Serial.begin(9600); 
}

/*virtual */
size_t ArduinoSerialInterface::SerialInterface_Write(const uint8_t *data, size_t length)
{
    size_t written = ::Serial.write(data, length);
    ::Serial.flush();
    return written; 
}

/*virtual */
size_t ArduinoSerialInterface::SerialInterface_Read(uint8_t *dataBuffer, size_t length)
{
    return ::Serial.readBytes(reinterpret_cast<char*>(dataBuffer), length);
}

#endif // TESTS
