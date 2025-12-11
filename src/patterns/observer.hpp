#pragma once
#ifndef PATTERNS_OBSERVER_HPP_
#define PATTERNS_OBSERVER_HPP_

#include <functional>
#include <iostream>
#include <memory>
#include <list>
#include <string>

namespace Patterns
{
    class IObserver
    {
    public:
        virtual ~IObserver() { };
        virtual void Update(const std::string& message) = 0;
    };

    class ISubject
    {
    public:
        virtual ~ISubject() { };
        virtual void Attach(IObserver* observer) = 0;
        virtual void Detach(IObserver* observer) = 0;
        virtual void Notify() = 0;
    };

    class Subject : public ISubject
    {
    public:
        virtual ~Subject() override
        {
            std::cout << "Subject desctructor\n";
        }

        /**
         * The subscription management methods.
         */
        void Attach(IObserver* observer) override
        {
            m_listObserver.push_back(observer);
        }
        
        void Detach(IObserver* observer) override
        {
            m_listObserver.remove(observer);
        }

        void Notify() override
        {
            std::list<IObserver *>::iterator iterator = m_listObserver.begin();
            HowManyObserver();

            while (iterator != m_listObserver.end())
            {
                (*iterator)->Update(m_message);
                ++iterator;
            }
        }

        void CreateMessage(std::string message = "Empty")
        {
            m_message = message;
            Notify();
        }
        
        void HowManyObserver()
        {
            std::cout << "There are " << m_listObserver.size() << " observers in the list.\n";
        }

        /**
         * Usually, the subscription logic is only a fraction of what a Subject can
         * really do. Subjects commonly hold some important business logic, that
         * triggers a notification method whenever something important is about to
         * happen (or after it).
         */
        void SomeBusinessLogic()
        {
            m_message = "Change message message";
            Notify();
            std::cout << "Notifying events\n";
        }

    private:
        std::list<IObserver*> m_listObserver;
        std::string m_message;
    };

    class Observer : public IObserver
    {
    public:
        Observer(Subject& subject) : m_subject(subject)
        {
            m_subject.Attach(this);
            std::cout << "Observer \"" << ++Observer::m_staticNumber << "\" Created\n";
            m_number = Observer::m_staticNumber;
        }

        virtual ~Observer()
        {
            std::cout << "Observer \"" << m_number << "\" Destroyed\n";
        }

        void Update(const std::string& message) override
        {
            m_message = message;
            PrintInfo();
        }
        
        void RemoveMeFromTheList()
        {
            m_subject.Detach(this);
            std::cout << "Observer \"" << m_number << "\" removed from the list.\n";
        }
        
        void PrintInfo()
        {
            std::cout << "Observer \"" << m_number << "\": a new message is available --> " << m_message << "\n";
        }

    private:
        std::string m_message = "";
        Subject& m_subject;
        inline static int m_staticNumber = 0;
        int m_number = 0;
    };

    void TestObserver()
    {
        Subject* subject = new Subject();
        Observer* observer1 = new Observer(*subject);
        Observer* observer2 = new Observer(*subject);
        Observer* observer3 = new Observer(*subject);
        Observer* observer4;
        Observer* observer5;

        subject->CreateMessage("Hello World");
        observer3->RemoveMeFromTheList();

        subject->CreateMessage("The weather is hot today");
        observer4 = new Observer(*subject);

        observer2->RemoveMeFromTheList();
        observer5 = new Observer(*subject);

        subject->CreateMessage("My new car is great");
        observer5->RemoveMeFromTheList();

        observer4->RemoveMeFromTheList();
        observer1->RemoveMeFromTheList();

        delete observer5;
        delete observer4;
        delete observer3;
        delete observer2;
        delete observer1;
        delete subject;
    }
}

#endif // PATTERNS_OBSERVER_HPP_