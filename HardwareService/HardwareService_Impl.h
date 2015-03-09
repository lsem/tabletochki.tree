#pragma once

#include "HardwareService.h"
#include "HardwareService_types.h"
#include "ServiceConfiguration.h"
#include "PacketUnFramer.h"
#include "CommunicationChannel.h"
#include "DeviceIOMapping.h"
#include "Utils.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>


using Tabletochki::StopPumpResult;
using Tabletochki::ServiceStatus;
using Tabletochki::PumpIdentifier;

typedef boost::chrono::steady_clock::time_point steady_time_point;

class ICommunicationChannel;
class IProblemReportingService;


enum ESERVICESTATE
{
    SS__BEGIN,

    SS_READY = SS__BEGIN,
    SS_SERVICENOTCONFIGURED,
    SS_EMERGENCYSTOPPED,
    SS_FAILED,

    SS__END,
    
    SS__DEFAULT = SS_FAILED,
};

enum ECONNECTIONSTATE
{
    DS__BEGIN,

    DS_CONNECTED = DS__BEGIN,
    DS_DISCONNECTED,
    DS_READY,

    DS__END,
    DS__DEFAULT = DS_DISCONNECTED,
};

enum ESERVICETASKID
{
    TI__BEGIN,
    
    TI_HEARTBEATTASK = TI__BEGIN,
    TI_STATUSTASK,
    TI_LEVELMANAGERTASK,
    TI_INPUTWATERPROCESSINGTASK,

    TI__PUMPS_TASKS_BEGIN,
    TI_INPUTPUMPTASK = TI__PUMPS_TASKS_BEGIN,
    TI_OUTPUTPUMTASK,
    TI__PUMPS_TASKS_END,

    TI__END = TI__PUMPS_TASKS_END,
};

enum EPUMPSTATE
{
    PS__BEGIN,

    PS_ENABLED = PS__BEGIN,
    PS_DISABLED,
    PS_FAILED,

    PS__END,
    PS__DEFAULT = PS_FAILED
};

enum ELEVELPUMPOUTSTATE
{
    LPS__BEGIN,

    LPS_INPROGRESS = LPS__BEGIN,
    LPS_IDLE,

    LPS__END,
    LPS__DEFAULT = LPS_IDLE,
};


struct PumpStateDescriptor
{
    PumpStateDescriptor() {}
    PumpStateDescriptor(void *): m_state(PS__DEFAULT), m_startTime(), m_scheduledStopTime() {}

    void Assign(EPUMPSTATE state, const steady_time_point &startTime, const steady_time_point &scheduledStopTime)
    {
        m_state = state;
        m_startTime = startTime;
        m_scheduledStopTime = scheduledStopTime;
    }

    EPUMPSTATE GetState() const { return m_state; }
    void SetState(EPUMPSTATE value) { m_state = value; }

    void SetStartTime(const steady_time_point &value) { m_startTime = value; }
    void SetStartTimeNow() { m_startTime = boost::chrono::steady_clock::now(); }
    const steady_time_point &GetStartTime() const { return m_startTime; }

    unsigned CalcWorkTimeDurationSeconds() const { ASSERT(m_state == PS_ENABLED); return static_cast<unsigned>(boost::chrono::duration_cast<boost::chrono::seconds>(boost::chrono::steady_clock::now() - m_startTime).count()); }
    
    const steady_time_point &GetScheduledStopTime() const { return m_scheduledStopTime; }
    void SetScheduledStopTime(const steady_time_point &value) { m_scheduledStopTime = value; }
    
    void ResetStateData(EPUMPSTATE state) { Assign(state, steady_time_point(), steady_time_point()); }

    EPUMPSTATE             m_state;
    steady_time_point      m_startTime;
    steady_time_point      m_scheduledStopTime;
    steady_time_point      m_lastWorkingTimePoint;
};


