// Copyright (c) 1995-2014 by FEI Company
// All rights reserved. This file includes confidential and
// proprietary information of FEI Company.

#ifndef EVENT_NOTIFICATION_H
#define EVENT_NOTIFICATION_H

#include <chrono>
#include <mutex>
#include <condition_variable>

namespace doozy
{
namespace test
{

class EventNotification
{
public:
    EventNotification()
    : m_EventOccurred(false)
    , m_HitCount(0)
    {
    }

    void triggerEvent()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_EventOccurred = true;
        ++m_HitCount;
        m_Condition.notify_all();
    }

    bool hasEventOccurred() const
    {
        return m_EventOccurred;
    }

    bool waitForEvent()
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        if (m_HitCount == 0)
        {
            return std::cv_status::no_timeout == m_Condition.wait_for(lock, std::chrono::seconds(5));
        }

        m_HitCount = 0;
        return true;
    }

    bool waitForMultipleEvents(long count)
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        bool ok = (m_HitCount >= count);
        long tries = 0;

        while (!ok && m_HitCount < count && tries++ < count)
        {
            m_Condition.wait_for(lock, std::chrono::seconds(5));
            ok = (m_HitCount >= count);
        }     
        
        m_HitCount = 0;
        return ok;
    }

    void reset()
    {
        m_EventOccurred = false;
        m_HitCount = 0;
    }

private:
    bool                        m_EventOccurred;
    std::mutex                  m_Mutex;
    std::condition_variable     m_Condition;
    long                        m_HitCount;
};

}
}

#endif