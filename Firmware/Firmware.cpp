#include "CoreDefs.h"
#include "Dataconst.h"
#include "Firmware.h"


#ifndef TESTS


void Firmware::Initialize()
{
    m_serialInterface.SerialInterface_Initialize();
}

void Firmware::ProcessActions()
{
    size_t bytesAvailable = m_serialInterface.SerialInterface_BytesAvailable();
    if (bytesAvailable > 0)
    {
        const size_t bufferSize = bytesAvailable > Dataconst::InputBufferSize ? 
            Dataconst::InputBufferSize : bytesAvailable;

        char *buffer = (char *) ::alloca(bufferSize);

        m_serialInterface.SerialInterface_Read((uint8_t*)buffer, bufferSize);
        m_unframer.ProcessInputBytes((const uint8_t*)buffer, bufferSize);
    }
}

#endif // TESTS