struct DeviceInputValues
{
    DeviceInputValues() {}
    DeviceInputValues(void*) : m_visibleWaterLevelSensorRaw(~((unsigned)0)), m_magicButtonPressed(false), m_visibleContainerWaterLevelCm(0) {}
    
    void Assign(unsigned visibleWaterLevelSensor, bool magicButtonPressed, unsigned visibleContainerWaterLevelCm)
    {
        m_visibleWaterLevelSensorRaw = visibleWaterLevelSensor;
        m_magicButtonPressed = magicButtonPressed;
        m_visibleContainerWaterLevelCm = visibleContainerWaterLevelCm;
    }

    unsigned m_visibleWaterLevelSensorRaw;
    bool     m_magicButtonPressed;
    unsigned m_visibleContainerWaterLevelCm;
};

struct ContainerStateData
{
    ContainerStateData() {}
    ContainerStateData(void *): m_waterAmountMilliliters(0), m_waterLevelMillimeters(0) {}

    void Assign(unsigned waterAmountMilliliters, unsigned waterLevelMillimeters)
    {
        m_waterAmountMilliliters = waterAmountMilliliters;
        m_waterLevelMillimeters = waterLevelMillimeters;
    }

    unsigned        m_waterAmountMilliliters;
    unsigned        m_waterLevelMillimeters;
};


struct LevelPumpOutStateData
{
    static const unsigned InvalidIndexValue = ~((unsigned)0);

    LevelPumpOutStateData() {}
    LevelPumpOutStateData(void *) : m_state(LPS__DEFAULT), m_activationTime()  {}

    void SetState(ELEVELPUMPOUTSTATE state)  { m_state = state; }
    ELEVELPUMPOUTSTATE GetState() const { return m_state; }

    void SetActivationTimeNow() { m_activationTime = boost::chrono::steady_clock::now(); }
    void SetActivationTime(const steady_time_point  &value){ m_activationTime = value; }
    steady_time_point GetActivationTime() const { return m_activationTime; }

    ELEVELPUMPOUTSTATE      m_state;
    steady_time_point       m_activationTime;
};

struct DefferedWaterInputItem
{
    DefferedWaterInputItem() {}
    DefferedWaterInputItem(unsigned amount) : m_amount(amount) {}

    unsigned        m_amount;
};


// TODO: Add simple statistic aggregator to able to see water level dynamics per month
class HardwareServiceImplementation : public IPacketUnFramerListener
{
public:
    HardwareServiceImplementation();
    ~HardwareServiceImplementation();

public:
    void ApplyConfiguration(const string &jsonDocumentText);
    void StartPump(const PumpIdentifier::type);
    void StopPump(StopPumpResult& _return, const PumpIdentifier::type);
    void GetServiceStatus(ServiceStatus& _return);
    void GetServiceStateJson(string &jsonDocumentReceiver);
    void FillVisibleContainerMillilitres(const int32_t amount);
    void EmptyVisiableContainerMillilitres(const int32_t amount);
    void DbgSetContainerWaterLevel(const int32_t amount);

public:
    void StartService();
    void ShutdownService();

private:
    void InitializePumpsState(EPUMPSTATE initialState = PS__DEFAULT);
    void InitializeServiceState();
    void CreateTimerThreads();
    void LoadConfiguration();
    void StartBackgroundTasks();
    void CreateTasksTimers();
    void DestroyTasksTimers();
    void RestartTask(ESERVICETASKID timerId);
    void RestartTaskTimeSpecified(ESERVICETASKID timerId, unsigned timeFromNowMilliseconds);
    void CancelTask(ESERVICETASKID timerId);
    void PreparePeripheral();
    bool DiscoverDevicePort();

private:
    void HeartBeatTask();
    void QueryInputTask();
    void InputPumpControlTask();
    void OutputPumpControlTask();
    void WaterLevelManagerTask();
    void ProcessWaterLevelManagerTaskActions();
    void DoProcessWaterLevelManagerTaskActions();
    void ProcessLevelIndexChangeIfNecessary();
    void ActivateOutomatedOutputPumpingIfNecessary();
    void ShceduleNextPlannedPumpOutActivation();
    void ShceduleFirstPlannedPumpOutActivation();
    void PerformPlannedPumpOutActivation();
    void ActivateOutomatedOutputPumping();
    void InputWaterPorcessManagerTask();
    
private:
    void OnOutputPumpEndedWorking();
    void OnOutputPumpStartedWorking();

private:
    void EnablePumpForSpecifiedTime(EPUMPIDENTIFIER pumpId, unsigned timeMilliseconds);
    void EnsurePumpReadyForWork_RaiseExceptionIfNot(EPUMPIDENTIFIER pumpId);
    void EnsureOutputPumpIsAvailableForFork_RaiseIfNot();
    void ProcessPumpControlActions(EPUMPIDENTIFIER pumpId);
    void ProcessPumpControlActions_ManagePumpControl(EPUMPIDENTIFIER pumpId);
    void ProcessPumpControlActions_SimulatedSensors(EPUMPIDENTIFIER pumpId);
    void EmergencyStop();
    void EmergencyStopWithMessage(const string &message);
    bool StopPumpIfNecessary(EPUMPIDENTIFIER pumpId, bool &out_stopped);
    bool StopPumpIfNecessary(EPUMPIDENTIFIER pumpId);

