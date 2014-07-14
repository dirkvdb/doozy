// Copyright (c) 1995-2014 by FEI Company
// All rights reserved. This file includes confidential and
// proprietary information of FEI Company.

#pragma once

#include "stdafx.h"
#include "EventNotification.h"

#pragma warning(push)
#pragma warning(disable : 4482)

namespace Fei { namespace MdlStage { namespace Test {

template <typename EventEnum, int NumEvents>
class EventListenerMock
{
public:
    virtual ~EventListenerMock() {}

    bool WaitForEvent(EventEnum event)
    {
        return m_notifications[event].WaitForEvent();
    }

    bool WaitForMultipleEvents(EventEnum event, long count)
    {
        return m_notifications[event].WaitForMultipleEvents(count);
    }

    void Reset()
    {
        for (int i = 0; i < NumEvents; ++i)
        {
            m_notifications[i].Reset();
        }
    }

protected:
    EventNotification   m_notifications[NumEvents];
};

}}}

