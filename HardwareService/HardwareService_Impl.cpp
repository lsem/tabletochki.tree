#include "Global.h"
#include "CoreDefs.h"
#include "CommonDefs.h"
#include "HardwareService_Impl.h"
#include "CommunicationChannel.h"
#include "PacketDefs.h"
#include "PacketParserTypes.h"
#include "PacketFramer.h"
#include "DataConst.h"
#include "ServiceConfiguration.h"
#include "ThriftHelpers.h"
#include <easylogging++.h>



HardwareServiceImplementation::HardwareServiceImplementation() :
    m_communicationLock(),
    m_communicationChannel(),
    m_frameBuffer(),
    m_frameBufferSize(0),
    m_unframer(this),
    m_deviceState(DS__DEFAULT),
    m_serviceState(SS__DEFAULT),
    m_timerThreads(),
    m_timersIOService(),
    m_endlessWork(m_timersIOService),
    m_deviceInput(NULL),
    m_timers(),
    m_pumpsState(),
//    m_tasksDescriptors(),
    m_configuration(),
    m_visibleContainerStateData(NULL),
    m_hiddenContainerStateData(NULL),
    m_levelPumpOutState(NULL),
    m_defferedInputList(),
    m_defferedInputListLock()
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

    auto &pumpDescriptor = GetPumpStateDescriptorRef((EPUMPIDENTIFIER)pumpId);
    pumpDescriptor.m_startTime = boost::chrono::steady_clock::now();

    if (ExecuteEnablePumpHardwareCommand((EPUMPIDENTIFIER)pumpId))
    {
        LOG(INFO) << "EnablePump command executed";
    }
    else
    {
        LOG(ERROR) << "Failed executing EnablePump command";
    }
}

void HardwareServiceImplementation::StopPump(StopPumpResult& _return, const PumpIdentifier::type pumpId)
{
    LOG(INFO) << "Stop pump: " << pumpId;

    const auto &pumpDescriptor = GetPumpStateDescriptorRef((EPUMPIDENTIFIER)pumpId);
    const boost::chrono::duration<double> pumpWorkTime = boost::chrono::steady_clock::now() - pumpDescriptor.m_startTime;

    _return.workingTimeSecond = (uint32_t)(pumpWorkTime.count() * 1000);

    if (ExecuteDisablePumpHardwareCommand((EPUMPIDENTIFIER)pumpId))
    {
        LOG(INFO) << "DisablePump command executed";
    }
    else
    {
        LOG(ERROR) << "Failed executing EnablePump command";
    }
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
    const auto &inputPumpDescriptor = GetPumpStateDescriptorRef(PI_INPUTPUMP);
    const auto &outputPumpDescriptor = GetPumpStateDescriptorRef(PI_OUTPUTPUMP);
    const auto inputPumpStateName = DecodePumpState(inputPumpDescriptor.m_state);
    const auto outputPumpStateName = DecodePumpState(outputPumpDescriptor.m_state);
    const auto inputValues = GetDeviceInputValues();

    const auto currentLevelPumpOutState = GetLevelPumpOutStateData().GetState();
    const auto currentLevelPumpOutStateStr = PumpingOutStateNames.GetMappedValue(currentLevelPumpOutState);

    size_t pendingInputTasksCount = 0;
    {
        ScopedLock locked(GetDefferedInputListLock());
        auto &defferedTasksList = GetDefferedInputList();
        pendingInputTasksCount = defferedTasksList.size();
    }

#pragma region ResultinJsonFormatting
    j << "{ ";
        j << "\"general\": ";
        j << "{ ";
            j << "\"pendingInputTasksCount\": " << pendingInputTasksCount << ",";
            j << "\"svcState\": " << serviceState << ",";
            j << "\"svcStateStr\": \"" << DecodeServiceState(serviceState) << "\", ";
            j << "\"deviceState\": " << deviceState << ", ";
            j << "\"deviceStateStr\": \"" << DecodeDeviceConnectionState(deviceState) << "\", ";
            j << "\"pumpingOutState\": \"" << currentLevelPumpOutState << "\", ";
            j << "\"pumpingOutStateStr\": \"" << currentLevelPumpOutStateStr << "\"";
        j << "}, "; // general
        j << "\"pumps\": ";
        j << "{ ";
            j << "\"input\":";
            j << "{ ";
                j << "\"state\": " << inputPumpDescriptor.m_state << ",";
                j << "\"stateStr\": \"" << inputPumpStateName << "\",";
                j << "\"startTime\": " << inputPumpDescriptor.m_startTime.time_since_epoch().count();
            j << "}, "; // input
            j << "\"output\":";
            j << "{ ";
                j << "\"state\": " << outputPumpDescriptor.m_state << ",";
                j << "\"stateStr\": \"" << outputPumpStateName << "\",";
                j << "\"startTime\": " << outputPumpDescriptor.m_startTime.time_since_epoch().count();
            j << "} "; // output
        j << "}, "; // pumps
        j << "\"input\":";
        j << "{ ";
            j << "\"proximitySensor\": " << inputValues.m_proximitySensorValue << ","; 
            j << "\"magicButton\": " << inputValues.m_magicButtonPressed << ", ";
            j << "\"visibleLevel\": " << inputValues.m_visibleContainerWaterLevelMillimeters<< ", ";
            j << "\"visibleLevelIndex\": " << GetCurrentWaterLevelIndex() << "";
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
    // TODO: Handle properly consistency with automated logic
    // ...
#pragma message ("WARNING: TODO: Handle properly consistency with automated logic")

    EnsurePumpReadyForWork_RaiseExceptionIfNot(PI_OUTPUTPUMP);
    EnablePumpForSpecifiedTime(PI_OUTPUTPUMP, 1000);
}

