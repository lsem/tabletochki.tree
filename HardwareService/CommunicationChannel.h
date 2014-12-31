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
private:
    SerialLibCommunicationChannel(const SerialLibCommunicationChannel &);
    void operator=(const SerialLibCommunicationChannel &);

public:
    SerialLibCommunicationChannel();
    bool Open(const char *comportDescriptor);
    void Close();

public:
    virtual bool CommunicationChannel_SerialWrite(const void *data, size_t dataSize);
    virtual bool CommunicationChannel_SerialRead(void *dataBuffer, size_t dataBufferSize, unsigned timeout);

private:
    bool DoSerialWrite(const void *data, size_t dataSize);
    bool DoSerialRead(void *dataBuffer, size_t dataBufferSize, unsigned timeout);
    
private:
    unique_ptr<serialib>    m_serialLibInstance;
};