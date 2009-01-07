#include <iostream> 
#include <sstream> 
#include "proj++.h" 

proj::proj(std::string param) 
{ 
   char **prm;
   params=param+" no_defs";
   std::istringstream si(params);

   int n_pair=1;         // add one for last token "no_defs" 
   for ( unsigned long i=0; i < params.size(); i++ ) 
     if ( params[i] == '=' ) n_pair++;

   prm = new char * [n_pair];
   for ( int i=0; i < n_pair; i++ ) { 
     prm[i] = new char[256];
     si >> prm[i];
   }  

   p_proj=pj_init(n_pair, prm);
  

   for ( int i=n_pair-1; i >=0; i-- ) { 
     delete [] prm[i];
   }  
   delete [] prm;

   if ( !p_proj ) {  
     std::cerr << "Failed to initialize the PROJ library\n";
     exit(1);
   }  
 
 }  

 proj::~proj() 
 {  
   pj_free(p_proj);
 }  

 projUV proj::ll2xy( projUV ll) 
 {  
   return pj_fwd(ll, p_proj);
 }  

 projUV proj::ll2xy( std::string lat, std::string lon ) 
 {  
   projUV ll;
   ll.u = dmstor(lon.c_str(),0);
   ll.v = dmstor(lat.c_str(),0);
   return pj_fwd(ll, p_proj);
 }  

 projUV proj::xy2ll( projUV xy)  
 {  
   return pj_inv(xy, p_proj);
 }  

 projUV proj::xy2ll( double x, double y )  
 {  
   projUV xy;
   xy.u=x;
   xy.v=y;
   return pj_inv(xy, p_proj);
 }  
        
