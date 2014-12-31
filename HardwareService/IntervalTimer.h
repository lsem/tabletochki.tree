#pragma once

#include <thread>
#include <list>
#include <chrono>
#include <cassert>
#include <condition_variable>
#include <map>


// Simple, naive interval timer implementation. Performance is linear to the number of timers created.
class IntervalTimer
{
public:
    IntervalTimer():
        m_callBack(),
        m_nextShot(),
        m_interval(),
        m_enabled(false),
        m_oneShot()
    {}

    IntervalTimer(unsigned interval, bool oneShot=false):
        m_callBack(),
        m_nextShot(),
        m_interval(interval),
        m_enabled(false),
        m_oneShot(oneShot)
    {}

    void Configure(int milliseconds, std::function<void()> callBack)
    {
        assert(m_callBack == nullptr);
        assert(m_interval == 0);

        m_interval = milliseconds;
        m_callBack = callBack;
    }

    void SetInterval(int milliseconds)
    {
        m_interval = milliseconds;
    }
    
    void Enable()
    {
        m_nextShot = std::chrono::system_clock::now() + std::chrono::milliseconds(m_interval);
        m_enabled = true;
        
        ScheduleTimer(this);
    }

    void Disable()
    {
        m_enabled = false;
        
        IntervalTimer::m_jobAvailalbe = true;
        IntervalTimer::m_wakeUp.notify_all();
    }

    void SetOnTimerAction(std::function<void()> callBack)
    {
        assert(m_callBack == nullptr);

        m_callBack = callBack;
    }

    static void StopAllThreads()
    {
        m_stopAll = true;
        m_callbacksAvailable.notify_all();
        m_wakeUp.notify_all();

        //std::unique_lock<std::mutex> lock(m_stoppedLock);
        //while (m_stoppedCount < 2)
        //    m_stopped.wait(lock);
    }


private:
    static IntervalTimer *GetClosestShotTimer()
    {
        IntervalTimer *closestShotTimer = nullptr;

        std::lock_guard<std::mutex> lock(m_listLock);

        for (auto timer : m_entriesList)
        {
            if (timer->m_enabled)
            {
                if (closestShotTimer == nullptr)
                    closestShotTimer = timer;
                else
                {
                    if (timer->m_nextShot < closestShotTimer->m_nextShot)
                        closestShotTimer = timer;
                }
            }
        }

        return closestShotTimer;
    }

    static void CallbackCallerThreadMethod()
    {
        while (!m_stopAll)
        {
            {
                std::unique_lock<std::mutex> waitLock(m_callbacksAvailableLock);
                while (!m_callbacksAvailableVar && !m_stopAll)
                    m_callbacksAvailable.wait(waitLock);
                m_callbacksAvailableVar = false;
            }

            if (m_stopAll)
                break;

            {
                std::lock_guard<std::mutex> accessLock(m_callbacksAvailableLock);
                
                while (!m_callbacksQueue.empty())
                {
                    CallbackQueueEntry entry = m_callbacksQueue.front();
                    m_callbacksQueue.pop_front();
                    entry.m_callBack();
                }
            }
        }

        ++m_stoppedCount;
        m_stopped.notify_one();
    }

    static void ThreadMethod() 
    {
        using namespace std;
        
        while (!m_stopAll)
        {
            auto closestShotTimer = GetClosestShotTimer();

            if (closestShotTimer != nullptr)
            {
                auto sleepDuration= std::chrono::duration_cast<chrono::milliseconds>(
                    closestShotTimer->m_nextShot - chrono::high_resolution_clock::now());
                
                const auto sleepStart = chrono::system_clock::now();

                if (sleepDuration.count() < TimerSensitivityMs || sleepDuration.count() < 0)
                {
                    FireTimerIfEnabled(closestShotTimer);
                }
                else
                {
                    unique_lock<mutex> lock(m_waitLock);
                    
                    m_wakeUp.wait_for(lock, sleepDuration);
                    
                    if (m_stopAll)
                        break;

                    if (m_jobAvailalbe)
                        m_jobAvailalbe = false;

                    const auto wakeUpTime = chrono::high_resolution_clock::now();

                    if (wakeUpTime < closestShotTimer->m_nextShot)
                    {
                        auto nextShotDistance = chrono::duration_cast<chrono::milliseconds>(closestShotTimer->m_nextShot - wakeUpTime);
                        
                        if (nextShotDistance.count() <= TimerSensitivityMs)
                        {
                            FireTimerIfEnabled(closestShotTimer);
                        }
                    }
                    else if (wakeUpTime >= closestShotTimer->m_nextShot)
                    {
                        FireTimerIfEnabled(closestShotTimer);
                    }
                }
            }
            else
            {
                unique_lock<mutex> lock(m_waitLock);
                while (!m_jobAvailalbe && !m_stopAll)
                    m_wakeUp.wait(lock);
                m_jobAvailalbe = false;
            }
        }

        ++m_stoppedCount;
        m_stopped.notify_one();

    }

    static void FireTimerIfEnabled(IntervalTimer *instance)
    {
        if (instance->m_enabled)
        {
            instance->m_nextShot = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(instance->m_interval);

            bool wasEmpty = false;

            {
                std::lock_guard<std::mutex> lock(m_callbacksAvailableLock);
                wasEmpty = m_callbacksQueue.empty();
                m_callbacksQueue.push_back(CallbackQueueEntry(instance->m_callBack));
            }

            if (wasEmpty)
            {
                m_callbacksAvailableVar = true;
                m_callbacksAvailable.notify_one();
            }
        }
    }

    static void const ScheduleTimer(IntervalTimer *timerInstance)
    {
        m_listLock.lock();
        m_entriesList.push_back(timerInstance);
        m_listLock.unlock();

        m_jobAvailalbe = true;
        m_wakeUp.notify_all();
    }

private:
    std::function<void()>                       m_callBack;
    std::chrono::system_clock::time_point       m_nextShot;
    unsigned                                    m_interval;
    bool                                        m_enabled;
    bool                                        m_oneShot;

private:
    struct CallbackQueueEntry 
    {
        CallbackQueueEntry(std::function<void()> callback)
            : m_callBack(callback) {}

        std::function<void()> m_callBack;
    };

private:
    static std::list<IntervalTimer*>            m_entriesList;
    static std::list<CallbackQueueEntry>        m_callbacksQueue;
    static std::mutex                           m_waitLock;
    static std::condition_variable              m_wakeUp;
    static std::condition_variable              m_stopped;
    static std::mutex                           m_stoppedLock;
    static int                                  m_stoppedCount;
    static std::condition_variable              m_callbacksAvailable;
    static bool                                 m_callbacksAvailableVar;
    static std::mutex                           m_callbacksAvailableLock;
    static bool                                 m_jobAvailalbe;
    static bool                                 m_newTimerAdded;
    static volatile bool                        m_stopAll;
    static std::mutex                           m_listLock;
    static std::thread                          m_thread;
    static std::thread                          m_callbackCallerThread;
    static const unsigned                       TimerSensitivityMs = 5;
};
