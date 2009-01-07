#ifndef TESTQC2D_H
#define TESTQC2D_H

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <kvalobs/kvDataOperations.h>

#include "algorithms/Qc2D.h"

using namespace std;

class TestQc2D : public CPPUNIT_NS :: TestFixture
{
        CPPUNIT_TEST_SUITE (TestQc2D);
        CPPUNIT_TEST (TestTest);
        CPPUNIT_TEST_SUITE_END ();

        public:
                void setUp (void);
                void tearDown (void);

        protected:
                void TestTest (void);

        private:
                                           
                Qc2D *Qc2DVehicle;
};

#endif

