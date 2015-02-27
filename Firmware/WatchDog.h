#pragma once



class IWatchDogManagerListener
{
public:
    virtual void WatchDogManagerListener_OnWatchdogTimerExpired() = 0;
};

class IWatchDogManager
{
public:
    virtual void EnableWatchdog() = 0;
    virtual void DisableWatchdog() = 0;
    virtual void ResetWatchdog() = 0;
    virtual void CheckExpired() = 0;
};


#ifndef _WIN32
class ArduinoSoftWatchDogTimerManager : public IWatchDogManager
{
public:
    ArduinoSoftWatchDogTimerManager(IWatchDogManagerListener *i_listener) :
        m_started(false), 
        m_time(),
        m_ilistener(i_listener)
    {
    }

    ArduinoSoftWatchDogTimerManager() :
        m_started(false),
        m_time(),
        m_ilistener(NULL)
    {
    }

public:
    void SetListener(IWatchDogManagerListener *i_listener)
    {
        m_ilistener = i_listener;
    }

public:
    virtual void EnableWatchdog()
    {
        ASSERT(m_ilistener != NULL);

        m_started = true;
        m_time = ::millis();
    }

    virtual void DisableWatchdog()
    {
        m_started = false;
    }

    virtual void ResetWatchdog()
    {
        ASSERT(m_ilistener != NULL);

        m_time = ::millis();
    }

    virtual void CheckExpired()
    {
        ASSERT(m_ilistener != NULL);

        bool result = false;

        if (m_started)
        {
            unsigned long timeMs = ::millis();
            if (timeMs < m_time)
            {
                // overflow; according to Reference, happens ~ each 50 days
                m_time = ::millis(); // reset, it is ok for our purposes to allow to not reset for 2 seconds once for 50 days                
            }
            else
            {
                const bool wasResetInOneSecond = (millis() - m_time) <= 1000;
                result = !wasResetInOneSecond;
            }
        }

        if (result)
        {
            m_ilistener->WatchDogManagerListener_OnWatchdogTimerExpired();
        }
    }

private:
    bool m_started;
    unsigned long m_time;
    IWatchDogManagerListener *m_ilistener;
};
#endif // _WIN32
