#ifndef ABSTRACTAGREGATORTEST_H_
#define ABSTRACTAGREGATORTEST_H_

#include <cppunit/extensions/HelperMacros.h>
#include <AbstractAgregator.h>

class AbstractAgregatorTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( AbstractAgregatorTest );
    CPPUNIT_TEST( testGetTimeSpanAtGenerationPoint );
    CPPUNIT_TEST( testGetTimeSpan );
    CPPUNIT_TEST_SUITE_END_ABSTRACT();
public:
	AbstractAgregatorTest();
	virtual ~AbstractAgregatorTest();
	
	/**
	 * Must be overridden to allocate a new AbstractAgregator object, 
	 * agregator. The object will be deleted on tearDown()
	 */
	virtual void setUp() = 0;
	
	virtual void tearDown()
	{
		delete agregator;
		agregator = 0;
	}
	
	virtual void testGetTimeSpanAtGenerationPoint();
	virtual void testGetTimeSpan();
	
protected:
	agregator::AbstractAgregator * agregator;
};

#endif /*ABSTRACTAGREGATORTEST_H_*/