void HardwareServiceImplementation::DbgSetContainerWaterLevel(const int32_t amount)
{
    //auto &inputValues = GetDeviceInputValuesRef();
    //inputValues.m_visibleContainerWaterLevelMillimeters = amount;

    ScopedLock locked(GetDefferedInputListLock());
    auto &inputList = GetDefferedInputList();
    inputList.push_back(DefferedWaterInputItem(1000));

    LOG(INFO) << "Scheduled task for processing";
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


void HardwareServiceImplementation::InitializePumpsState(EPUMPSTATE initialState/*=PS__DEFAULT*/)
{
    for (unsigned pumpIndex = PI__BEGIN; pumpIndex != PI__END; ++pumpIndex)
    {
        m_pumpsState[pumpIndex].Assign(initialState, steady_time_point(), steady_time_point());
    }
}

void HardwareServiceImplementation::InitializeServiceState()
{
    SetServiceState(SS__DEFAULT);
    SetDeviceState(DS__DEFAULT);
}

void HardwareServiceImplementation::CreateTimerThreads()
{
    ASSERT(Dataconst::TimerThreadPoolSize == 1); // Disabled for good days to make tasks execution fully serialized 

    for (unsigned i = 0; i != Dataconst::TimerThreadPoolSize; ++i)
    {
        m_timerThreads.create_thread(boost::bind(&boost::asio::io_service::run, &m_timersIOService));
    }
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
    ProcessPumpControlActions(PI_INPUTPUMP);

    RestartTask(TI_INPUTPUMPTASK);
}

void HardwareServiceImplementation::OutputPumpControlTask()
{
    ProcessPumpControlActions(PI_OUTPUTPUMP);

    RestartTask(TI_OUTPUTPUMTASK);
}

void HardwareServiceImplementation::WaterLevelManagerTask()
{
    LOG(DEBUG) << "[WATER LEVEL MANAGER] Activated";

    // This task responsible for automatic pumping out the water from visible container.
    // There is also manual triggering pumping out, but for the sake of simplicity water is 
    // pumped out automatically without making business logic worry about it.
    // The amounT of water to pump out, its period which could be called velocity is defined by configuration as 
    // a set of levels (or milliliters) associated with amount of water to pump out per time unit. E.g.:
    //
    //  Level (liters)      WaterToPumpOut (liters per hour)      Comments
    //  --------------------------------------------------------------------
    //  100-120             1                                     Should be equal to performance of input pump
    //  80-100              0.8
    /// 60-80               0.6
    /// 40-60               0.4
    /// 20-40               0.2
    /// 10-20               0.1
    //  0-10                0.00001                               (pump will ignore so small amount of work)
    //

    //

    ProcessWaterLevelManagerTaskActions();
    RestartTask(TI_LEVELMANAGERTASK);
}

void HardwareServiceImplementation::ProcessWaterLevelManagerTaskActions()
{
    if (GetServiceState() != SS_READY) return;
    if (GetDeviceState() != DS_READY) return;
    
    auto &stateDataRef = GetLevelPumpOutStateDataRef();

    const auto currentWaterLevelIndex = GetCurrentWaterLevelIndex();
    const auto lastLevelIndex = stateDataRef.GetLastDiscreteLevelIndex();
    const bool waterLevelChanged = (lastLevelIndex == ItemNotFoundIndex) || (lastLevelIndex != currentWaterLevelIndex);

    if (waterLevelChanged)
    {
        LOG(INFO) << "Level changed to: " << currentWaterLevelIndex;
        stateDataRef.SetLastDiscreteLevelIndex(currentWaterLevelIndex);

        if (currentWaterLevelIndex != ItemNotFoundIndex)
        {
            bool stopped;

            if (StopPumpIfNecessary(PI_OUTPUTPUMP, stopped))
            {
                const auto amountToPumpOutMl = GetPumpingOutvelocityAtLevel(currentWaterLevelIndex);
                const unsigned workTimeSec = CalculateWorkTimeForSpecifiedPump(PI_OUTPUTPUMP, amountToPumpOutMl);
                unsigned unsigned hourCenteredTimeOffsetSec = (3600 / 2) - (workTimeSec / 2);
                stateDataRef.SetActivationTime(GetNowSteadyClockTime() + boost::chrono::seconds(hourCenteredTimeOffsetSec));
            }
        }
    }
    
    if (stateDataRef.GetActivationTime() <= GetNowSteadyClockTime())
    {
        bool stopped;

        if (StopPumpIfNecessary(PI_OUTPUTPUMP, stopped))
        {
            if (currentWaterLevelIndex != ItemNotFoundIndex)
            {
                const auto amountToPumpOutMl = GetPumpingOutvelocityAtLevel(currentWaterLevelIndex);
                stateDataRef.SetActivationTime(GetNowSteadyClockTime() + Dataconst::PumpOutActivationPeriod);

                bool started;
                if (StartPumpingMilliliters(PI_OUTPUTPUMP, amountToPumpOutMl, started))
                {
                    if (started)
                    {
                        stateDataRef.SetState(LPS_INPROGRESS);
                    }
                    else
                    {
                        stateDataRef.SetState(LPS_IDLE);
                        auto &pumpDescriptorRef = GetPumpStateDescriptorRef(PI_OUTPUTPUMP);
                        pumpDescriptorRef.SetScheduledStopTime(GetNowSteadyClockTime());
                    }
                }
            }
            else
            {
                LOG(ERROR) << "Invalid index reached. Seems like a problem with level sensor or pumps; raising emergency stop exception";

                stateDataRef.SetState(LPS_IDLE);
                EmergencyStop();
            }
        }
    }
}

void HardwareServiceImplementation::InputWaterPorcessManagerTask()
{
    const auto &inputPumpState = GetPumpStateDescriptorRef(PI_INPUTPUMP);
    
    if (inputPumpState.GetState() == PS_DISABLED)
    {
        ScopedLock locked(GetDefferedInputListLock());

        auto &defferedTasksList = GetDefferedInputList();
        if (defferedTasksList.size() > 0)
        {
            const auto &nextTask = defferedTasksList.front();

            bool started;
            if (StartPumpingMilliliters(PI_INPUTPUMP, nextTask.m_amount, started))
            {
                defferedTasksList.pop_front();

                LOG(INFO) << "Processing task";
            }
        }
    }

    RestartTask(TI_INPUTWATERPROCESSINGTASK);
}


void HardwareServiceImplementation::OnPumpEndWorking()
{
    auto &currentLevelPumpOutState = GetLevelPumpOutStateDataRef();
    currentLevelPumpOutState.SetState(LPS_IDLE);
}

void HardwareServiceImplementation::OnPumpStartedWorking()
{
    // Do nothing
}


void HardwareServiceImplementation::EnablePumpForSpecifiedTime(EPUMPIDENTIFIER pumpId, unsigned timeMilliseconds)
{
    auto &pumpDescriptor = GetPumpStateDescriptorRef(pumpId);

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

    const auto &pumpDescriptor = GetPumpStateDescriptorRef(pumpId);

    if (pumpDescriptor.GetState() != PS_DISABLED)
    {
        // TODO: Decide what to do when pump already enabled
        RaiseInvalidOperationException(Tabletochki::ErrorCode::PUMP_NOT_READY,
            "Actual state is: " + DecodePumpState(pumpDescriptor.m_state));
    }
}

void HardwareServiceImplementation::ProcessPumpControlActions(EPUMPIDENTIFIER pumpId)
{
    ProcessPumpControlActions_ManagePumpControl(pumpId);
    ProcessPumpControlActions_SimulatedSensors(pumpId);
}


void HardwareServiceImplementation::ProcessPumpControlActions_ManagePumpControl(EPUMPIDENTIFIER pumpId)
{
    auto &pumpDescriptor = GetPumpStateDescriptorRef(pumpId);
    if (pumpDescriptor.GetState() == PS_ENABLED)
    {
        const auto stopTime = pumpDescriptor.GetScheduledStopTime();
        const auto nowTime = GetNowSteadyClockTime();
        const auto workTimeSeconds = pumpDescriptor.CalcWorkTimeDurationSeconds();

        if (stopTime <= nowTime)
        {
            if (ExecuteDisablePumpHardwareCommand(pumpId))
            {
                LOG(INFO) << "The pump " << DecodePumpIdentifierName(pumpId) << " has been disabled. Work time is " <<
                    pumpDescriptor.CalcWorkTimeDurationSeconds() << " seconds";

                pumpDescriptor.ResetStateData(PS_DISABLED);
                OnPumpEndWorking();
            }
            else
            {
                LOG(ERROR) << "Failed disabling pump: " << DecodePumpIdentifierName(pumpId);
                EmergencyStop();
            }
        }
    }
}

void HardwareServiceImplementation::ProcessPumpControlActions_SimulatedSensors(EPUMPIDENTIFIER pumpId)
{
    auto &pumpDescriptor = GetPumpStateDescriptorRef(pumpId);

    steady_time_point &lastWorkingTimePointRef = pumpDescriptor.m_lastWorkingTimePoint;
    
    if (pumpDescriptor.GetState() == PS_ENABLED)
    {
        if (lastWorkingTimePointRef == steady_time_point())
        {
            LOG(INFO) << "Pump IS ASSUMED TO BE ENABLED FOR FIRST TIME: " << DecodePumpIdentifierName(pumpId);
            lastWorkingTimePointRef = GetNowSteadyClockTime();
        }

        const auto nowTime = GetNowSteadyClockTime();
        const auto timeElapsedFromPrevioisCall = boost::chrono::duration_cast<boost::chrono::seconds>(nowTime - lastWorkingTimePointRef).count();

        {
            const auto &configuration = GetServiceConfiguration();
            const unsigned pumpPerformancePerSecondsMl = configuration.Pumps[pumpId].m_performanceMlPerHour / 3600;
            const unsigned millitersPumpedOut = pumpPerformancePerSecondsMl * timeElapsedFromPrevioisCall;

            const double levelDecraseCm = (double)millitersPumpedOut / (configuration.VisibleContainerConfiguration.m_width * configuration.VisibleContainerConfiguration.m_height);

            auto &inputValuesReference = GetDeviceInputValuesRef();
            
            if (levelDecraseCm >= 0.1)
            {
                const unsigned levelDecraseml = std::rint(levelDecraseCm * 10.0);

                LOG(INFO) << "Level decreased/increased for " << levelDecraseml << " ml by " << DecodePumpIdentifierName(pumpId) << "pump";

                //if (inputValuesReference.m_visibleContainerWaterLevelMillimeters >= 0)
                {
                    const auto multiplier = PumpsTasksSimulatedSensorsMultipliers.GetMappedValue(pumpId);
                    inputValuesReference.m_visibleContainerWaterLevelMillimeters += (levelDecraseml * multiplier);
                    lastWorkingTimePointRef = nowTime;
                }
            }
        }
    }
    else
    {
        lastWorkingTimePointRef = steady_time_point();
    }
}

void HardwareServiceImplementation::EmergencyStop()
{
#pragma message("WARNING:  Emergeny stop is not implemented")
    // ...
}


bool HardwareServiceImplementation::StopPumpIfNecessary(EPUMPIDENTIFIER pumpId, bool &out_stopped)
{
    bool result = false;
    
    auto &pumpStateDescriptor = GetPumpStateDescriptorRef(pumpId);

    if (pumpStateDescriptor.GetState() == PS_ENABLED)
    {
        if (ExecuteDisablePumpHardwareCommand(pumpId))
        {
            LOG(INFO) << "Work time duration: " << pumpStateDescriptor.CalcWorkTimeDurationSeconds();
            pumpStateDescriptor.ResetStateData(PS_DISABLED);
            out_stopped = true;
            result = true;
        }
        else
        {
            EmergencyStop();
        }
    }
    else
    {
        out_stopped = false;
        result = true;
    }

    return result;
}

bool HardwareServiceImplementation::StartPumpingMilliliters(EPUMPIDENTIFIER pumpId, unsigned millilitersToPump, bool &out_result)
{
#pragma message ("WARNING: Need to check if it is already acquired by someone ")
    
    bool result = false;

    auto &pumpStateDescriptor = GetPumpStateDescriptorRef(pumpId);

    if (pumpStateDescriptor.GetState() != PS_ENABLED)
    {
        const unsigned workTimeSeconds = CalculateWorkTimeForSpecifiedPump(pumpId, millilitersToPump);

        static const unsigned minimumWorkTimeSeconds = 1;

        if (workTimeSeconds >= minimumWorkTimeSeconds)
        {
            LOG(INFO) << "Started new pumping task. for time: " << workTimeSeconds;

            const auto nowTime = GetNowSteadyClockTime();
            const auto scheduledWorkEndTime = nowTime + boost::chrono::seconds(workTimeSeconds);

            if (ExecuteEnablePumpHardwareCommand(pumpId))
            {
                pumpStateDescriptor.Assign(PS_ENABLED, nowTime, scheduledWorkEndTime);
                result = true;
                out_result = true;
            }
            else
            {
                EmergencyStop();
            }
        }
        else
        {
            result = true;
            out_result = false;
        }
    }
    else
    {

    }

    return result;
}


void HardwareServiceImplementation::DoHeartBeatTask()
{
    uint32_t deviceStatus;

    if (GetServiceState() != SS_READY)
        return;

    const auto currentState = GetDeviceState();

    if (ExecuteHeartbeatCommand(deviceStatus))
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
            if (ExecuteConfigureDeviceCommand())
            {
                SetDeviceState(DS_READY);
                InitializePumpsState(PS_DISABLED);

                LOG(INFO) << "Device changed state to ready";
            }
        }
        else if (currentState == DS_DISCONNECTED)
        {
            if (IsDeviceConnected())
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
        DeviceInputValues deviceInput(GetDeviceInputValues());

        if (ExecuteReadIOCommand(deviceInput))
        {
            
            SetDeviceInputValues(deviceInput);
        }
    }
}


