/*
* Header with tools enabling simple debug.
*
* This code is licensed under the GNU General Public License v3.0
*/

#include <iostream>
#include <typeinfo>

// Pretty function tracing in terminal.



#define SPACES    for (uint32_t i = 0u; i < functionLevel*4u; i++) std::cout << " ";

#define MET_BEGIN      \
    SPACES             \
    std::cout << "--> " << typeid(*this).name() << "::" << __FUNCTION__ << "()\n"; \
    functionLevel++; \

#define MET_END        \
    functionLevel--; \
    SPACES             \
    std::cout << "<-- " << typeid(*this).name() << "::" << __FUNCTION__ << "()\n";

#define FUN_BEGIN      \
    SPACES             \
    std::cout << "--> " << "function" << " " << __FUNCTION__ << "()\n"; \
    functionLevel++; \

#define FUN_END        \
    functionLevel--; \
    SPACES             \
    std::cout << "<-- " << "function" << " " << __FUNCTION__ << "()\n";
