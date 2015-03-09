#ifndef __DATACONST_H_INCLUDED
#define __DATACONST_H_INCLUDED

#if defined(_WIN32) && !defined(TESTS)
#include <boost/chrono.hpp>
#endif // _WIN32 && TESTS

namespace Dataconst
{
    static const int APIDefaultPortNumber = 35001;
	static const int PacketMaxSizeBytes = 256;
    static const int InputBufferSize = 12;
    static const int CommandSendReceiveTimeout = 500;

    static const int HearBeatCommandPeriodMs = 500;
    static const int QueryInputTaskPeriodMs = 100;
    static const int InputPipeTaskPeriodMs = 500;
    static const int OutputPipeTaskPeriodMs = 500;
    static const int DeviceStatusTaskPeriodMs = 1000;
    static const int LevelManagerTaskPeriodMs = 150;
    static const int InputProcessingTaskPeriodMs = 1000;

    static const int TimerThreadPoolSize = 1;
    static const char ServiceConfigurationFilePath[] = "ServiceConfiguration.json";
    static const char PumpoutTableFixDeltaCm = 3;

#if defined(_WIN32) && !defined(TESTS)
    static const auto PumpOutActivationPeriod = boost::chrono::minutes(60);
    static const auto EmergencyStopHartbeatTaskPauseSec = 10;
#endif // _WIN32 && TESTS
}

#endif // __DATACONST_H_INCLUDED

