#include "pimpl.hpp"

#include <iostream>
#include <format>

namespace Patterns
{
    struct User::Impl
    {
        Impl(std::string name) : Name(std::move(name)) { };
        ~Impl() { }

        void WelcomeMessage()
        {
            std::cout << std::format("Welcome, {}", Name) << std::endl;
        }

        std::string Name = "";
        int Salary = -1;
    };

    User::User(std::string name) : m_pimpl(new Impl(std::move(name)))
    {
        m_pimpl->WelcomeMessage();
    }

    User::~User() = default;

    // Assignment operator and Copy constructor

    User::User(const User& other) : m_pimpl(new Impl(*other.m_pimpl)) { }

    User& User::operator=(User rhs)
    {
        swap(m_pimpl, rhs.m_pimpl);
        return *this;
    }

    int User::GetSalary()
    {
        return m_pimpl->Salary;
    }

    void User::SetSalary(int salary)
    {
        m_pimpl->Salary = salary;
        std::cout << std::format("Salary set to {}", salary) << std::endl;
    }
}
