#pragma once


class IWatchDogManagerListener
{
public:
    virtual void OnWatchdogTimerExpired() = 0;
};

class IWatchDogManager
{
public:
    virtual void EnableWatchdog() = 0;
    virtual void DisableWatchdog() = 0;
    virtual void ResetWatchdog() = 0;
};
