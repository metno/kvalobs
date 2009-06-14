/****************************************************************
**
** Implementation StationSelection class
**
****************************************************************/

#include "StationSelection.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "proj++.h"

#include "table_delaunay.h"



StationSelection::StationSelection()
{

   std::string line;
   int valis;
   int key;

   std::ifstream ind;
   ind.open("/metno/kvalobs/kvalobs-svn/src/kvQc2/algorithms/TriangulatedNeighbours.txt");

   /// Later make the name of the neighbours file as an option in the configuration file!!!!

   // Format of test file is this (avoids problems of having to write complicated code)
   // where we terminate lines with the number 0 which corresponds to no station!
   //
   //      StationId1 StationId2 StationId3 .... StationIdN 0
   //
   // StationId1 is the target
   // StationId2 ... StationIdN are the favoured neighbours
   // 0 terminates the set!

   if(ind) {
      ind >> key;
      while ( !ind.eof() ) {

         ind >> valis;
         if (valis!=0) {
             NeighbourMap[ key ].push_back(valis);
         }
         else if (valis==0) {
             ind >> key;
         }
         //getline(ind,line);
      }
   }
   else {
          std::cout << "Could not open best station file!" << std::endl;
   }

  ind.close();

}

/// This constructor will handle the Delauney triangles and Choice of Triangulated Neighbours
/// NO No No ------------ this is better handled directly inside Qc2D *****
StationSelection::StationSelection(std::list<int> StationIDList)
{

    const double radish = 0.01745329251994329509;


// Setup projection engine business

         std::string prms("proj=utm lon_0=15e datum=WGS84"); //From Matthias
         proj pj(prms);
         projUV utm;

         std::vector<double> lat;
         std::vector<double> lon;
         std::vector<int> stid;

         int node_num;
         double *table;
         int *triangle_neighbor;
         int *triangle_node;
         int triangle_num;

         int i=0;
         int j=0;
 
         node_num=StationIDList.size();   /// This shall all become "-1" when we calculate the triangel for each station !!!!
         table= new double[2*StationIDList.size()];

//Loop Through Stations And Convert To UTM co-ordinates

//******      for (std::list<int>::const_iterator sit=StationIDList.begin(); sit!=StationIDList.end(); ++ sit) {
//******      
     //******      stid.push_back( sit->stationID() );
//******      
     //******      utm.u= sit->lon()*radish;   /// Use DEG_TO_RAD instead !!!!! ????????
     //******      utm.v= sit->lat()*radish;
     //******      utm = pj.ll2xy(utm);
//******      
     //******      lon.push_back( utm.u );
     //******      lat.push_back( utm.v );
//******      
     //******      table[i]=utm.u;
     //******      table[i+1]=utm.v;
     //******      i=i+2;
//******      
     //******      //std::cout <<  sit->stationID() << " " << sit->lon() << " " << sit->lat() << " " << utm.u << " " << utm.v << " " << std::endl;
  //******      }
//******      
//******      
//******      //Calculate the Triangles For Each Target Station
//******      
//******      
//******      std::cout << "CALLING DTRIS!" << std::endl;
//******      dtris2 ( node_num, table, &triangle_num, triangle_node, triangle_neighbor );
//******      std::cout << "ESCAPING FROM GEOMPACK ... " << std::endl;

//dtris2( int point_num, double point_xy[], int *tri_num, int tri_vert[], int tri_nabe[] )

//Establish the Best Neighbours for that Target Station
//i.e. populate NeighbourMap
}

std::list<int> 
StationSelection::
Neighbours(int stationid) 
{
    return NeighbourMap[ stationid ];
}


std::map<int, std::list<int> > 
StationSelection::
ReturnMap()
{
    return NeighbourMap;
}

