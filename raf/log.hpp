#ifndef RAF_LOG_H_
#define RAF_LOG_H_

#include <sstream>
#include "../hlt/log.hpp"
namespace raf {

template< typename T >
inline std::string stringify(const T& t)
{
  std::stringstream string_stream;
  string_stream << t;
  return string_stream.str();
}

template< typename T, typename ... Args >
inline std::string stringify(const T& first, Args ... args)
{
  return stringify(first) + stringify(args...);
}

template< typename T>
inline void Log(const T& first)
{
  auto str = stringify(first);
  hlt::Log::log(str);
}

template< typename T, typename ... Args >
inline void Log(const T& first, Args ... args)
{
  auto str = stringify(first) + stringify(args...);
  hlt::Log::log(str);
}

}

#endif // !RAF_LOG_H_

