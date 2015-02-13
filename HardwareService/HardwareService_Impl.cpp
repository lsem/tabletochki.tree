#include "Global.h"
#include "CoreDefs.h"
#include "CommonDefs.h"

#include "HardwareService_Impl.h"
#include "CommunicationChannel.h"
#include "IntervalTimer.h"
#include "PacketDefs.h"
#include "PacketParserTypes.h"
#include "PacketFramer.h"
#include "DataConst.h"
#include "ServiceConfiguration.h"


#include <easylogging++.h>


const HardwareServiceImplementation::TableEntry HardwareServiceImplementation::m_tasksDescriptors[] = 
{
    { &HardwareServiceImplementation::HeartBeatTask,         Dataconst::HearBeatCommandPeriodMs },     // ST_HEARTBEATTIMER
    { &HardwareServiceImplementation::StatusTask,            Dataconst::StatusTaskPeriodMs },          // ST_STATUSTIMER
    { &HardwareServiceImplementation::InputPumpControlTask,  Dataconst::InputPipeTaskPeriodMs },       // ST_INPUTPUMPTIMER
    { &HardwareServiceImplementation::OutputPumpControlTask, Dataconst::OutputPipeTaskPeriodMs }       // ST_OUTPUTPUMPTIMER
};


HardwareServiceImplementation::HardwareServiceImplementation() :
    m_communicationLock(),
    m_currentTask(),
    m_communicationChannel(),
    m_frameBuffer(),
    m_frameBufferSize(0),
    m_unframer(this),
    m_connectionState(),
    m_timerThreads(),
    m_timersIOService(),
    m_endlessWork(m_timersIOService),
    m_timers(),
    m_pumpStartTime()
{
    CreateTasksTimers();
}

void HardwareServiceImplementation::Configure(const Configuration& configuration)
{
}

void HardwareServiceImplementation::Pour(const Container::type from, const Container::type to)
{
    //try
    //{
    //    PourBegin();
    //    SchedulePourEnd(10000);
    //}
    //catch (const std::runtime_error &e)
    //{
    //    Tabletochki::InvalidOperation exception;
    //    exception.what = Tabletochki::ErrorCode::DEVICE_ALREADY_IN_USE;
    //    exception.why = e.what();
    //    throw exception;
    //}
}

void HardwareServiceImplementation::GetInput(HardwareInput& _return)
{

}

void HardwareServiceImplementation::StartPump(const int32_t pumpId)
{
    LOG(ERROR) << "HardwareServiceImplementation: PUMP ENABLED !!!!";

    m_pumpStartTime = boost::chrono::steady_clock::now();
}

void HardwareServiceImplementation::StopPump(StopPumpResult& _return, const int32_t pumpId)
{
    const boost::chrono::duration<double> pumpWorkTime = 
            boost::chrono::steady_clock::now() - m_pumpStartTime;

    _return.workingTimeSecond = (uint32_t)(pumpWorkTime.count() * 1000);
}

void HardwareServiceImplementation::GetServiceStatus(ServiceStatus& _return)
{
    _return.statusCode = 0;
}

void HardwareServiceImplementation::StartBackgroundTasks()
{
    for (unsigned uiTimerIndex = 0; uiTimerIndex != ST__END; ++uiTimerIndex)
    {
        RestartTask(static_cast<ESERVICETIMERS> (uiTimerIndex));
    }
}


void HardwareServiceImplementation::CreateTasksTimers()
{
    for (unsigned uiTimerIndex = 0; uiTimerIndex != ST__END; ++uiTimerIndex)
    {
        m_timers[uiTimerIndex] = std::make_shared<DeadlineTimer>(m_timersIOService);
    }
}

void HardwareServiceImplementation::DestroyTasksTimers()
{
    for (unsigned uiTimerIndex = 0; uiTimerIndex != ST__END; ++uiTimerIndex)
    {
        m_timers[uiTimerIndex].reset();
    }
}

void HardwareServiceImplementation::RestartTask(ESERVICETIMERS timerId)
{
    auto actionMethod = m_tasksDescriptors[timerId].action;
    auto timeout = m_tasksDescriptors[timerId].timeout;
    GetTimerObjectById(timerId).expires_from_now(boost::posix_time::milliseconds(timeout));
    GetTimerObjectById(timerId).async_wait(std::bind(actionMethod, this));
}


void HardwareServiceImplementation::HeartBeatTask()
{
    LOG(INFO) << "[HEARTBEAT] Activated";

    RestartTask(ST_HEARTBEATTIMER);
}

void HardwareServiceImplementation::StatusTask()
{
    LOG(INFO) << "[STATUS] Activated";

    RestartTask(ST_STATUSTIMER);
}

void HardwareServiceImplementation::InputPumpControlTask()
{
    LOG(INFO) << "[INPUT CONTROL PUMP] Activated";

    RestartTask(ST_INPUTPUMPTIMER);
}

