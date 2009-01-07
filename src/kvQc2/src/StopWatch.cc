#include <iostream>
#include <ctime>

using namespace std;


class stopwatch
{
public:
 stopwatch() : start(std::clock()){} //start counting time
 ~stopwatch();
private:
 std::clock_t start;
};

stopwatch::~stopwatch()
{
 clock_t total = clock()-start; //get elapsed time
 cout<<"total of ticks for this activity: "<<total<<endl;
 cout<<"Block in seconds: "<<double(total)/CLOCKS_PER_SEC<<endl;
}
