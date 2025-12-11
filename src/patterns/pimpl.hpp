#pragma once
#ifndef PATTERNS_PIMPL_HPP_
#define PATTERNS_PIMPL_HPP_

#include <memory>
#include <string>

namespace Patterns
{
    class User
    {
    public:
        User(std::string name);
        ~User();

        User(const User& other);
        User& operator=(User rhs);

        int GetSalary();

        void SetSalary(int salary);

    private:
        // Internal implementation class
        class Impl;

        // Pointer to the internal implementation
        std::unique_ptr<Impl> m_pimpl;
    };
}

#endif // PATTERNS_PIMPL_HPP_