bool HardwareServiceImplementation::IsDeviceConnected()
{
    unsigned deviceStatus;
    const bool result = ExecuteHeartbeatCommand(deviceStatus);
    return result;
}


bool HardwareServiceImplementation::ExecuteEnablePumpHardwareCommand(EPUMPIDENTIFIER pumpId)
{
    bool result = false;
    
    Packets::Templates::WriteIOCommandRequest<1> outputRequest;

    unsigned requestPinUsed = 0;

    const unsigned pinNumber = PumpsPinsNumbers.GetMappedValue(pumpId);

    outputRequest.Pins[requestPinUsed].Assign(pinNumber, 1);
    ++requestPinUsed;

    Packets::Templates::WriteIOCommandResponse outputResponse;
    if (SendPacket(outputRequest, outputResponse))
    {
        if (outputResponse.Status.OperationResultCode == EC_OK)
        {
            result = true;
        }
        else
        {
            LOG(ERROR) << "Failed executing 'enable pump' command. Device returned: " << ((unsigned)outputResponse.Status.OperationResultCode) << ". " <<
                "Device status: " << ((unsigned)outputResponse.Status.DeviceStatus);
        }
    }

    return result;
}

bool HardwareServiceImplementation::ExecuteDisablePumpHardwareCommand(EPUMPIDENTIFIER pumpId)
{
    bool result = false;

    Packets::Templates::WriteIOCommandRequest<2> outputRequest;

    unsigned requestPinUsed = 0;

    const unsigned pinNumber = PumpsPinsNumbers.GetMappedValue(pumpId);

    outputRequest.Pins[requestPinUsed].Assign(pinNumber, 0);
    ++requestPinUsed;

    Packets::Templates::WriteIOCommandResponse outputResponse;

    if (SendPacket(outputRequest, outputResponse))
    {
        if (outputResponse.Status.OperationResultCode == EC_OK)
        {
            result = true;
        }
        else
        {
            LOG(ERROR) << "Failed executing 'disable pump' command. Device returned: " << ((unsigned)outputResponse.Status.OperationResultCode) << ". " <<
                "Device status: " << ((unsigned)outputResponse.Status.DeviceStatus);
        }
    }

    return result;
}

