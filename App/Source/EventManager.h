#include "UI/ImguiWidgetBase.h"
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
            static EventManager instance;
            return instance;
        }

        void Subscribe(Event event, ImguiWidgetBase* observer)
        {
            observers[event].push_back(observer);
        }

        void Emit(Event event)
        {
            if (observers.find(event) != observers.end())
            {
                for (ImguiWidgetBase* observer : observers[event])
                {
                    observer->OnEvent(event);
                }
            }
        }

    private:
        std::unordered_map<Event, std::vector<ImguiWidgetBase*>> observers;
    };
}