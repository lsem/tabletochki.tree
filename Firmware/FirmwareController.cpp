#include "CoreDefs.h"
#include "Dataconst.h"
#include "FirmwareController.h"


#ifndef TESTS



void FirmwareController::Initialize()
{
    m_serialInterface.SerialInterface_Initialize();
    m_watchManager.EnableWatchdog();
}

void FirmwareController::ProcessActions()
{
    CheckWatchDogs();
    ProcessInput();
}

void FirmwareController::CheckWatchDogs()
{
    m_watchManager.CheckExpired(); // will call packet processor if expired 
}

void FirmwareController::ProcessInput()
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