bool HardwareServiceImplementation::ExecuteHeartbeatCommand(unsigned &out_deviceStatus)
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
        else
        { 
            LOG(ERROR) << "Failed executing 'heartbeat' command. Device returned: " << ((unsigned)response.OperationResultCode) << ". " <<
                "Device status: " << ((unsigned)response.DeviceStatus);
        }
    }

    return result;
}

bool HardwareServiceImplementation::ExecuteReadIOCommand(DeviceInputValues &out_deviceInput)
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

            out_deviceInput.m_proximitySensorValue = readIOResponse.Pins[responsePinRead].Value;
            ASSERT(readIOResponse.Pins[responsePinRead].PinNumber == PROXIMITYSENSOR_PINNUMBER);
            ++responsePinRead;

            out_deviceInput.m_magicButtonPressed = readIOResponse.Pins[responsePinRead].Value;
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

bool HardwareServiceImplementation::ExecuteConfigureDeviceCommand()
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


unsigned HardwareServiceImplementation::GetCurrentWaterLevelMillimiters() const
{
#pragma message ("WARNING: Not implemented")
    //return 0;
    return GetDeviceInputValues().m_visibleContainerWaterLevelMillimeters;
}

unsigned HardwareServiceImplementation::DecodeDiscreteWaterLevelIndex(unsigned waterLevelMillimiters) const
{
    const auto &serviceConfiguration = GetServiceConfiguration();
    const auto &levelsDataVector = serviceConfiguration.PumpOutLevelsConfiguration.m_levelData;
    
    const auto positionIter = std::find_if(levelsDataVector.cbegin(), levelsDataVector.cend(), 
        [waterLevelMillimiters](const LevelConfiguration &levelItem) {
            return waterLevelMillimiters < levelItem.m_levelHeight;
        }
    );

    const unsigned result = positionIter != levelsDataVector.cend() ? 
            std::distance(levelsDataVector.cbegin(), positionIter) : ItemNotFoundIndex;
    return result;
}

