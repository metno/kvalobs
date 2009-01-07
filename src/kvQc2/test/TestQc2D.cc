#include "TestQc2D.h"

#include "ReadProgramOptions.h"

#include <kvalobs/kvDataOperations.h>

CPPUNIT_TEST_SUITE_REGISTRATION (TestQc2D);

void TestQc2D::setUp(void)
{
        Qc2DVehicle = new Qc2D;
}

void TestQc2D::tearDown(void)
{
        delete Qc2DVehicle; 
}

void TestQc2D::TestTest(void)
{
        ReadProgramOptions params;

        std::list<kvalobs::kvData> Qc2Data;
        std::list<kvalobs::kvStation> StationList;

        std::vector<string> config_files;
        params.SelectConfigFiles(config_files);
        std::vector<string>::const_iterator vit = config_files.begin();
        while ( vit != config_files.end() )  {
              params.clear(); // very important!!!!!!
              params.Parse( *vit );
              CPPUNIT_ASSERT( !params.Parse( *vit ) );
              //CPPUNIT_ASSERT( params.Parse( *vit ) );  // This causes a test failure :)
              ++vit;
        }

        //CPPUNIT_ASSERT( Qc2DVehicle(Qc2Data,StationList, params) );
        kvalobs::kvDataFactory f( 42, "2006-05-26 06:00:00", 302 );
        kvalobs::kvData inData = f.getData( 94, 33 );
        Qc2Data.push_back(inData);
        Qc2D QV(Qc2Data,StationList, params);

}

