#include "EventObserver.h"
#include "Enums.h"

#include <vector>
#include <unordered_map>

namespace App
{
    class EventManager
    {
    public:
        static EventManager& Instance()
        {
            static EventManager s_Instance;
            return s_Instance;
        }

        void Subscribe(Event event, EventObserver* observer)
        {
            m_Observers[event].push_back(observer);
        }

        void Emit(Event event)
        {
            if (m_Observers.find(event) != m_Observers.end())
            {
                for (EventObserver* observer : m_Observers[event])
                {
                    observer->OnEvent(event);
                }
            }
        }

    private:
        std::unordered_map<Event, std::vector<EventObserver*>> m_Observers;
    };
}