const LevelConfiguration &HardwareServiceImplementation::GetLevelConfiguration(unsigned index) const
{
    const auto &serviceConfiguration = GetServiceConfiguration();
    ASSERT(index < serviceConfiguration.PumpOutLevelsConfiguration.m_levelData.size());
    return serviceConfiguration.PumpOutLevelsConfiguration.m_levelData[index];
}

unsigned HardwareServiceImplementation::GetPumpingOutvelocityAtLevel(unsigned index)
{
    return GetLevelConfiguration(index).m_velocityMillilters;
}

unsigned HardwareServiceImplementation::GetPumpPerformance(EPUMPIDENTIFIER pumpId)
{
    const auto &serviceConfiguration = GetServiceConfiguration();
    
    ASSERT(Utils::InRange(pumpId, PI__BEGIN, PI__END) );
    
    const auto result = serviceConfiguration.Pumps[pumpId].m_performanceMlPerHour;
    return result;
}


bool HardwareServiceImplementation::IsTimeToProcessLevel(unsigned waterLevel) const
{

    //#pragma message ("WARNING: Not implemented")
    const auto &stateData =  GetLevelPumpOutStateData();

    const auto nowTime = GetNowSteadyClockTime();

    if (stateData.GetActivationTime() <= nowTime)
    {
        return true;
    }
    else
    {
        auto left = boost::chrono::duration_cast<boost::chrono::seconds>(stateData.GetActivationTime() - nowTime);
        LOG(INFO) << "left: " << left.count();
        return false;
    }
    
    return false;
}

