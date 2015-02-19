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
    m_deviceState(),
    m_serviceState(SS__DEFAULT),
    m_timerThreads(),
    m_timersIOService(),
    m_endlessWork(m_timersIOService),
    m_timers(),
    m_pumpStartTime(),
    m_pumpsState(),
    m_configuration()
{
    CreateTasksTimers();
    InitializeServiceState();
    InitializePumpsState();
}

void HardwareServiceImplementation::ApplyConfiguration(const string &jsonDocumentText)
{
    ServiceConfiguration newServiceConfiguration;

    if (ServiceConfigurationManager::LoadFromJsonString(jsonDocumentText, newServiceConfiguration))
    {
        if (ServiceConfigurationManager::SaveToJsonFile(Dataconst::ServiceConfigurationFilePath,
            jsonDocumentText))
        {
            LOG(INFO) << "New configuration applied";
        }
        else
        {
            LOG(ERROR) << "Failed saving configuration to file; last error: " << Utils::GetLastSystemErrorMessage();
        }

        SetServiceConfiguration(newServiceConfiguration);
        SetServiceState(SS_READY);
    }
    else
    {
        
    }
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
#pragma message("WARNING: Obsolete API")
}

void HardwareServiceImplementation::GetServiceStateJson(string &jsonDocumentReceiver)
{
    std::stringstream j;

    const auto serviceState = GetServiceState();
    const auto deviceState = GetDeviceState();
    const auto logicalTime = 0; // ... (TODO)
#pragma message("WARNING: LogicalTime is not implemented")
    const auto &inputPumpDescriptor = GetPumpDescriptorRef(PI_INPUTPUMP);
    const auto &outputPumpDescriptor = GetPumpDescriptorRef(PI_OUTPUTPUMP);
    const auto inputPumpStateName = PumpStateNames.GetMappedValue(inputPumpDescriptor.m_state);
    const auto outputPumpStateName = PumpStateNames.GetMappedValue(inputPumpDescriptor.m_state);

    j << "{ ";
        j << "\"general\": ";
        j << "{ ";
            j << "\"svcState\": " << serviceState << ",";
            j << "\"svcStateStr\": \"" << DecodeServiceState(serviceState) << "\", ";
            j << "\"deviceState\": " << deviceState << ", ";
            j << "\"deviceStateStr\": \"" << DecodeDeviceConnectionState(deviceState) << "\", ";
            j << "\"logicalTime\": " << logicalTime << "";
        j << "}, "; // general
        j << "\"pumps\": ";
        j << "{ ";
            j << "\"input\":";
            j << "{ ";
                j << "\"state\": " << inputPumpDescriptor.m_state << ",";
                j << "\"stateStr\": \"" << inputPumpStateName << "\",";
                j << "\"startTime\": " << inputPumpDescriptor.m_startTime << ",";
                j << "\"workingTime\": " << inputPumpDescriptor.m_workingTime << "";
            j << "}, "; // input
            j << "\"output\":";
            j << "{ ";
                j << "\"state\": " << outputPumpDescriptor.m_state << ",";
                j << "\"stateStr\": \"" << outputPumpStateName << "\",";
                j << "\"startTime\": " << outputPumpDescriptor.m_startTime << ",";
                j << "\"workingTime\": " << outputPumpDescriptor.m_workingTime << "";
            j << "} "; // output
        j << "} "; // pumps
    j << "} ";

    jsonDocumentReceiver = j.str();
}

void HardwareServiceImplementation::StartBackgroundTasks()
{
    for (unsigned timerIndex = ST__BEGIN; timerIndex != ST__END; ++timerIndex)
    {
        RestartTask(static_cast<ESERVICETIMERS> (timerIndex));
    }
}

void HardwareServiceImplementation::CreateTasksTimers()
{
    for (unsigned timerIndex = ST__BEGIN; timerIndex != ST__END; ++timerIndex)
    {
        m_timers[timerIndex] = std::make_shared<DeadlineTimer>(m_timersIOService);
    }
}

void HardwareServiceImplementation::DestroyTasksTimers()
{
    for (unsigned timerIndex = ST__BEGIN; timerIndex != ST__END; ++timerIndex)
    {
        m_timers[timerIndex].reset();
    }
}

