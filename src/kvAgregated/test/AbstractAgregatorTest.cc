#include "AbstractAgregatorTest.h"
#include <kvalobs/kvDataOperations.h>

using agregator::AbstractAgregator;

AbstractAgregatorTest::AbstractAgregatorTest()
	: agregator(0)
{
}

AbstractAgregatorTest::~AbstractAgregatorTest()
{
}

void AbstractAgregatorTest::testGetTimeSpanAtGenerationPoint()
{
	const std::set<miutil::miClock> & generateWhen = agregator->generateWhen(); 
	const miutil::miClock toTest(* generateWhen.begin());
	const kvalobs::kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	miutil::miTime time(miutil::miDate("2007-06-06"),toTest);
	const kvalobs::kvData d = dataFactory.getData( 15, 1, time );
	
	const AbstractAgregator::TimeSpan timeSpan = agregator->getTimeSpan(d);

	CPPUNIT_ASSERT_EQUAL(time, timeSpan.second);
	
	time.addHour(- agregator->interestingHours());
	CPPUNIT_ASSERT_EQUAL(time, timeSpan.first);	
}


void AbstractAgregatorTest::testGetTimeSpan()
{
	const std::set<miutil::miClock> & generateWhen = agregator->generateWhen();
	const miutil::miClock toTest(* generateWhen.begin());
	const kvalobs::kvDataFactory dataFactory( 42, "2007-06-06 06:00:00", 302 );
	miutil::miTime time(miutil::miDate("2007-06-06"),toTest);
	
	miutil::miTime triggerTime = time;
	triggerTime.addHour(-1);
	const kvalobs::kvData d = dataFactory.getData( 15, 1, triggerTime );
	
	const AbstractAgregator::TimeSpan timeSpan = agregator->getTimeSpan(d);
	
	CPPUNIT_ASSERT_EQUAL(time, timeSpan.second);
	
	time.addHour(- agregator->interestingHours());
	CPPUNIT_ASSERT_EQUAL(time, timeSpan.first);
}
