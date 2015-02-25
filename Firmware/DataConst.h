#ifndef __DATACONST_H_INCLUDED
#define __DATACONST_H_INCLUDED

#include <boost/chrono.hpp>

namespace Dataconst
{
    static const int APIDefaultPortNumber = 9090;
	static const int PacketMaxSizeBytes = 256;
    static const int InputBufferSize = 12;
    static const int CommandSendReceiveTimeout = 500;

    static const int HearBeatCommandPeriodMs = 1000;
    static const int QueryInputTaskPeriodMs = 1000;
    static const int InputPipeTaskPeriodMs = 100;
    static const int OutputPipeTaskPeriodMs = 100;
    static const int DeviceStatusTaskPeriodMs = 1000;
    static const int LevelManagerTaskPeriodMs = 1000;
    static const int InputProcessingTaskPeriodMs = 1000;

    static const int TimerThreadPoolSize = 1;
    static const char ServiceConfigurationFilePath[] = "ServiceConfiguration.json";

    static const auto PumpOutActivationPeriod = boost::chrono::minutes(60);
}

#endif // __DATACONST_H_INCLUDED