    bool StartPumpingMilliliters(EPUMPIDENTIFIER pumpId, unsigned millilitersToPump, bool &out_result);

private:
    void DoHeartBeatTask();
    void DoQueryInputTask();

private:
    bool IsDeviceConnected();

private:
    bool ExecuteEnablePumpHardwareCommand(EPUMPIDENTIFIER pumpId);
    bool ExecuteDisablePumpHardwareCommand(EPUMPIDENTIFIER pumpId);
    bool ExecuteHeartbeatCommand(unsigned &out_deviceStatus);
    bool ExecuteReadIOCommand(DeviceInputValues &out_deviceInput);
    bool ExecuteConfigureDeviceCommand();

private:
    template <class TRequest, class TResponse>
    bool SendPacket(const TRequest &request, TResponse &out_response);
    bool SendPacketData(const void *packetData, size_t packetDataSize, void *buffer, size_t packetSize);
    bool DoSendPacketData(const void *packetData, size_t packetDataSize, void *buffer, size_t packetSize);

protected:
    virtual void PacketUnFramerListener_OnCommandParsed(const uint8_t* packetBuffer, size_t packetSize) override;

private:
    bool UnsafeReceivePacket(uint8_t *buffer, const size_t size, size_t &out_size, unsigned timeoutMilliseconds);
    bool UnsafeSendPacket(const uint8_t *packetData, size_t packetDataSize);
    bool UnsafeResetCommunicationState();
    void ResetUnframerState();

private:
    static const unsigned ItemNotFoundIndex = (~(unsigned)0);
    static const unsigned InvalidIndex = ItemNotFoundIndex;

