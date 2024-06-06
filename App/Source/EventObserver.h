#pragma once

#include "UI/ImguiWidgetBase.h"
#include "Enums.h"


namespace App
{
    class EventObserver
    {
    public:
        virtual void OnEvent(Event event) {};
    };
}