#pragma once

#include <exception>
#include <iostream>

#if _MSC_VER
#define FNAME __FUNCTION__
#else
#define FNAME __PRETTY_FUNCTION__
#endif

// For VS2013 and younger constxpr/noexcept isn't implemented
#if _MSC_VER < 1900
#define CONSTXPR 
#define NOEXEPT 
#else
#define CONSTXPR constexpr
#define NOEXEPT noexcept
#endif

/*
#define TRACE_ENTRYEXIT Logging::AutoLog __funcTrace(FNAME)
#define LOG_TRACE(msg) (Log(FNAME, LogData<None>() << setcolour(WHITE) << " [T] " << msg << std::flush))
#define LOG_INFO(msg) (Log(FNAME, LogData<None>() << setcolour(GREEN) << " [I] " << msg << std::flush))
#define LOG_WARNING(msg) (Log(FNAME, LogData<None>() << setcolour(YELLOW) << " [W] " << msg << std::flush))
#define LOG_ERROR(msg) (Log(FNAME, LogData<None>() << setcolour(RED) << " [E] " << msg << std::flush))

#define LOG_NOFUNC_TRACE(msg) (Log(LogData<None>() << setcolour(WHITE) << "[T] " << msg << std::flush))
#define LOG_NOFUNC_INFO(msg) (Log(LogData<None>() << setcolour(GREEN) << "[I] " << msg << std::flush))
#define LOG_NOFUNC_WARNING(msg) (Log(LogData<None>() << setcolour(YELLOW) << "[W] " << msg << std::flush))
#define LOG_NOFUNC_ERROR(msg) (Log(LogData<None>() << setcolour(RED) << "[E] " << msg << std::flush))

#define LOG(msg) (Log(LogData<None>() << msg))
*/

#define TRACE_ENTRYEXIT 
#define LOG_TRACE(msg) 
#define LOG_INFO(msg)
#define LOG_WARNING(msg) 
#define LOG_ERROR(msg)

#define LOG_NOFUNC_TRACE(msg) 
#define LOG_NOFUNC_INFO(msg) 
#define LOG_NOFUNC_WARNING(msg)
#define LOG_NOFUNC_ERROR(msg)

#define LOG(msg)