    unsigned GetCurrentWaterLevelMillimiters() const;
    unsigned DecodeDiscreteWaterLevelIndex(unsigned waterLevelMillimiters) const;
    unsigned DecodeFixedWaterLevelIndex(unsigned waterLevelMillimiters) const;
    const LevelConfiguration &GetLevelConfiguration(unsigned index) const;
    unsigned GetPumpingOutvelocityAtLevel(unsigned index);
    unsigned GetPumpPerformance(EPUMPIDENTIFIER pumpId);
    unsigned GetCurrentWaterLevelIndex();
    void BuildFixedWaterLevelHeightsTable();
    unsigned CalculateWorkTimeForSpecifiedPump(EPUMPIDENTIFIER pumpId, unsigned waterAmountMl);

private:
    void ReportEmergencyStop(const string &message);

private:
    void ScheduleWaterInputTask(unsigned amount);
    unsigned GetWaterInputPendingTasksCount() const;

private:
    static steady_time_point GetNowSteadyClockTime() { return boost::chrono::steady_clock::now(); }

private:
    static string DecodeServiceState(ESERVICESTATE code);
    static string DecodeDeviceConnectionState(ECONNECTIONSTATE code);
    static string DecodePumpState(EPUMPSTATE code);
    static string DecodePumpIdentifierName(EPUMPIDENTIFIER code);
    static ESERVICETASKID DecodePumpTaskId(EPUMPIDENTIFIER code);

private:
    typedef lock_guard<mutex> ScopedLock;
    typedef boost::asio::deadline_timer    DeadlineTimer;
    typedef std::shared_ptr<DeadlineTimer> DeadlineTimerPtr;
    typedef void(HardwareServiceImplementation::*TimerExpiredActionType)();
    struct TableEntry
    {
        HardwareServiceImplementation::TimerExpiredActionType action;
        unsigned timeout;
    };
    typedef list<DefferedWaterInputItem> DeferedInputsList;
    typedef vector<pair<unsigned, unsigned>> PumpoutLevelHeightsMinMaxTable;

private:
    void SetServiceState(ESERVICESTATE state) { m_serviceState = state; }
    ESERVICESTATE GetServiceState() { return m_serviceState; }
    void SetDeviceState(ECONNECTIONSTATE value) { m_deviceState = value; }
    ECONNECTIONSTATE GetDeviceState() const { return m_deviceState; }
    void SetServiceConfiguration(const ServiceConfiguration  &value) { m_configuration = value; }
    const ServiceConfiguration  &GetServiceConfiguration() const { return m_configuration; }
    const DeviceInputValues &GetDeviceInputValues() const { return m_deviceInput; }
    DeviceInputValues &GetDeviceInputValuesRef() { return m_deviceInput; }
    void SetDeviceInputValues(const DeviceInputValues &value) { m_deviceInput = value; }
    PumpStateDescriptor &GetPumpStateDescriptorRef(EPUMPIDENTIFIER pumpId) { ASSERT(Utils::InRange(pumpId, PI__BEGIN, PI__END));  return m_pumpsState[pumpId]; }
    const PumpStateDescriptor &GetPumpStateDescriptorRef(EPUMPIDENTIFIER pumpId) const { ASSERT(Utils::InRange(pumpId, PI__BEGIN, PI__END));  return m_pumpsState[pumpId]; }
    mutex  &GetCommunicationLock() { return m_communicationLock; }
    SerialLibCommunicationChannel *GetCommunicationChannel() { return  &m_communicationChannel; }
    DeadlineTimer &GetTimerObjectById(ESERVICETASKID timerId) { return *m_timers[timerId]; }
    const ContainerStateData &GetVisibleContainerStateData() const { return m_visibleContainerStateData; }
    ContainerStateData &GetVisibleContainerStateData() { return m_visibleContainerStateData; }
    const ContainerStateData &GetHiddenContainerStateData() const { return m_hiddenContainerStateData; }
    ContainerStateData &GetHiddenContainerStateDataRef() { return m_hiddenContainerStateData; }
    const LevelPumpOutStateData &GetLevelPumpOutStateData() const { return m_levelPumpOutState; }
    LevelPumpOutStateData &GetLevelPumpOutStateDataRef() { return m_levelPumpOutState; }
    const DeferedInputsList &GetDefferedInputList() const { return m_defferedInputList; }
    DeferedInputsList &GetDefferedInputList() { return m_defferedInputList; }
    mutex &GetDefferedInputListLock() const { return m_defferedInputListLock; }
    const PumpoutLevelHeightsMinMaxTable &GetLevelHeightsMinMaxTable() const { return m_levelHeighsMinMaxTable; }
    PumpoutLevelHeightsMinMaxTable &GetLevelHeightsMinMaxTable()  { return m_levelHeighsMinMaxTable; }
    unsigned GetPreviousWaterLevelIndex() const { return m_previousWaterLevelIndex; }
    void SetPreviousWaterLevelIndex(unsigned value) { m_previousWaterLevelIndex = value; }
    void SetDeviceComportId(const string &value) { m_deviceComportId = value; }
    const string &GetDeviceComportId() const { return m_deviceComportId; }
    IProblemReportingService &GetProblemReportingService() const { return *m_reportingService; }

private:
    mutex                               m_communicationLock;
    SerialLibCommunicationChannel       m_communicationChannel;
    uint8_t                             m_frameBuffer[128];
    size_t                              m_frameBufferSize;
    PacketUnFramer                      m_unframer;
    ECONNECTIONSTATE                    m_deviceState;
    ESERVICESTATE                       m_serviceState;
    boost::thread_group                 m_timerThreads;
    boost::asio::io_service             m_timersIOService;
    boost::asio::io_service::work       m_endlessWork;
    DeviceInputValues                   m_deviceInput;
    DeadlineTimerPtr                    m_timers[TI__END];
    PumpStateDescriptor                 m_pumpsState[PI__END];
    static const TableEntry             m_tasksDescriptors[TI__END];
    ServiceConfiguration                m_configuration;
    ContainerStateData                  m_visibleContainerStateData;
    ContainerStateData                  m_hiddenContainerStateData;
    LevelPumpOutStateData               m_levelPumpOutState;
    DeferedInputsList                   m_defferedInputList;
    mutable mutex                       m_defferedInputListLock;
    PumpoutLevelHeightsMinMaxTable      m_levelHeighsMinMaxTable;
    unsigned                            m_previousWaterLevelIndex;
    string                              m_deviceComportId;
    unique_ptr<IProblemReportingService> m_reportingService;
};