void HardwareServiceImplementation::RestartTask(ESERVICETIMERS timerId)
{
    auto actionMethod = m_tasksDescriptors[timerId].action;
    auto timeout = m_tasksDescriptors[timerId].timeout;
    auto &timerObject = GetTimerObjectById(timerId);

    timerObject.expires_from_now(boost::posix_time::milliseconds(timeout));
    timerObject.async_wait(std::bind(actionMethod, this));
}


void HardwareServiceImplementation::HeartBeatTask()
{
    LOG(DEBUG) << "[HEARTBEAT] Activated";

    DoHeartBeatTask();

    RestartTask(ST_HEARTBEATTIMER);
}

void HardwareServiceImplementation::StatusTask()
{
    LOG(DEBUG) << "[STATUS] Activated";

    DoQueryInputTask();

    RestartTask(ST_STATUSTIMER);
}

void HardwareServiceImplementation::InputPumpControlTask()
{
    LOG(DEBUG) << "[INPUT CONTROL PUMP] Activated";

    RestartTask(ST_INPUTPUMPTIMER);
}

void HardwareServiceImplementation::OutputPumpControlTask()
{
    LOG(DEBUG) << "[OUTPUT CONTROL PUMP] Activated";

    RestartTask(ST_OUTPUTPUMPTIMER);
}


void HardwareServiceImplementation::DoHeartBeatTask()
{
    uint32_t deviceStatus;

    const auto currentState = GetDeviceState();

    if (SendHeartbeatCommand(deviceStatus))
    {
        if (currentState == CS_READY)
        {
            if (deviceStatus == PDS_UNCONFIGURED)
            {
                LOG(INFO) << "Seems like device was restarted; forcing reconfiguration";
                SetDeviceState(CS_CONNECTED);
            }
            else
            {
                // Do nothing
            }
        }
        else if (currentState == CS_CONNECTED)
        {
            if (DoConfigureDevice())
            {
                SetDeviceState(CS_READY);

                LOG(INFO) << "Device changed state to ready";
            }
        }
        else if (currentState == CS_DISCONNECTED)
        {
            if (DoCheckDeviceConnection())
            {
                SetDeviceState(CS_CONNECTED);

                LOG(INFO) << "Device changed state to connected";
            }
        }
    }
    else
    {
        SetDeviceState(CS_DISCONNECTED);

        if (currentState != CS_DISCONNECTED)
        {
            LOG(INFO) << "Device changed state to disconnected";
        }
    }
}

void HardwareServiceImplementation::DoQueryStatusTask()
{
    
    Packets::Templates::ConfigureIORequest<1> configureIORequest;

}

void HardwareServiceImplementation::DoQueryInputTask()
{
    //if (IsDeviceConnected())
    //{
    //    Packets::Templates::ConfigureIORequest<1> configureIORequest;
    //    configureIORequest.Pins[0].Assign(0, PF_INPUTPULLUP | PF_ANALOG, 0, 10);
    //    Packets::Templates::ConfigureIOResponse configureResponse;
    //    if (SendPacketData(&configureIORequest, sizeof(configureIORequest), &configureResponse, sizeof(configureResponse)))
    //    {
    //        if (configureResponse.Status.Status == EC_OK)
    //        {
    //            
    //            Packets::Templates::ReadIOCommandRequest<1> readIORequest;
    //            readIORequest.Pins[0].PinNumber = 0;
    //            Packets::Templates::ReadIOCommandResponse<1> readIOResponse;
    //            std::memset(&readIOResponse, 0, sizeof(readIOResponse));

    //            if (SendPacketData(&readIORequest, sizeof(readIORequest), &readIOResponse, sizeof(readIOResponse)))
    //            {
    //                //LOG(INFO) << "VALUE: " << readIOResponse.Pins[0].Value;
    //                
    //                std::printf("\r                            \rDISTANCE: %d", readIOResponse.Pins[0].Value);
    //            }
    //        }
    //    }
    //}
    ////else
    //{
    //    LOG(ERROR) << "WaterLevelTask: Device is not connected";
    //}
}

