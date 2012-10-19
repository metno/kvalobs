#include <stdio.h>
#include <puTools/miTime.h>
#include <fstream>

using namespace miutil;
using namespace std;


int
main( int argc, char **argv )
{
	miTime nowTime( miTime::nowTime() );
	miTime endTime;
	miTime itTime;
	char buf[32];
	float ta;
	ofstream ofs;
	
	ofs.open( "18700.txt" );
	
	nowTime = miTime( nowTime.year(), nowTime.month(), nowTime.day(), nowTime.hour() );
	endTime = nowTime;
	
	endTime.addDay( -3 );
	
	cerr << "Now:     " << nowTime << endl;
	cerr << "endTime: " << endTime << endl;
	
	ofs << "autoobs/nationalnr=018700/type=3" << endl;
	ofs << "TA" << endl;
	
	for( ta=-10.0, itTime = endTime; itTime<=nowTime; itTime.addHour( 1 ), ta+=0.5) {
		sprintf( buf, "%4d%02d%02d%02d%02d%02d", itTime.year(), itTime.month(), itTime.day(),
				itTime.hour(), itTime.min(), itTime.sec() );       
		
		ofs << buf << "," << ta << endl;
	}
	
	ofs.close();
	
}