//////////////////////////////////////////////////////////////////////////

STATIC_MAP(ServiceStateNames, ESERVICESTATE, string, SS__BEGIN, SS__END)
{
    "READY",            // SS_READY
    "NOT_CONFIGURED",   // SS_SERVICENOTCONFIGURED
    "FAILED",           // SS_FAILED
};

STATIC_MAP(DeviceStateNames, ECONNECTIONSTATE, string, DS__BEGIN, DS__END)
{
    "CONNECTED",        // CS_CONNECTED
    "DISCONNECTED",     // CS_DISCONNECTED
    "READY"             // CS_READY
};

STATIC_MAP(PumpStateNames, EPUMPSTATE, string, PS__BEGIN, PS__END)
{
    "ENABLED",      // PS_ENABLED
    "DISABLED",     // PS_DISABLED
    "FAILED",       // PS_FAILED
};

STATIC_MAP(PumpIdentifierNames, EPUMPIDENTIFIER, string, PI__BEGIN, PI__END)
{
    "INPUTPUMP",        // PI_INPUTPUMP
    "OUTPUTPUMP"        // PI_OUTPUTPUMP,
};

STATIC_MAP(PumpsTasksIdentifiers, EPUMPIDENTIFIER, ESERVICETASKID, PI__BEGIN, PI__END)
{
    TI_INPUTPUMPTASK,       // PI_INPUTPUMP
    TI_OUTPUTPUMTASK        // PI_OUTPUTPUMP,
};

STATIC_MAP(PumpsTasksSimulatedSensorsMultipliers, EPUMPIDENTIFIER, int, PI__BEGIN, PI__END)
{
    +1,       // PI_INPUTPUMP
    -1        // PI_OUTPUTPUMP,
};

STATIC_MAP(PumpingOutStateNames, ELEVELPUMPOUTSTATE, string, LPS__BEGIN, LPS__END)
{
     "INPROGRESS",      // LPS_INPROGRESS
     "IDLE"             // LPS_IDLE,
};

STATIC_MAP(PumpsPinsNumbers, EPUMPIDENTIFIER, unsigned, PI__BEGIN, PI__END)
{
    INPUTPUMP_PINNUMBER,       // PI_INPUTPUMP
    OUTPUTPUMP_PINNUMBER        // PI_OUTPUTPUMP,
};

struct Range
{
    unsigned Min;
    unsigned Max;
};

