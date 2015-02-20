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
#include "DeviceIOMapping.h"
#include "ThriftHelpers.h"

#include <easylogging++.h>


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
    m_deviceInput(NULL),
    m_timers(),
    m_pumpsState(),
    m_configuration()
{
    CreateTasksTimers();
    InitializeServiceState();
    InitializePumpsState();
}

HardwareServiceImplementation::~HardwareServiceImplementation()
{
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
        LOG(ERROR) << "Failed applying configuration";
    }
}

void HardwareServiceImplementation::StartPump(const PumpIdentifier::type pumpId)
{
    LOG(INFO) << "Start pump: " << pumpId;

    auto &pumpDescriptor = GetPumpDescriptorRef((EPUMPIDENTIFIER)pumpId);
    pumpDescriptor.m_startTime = boost::chrono::steady_clock::now();
}

void HardwareServiceImplementation::StopPump(StopPumpResult& _return, const PumpIdentifier::type pumpId)
{
    LOG(INFO) << "Stop pump: " << pumpId;

    const auto &pumpDescriptor = GetPumpDescriptorRef((EPUMPIDENTIFIER)pumpId);
    const boost::chrono::duration<double> pumpWorkTime = boost::chrono::steady_clock::now() - pumpDescriptor.m_startTime;

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
    const auto &inputPumpDescriptor = GetPumpDescriptorRef(PI_INPUTPUMP);
    const auto &outputPumpDescriptor = GetPumpDescriptorRef(PI_OUTPUTPUMP);
    const auto inputPumpStateName = DecodePumpState(inputPumpDescriptor.m_state);
    const auto outputPumpStateName = DecodePumpState(inputPumpDescriptor.m_state);
    const auto inputValues = GetDeviceInputValues();

#pragma region ResultinJsonFormatting
    j << "{ ";
        j << "\"general\": ";
        j << "{ ";
            j << "\"svcState\": " << serviceState << ",";
            j << "\"svcStateStr\": \"" << DecodeServiceState(serviceState) << "\", ";
            j << "\"deviceState\": " << deviceState << ", ";
            j << "\"deviceStateStr\": \"" << DecodeDeviceConnectionState(deviceState) << "\"";
        j << "}, "; // general
        j << "\"pumps\": ";
        j << "{ ";
            j << "\"input\":";
            j << "{ ";
                j << "\"state\": " << inputPumpDescriptor.m_state << ",";
                j << "\"stateStr\": \"" << inputPumpStateName << "\",";
                j << "\"startTime\": " << inputPumpDescriptor.m_startTime.time_since_epoch().count() << ",";
                j << "\"workingTime\": " << inputPumpDescriptor.m_workingTime << "";
            j << "}, "; // input
            j << "\"output\":";
            j << "{ ";
                j << "\"state\": " << outputPumpDescriptor.m_state << ",";
                j << "\"stateStr\": \"" << outputPumpStateName << "\",";
                j << "\"startTime\": " << outputPumpDescriptor.m_startTime.time_since_epoch().count() << ",";
                j << "\"workingTime\": " << outputPumpDescriptor.m_workingTime << "";
            j << "} "; // output
        j << "}, "; // pumps
        j << "\"input\":";
        j << "{ ";
            j << "\"proximitySensor\": " << inputValues.ProximitySensorValue << ",";
            j << "\"magicButton\": " << inputValues.MagicButtonPressed << "";
        j << "} "; // input
    j << "} ";
#pragma endregion Resultin Json Formatting

    jsonDocumentReceiver = j.str();
}

void HardwareServiceImplementation::FillVisibleContainerMillilitres(const int32_t amount)
{
    EnsurePumpReadyForWork_RaiseExceptionIfNot(PI_INPUTPUMP);
    EnablePumpForSpecifiedTime(PI_INPUTPUMP, 1000);
}

void HardwareServiceImplementation::EmptyVisiableContainerMillilitres(const int32_t amount)
{
    EnsurePumpReadyForWork_RaiseExceptionIfNot(PI_OUTPUTPUMP);
    EnablePumpForSpecifiedTime(PI_OUTPUTPUMP, 1000);
}


void HardwareServiceImplementation::InitializePumpsState(EPUMPSTATE initialState/*=PS__DEFAULT*/)
{
    for (unsigned pumpIndex = PI__BEGIN; pumpIndex != PI__END; ++pumpIndex)
    {
        m_pumpsState[pumpIndex].Assign(initialState, time_point(), 0);
    }
}

