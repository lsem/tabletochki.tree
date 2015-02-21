#pragma once

#include "HardwareService.h"
#include "HardwareService_types.h"
#include "ServiceConfiguration.h"
#include "PacketUnFramer.h"
#include "CommunicationChannel.h"
#include "Utils.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>


using Tabletochki::StopPumpResult;
using Tabletochki::ServiceStatus;
using Tabletochki::PumpIdentifier;

typedef boost::chrono::steady_clock::time_point time_point;

class ICommunicationChannel;


enum ESERVICESTATE
{
    SS__BEGIN,

    SS_READY = SS__BEGIN,
    SS_SERVICENOTCONFIGURED,
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


struct PumpStateDescriptor
{
    void Assign(EPUMPSTATE state, const time_point &startTime, unsigned workingTime)
    {
        m_state = state;
        m_startTime = startTime;
        m_workingTime = workingTime;
    }

    EPUMPSTATE GetState() const { return m_state; }
    void SetState(EPUMPSTATE value) { m_state = value; }
    
    void SetStartTimeNow() { m_startTime = boost::chrono::steady_clock::now(); }
    const time_point &GetStartTime() const { return m_startTime; }
    
    mutex           &GetAccessLock() const { return m_accessLock; }

    EPUMPSTATE      m_state;
    time_point      m_startTime;
    unsigned        m_workingTime;
    mutable mutex   m_accessLock;
};


struct DeviceInputValues
{
    DeviceInputValues() {}
    DeviceInputValues(void*) : m_proximitySensorValue(~((unsigned)0)), m_magicButtonPressed(false), m_visibleContainerWaterLevelMillimeters(0) {}
    
    void Assign(unsigned proximitySensorValue, bool magicButtonPressed, unsigned visibleContainerWaterLevelMillimeters)
    {
        m_proximitySensorValue = proximitySensorValue;
        m_magicButtonPressed = magicButtonPressed;
        m_visibleContainerWaterLevelMillimeters = visibleContainerWaterLevelMillimeters;
    }

    unsigned m_proximitySensorValue;
    bool     m_magicButtonPressed;
    unsigned m_visibleContainerWaterLevelMillimeters;
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

enum ELEVELPUMPOUTSTATE
{
    LPS__BEGIN,

    LPS_INPROGRESS = LPS__BEGIN,
    LPS_DONE,

    LPS__END,
    LPS__DEFAULT = LPS_DONE,
};

struct LevelPumpOutStateData
{
    static const unsigned DefaultLastDiscreteLevelIndex = 0;

    LevelPumpOutStateData() {}
    LevelPumpOutStateData(void *) : m_state(LPS__DEFAULT), m_lastActivationTime(), m_lastDiscreteLevelIndex(DefaultLastDiscreteLevelIndex) {}

    void SetState(ELEVELPUMPOUTSTATE state)  { m_state = state; }
    ELEVELPUMPOUTSTATE GetState() const { return m_state; }

    const time_point &GetLastActivationTime() const {return m_lastActivationTime; }
    void SetLastActivationTimeNow() { m_lastActivationTime = boost::chrono::steady_clock::now(); }

    unsigned GetLastDiscreteLevelIndex() const { return m_lastDiscreteLevelIndex; }
    void SetLastDiscreteLevelIndex(unsigned value) { m_lastDiscreteLevelIndex = value; }

    void Reset()
    {
        m_state = LPS__DEFAULT;
        m_lastActivationTime = time_point();
        m_lastDiscreteLevelIndex = DefaultLastDiscreteLevelIndex;
    }