bool HardwareServiceImplementation::DoConfigureDevice()
{
    bool result = false;

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
        if (configureResponse.Status.OperationResultCode == EC_OK)
        {
            result = true;
        }
    }

    return result;
}

bool HardwareServiceImplementation::DoCheckDeviceConnection()
{
    bool result = false;

    Packets::Command heartbeatCommand(CMD_HEARTBEAT);

    Packets::Output::StatusResponse response(nullptr);

    if (SendPacketData(&heartbeatCommand, sizeof(heartbeatCommand), &response, sizeof(response)))
    {
        // TODO: check response.Id to detect device restarts (whether need to be reconfigured)
#pragma message("WARNING: Unclosed TODO")
        result = true;
    }

    return result;
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
            LOG(ERROR) << "Failed opening device port. Error: '" << Utils::GetLastSystemErrorMessage() << "'";
            break;
        }

        LOG(DEBUG) << "Communication channel opened";

        portOpened = true;

        if (!UnsafeSendPacket((const uint8_t*)packetData, packetDataSize))
        {
            LOG(ERROR) << "Failed sending packet to the device. Error: '" << Utils::GetLastSystemErrorMessage() << "'";
            break;
        }

        LOG(DEBUG) << "Data sent; receiving";

        size_t receivedPacketSize;
        if (!UnsafeReceivePacket((uint8_t*)buffer, packetSize, receivedPacketSize, Dataconst::CommandSendReceiveTimeout))
        {
            LOG(ERROR) << "Failed receiving response packet. Error: '" << Utils::GetLastSystemErrorMessage() << "'";

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

bool HardwareServiceImplementation::SendHeartbeatCommand(unsigned &out_deviceStatus)
{
    bool result = false;

    Packets::Command heartbeatCommand(CMD_HEARTBEAT);
    Packets::Output::StatusResponse response(nullptr);

    if (SendPacketData(&heartbeatCommand, sizeof(heartbeatCommand), &response, sizeof(response)))
    {
        if (response.OperationResultCode == EC_OK)
        {
            out_deviceStatus = response.DeviceStatus;
            result = true;
        }
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

void HardwareServiceImplementation::InitializePumpsState()
{
    for (unsigned pumpIndex = 0; pumpIndex != PI__END; ++pumpIndex)
    {
        m_pumpsState[pumpIndex].Assign(PS__DEFAULT, 0, 0);
    }
}

void HardwareServiceImplementation::InitializeServiceState()
{
    SetServiceState(SS__DEFAULT);
    SetDeviceState(CS__DEFAULT);
}

void HardwareServiceImplementation::ResetUnframerState()
{
    m_unframer.Reset();
}


string HardwareServiceImplementation::DecodeServiceState(ESERVICESTATE code) const
{ 
    const auto result = ServiceStateNames.GetMappedValue(code);
    return result;
}

string HardwareServiceImplementation::DecodeDeviceConnectionState(ECONNECTIONSTATE code) const 
{ 
    const auto result = DeviceStateNames.GetMappedValue(code);
    return result;
}


void HardwareServiceImplementation::CreateTimerThreads()
{
    for (unsigned i = 0; i != Dataconst::TimerThreadPoolSize; ++i)
    {
        m_timerThreads.create_thread(boost::bind(&boost::asio::io_service::run, &m_timersIOService));
    }
}

void HardwareServiceImplementation::StartService()
{
    LoadConfiguration();
    CreateTimerThreads();
    StartBackgroundTasks();
}

void HardwareServiceImplementation::ShutdownService()
{
    m_timersIOService.stop();
    m_timerThreads.join_all();
}

void HardwareServiceImplementation::LoadConfiguration()
{
    ServiceConfiguration loadedConfiguration;
    
    if (ServiceConfigurationManager::LoadFromJsonFile(Dataconst::ServiceConfigurationFilePath, /*out*/loadedConfiguration))
    {
        SetServiceConfiguration(loadedConfiguration);
        SetServiceState(SS_READY);
    }
    else
    {
        SetServiceState(SS_SERVICENOTCONFIGURED);
    }
}