unsigned HardwareServiceImplementation::CalculateWorkTimeForSpecifiedPump(EPUMPIDENTIFIER pumpId, unsigned waterAmountMl)
{
    const auto pumpPerformanceMillilitersPerHour = GetPumpPerformance(pumpId);
    const unsigned workTimeSec = (unsigned)(((double)waterAmountMl / (double)pumpPerformanceMillilitersPerHour) * 3600.0);
    return workTimeSec;
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
    { &HardwareServiceImplementation::HeartBeatTask, Dataconst::HearBeatCommandPeriodMs },          // TI_HEARTBEATTASK
    { &HardwareServiceImplementation::QueryInputTask, Dataconst::QueryInputTaskPeriodMs },          // TI_STATUSTASK
    { &HardwareServiceImplementation::WaterLevelManagerTask, Dataconst::LevelManagerTaskPeriodMs }, // TI_LEVELMANAGERTASK
    { &HardwareServiceImplementation::InputWaterPorcessManagerTask, Dataconst::InputProcessingTaskPeriodMs }, // TI_INPUTWATERPROCESSINGTASK   
    { &HardwareServiceImplementation::InputPumpControlTask, Dataconst::InputPipeTaskPeriodMs },     // TI_INPUTPUMPTASK
    { &HardwareServiceImplementation::OutputPumpControlTask, Dataconst::OutputPipeTaskPeriodMs }    // TI_OUTPUTPUMTASK
};
