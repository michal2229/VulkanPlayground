/*
* Header with tools enabling simple debug.
*
* This code is licensed under the GNU General Public License v3.0
*/

#include <iostream>
#include <typeinfo>

// Pretty function tracing in terminal.

#define DEBUG229
#define DEBUG_INLOOP
#define DEBUG_CALLEDONCE
#define DEBUG_COND


#define SPACES for(uint32_t i = 0u; i < functionLevel*4u; i++) { std::cout << " "; }

#define PRINTFUNNAME std::cout << "--> " << "function" << " " << __FUNCTION__ << "()\n";
#define PRINTMETNAME std::cout << "<-- " << typeid(*this).name() << "::" << __FUNCTION__ << "()\n";
#define ONCOND       std::cout << "ON COND: ";

#if defined(DEBUG_INLOOP)
    #define FUN_BEGIN    SPACES PRINTFUNNAME functionLevel++;
    #define FUN_END      functionLevel--; SPACES PRINTFUNNAME
    #define MET_BEGIN    SPACES PRINTMETNAME functionLevel++;
    #define MET_END      functionLevel--; SPACES PRINTMETNAME
#else
    #define FUN_BEGIN    {}
    #define FUN_END      {}
    #define MET_BEGIN    {}
    #define MET_END      {}
#endif

#if defined(DEBUG_INLOOP)
    #define INLOOP_FUN_BEGIN FUN_BEGIN
    #define INLOOP_FUN_END   FUN_END
    #define INLOOP_MET_BEGIN MET_BEGIN
    #define INLOOP_MET_END   MET_END
#else
    #define INLOOP_FUN_BEGIN {}
    #define INLOOP_FUN_END   {}
    #define INLOOP_MET_BEGIN {}
    #define INLOOP_MET_END   {}
#endif

#if defined(DEBUG_CALLEDONCE)
    #define CALLEDONCE_FUN_BEGIN FUN_BEGIN
    #define CALLEDONCE_FUN_END   FUN_END
    #define CALLEDONCE_MET_BEGIN MET_BEGIN
    #define CALLEDONCE_MET_END   MET_END
#else
    #define CALLEDONCE_FUN_BEGIN {}
    #define CALLEDONCE_FUN_END   {}
    #define CALLEDONCE_MET_BEGIN {}
    #define CALLEDONCE_MET_END   {}
#endif

#if defined(DEBUG_COND)
    #define COND_FUN_BEGIN FUN_BEGIN
    #define COND_FUN_END   FUN_END
    #define COND_MET_BEGIN MET_BEGIN
    #define COND_MET_END   MET_END
#else
    #define COND_FUN_BEGIN {}
    #define COND_FUN_END   {}
    #define COND_MET_BEGIN {}
    #define COND_MET_END   {}
#endif