void HardwareServiceImplementation::OutputPumpControlTask()
{
    LOG(INFO) << "[OUTPUT CONTROL PUMP] Activated";

    RestartTask(ST_OUTPUTPUMPTIMER);
}


void HardwareServiceImplementation::DoHeartBeatTask()
{
    Packets::Command heartbeatCommand(CMD_HEARTBEAT);
    Packets::Output::StatusResponse response(nullptr);

    if (SendPacketData(&heartbeatCommand, sizeof(heartbeatCommand), &response, sizeof(response)))
    {
        SetConnectedState();
    }
    else
    {
        SetDisconnectedState();
    }
}

void HardwareServiceImplementation::DoQueryStatusTask()
{
    ScopedLock locked(GetCommunicationLock());
}

void HardwareServiceImplementation::DoQueryInputTask()
{
    if (IsDeviceConnected())
    {
        Packets::Templates::ConfigureIORequest<1> configureIORequest;
        configureIORequest.Pins[0].Assign(0, PF_INPUTPULLUP | PF_ANALOG, 0, 10);
        Packets::Templates::ConfigureIOResponse configureResponse;
        if (SendPacketData(&configureIORequest, sizeof(configureIORequest), &configureResponse, sizeof(configureResponse)))
        {
            if (configureResponse.Status.Status == EC_OK)
            {
                
                Packets::Templates::ReadIOCommandRequest<1> readIORequest;
                readIORequest.Pins[0].PinNumber = 0;
                Packets::Templates::ReadIOCommandResponse<1> readIOResponse;
                std::memset(&readIOResponse, 0, sizeof(readIOResponse));

                if (SendPacketData(&readIORequest, sizeof(readIORequest), &readIOResponse, sizeof(readIOResponse)))
                {
                    //LOG(INFO) << "VALUE: " << readIOResponse.Pins[0].Value;
                    
                    std::printf("\r                            \rDISTANCE: %d", readIOResponse.Pins[0].Value);
                }
            }
        }
    }
    else
    {
        LOG(ERROR) << "WaterLevelTask: Device is not connected";
    }
}

void HardwareServiceImplementation::ConfigureIODevice()
{
    Packets::Templates::ConfigureIORequest<3> configureIORequest;

    size_t pinNumber = 0;

    configureIORequest.Pins[pinNumber].Assign(0, PF_INPUTPULLUP | PF_ANALOG, 0, 10);
    ++pinNumber;
    configureIORequest.Pins[pinNumber].Assign(2, PF_OUTPUT, 0, 0);
    ++pinNumber;
    configureIORequest.Pins[pinNumber].Assign(5, PF_OUTPUT, 0, 0);
    ++pinNumber;

    assert(pinNumber == configureIORequest.PinsCount);

    Packets::Templates::ConfigureIOResponse configureResponse;
    if (SendPacketData(&configureIORequest, sizeof(configureIORequest), &configureResponse, sizeof(configureResponse)))
    {
        if (configureResponse.Status.Status == EC_OK)
        {
            SetConnectedState();
        }
    }
}

void HardwareServiceImplementation::DoEnablePump()
{
    // TODO: Read IO pins from configuration

    Packets::Templates::WriteIOCommandRequest<2> outputRequest;
    outputRequest.Pins[0].Assign(2, 1);
    outputRequest.Pins[1].Assign(5, 1);

    Packets::Templates::WriteIOCommandResponse outputResponse;
    if (SendPacketData(&outputRequest, sizeof(outputRequest), &outputResponse, sizeof(outputResponse)))
    {
        std::printf("Pump enabled\n");
    }
}

void HardwareServiceImplementation::DoDisablePump()
{
    // TODO: Read IO pins from configuration

    Packets::Templates::WriteIOCommandRequest<2> outputRequest;
    outputRequest.Pins[0].Assign(2, 0);
    outputRequest.Pins[1].Assign(5, 0);

    Packets::Templates::WriteIOCommandResponse outputResponse;
    if (SendPacketData(&outputRequest, sizeof(outputRequest), &outputResponse, sizeof(outputResponse)))
    {
        std::printf("Pump disabled\n");
    }
}


bool HardwareServiceImplementation::SendPacketData(void *packetData, size_t packetDataSize, void *buffer, size_t packetSize)
{
    const auto sizeNeeded = PacketFramer::CalculatePacketTotalSize(packetDataSize);
    auto memory = ::alloca(sizeNeeded);

    size_t bytesWritten = 0;
    uint16_t checkSum;
    
    bytesWritten += PacketFramer::WriteFrameBegin((uint8_t*)memory + bytesWritten, packetDataSize);
    bytesWritten += PacketFramer::WriteFrameData((uint8_t*)memory + bytesWritten, (uint8_t*)packetData, packetDataSize, checkSum);
    bytesWritten += PacketFramer::WriteFrameEnd((uint8_t*)memory + bytesWritten, checkSum);

    const auto framedPacketSize = bytesWritten;
    
    bool result = DoSendPacketData(memory, framedPacketSize, buffer, packetSize);
    return result;
}

