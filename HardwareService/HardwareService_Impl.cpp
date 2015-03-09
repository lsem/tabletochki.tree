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
    m_defferedInputListLock(),
    m_levelHeighsMinMaxTable(),
    m_previousWaterLevelIndex(InvalidIndex),
    m_emergencyStopMessage(),
    m_deviceComportId()
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
        LOG(ERROR) << "Failed executing StartPump command";
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
        LOG(ERROR) << "Failed executing StopPump command";
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

    const auto &pumpOutStateData = GetLevelPumpOutStateData();
    const auto currentLevelPumpOutState = pumpOutStateData.GetState();
    const auto currentLevelPumpOutStateStr = PumpingOutStateNames.GetMappedValue(currentLevelPumpOutState);
    const auto pendingInputTasksCount = GetWaterInputPendingTasksCount();
    const auto nowTime = GetNowSteadyClockTime();
#pragma message ("WARNING: Thread-unsafe ecode")
    const steady_time_point activationTime = pumpOutStateData.GetActivationTime();
    const auto nextPumpOutTimeLeftSec = activationTime > nowTime 
        ? boost::chrono::duration_cast<boost::chrono::seconds>(activationTime - nowTime).count() : 0;

#pragma region ResultingJsonFormatting
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
        j << "\"m_visibleWaterLevelSensorRaw\": " << inputValues.m_visibleWaterLevelSensorRaw << ",";
            j << "\"magicButton\": " << inputValues.m_magicButtonPressed << ", ";
            j << "\"visibleLevel\": " << inputValues.m_visibleContainerWaterLevelCm<< ", ";
            j << "\"visibleLevelIndex\": " << GetCurrentWaterLevelIndex() << ", ";
            j << "\"nextPumpOutTimeLeft\": " << nextPumpOutTimeLeftSec << "";
        j << "} "; // input
    j << "} ";
#pragma endregion Resulting Json Formatting

    jsonDocumentReceiver = j.str();
}

void HardwareServiceImplementation::FillVisibleContainerMillilitres(const int32_t amount)
{
    EnsurePumpReadyForWork_RaiseExceptionIfNot(PI_INPUTPUMP);
    ScheduleWaterInputTask(amount);
}

void HardwareServiceImplementation::EmptyVisiableContainerMillilitres(const int32_t amount)
{
#pragma message ("WARNING: TODO: Handle properly consistency with automated logic")

    EnsurePumpReadyForWork_RaiseExceptionIfNot(PI_OUTPUTPUMP);
    EnsureOutputPumpIsAvailableForFork_RaiseIfNot();
    ScheduleWaterInputTask(amount);
}

void HardwareServiceImplementation::DbgSetContainerWaterLevel(const int32_t amount)
{
    if (!Utils::InRange(amount, 1, 100000))
    {
        RaiseInvalidOperationException(Tabletochki::ErrorCode::INVALID_CONFIGURATION, "Invlaid amount. Should be in range 1..100000");
    }
    else
    {
        ScheduleWaterInputTask(amount);
        LOG(DEBUG) << "Scheduled task for processing: " << amount;
    }
}


