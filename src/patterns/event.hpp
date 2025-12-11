#pragma once
#ifndef PATTERNS_EVENTS_HPP_
#define PATTERNS_EVENTS_HPP_

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <string>

namespace Patterns
{
    enum class EventType
	{
		None = 0, KeyPressed,
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
                               virtual EventType GetEventType() const override { return GetStaticType(); }\
                               virtual const char* GetName() const override { return #type; }

    class IEvent
    {
    public:
        virtual ~IEvent() { };
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }
    };

    class ClickEvent : public IEvent
    {
    public:
        ClickEvent() = default;
        virtual ~ClickEvent() override { };

        EVENT_CLASS_TYPE(KeyPressed)

        std::string ToString() const override
		{
			return "ClickEvent";
		}
    };
    
    class Dispatcher
    {
    public:
        using SlotType = std::function<void(const IEvent&)>;

        void Subscribe(const EventType& descriptor, SlotType&& slot)
        {
            m_observers[descriptor].push_back(slot);
        }

        void Post(const IEvent& event) const
        {
            EventType type = event.GetEventType();

            if (m_observers.find(type) == m_observers.end())
                return;

            auto&& observers = m_observers.at(type);

            for (auto&& observer : observers)
            {
                observer(event);
            }
        }

    private:
        std::map<EventType, std::list<SlotType>> m_observers;
    };


    class ClassObserver
    {
    public:
        void Handle(const IEvent& event)
        {
            if (event.GetEventType() == EventType::KeyPressed)
            {
                const ClickEvent& clickEvent = static_cast<const ClickEvent&>(event);
                std::cout << __PRETTY_FUNCTION__ << ": " << clickEvent.ToString() << std::endl;
            }
        }
    };

    void TestEvent()
    {
        ClassObserver classObserver;
        Dispatcher dispatcher;

        dispatcher.Subscribe(EventType::KeyPressed, std::bind(&ClassObserver::Handle, classObserver, std::placeholders::_1));

        dispatcher.Post(ClickEvent());
    }
}

#endif // PATTERNS_EVENTS_HPP_
