#include "Global.h"

#include "CommunicationChannel.h"

#include <serialib.h>
#include <easylogging++.h>


SerialLibCommunicationChannel::SerialLibCommunicationChannel():
    m_serialLibInstance(new serialib())
{
}

bool SerialLibCommunicationChannel::Open(const char *comportDescriptor)
{
    bool anyFault = false;

    LOG(DEBUG) << "SerialLibCommunicationChannel: Opening the channel";

    const auto result = m_serialLibInstance->Open(comportDescriptor, 9600);
    if (result != 1)
    {
        anyFault = true;
    }

    return !anyFault;
}

void SerialLibCommunicationChannel::Close()
{
    LOG(DEBUG) << "SerialLibCommunicationChannel: Closing the channel";

    m_serialLibInstance->Close();
}

/*virtual */
bool SerialLibCommunicationChannel::CommunicationChannel_SerialWrite(const void *data, size_t dataSize)
{
    return DoSerialWrite(data, dataSize);
}
    
/*virtual */
bool SerialLibCommunicationChannel::CommunicationChannel_SerialRead(void *dataBuffer, size_t dataBufferSize, unsigned timeout)
{
    return DoSerialRead(dataBuffer, dataBufferSize, timeout);
}

bool SerialLibCommunicationChannel::DoSerialWrite(const void *data, size_t dataSize)
{
    const auto result = m_serialLibInstance->Write(data, dataSize);
    return result == 1;
}

bool SerialLibCommunicationChannel::DoSerialRead(void *dataBuffer, size_t dataBufferSize, unsigned timeout)
{
    const auto result = m_serialLibInstance->Read(dataBuffer, dataBufferSize, timeout);
    return result == 1;
}