    ELEVELPUMPOUTSTATE      m_state;
    time_point              m_lastActivationTime;
    unsigned                m_lastDiscreteLevelIndex;
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

private:
    void HeartBeatTask();
    void QueryInputTask();
    void InputPumpControlTask();
    void OutputPumpControlTask();
    void WaterLevelManagerTask();

private:
    void EnablePumpForSpecifiedTime(EPUMPIDENTIFIER pumpId, unsigned timeMilliseconds);
    void EnsurePumpReadyForWork_RaiseExceptionIfNot(EPUMPIDENTIFIER pumpId);
    void ProcessPumpControlActions(EPUMPIDENTIFIER pumpId);
    void EmergencyStop();

private:
    void DoHeartBeatTask();
    void DoQueryInputTask();
    bool DoCheckDeviceConnection();
    bool DoEnablePump(EPUMPIDENTIFIER pumpId);
    bool DoDisablePump(EPUMPIDENTIFIER pumpId);

private:
    bool SendHeartbeatCommand(unsigned &out_deviceStatus);
    bool SendReadIOCommand(DeviceInputValues &out_deviceInput);
    bool SendConfigureDeviceCommand();

private:
    template <class TRequest, class TResponse>
    bool SendPacket(const TRequest &request, TResponse &out_response);
    bool SendPacketData(const void *packetData, size_t packetDataSize, void *buffer, size_t packetSize);
    bool DoSendPacketData(const void *packetData, size_t packetDataSize, void *buffer, size_t packetSize);

protected:
    virtual void PacketUnFramerListener_OnCommandParsed(const uint8_t* packetBuffer, size_t packetSize);

private:
    bool UnsafeReceivePacket(uint8_t *buffer, const size_t size, size_t &out_size, unsigned timeoutMilliseconds);
    bool UnsafeSendPacket(const uint8_t *packetData, size_t packetDataSize);
    bool UnsafeResetCommunicationState();
    void ResetUnframerState();

private:
    unsigned GetCurrentWaterLevelMillimiters() const;
    unsigned DecodeDiscreteWaterLevelIndex(unsigned waterLevelMillimiters) const;
    static const unsigned ItemNotFoundIndex = (~(unsigned)0);
    const LevelConfiguration &GetLevelConfiguration(unsigned index) const;

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

private:
    void SetServiceState(ESERVICESTATE state) { m_serviceState = state; }
    ESERVICESTATE GetServiceState() { return m_serviceState; }
    void SetDeviceState(ECONNECTIONSTATE value) { m_deviceState = value; }
    ECONNECTIONSTATE GetDeviceState() const { return m_deviceState; }
    void SetServiceConfiguration(const ServiceConfiguration  &value) { m_configuration = value; }
    const ServiceConfiguration  &GetServiceConfiguration() const { return m_configuration; }
    const DeviceInputValues &GetDeviceInputValues() const { return m_deviceInput; }
    void SetDeviceInputValues(const DeviceInputValues &value) { m_deviceInput = value; }
    PumpStateDescriptor &GetPumpDescriptorRef(EPUMPIDENTIFIER pumpId) { ASSERT(Utils::InRange(pumpId, PI__BEGIN, PI__END));  return m_pumpsState[pumpId]; }
    const PumpStateDescriptor &GetPumpDescriptorRef(EPUMPIDENTIFIER pumpId) const { ASSERT(Utils::InRange(pumpId, PI__BEGIN, PI__END));  return m_pumpsState[pumpId]; }
    mutex  &GetCommunicationLock() { return m_communicationLock; }
    SerialLibCommunicationChannel *GetCommunicationChannel() { return  &m_communicationChannel; }
    DeadlineTimer &GetTimerObjectById(ESERVICETASKID timerId) { return *m_timers[timerId]; }
    const ContainerStateData &GetVisibleContainerStateData() const { return m_visibleContainerStateData; }
    ContainerStateData &GetVisibleContainerStateData() { return m_visibleContainerStateData; }
    const ContainerStateData &GetHiddenContainerStateData() const { return m_hiddenContainerStateData; }
    ContainerStateData &GetHiddenContainerStateDataRef() { return m_hiddenContainerStateData; }
    const LevelPumpOutStateData &GetLevelPumpOutStateData() const { return m_levelPumpOutState; }
    LevelPumpOutStateData &GetLevelPumpOutStateDataRef() { return m_levelPumpOutState; }

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
