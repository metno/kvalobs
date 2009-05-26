/****************************************************************
**
** Definition of PaperField class
**
****************************************************************/

#ifndef _EPS_X_               
#define _EPS_X_               

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <vector>

class epsx                                  
{
public:
    epsx( std::vector<int> XXX, std::vector<int> YYY, char *file_name );
    epsx( std::vector<float> XXX, std::vector<float> YYY, char *file_name );
    epsx( char *file_name );

protected:

private:

};


#endif // _EPS_X_   
