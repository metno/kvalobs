#ifndef PROJPP_H 
#define PROJPP_H 
#include <string> 
#include <projects.h>  
#include <string.h>

class proj { 
   std::string params;
   PJ *p_proj;

public:  

   proj( std::string param );
   ~proj();
        
   projUV ll2xy( projUV ll);
   projUV xy2ll( projUV xy);
   projUV ll2xy( std::string lat, std::string lon);
   projUV xy2ll( double x, double y);

};

#endif /* PROJPP_H */ 
