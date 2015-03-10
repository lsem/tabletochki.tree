#pragma once

#include "serialib.h"

class ICommunicationChannel
{
public:
    virtual bool CommunicationChannel_SerialWrite(const void *data, size_t dataSize) = 0;
    virtual bool CommunicationChannel_SerialRead(void *dataBuffer, size_t dataBufferSize, unsigned timeout) = 0;
};



class serialib;


class SerialLibCommunicationChannel : public ICommunicationChannel
{
public:
    SerialLibCommunicationChannel(const SerialLibCommunicationChannel &) =delete;
    void operator=(const SerialLibCommunicationChannel &) = delete;

public:
    SerialLibCommunicationChannel();
    bool Open(const char *comportDescriptor);
    void Close();

public:
    virtual bool CommunicationChannel_SerialWrite(const void *data, size_t dataSize) override;
    virtual bool CommunicationChannel_SerialRead(void *dataBuffer, size_t dataBufferSize, unsigned timeout) override;

private:
    bool DoSerialWrite(const void *data, size_t dataSize);
    bool DoSerialRead(void *dataBuffer, size_t dataBufferSize, unsigned timeout);
    
private:
    unique_ptr<serialib>    m_serialLibInstance;
};
