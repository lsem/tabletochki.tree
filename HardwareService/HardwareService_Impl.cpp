#include "Global.h"

#include "CommonDefs.h"

#include "HardwareService_Impl.h"
#include "CommunicationChannel.h"
#include "IntervalTimer.h"
#include "PacketDefs.h"
#include "PacketParserTypes.h"
#include "PacketFramer.h"
#include "DataConst.h"


#include <easylogging++.h>



WateringServiceImplementation::WateringServiceImplementation() :
    m_communicationLock(),
    m_currentTask(),
    m_communicationChannel(),
    m_frameBuffer(),
    m_frameBufferSize(0),
    m_unframerInstnace(this),
    m_connectionState(),
    m_timerThreads(),
    m_timersIOService(),
    m_endlessWork(m_timersIOService),
    m_heartBeatTimer(m_timersIOService),
    m_queryInputTimer(m_timersIOService),
    m_pumpsControlTimers(PI__END, std::make_shared<boost::asio::deadline_timer>(m_timersIOService))
{
}

void WateringServiceImplementation::Configure(const Configuration& configuration)
{
}

void WateringServiceImplementation::Pour(const Container::type from, const Container::type to)
{
    try
    {
        PourBegin();
        SchedulePourEnd(10000);
    }
    catch (const std::runtime_error &e)
    {
        Tabletochki::InvalidOperation exception;
        exception.what = Tabletochki::ErrorCode::DEVICE_ALREADY_IN_USE;
        exception.why = e.what();
        throw exception;
    }
}

void WateringServiceImplementation::GetInput(HardwareInput& _return)
{

}

void WateringServiceImplementation::StartPump(const int32_t pumpId)
{
    //DoEnablePump(pumpId);
}

void WateringServiceImplementation::StopPump(StopPumpResult& _return, const int32_t pumpId)
{
    //DoDisablePump(pumpId);

    // gather statistic 

    _return.workingTimeSecond = 10;
}

void WateringServiceImplementation::StartBackgroundTasks()
{
    RestartHeartBeatTask();
    RestartQueryInputTask();
}


void WateringServiceImplementation::RestartHeartBeatTask()
{
    m_heartBeatTimer.expires_from_now(boost::posix_time::milliseconds(Dataconst::HearBeatCommandPeriodMs));
    m_heartBeatTimer.async_wait(std::bind(&WateringServiceImplementation::HeartBeatTask, this));
}

void WateringServiceImplementation::RestartQueryInputTask()
{
    m_queryInputTimer.expires_from_now(boost::posix_time::milliseconds(Dataconst::QueryInputTaskPeriodMs));
    m_queryInputTimer.async_wait(std::bind(&WateringServiceImplementation::QueryInputTask, this));
}

void WateringServiceImplementation::HeartBeatTask()
{
    RestartHeartBeatTask();
    DoHeartBeatTask();
}

void WateringServiceImplementation::QueryInputTask()
{
    RestartQueryInputTask();
    DoQueryInputTask();
}



void WateringServiceImplementation::DoHeartBeatTask()
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

void WateringServiceImplementation::DoQueryStatusTask()
{
    ScopedLock locked(GetCommunicationLock());
}

void WateringServiceImplementation::DoQueryInputTask()
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

void WateringServiceImplementation::ConfigureIODevice()
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

void WateringServiceImplementation::DoEnablePump()
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

void WateringServiceImplementation::DoDisablePump()
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

void WateringServiceImplementation::SchedulePourEnd(unsigned timeFromNow)
{
    auto &pumpControlTimer = GetPumpControlTimer(PI_PUMP0);

    pumpControlTimer.expires_from_now(boost::posix_time::milliseconds(timeFromNow));
    pumpControlTimer.async_wait(std::bind(&WateringServiceImplementation::PourEnd, this));
}

void WateringServiceImplementation::PourBegin()
{
    DoEnablePump();
}

void WateringServiceImplementation::PourEnd()
{
    DoDisablePump();
}


bool WateringServiceImplementation::SendPacketData(void *packetData, size_t packetDataSize, void *buffer, size_t packetSize)
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

bool WateringServiceImplementation::DoSendPacketData(void *packetData, size_t packetDataSize, void *buffer, size_t packetSize)
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
void WateringServiceImplementation::PacketUnFramerListener_OnCommandParsed(const uint8_t* packetBuffer, size_t packetSize)
{
    if (packetSize <= sizeof(m_frameBuffer))
    {
        std::copy(packetBuffer, packetBuffer + packetSize, m_frameBuffer);
        m_frameBufferSize = packetSize;
    }
}


bool WateringServiceImplementation::UnsafeReceivePacket(uint8_t *buffer, const size_t size,  size_t &out_size, unsigned timeoutMilliseconds)
{
    bool anyFault = false;
    
    auto commChannel = GetCommunicationChannel();

    char bufferCharacter;

    while (m_frameBufferSize == 0)
    {
        if (commChannel->CommunicationChannel_SerialRead(&bufferCharacter, 1, timeoutMilliseconds))
        {
            m_unframerInstnace.ProcessInputBytes((const uint8_t *) &bufferCharacter, 1);
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

bool WateringServiceImplementation::UnsafeSendPacket(const uint8_t *packetData, size_t packetDataSize)
{
    const auto communicatioChannel = GetCommunicationChannel();
    return communicatioChannel->CommunicationChannel_SerialWrite(packetData, packetDataSize);
}

bool WateringServiceImplementation::UnsafeResetCommunicationState()
{
    vector<uint8_t> resetSequence(Dataconst::PacketMaxSizeBytes, 0);
    const auto communicatioChannel = GetCommunicationChannel();
    return communicatioChannel->CommunicationChannel_SerialWrite(resetSequence.data(), resetSequence.size());
}

void WateringServiceImplementation::ResetUnframerState()
{
    m_unframerInstnace.Reset();
}


// TODO: move to something line SystemUtils or PlatformUtils
string WateringServiceImplementation::GetLastSystemErrorMessage()
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

void WateringServiceImplementation::SetDisconnectedState() 
{
    m_connectionState = ConnectionState::Disconnected;
}

void WateringServiceImplementation::SetConnectedState() 
{ 
    m_connectionState = ConnectionState::Connected;
}


void WateringServiceImplementation::CreateTimerThreads()
{
    for (unsigned i = 0; i != Dataconst::TimerThreadPoolSize; ++i)
        m_timerThreads.create_thread(boost::bind(&boost::asio::io_service::run, &m_timersIOService));
}

void WateringServiceImplementation::StartService()
{
    CreateTimerThreads();
    StartBackgroundTasks();
}

void WateringServiceImplementation::ShutdownService()
{
    m_timersIOService.stop();
    m_timerThreads.join_all();

}
