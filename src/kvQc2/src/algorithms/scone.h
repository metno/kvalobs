
#include <sstream>

template<typename T, typename F> T StrmConvert( F from )
{
  std::stringstream temp;
  temp << from;
  T to = T();
  temp >>to;
  return to;
}

template<typename F>std::string StrmConvert( F from )
{
  return StrmConvert<std::string>( from );
}

