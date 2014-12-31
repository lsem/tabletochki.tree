#pragma once


class ISerialInterface
{
public:
    virtual size_t SerialInterface_BytesAvailable() = 0;
    virtual void SerialInterface_Initialize() = 0;
    virtual size_t SerialInterface_Write(const uint8_t *data, size_t length) = 0;
    virtual size_t SerialInterface_Read(uint8_t *dataBuffer, size_t length) = 0;
};


#ifndef TESTS
class ArduinoSerialInterface : public ISerialInterface
{
public:
    virtual size_t SerialInterface_BytesAvailable();
    virtual void SerialInterface_Initialize();
    virtual size_t SerialInterface_Write(const uint8_t *data, size_t length);
    virtual size_t SerialInterface_Read(uint8_t *dataBuffer, size_t length);
};
#else
class ArduinoSerialInterfaceMock : public ISerialInterface
{
public:
    MOCK_METHOD0(SerialInterface_BytesAvailable, size_t());
    MOCK_METHOD0(SerialInterface_Initialize, void());

    MOCK_METHOD2(SerialInterface_Write, size_t(const uint8_t *data, size_t length));
    MOCK_METHOD2(SerialInterface_Read, size_t(uint8_t *dataBuffer, size_t length));
};


#endif // #ifndef TESTS