void HardwareServiceImplementation::InitializeServiceState()
{
    SetServiceState(SS__DEFAULT);
    SetDeviceState(DS__DEFAULT);
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

void HardwareServiceImplementation::StartBackgroundTasks()
{
    for (unsigned timerIndex = TI__BEGIN; timerIndex != TI__END; ++timerIndex)
    {
        RestartTask(static_cast<ESERVICETASKID> (timerIndex));
    }
}

void HardwareServiceImplementation::CreateTasksTimers()
{
    for (unsigned timerIndex = TI__BEGIN; timerIndex != TI__END; ++timerIndex)
    {
        m_timers[timerIndex] = std::make_shared<DeadlineTimer>(m_timersIOService);
    }
}

void HardwareServiceImplementation::DestroyTasksTimers()
{
    for (unsigned timerIndex = TI__BEGIN; timerIndex != TI__END; ++timerIndex)
    {
        m_timers[timerIndex].reset();
    }
}

void HardwareServiceImplementation::RestartTask(ESERVICETASKID timerId)
{
    auto timeout = m_tasksDescriptors[timerId].timeout;
    RestartTaskTimeSpecified(timerId, timeout);
}

void HardwareServiceImplementation::RestartTaskTimeSpecified(ESERVICETASKID timerId, unsigned timeFromNowMilliseconds)
{
    auto actionMethod = m_tasksDescriptors[timerId].action;
    auto &timerObject = GetTimerObjectById(timerId);
    timerObject.expires_from_now(boost::posix_time::milliseconds(timeFromNowMilliseconds));
    timerObject.async_wait(std::bind(actionMethod, this));
}


void HardwareServiceImplementation::HeartBeatTask()
{
    LOG(DEBUG) << "[HEARTBEAT] Activated";

    DoHeartBeatTask();

    RestartTask(TI_HEARTBEATTASK);
}

void HardwareServiceImplementation::QueryInputTask()
{
    LOG(DEBUG) << "[STATUS] Activated";

    DoQueryInputTask();

    RestartTask(TI_STATUSTASK);
}

void HardwareServiceImplementation::InputPumpControlTask()
{
    LOG(INFO) << "[INPUT CONTROL PUMP] Activated";

    ProcessPumpControlActions(PI_INPUTPUMP);
}

void HardwareServiceImplementation::OutputPumpControlTask()
{
    LOG(DEBUG) << "[OUTPUT CONTROL PUMP] Activated";

    ProcessPumpControlActions(PI_OUTPUTPUMP);
}


void HardwareServiceImplementation::EnablePumpForSpecifiedTime(EPUMPIDENTIFIER pumpId, unsigned timeMilliseconds)
{
    auto &pumpDescriptor = GetPumpDescriptorRef(pumpId);

    pumpDescriptor.SetState(PS_ENABLED);
    pumpDescriptor.SetStartTimeNow();

    const auto taskId = DecodePumpTaskId(pumpId);
    RestartTaskTimeSpecified(taskId, timeMilliseconds);
}

void HardwareServiceImplementation::EnsurePumpReadyForWork_RaiseExceptionIfNot(EPUMPIDENTIFIER pumpId)
{
    const auto serviceState = GetServiceState();

    if (serviceState != SS_READY)
    {
        RaiseInvalidOperationException(Tabletochki::ErrorCode::SERVICE_NOT_READY,
            "Actual state is: " + DecodeServiceState(serviceState));
    }

    const auto deviceState = GetDeviceState();

    if (deviceState != DS_READY)
    {
        RaiseInvalidOperationException(Tabletochki::ErrorCode::DEVICE_NOT_READY,
            "Actual state is: " + DecodeDeviceConnectionState(deviceState));
    }

    const auto &pumpDescriptor = GetPumpDescriptorRef(pumpId);

    if (pumpDescriptor.GetState() != PS_DISABLED)
    {
        RaiseInvalidOperationException(Tabletochki::ErrorCode::PUMP_NOT_READY,
            "Actual state is: " + DecodePumpState(pumpDescriptor.m_state));
    }
}

void HardwareServiceImplementation::ProcessPumpControlActions(EPUMPIDENTIFIER pumpId)
{
    auto &pumpDescriptor = GetPumpDescriptorRef(pumpId);
    if (pumpDescriptor.GetState() == PS_ENABLED)
    {
        if (DoDisablePump(pumpId))
        {
            pumpDescriptor.SetState(PS_DISABLED);
        }
        else
        {
            LOG(ERROR) << "Failed disabling pump: " << DecodePumpIdentifierName(pumpId);
            EmergencyStop();
        }
    }
}

void HardwareServiceImplementation::EmergencyStop()
{
#pragma message("WARNING:  Emergeny stop is not implemented")
    // ...
}


void HardwareServiceImplementation::DoHeartBeatTask()
{
    uint32_t deviceStatus;

    const auto currentState = GetDeviceState();

    if (SendHeartbeatCommand(deviceStatus))
    {
        if (currentState == DS_READY)
        {
            if (deviceStatus == PDS_UNCONFIGURED)
            {
                LOG(INFO) << "Seems like device was restarted; forcing reconfiguration";
                SetDeviceState(DS_CONNECTED);
            }
            else
            {
                // Do nothing
            }
        }
        else if (currentState == DS_CONNECTED)
        {
            if (SendConfigureDeviceCommand())
            {
                SetDeviceState(DS_READY);
                InitializePumpsState(PS_DISABLED);

                LOG(INFO) << "Device changed state to ready";
            }
        }
        else if (currentState == DS_DISCONNECTED)
        {
            if (DoCheckDeviceConnection())
            {
                SetDeviceState(DS_CONNECTED);

                LOG(INFO) << "Device changed state to connected";
            }
        }
    }
    else
    {
        SetDeviceState(DS_DISCONNECTED);

        if (currentState != DS_DISCONNECTED)
        {
            LOG(INFO) << "Device changed state to disconnected";
        }
    }
}

void HardwareServiceImplementation::DoQueryInputTask()
{
    const auto deviceState = GetDeviceState();

    if (deviceState == DS_READY)
    {
        DeviceInputValues deviceInput;

        if (SendReadIOCommand(deviceInput))
        {
            //LOG(INFO) << "MagicButton pressed: " << (deviceInput.MagicButtonPressed ? "true" : "false");
            //LOG(INFO) << "Proximity sensor value: " << deviceInput.ProximitySensorValue;

            SetDeviceInputValues(deviceInput);
        }
    }
}

bool HardwareServiceImplementation::DoCheckDeviceConnection()
{
    bool result = false;

    Packets::Command heartbeatCommand(CMD_HEARTBEAT);

    Packets::Output::StatusResponse response(nullptr);

    if (SendPacketData(&heartbeatCommand, sizeof(heartbeatCommand), &response, sizeof(response)))
    {
        result = true;
    }

    return result;
}

bool HardwareServiceImplementation::DoEnablePump(EPUMPIDENTIFIER pumpId)
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

    return false;
}

