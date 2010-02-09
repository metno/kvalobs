#include "AbstractAgregatorTest.h"
#include <kvalobs/kvDataOperations.h>

using agregator::AbstractAgregator;

TEST_P(AbstractAgregatorTest, testGetTimeSpanAtGenerationPoint)
{
	const std::set<miutil::miClock> & generateWhen = GetParam()->generateWhen();
	const miutil::miClock toTest(* generateWhen.begin());
	const kvalobs::kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	miutil::miTime time(miutil::miDate("2007-06-06"),toTest);
	const kvalobs::kvData d = dataFactory.getData( 15, 1, time );
	
	const AbstractAgregator::TimeSpan timeSpan = GetParam()->getTimeSpan(d);

	ASSERT_EQ(time, timeSpan.second);
	
	time.addHour(- GetParam()->interestingHours());
	ASSERT_EQ(time, timeSpan.first);
}


TEST_P(AbstractAgregatorTest, testGetTimeSpan)
{
	const std::set<miutil::miClock> & generateWhen = GetParam()->generateWhen();
	const miutil::miClock toTest(* generateWhen.begin());
	const kvalobs::kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	miutil::miTime time(miutil::miDate("2007-06-06"),toTest);
	
	miutil::miTime triggerTime = time;
	triggerTime.addHour(-1);
	const kvalobs::kvData d = dataFactory.getData( 15, 1, triggerTime );
	
	const AbstractAgregator::TimeSpan timeSpan = GetParam()->getTimeSpan(d);
	
	ASSERT_EQ(time, timeSpan.second);
	
	time.addHour(- GetParam()->interestingHours());
	ASSERT_EQ(time, timeSpan.first);
}