void HardwareServiceImplementation::StartService()
{
    LoadConfiguration();
    
    while (!DiscoverDevicePort())
    {
        LOG(ERROR) << "Device disconnected";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

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
        BuildFixedWaterLevelHeightsTable();
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

void HardwareServiceImplementation::PreparePeripheral()
{
    SetPreviousWaterLevelIndex(InvalidIndex);
}

bool HardwareServiceImplementation::DiscoverDevicePort()
{
    bool result = false;

    string comportId;
    unsigned comportNumberToTry = 0;
    bool found = false;

    while (true)
    {
        found = false;

        {
            ScopedLock locked(GetCommunicationLock());
            auto *communicationChannel = GetCommunicationChannel();

            comportId = "COM" + std::to_string(comportNumberToTry);

            if (communicationChannel->Open(comportId.c_str()))
            {
                communicationChannel->Close();
                found = true;
            }
        }

        if (found)
        {
            SetDeviceomportId(comportId);
            
            unsigned deviceStatus;
            if (ExecuteHeartbeatCommand(deviceStatus))
            {
                LOG(INFO) << "Found device at: " << comportId;

                SetDeviceomportId(comportId);
                
                result = true;
                break;
            }
        }

        if (!result)
        {
            LOG(DEBUG) << "Failed opening device at port: " << comportId;

            ++comportNumberToTry;

            if (comportNumberToTry == MAXIMUM_DEVICEOPEN_COMPORTNUMBER)
            {
                break;
            }
        }
    }

    return result;
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

    ProcessWaterLevelManagerTaskActions();
    RestartTask(TI_LEVELMANAGERTASK);
}

void HardwareServiceImplementation::ProcessWaterLevelManagerTaskActions()
{
    if (GetServiceState() != SS_READY)
    {
        return;
    }
    if (GetDeviceState() != DS_READY)
    {
        return;
    }
    if (GetCurrentWaterLevelIndex() == InvalidIndex)
    {
        LOG(ERROR) << "Output pumping maintenance activity is not possible due to invalid water level. "
                        "Check the configuration or hardware";

        EmergencyStop();
        return;
    }

    DoProcessWaterLevelManagerTaskActions();
}

void HardwareServiceImplementation::DoProcessWaterLevelManagerTaskActions()
{
    ProcessLevelIndexChangeIfNecessary();
    ActivateOutomatedOutputPumpingIfNecessary();
}

void HardwareServiceImplementation::ProcessLevelIndexChangeIfNecessary()
{
    const auto currentWaterLevelIndex = GetCurrentWaterLevelIndex();

    LOG(DEBUG) << "Sensor value: " << GetDeviceInputValues().m_visibleWaterLevelSensorRaw;
    LOG(DEBUG) << "Level Index: " << currentWaterLevelIndex;

    const auto lastLevelIndex = GetPreviousWaterLevelIndex();
    const bool waterLevelChanged = (lastLevelIndex == ItemNotFoundIndex) || (lastLevelIndex != currentWaterLevelIndex);

    if (waterLevelChanged)
    {
        SetPreviousWaterLevelIndex(currentWaterLevelIndex);

        LOG(DEBUG) << "Level changed to: " << currentWaterLevelIndex;

        if (StopPumpIfNecessary(PI_OUTPUTPUMP))
        {
            ShceduleFirstPlannedPumpOutActivation();
        }
    }
}

void HardwareServiceImplementation::ActivateOutomatedOutputPumpingIfNecessary()
{
    auto &stateDataRef = GetLevelPumpOutStateDataRef();
    if (stateDataRef.GetActivationTime() <= GetNowSteadyClockTime())
    {
        ActivateOutomatedOutputPumping();
    }
}

void HardwareServiceImplementation::ShceduleNextPlannedPumpOutActivation()
{
    auto &stateDataRef = GetLevelPumpOutStateDataRef();
    stateDataRef.SetActivationTime(GetNowSteadyClockTime() + Dataconst::PumpOutActivationPeriod);
}

void HardwareServiceImplementation::ShceduleFirstPlannedPumpOutActivation()
{
    const auto currentWaterLevelIndex = GetCurrentWaterLevelIndex();
    auto &stateDataRef = GetLevelPumpOutStateDataRef();
    const auto amountToPumpOutMl = GetPumpingOutvelocityAtLevel(currentWaterLevelIndex);
    const unsigned workTimeSec = CalculateWorkTimeForSpecifiedPump(PI_OUTPUTPUMP, amountToPumpOutMl);
    unsigned hourCenteredTimeOffsetSec = (3600 / 2) - (workTimeSec / 2);
    stateDataRef.SetActivationTime(GetNowSteadyClockTime() + boost::chrono::seconds(hourCenteredTimeOffsetSec));
}

void HardwareServiceImplementation::PerformPlannedPumpOutActivation()
{
    auto &stateDataRef = GetLevelPumpOutStateDataRef();
    const auto currentWaterLevelIndex = GetCurrentWaterLevelIndex();
    const auto amountToPumpOutMl = GetPumpingOutvelocityAtLevel(currentWaterLevelIndex);

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

void HardwareServiceImplementation::ActivateOutomatedOutputPumping()
{
    if (StopPumpIfNecessary(PI_OUTPUTPUMP))
    {
        ShceduleNextPlannedPumpOutActivation();
        PerformPlannedPumpOutActivation();
    }
    else
    {
        LOG(ERROR) << "Failed to stop output pump that needed for starting planned water output";
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


void HardwareServiceImplementation::OnOutputPumpEndedWorking()
{
    auto &currentLevelPumpOutState = GetLevelPumpOutStateDataRef();
    currentLevelPumpOutState.SetState(LPS_IDLE);
}

void HardwareServiceImplementation::OnOutputPumpStartedWorking()
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

void HardwareServiceImplementation::EnsureOutputPumpIsAvailableForFork_RaiseIfNot()
{
    const auto &atomatedPumpOutState = GetLevelPumpOutStateData();
    if (atomatedPumpOutState.GetState() != LPS_IDLE)
    {
        RaiseInvalidOperationException(Tabletochki::ErrorCode::PUMP_NOT_READY,
            "Pump is used by automated pumping out service");
    }
}

void HardwareServiceImplementation::ProcessPumpControlActions(EPUMPIDENTIFIER pumpId)
{
    ProcessPumpControlActions_ManagePumpControl(pumpId);
    //ProcessPumpControlActions_SimulatedSensors(pumpId);
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

                if (pumpId == PI_OUTPUTPUMP)
                {
                    OnOutputPumpEndedWorking();
                }
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
        const auto timeElapsedFromPrevioisCallMsec = boost::chrono::duration_cast<boost::chrono::milliseconds>(nowTime - lastWorkingTimePointRef).count();
        
        const auto &configuration = GetServiceConfiguration();
        const unsigned pumpPerformancePerSecondsMl = configuration.Pumps[pumpId].m_performanceMlPerHour / 3600;
        const unsigned millitersPumpedOut = static_cast<unsigned>(pumpPerformancePerSecondsMl * timeElapsedFromPrevioisCallMsec) / 1000;
        const auto visibleHeight = configuration.VisibleContainerConfiguration.m_depth;
        const auto visibleWidth = configuration.VisibleContainerConfiguration.m_width;

        const double levelDecraseCm = (double)millitersPumpedOut / (configuration.VisibleContainerConfiguration.m_width * configuration.VisibleContainerConfiguration.m_depth);

        auto &inputValuesReference = GetDeviceInputValuesRef();
            
        if (levelDecraseCm >= 0.1)
        {
            const unsigned levelDecraseml = static_cast<unsigned>(std::rint(levelDecraseCm * 10.0));

            LOG(INFO) << "Level decreased/increased for " << levelDecraseml << " ml by " << DecodePumpIdentifierName(pumpId) << "pump";

            const auto multiplier = PumpsTasksSimulatedSensorsMultipliers.GetMappedValue(pumpId);
            const auto increaseValue = levelDecraseml * multiplier;
            
            static const auto MaximumValue = GetServiceConfiguration().PumpOutLevelsConfiguration.m_levelData[GetServiceConfiguration().PumpOutLevelsConfiguration.m_levelData.size()-1].m_levelHeight - 1;
            if ((inputValuesReference.m_visibleContainerWaterLevelCm + increaseValue) <= MaximumValue)
            {
                inputValuesReference.m_visibleContainerWaterLevelCm += increaseValue;
            }

            lastWorkingTimePointRef = nowTime;
        }
       
    }
    else
    {
        lastWorkingTimePointRef = steady_time_point();
    }
}

void HardwareServiceImplementation::EmergencyStop()
{
    // By setting this state, heartbeat task should be suppressed which in turn should 
    // activate fault tolerance in Arduino which should stop all hardware
    //SetServiceState(SS_EMERGENCYSTOPPED);

    //// Reset all states to failed
    //GetLevelPumpOutStateDataRef().SetState(LPS_IDLE);
    //GetPumpStateDescriptorRef(PI_INPUTPUMP).SetState(PS_FAILED);
    //GetPumpStateDescriptorRef(PI_OUTPUTPUMP).SetState(PS_FAILED);
    //SetDeviceState(DS_DISCONNECTED);
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

            if (pumpId == PI_OUTPUTPUMP)
            {
                OnOutputPumpEndedWorking();
            }

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

bool HardwareServiceImplementation::StopPumpIfNecessary(EPUMPIDENTIFIER pumpId)
{
    bool dontCare;
    return StopPumpIfNecessary(pumpId, dontCare);
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
                EmergencyStopMsg("Failed start pump");
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
        LOG(ERROR) << "Failed to enable pump " << DecodePumpIdentifierName(pumpId) << ". Already working";
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
                PreparePeripheral();
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
            const unsigned levelSensorRaw = deviceInput.m_visibleWaterLevelSensorRaw;
            double levelSensorValue = (levelSensorRaw / (double) WATERLEVEL_SENSOR_MAX);

            if (levelSensorValue > 1.0)
            {
                if (levelSensorValue > WATERLEVEL_SENSOR_MAX_THRESHOLD_KOEF)
                {
                    EmergencyStopMsg("Level sensor invalid value");
                    LOG(ERROR) << "Level sensor invalid value!!";
                }
                else
                {
                    levelSensorValue = 1.0;
                }
            }

            deviceInput.m_visibleContainerWaterLevelCm = levelSensorValue * WATERLEVEL_SENSOR_HEIGHT_CM;
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
    else
    {
        LOG(ERROR) << "Failed sending/receiving 'enable pump' command.";
    }

    return result;
}

bool HardwareServiceImplementation::ExecuteDisablePumpHardwareCommand(EPUMPIDENTIFIER pumpId)
{
    bool result = false;

    Packets::Templates::WriteIOCommandRequest<1> outputRequest;

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
    else
    {
        LOG(ERROR) << "Failed sending/receiving 'disable pump' command.";
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
    else
    {
        LOG(ERROR) << "Failed sending/receiving 'heartbeat' command.";
    }

    return result;
}

bool HardwareServiceImplementation::ExecuteReadIOCommand(DeviceInputValues &out_deviceInput)
{
    bool result = false;
    
    unsigned requestPinWritten = 0;

    Packets::Templates::ReadIOCommandRequest<INPUT_PINS_COUNT> readIORequest;

    readIORequest.Pins[requestPinWritten].PinNumber = WATERLEVELSENSOR_PINNUMBER;
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

            out_deviceInput.m_visibleWaterLevelSensorRaw = readIOResponse.Pins[responsePinRead].Value;
            ASSERT(readIOResponse.Pins[responsePinRead].PinNumber == WATERLEVELSENSOR_PINNUMBER);
            ++responsePinRead;

            out_deviceInput.m_magicButtonPressed = readIOResponse.Pins[responsePinRead].Value > 0;
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
    else
    {
        LOG(ERROR) << "Failed sending/receiving 'read io' command.";
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
    configureIORequest.Pins[pinNumber].Assign(WATERLEVELSENSOR_PINNUMBER, WATERLEVELSENSOR_FLAGS, WATERLEVELSENSOR_DEFVALUE, WATERLEVELSENSOR_SPECDATA);
    ++pinNumber;
    configureIORequest.Pins[pinNumber].Assign(MAGICBUTTON_PINNUMBER, MAGICBUTTON_FLAGS, MAGICBUTTON_DEFVALUE, MAGICBUTTON_SPECDATA);
    ++pinNumber;

    assert(pinNumber == configureIORequest.PinsCount);

    Packets::Templates::ConfigureIOResponse configureResponse;
    if (SendPacket(configureIORequest, configureResponse))
    {
        if (configureResponse.Status.OperationResultCode == EC_OK)
        {
            LOG(INFO) << "'configure device' command sent. New device status is: " << (unsigned) configureResponse.Status.DeviceStatus;
            result = true;
        }
        else
        {
            LOG(ERROR) << "Failed to configure device. Device respond with: " << (unsigned) configureResponse.Status.OperationResultCode;
        }
    }
    else
    {
        LOG(ERROR) << "Failed sending/receiving 'configure device' command.";
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

    auto *communicationChannel = GetCommunicationChannel();

    ScopedLock locked(GetCommunicationLock());

    bool portOpened = false;

    do
    {
        LOG(DEBUG) << "Opening communication channel on COM4";

        if (!communicationChannel->Open(GetDeviceomportId().c_str()))
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
    return GetDeviceInputValues().m_visibleContainerWaterLevelCm;
}

unsigned HardwareServiceImplementation::DecodeDiscreteWaterLevelIndex(unsigned waterLevelMm) const
{
    const auto &serviceConfiguration = GetServiceConfiguration();
    const auto &levelsDataVector = serviceConfiguration.PumpOutLevelsConfiguration.m_levelData;
    
    const auto positionIter = std::find_if(levelsDataVector.cbegin(), levelsDataVector.cend(), 
        [waterLevelMm](const LevelConfiguration &levelItem) {
            return waterLevelMm < levelItem.m_levelHeight;
        }
    );

    const unsigned result = positionIter != levelsDataVector.cend() ? 
            std::distance(levelsDataVector.cbegin(), positionIter) : ItemNotFoundIndex;
    return result;
}

unsigned HardwareServiceImplementation::DecodeFixedWaterLevelIndex(unsigned waterLevelMm) const
{
    const auto &waterLevelTable = GetLevelHeightsMinMaxTable();

    unsigned resultIndex = InvalidIndex;
    for (unsigned index = 0; index != waterLevelTable.size(); ++index)
    {
        const auto &levelPair = waterLevelTable[index];
        if (Utils::InRange(waterLevelMm, levelPair.first, levelPair.second))
        {
            resultIndex = index;
            break;
        }
    }
    
    if (resultIndex == InvalidIndex)
    {
        resultIndex = GetPreviousWaterLevelIndex();
    }

    return resultIndex;
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

unsigned HardwareServiceImplementation::GetCurrentWaterLevelIndex()
{
    unsigned resultIndex = InvalidIndex;

    const auto waterLevelValue = GetCurrentWaterLevelMillimiters();    
    const auto unfixedWaterLevelIndex = DecodeDiscreteWaterLevelIndex(waterLevelValue);
    const auto previousIndex = GetPreviousWaterLevelIndex();

    LOG(DEBUG) << "waterLevelValue: " << waterLevelValue;
    LOG(DEBUG) << "unfixedWaterLevelIndex: " << unfixedWaterLevelIndex;
    LOG(DEBUG) << "previousIndex: " << previousIndex;

    if (previousIndex == InvalidIndex)
    {
        resultIndex = unfixedWaterLevelIndex;
    }
    else
    {
        resultIndex = DecodeFixedWaterLevelIndex(waterLevelValue);
    }

    LOG(DEBUG) << "resultIndex: " << resultIndex;

    return resultIndex;
}

void HardwareServiceImplementation::BuildFixedWaterLevelHeightsTable()
{
    auto &waterLevelTableRef = GetLevelHeightsMinMaxTable();

    const auto &serviceConfiguration = GetServiceConfiguration();
    const auto &levelsDataVector = serviceConfiguration.PumpOutLevelsConfiguration.m_levelData;

    const unsigned levelDataVectorSize = levelsDataVector.size();
    waterLevelTableRef.reserve(levelDataVectorSize);

    unsigned previosLevelHeight = 0;
    for (unsigned index = 0; index != levelDataVectorSize; ++index)
    {        
        const auto &levelData = levelsDataVector[index];
        const unsigned currentLevelHeight = levelData.m_levelHeight;

        const unsigned effectiveDeltaMin = index == 0 ? 0 : Dataconst::PumpoutTableFixDeltaMm;
        const unsigned effectiveDeltaMax = index == (levelDataVectorSize- 1) ? 0 : Dataconst::PumpoutTableFixDeltaMm;

        waterLevelTableRef.push_back(std::make_pair(previosLevelHeight + effectiveDeltaMin,
            currentLevelHeight - effectiveDeltaMax));
        previosLevelHeight = currentLevelHeight;
    }
}

unsigned HardwareServiceImplementation::CalculateWorkTimeForSpecifiedPump(EPUMPIDENTIFIER pumpId, unsigned waterAmountMl)
{
    const auto pumpPerformanceMillilitersPerHour = GetPumpPerformance(pumpId);
    const unsigned workTimeSec = (unsigned)(((double)waterAmountMl / (double)pumpPerformanceMillilitersPerHour) * 3600.0);
    return workTimeSec;
}

void HardwareServiceImplementation::ScheduleWaterInputTask(unsigned amount)
{
    ScopedLock locked(GetDefferedInputListLock());
    auto &inputList = GetDefferedInputList();
    inputList.push_back(DefferedWaterInputItem(amount));
}

unsigned HardwareServiceImplementation::GetWaterInputPendingTasksCount() const
{
    ScopedLock locked(GetDefferedInputListLock());
    const auto &defferedTasksList = GetDefferedInputList();
    return defferedTasksList.size();
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