bool HardwareServiceImplementation::DoDisablePump(EPUMPIDENTIFIER pumpId)
{
    // TODO: Read IO pins from configuration

    Packets::Templates::WriteIOCommandRequest<2> outputRequest;
    outputRequest.Pins[0].Assign(2, 0);
    outputRequest.Pins[1].Assign(5, 0);

    Packets::Templates::WriteIOCommandResponse outputResponse;
    if (SendPacketData(&outputRequest, sizeof(outputRequest), &outputResponse, sizeof(outputResponse)))
    {
        // ...
    }

    return false;
}


bool HardwareServiceImplementation::SendHeartbeatCommand(unsigned &out_deviceStatus)
{
    bool result = false;

    Packets::Command heartbeatCommand(CMD_HEARTBEAT);
    Packets::Output::StatusResponse response(nullptr);

    if (SendPacket(heartbeatCommand, response))
    {
        if (response.OperationResultCode == EC_OK)
        {
            out_deviceStatus = response.DeviceStatus;
            result = true;
        }
    }

    return result;
}

bool HardwareServiceImplementation::SendReadIOCommand(DeviceInputValues &out_deviceInput)
{
    bool result = false;
    
    unsigned requestPinWritten = 0;

    Packets::Templates::ReadIOCommandRequest<INPUT_PINS_COUNT> readIORequest;

    readIORequest.Pins[requestPinWritten].PinNumber = PROXIMITYSENSOR_PINNUMBER;
    ++requestPinWritten;
    readIORequest.Pins[requestPinWritten].PinNumber = MAGICBUTTON_PINNUMBER;
    ++requestPinWritten;
    
    Packets::Templates::ReadIOCommandResponse<INPUT_PINS_COUNT> readIOResponse;
    std::memset(&readIOResponse, 0, sizeof(readIOResponse));

    if (SendPacket(readIORequest, readIOResponse))
    {
        if (readIOResponse.Header.OperationResultCode == EC_OK)
        {
            unsigned responsePinRead = 0;

            out_deviceInput.ProximitySensorValue = readIOResponse.Pins[responsePinRead].Value;
            ASSERT(readIOResponse.Pins[responsePinRead].PinNumber == PROXIMITYSENSOR_PINNUMBER);
            ++responsePinRead;

            out_deviceInput.MagicButtonPressed = readIOResponse.Pins[responsePinRead].Value;
            ASSERT(readIOResponse.Pins[responsePinRead].PinNumber == MAGICBUTTON_PINNUMBER);
            ++responsePinRead;

            // ...

            ASSERT(requestPinWritten == responsePinRead && responsePinRead == INPUT_PINS_COUNT);

            result = true;
        }
        else
        {
            LOG(ERROR) << "Failed getting device input state. Device returned: " << ((unsigned)readIOResponse.Header.OperationResultCode) << ". " <<
                                                             "Device status: " << ((unsigned)readIOResponse.Header.DeviceStatus);
        }
    }

    return result;
}