bool HardwareServiceImplementation::DoSendPacketData(void *packetData, size_t packetDataSize, void *buffer, size_t packetSize)
{
    bool result = false;

    LOG(DEBUG) << "Acquiring communication channel lock";

    auto communicationChannel = GetCommunicationChannel();

    ScopedLock locked(GetCommunicationLock());

    bool portOpened = false;

    do
    {
        LOG(DEBUG) << "Opening communication channel on COM4";

        

        if (!communicationChannel->Open("COM4"))
        {
            break;
        }

        LOG(DEBUG) << "Communication channel opened";

        portOpened = true;

        if (!UnsafeSendPacket((const uint8_t*)packetData, packetDataSize))
        {
            LOG(ERROR) << "Failed sending packet to the device. Error: '" << GetLastSystemErrorMessage() << "'";
            break;
        }

        LOG(DEBUG) << "Data sent; receiving";

        size_t receivedPacketSize;
        if (!UnsafeReceivePacket((uint8_t*)buffer, packetSize, receivedPacketSize, Dataconst::CommandSendReceiveTimeout))
        {
            LOG(ERROR) << "Failed receiving response packet. Error: '" << GetLastSystemErrorMessage() << "'";

            ResetUnframerState();
            UnsafeResetCommunicationState();
            break;
        }

        LOG(DEBUG) << "Data received";

        result = true;
    } 
    while (false);


    if (portOpened)
    {
        communicationChannel->Close();
    }

    return result;

}

/*virtual */
void HardwareServiceImplementation::PacketUnFramerListener_OnCommandParsed(const uint8_t* packetBuffer, size_t packetSize)
{
    if (packetSize <= sizeof(m_frameBuffer))
    {
        std::copy(packetBuffer, packetBuffer + packetSize, m_frameBuffer);
        m_frameBufferSize = packetSize;
    }
}


bool HardwareServiceImplementation::UnsafeReceivePacket(uint8_t *buffer, const size_t size,  size_t &out_size, unsigned timeoutMilliseconds)
{
    bool anyFault = false;
    
    auto commChannel = GetCommunicationChannel();

    char bufferCharacter;

    while (m_frameBufferSize == 0)
    {
        if (commChannel->CommunicationChannel_SerialRead(&bufferCharacter, 1, timeoutMilliseconds))
        {
            m_unframer.ProcessInputBytes((const uint8_t *) &bufferCharacter, 1);
        }
        else
        {
            anyFault = true;
            break;
        }
    }

    anyFault = anyFault || (size < m_frameBufferSize);

    if (!anyFault)
    {
        std::copy(m_frameBuffer, m_frameBuffer + m_frameBufferSize, buffer);
        out_size = m_frameBufferSize;
        m_frameBufferSize = 0;
    }

    return !anyFault;
}

bool HardwareServiceImplementation::UnsafeSendPacket(const uint8_t *packetData, size_t packetDataSize)
{
    const auto communicatioChannel = GetCommunicationChannel();
    return communicatioChannel->CommunicationChannel_SerialWrite(packetData, packetDataSize);
}

bool HardwareServiceImplementation::UnsafeResetCommunicationState()
{
    vector<uint8_t> resetSequence(Dataconst::PacketMaxSizeBytes, 0);
    const auto communicatioChannel = GetCommunicationChannel();
    return communicatioChannel->CommunicationChannel_SerialWrite(resetSequence.data(), resetSequence.size());
}

void HardwareServiceImplementation::ResetUnframerState()
{
    m_unframer.Reset();
}


// TODO: move to something line SystemUtils or PlatformUtils
string HardwareServiceImplementation::GetLastSystemErrorMessage()
{
    DWORD errorCode = GetLastError();
    
    if (errorCode)
    {
        LPVOID messageData;
        DWORD messageLength = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                     FORMAT_MESSAGE_FROM_SYSTEM |
                                     FORMAT_MESSAGE_IGNORE_INSERTS,
                                     NULL,
                                     errorCode,
                                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                     (LPTSTR)&messageData,
                                     0, NULL);

        if (messageLength > 0)
        {
            std::string result((LPCSTR)messageData, (LPCSTR)messageData + messageLength);
            ::LocalFree(messageData);
            return std::to_string(errorCode) + ": " + result;
        }
    }
    
    return std::string();
}

void HardwareServiceImplementation::SetDisconnectedState() 
{
    m_connectionState = ConnectionState::Disconnected;
}

void HardwareServiceImplementation::SetConnectedState() 
{ 
    m_connectionState = ConnectionState::Connected;
}


void HardwareServiceImplementation::CreateTimerThreads()
{
    for (unsigned i = 0; i != Dataconst::TimerThreadPoolSize; ++i)
        m_timerThreads.create_thread(boost::bind(&boost::asio::io_service::run, &m_timersIOService));
}

void HardwareServiceImplementation::StartService()
{
    CreateTimerThreads();
    StartBackgroundTasks();
}

void HardwareServiceImplementation::ShutdownService()
{
    m_timersIOService.stop();
    m_timerThreads.join_all();

}
