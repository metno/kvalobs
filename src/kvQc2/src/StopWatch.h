#include <iostream>
#include <ctime>

using namespace std;

/// Simple class to record cpu time usage for performance
/// monitoring during development and testing.

class stopwatch
{
public:
 stopwatch() : start(std::clock()){} 
 ~stopwatch();
private:
 std::clock_t start;
};