bool HardwareServiceImplementation::SendConfigureDeviceCommand()
{
    bool result = false;

    Packets::Templates::ConfigureIORequest<INPUT_PINS_COUNT + OUTPUT_PINS_COUNT> configureIORequest;

    size_t pinNumber = 0;

    configureIORequest.Pins[pinNumber].Assign(INPUTPUMP_PINNUMBER, INPUTPUMP_FLAGS, INPUTPUMP_DEFVALUE, INPUTPUMP_SPECDATA);
    ++pinNumber;
    configureIORequest.Pins[pinNumber].Assign(OUTPUTPUMP_PINNUMBER, OUTPUTPUMP_FLAGS, OUTPUTPUMP_DEFVALUE, OUTPUTPUMP_SPECDATA);
    ++pinNumber;
    configureIORequest.Pins[pinNumber].Assign(PROXIMITYSENSOR_PINNUMBER, PROXIMITYSENSOR_FLAGS, PROXIMITYSENSOR_DEFVALUE, PROXIMITYSENSOR_SPECDATA);
    ++pinNumber;
    configureIORequest.Pins[pinNumber].Assign(MAGICBUTTON_PINNUMBER, MAGICBUTTON_FLAGS, MAGICBUTTON_DEFVALUE, MAGICBUTTON_SPECDATA);
    ++pinNumber;

    assert(pinNumber == configureIORequest.PinsCount);

    Packets::Templates::ConfigureIOResponse configureResponse;
    if (SendPacket(configureIORequest, configureResponse))
    {
        if (configureResponse.Status.OperationResultCode == EC_OK)
        {
            result = true;
        }
        else
        {
            LOG(ERROR) << "Failed to configure device. Device respond with: " << (unsigned) configureResponse.Status.OperationResultCode;
        }
    }

    return result;
}


template <class TRequest, class TResponse>
bool HardwareServiceImplementation::SendPacket(const TRequest &request, TResponse &out_response)
{
    return SendPacketData(&request, sizeof(request), &out_response, sizeof(out_response));
}

bool HardwareServiceImplementation::SendPacketData(const void *packetData, size_t packetDataSize, void *buffer, size_t packetSize)
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

bool HardwareServiceImplementation::DoSendPacketData(const void *packetData, size_t packetDataSize, void *buffer, size_t packetSize)
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
            if (UnsafeResetCommunicationState())
            {
                LOG(ERROR) << "Failed resetting communication state :(";
            }
            break;
        }

        LOG(DEBUG) << "Data received";

        result = true;
    } while (false);


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


/*static*/
string HardwareServiceImplementation::DecodeServiceState(ESERVICESTATE code)
{ 
    const auto result = ServiceStateNames.GetMappedValue(code);
    return result;
}

/*static*/
string HardwareServiceImplementation::DecodeDeviceConnectionState(ECONNECTIONSTATE code)
{ 
    const auto result = DeviceStateNames.GetMappedValue(code);
    return result;
}

/*static */
string HardwareServiceImplementation::DecodePumpState(EPUMPSTATE code)
{
    const auto result = PumpStateNames.GetMappedValue(code);
    return result;
}

/*static */
string HardwareServiceImplementation::DecodePumpIdentifierName(EPUMPIDENTIFIER code)
{
    const auto result = PumpIdentifierNames.GetMappedValue(code);
    return result;
}

/*static */
ESERVICETASKID HardwareServiceImplementation::DecodePumpTaskId(EPUMPIDENTIFIER code)
{
    const auto result = PumpsTasksIdentifiers.GetMappedValue(code);
    return result;
}


const HardwareServiceImplementation::TableEntry HardwareServiceImplementation::m_tasksDescriptors[] =
{
    { &HardwareServiceImplementation::HeartBeatTask, Dataconst::HearBeatCommandPeriodMs },      // ST_HEARTBEATTIMER
    { &HardwareServiceImplementation::QueryInputTask, Dataconst::QueryInputTaskPeriodMs },       // ST_STATUSTIMER
    { &HardwareServiceImplementation::InputPumpControlTask, Dataconst::InputPipeTaskPeriodMs },        // ST_INPUTPUMPTIMER
    { &HardwareServiceImplementation::OutputPumpControlTask, Dataconst::OutputPipeTaskPeriodMs }        // ST_OUTPUTPUMPTIMER
};
