/****************************************************************
**
** Implementation epsx class
**
****************************************************************/

#include "epsx.h"
#include <cmath>

epsx::epsx( std::vector<float> XXX, std::vector<float> YYY, char *file_name )
{
   std::vector<float>::const_iterator ix,iy;
   std::ofstream file_unit;
   file_unit.open ( file_name );
   if ( !file_unit ) {
     std::cout << "\n";
     std::cout << "TRIANGULATION_ORDER3_PLOT_EPS - Fatal error!\n";
     std::cout << "Could not open the output EPS file.\n";
     exit ( 1 );
   }
   std::vector<float>::iterator maxX = std::max_element( XXX.begin(),XXX.end() );
   std::vector<float>::iterator minX = std::min_element( XXX.begin(),XXX.end() );
   std::vector<float>::iterator maxY = std::max_element( YYY.begin(),YYY.end() );
   std::vector<float>::iterator minY = std::min_element( YYY.begin(),YYY.end() );
   ix=XXX.begin();
   iy=YYY.begin();

// Find a round set of values

  float xrange=*maxX-*minX;
  float yrange=*maxY-*minY;

// by default we will divide the axes into ten parts:

  float xstep=xrange/10.0;
  int step=0;
  if (xstep < 1000.0) step=500;
  if (xstep < 500.0) step=250;
  if (xstep < 250.0) step=100;
  if (xstep < 100.0) step=50;
  if (xstep < 50.0) step=25;
  if (xstep < 25.0) step=10;
  if (xstep < 10.0) step=5;
  //if (xstep < 5.0) step=1;
  if (step == 0) step=int(xstep);
  int xstepx=int(step);
  int xcounter=int(xrange/xstepx+1);

  float ystep=yrange/10.0;
  step=0;
  if (ystep < 1000.0) step=500;
  if (ystep < 500.0) step=250;
  if (ystep < 250.0) step=100;
  if (ystep < 100.0) step=50;
  if (ystep < 50.0) step=25;
  if (ystep < 25.0) step=10;
  if (ystep < 10.0) step=5;
  //if (ystep < 5.0) step=1;
  if (step == 0) step=int(ystep);
  int ystepy=step;
  int ycounter=int(yrange/ystepy+1);

  float xmin,ymin,xmax,ymax;

  xmin=(( *minX/xstepx )-1)*xstepx; 
  ymin=(( *minY/ystepy )-1)*ystepy;

  xmax=(( *maxX/xstepx )+1)*xstepx; 
  ymax=(( *maxY/ystepy )+1)*ystepy; 

   std::cout << "X Data Range: "<< *minX << " " << *maxX << std::endl;
   std::cout << "Y Data Range: "<< *minY << " " << *maxY << std::endl;
   std::cout <<"Xaxis and step: " << xmin << " " << xmax << " " << xstepx << std::endl;
   std::cout  <<"Yaxis and step: "<< ymin << " " << ymax << " " << ystepy << std::endl;
   std::cout << "X counter = " << xcounter << std::endl;
   std::cout << "Y counter = " << ycounter << std::endl;
// Scale everything to fit the page
  float scalefactorx=1.0;
  float scalefactory=1.0;
  scalefactorx=595.0/(2.0*xrange); // scales things to A4 portrait width
  scalefactory=850.0/(2.0*yrange); // scales things to A4 portrait width
  std::cout << "Scale Factor: "<< scalefactorx << " " << scalefactory << std::endl;

  file_unit << "%!PS-Adobe-3.0 EPSF-3.0\n";
  file_unit << "%%Creator: triangulation_order3_plot_eps.C\n";
  file_unit << "%%Title: TEST \n";
  file_unit << "%%CreationDate: \n";

  file_unit << "%%Pages: 1\n";
  file_unit << "%%BoundingBox:  "
    << *minX*scalefactorx-4.0*xstepx << "  "
    << *minY*scalefactory-4.0*ystepy << "  "
    << *maxX*scalefactorx+4.0*xstepx << "  "
    << *maxY*scalefactory+4.0*ystepy << "\n";
    //<< *minX*scalefactorx << "  "
    //<< *minY*scalefactory << "  "
    //<< *maxX*scalefactorx << "  "
    //<< *maxY*scalefactory << "\n";
    //<< *minX << "  "
    //<< *minY << "  "
    //<< *maxX << "  "
    //<< *maxY << "\n";
    //<< 0 << "  "
    //<< 0 << "  "
    //<< 595 << "  "
    //<< 850 << "\n";
  std::cout << "BoundingBox:  "
    << *minX-xstepx << "  "
    << *minY-ystepy << "  "
    << *maxX+xstepx << "  "
    << *maxY+ystepy << std::endl;
  std::cout << *minX << " " << *minY << " " << *maxX << " " << *maxY << std::endl;
  file_unit << "%%Document-Fonts: Times-Roman\n";
  file_unit << "%%LanguageLevel: 1\n";
  file_unit << "%%EndComments\n";
  file_unit << "%%BeginProlog\n";
  file_unit << "/inch {72 mul} def\n";
  file_unit << "/wt {moveto show} bind def\n";
  file_unit << "%%EndProlog\n";
  file_unit << "%%Page:      1     1\n";
  file_unit << "save\n";
  file_unit << "%\n";
  file_unit << "% Set the RGB line color to very light gray.\n";
  file_unit << "%\n";
  file_unit << " 0 0 0 setrgbcolor\n";
  file_unit << "%\n";
  file_unit << "% Draw a gray border around the page.\n";
  file_unit << "%\n";
  // file_unit << 2*xstepx << " " << 2*ystepy << " translate %\n";
  //file_unit << *minX*scalefactorx << " " << *minY*scalefactory << " " << " translate %\n";
  //file_unit << *minX*scalefactorx << " " << *minY*scalefactory << " " << " translate %\n";

  file_unit << scalefactorx << " " << scalefactory << " scale \n" << std::endl;
   while ( ix<XXX.end() || iy<YYY.end() ){
        file_unit << "newpath  "
        << *ix << "  "
        << *iy << "  "
        << sqrt(xstepx*xstepx +ystepy*ystepy)/100.0 << " 0 360 arc closepath fill\n";

       ++ix;
       ++iy;
    }
    if (scalefactorx<=0.5){
    file_unit << "/Times findfont 24 scalefont setfont\n";
    }
    if (scalefactorx>0.5 && scalefactorx<1.0){
    file_unit << "/Times findfont 12 scalefont setfont\n";
    }
    if (scalefactorx>=1.0 && scalefactorx<2.0){
    file_unit << "/Times findfont 8 scalefont setfont\n";
    }
    if (scalefactorx>=2.0 && scalefactorx<4.0){
    file_unit << "/Times findfont 4 scalefont setfont\n";
    }
    if (scalefactorx>=4.0){
    file_unit << "/Times findfont 2 scalefont setfont\n";
    }

// Draw the axes
  file_unit << "newpath\n";
  file_unit << "0 "<< ymin <<" moveto\n";
  file_unit << "0 "<< ymax <<" lineto\n";
  file_unit << "stroke\n";
  file_unit << "newpath\n";
  file_unit << xmin<<" 0 moveto\n";
  file_unit << xmax<<" 0 lineto\n";
  file_unit << "stroke\n";
//

   //for ( std::vector<int>::iterator iax=XXX.begin();iax<XXX.end();++iax) {
    //file_unit << "(" << *iax << ") 0 "<< *iax << " wt\n";
    //file_unit << "(" << *iax << ") "  << *iax << " 0 wt\n";
   //}
   for (int k=0;k<ycounter+2;++k) {
    //file_unit << "(" << xmin+k*xstepx << ") 0 "<< xmin+k*xstepx << " wt\n";
    //file_unit << "(" << ymin+k*ystepy << ") "  << ymin+k*ystepy << " 0 wt\n";
    //file_unit << "(" << ymin+k*ystepy << ") "  << ymin+k*ystepy <<" "<<  -ystepy*0.2 << " wt\n";
    file_unit << "(" << ymin+k*ystepy <<") "<< -ystepy*0.5 <<" "<<  ymin+k*ystepy << " wt\n";
   }
   for (int k=0;k<xcounter+2;++k) {
    //file_unit << "(" << xmin+k*xstepx << ") 0 "<< xmin+k*xstepx << " wt\n";
    //file_unit << "(" << ymin+k*ystepy << ") "  << ymin+k*ystepy << " 0 wt\n";
    //file_unit << "(" << xmin+k*xstepx <<") "<< -xstepx*0.5 <<" "<<  xmin+k*xstepx << " wt\n";
    file_unit << "(" << xmin+k*xstepx << ") "  << xmin+k*xstepx <<" "<<  -xstepx*0.5 << " wt\n";
   }
    file_unit.close ( );
    std::cout << "Integer Hello World" << std::endl;
}


epsx::epsx( std::vector<int> XXX, std::vector<int> YYY, char *file_name )
{
   std::vector<int>::const_iterator ix,iy;
   std::ofstream file_unit;
   file_unit.open ( file_name );
   if ( !file_unit ) {
     std::cout << "\n";
     std::cout << "TRIANGULATION_ORDER3_PLOT_EPS - Fatal error!\n";
     std::cout << "Could not open the output EPS file.\n";
     exit ( 1 );
   }
   ix=XXX.begin();
   iy=YYY.begin();
   while ( ix<XXX.end() || iy<YYY.end() ){
       std::cout << *ix << " " << *iy << std::endl;

  file_unit << "%!PS-Adobe-3.0 EPSF-3.0\n";
  file_unit << "%%Creator: triangulation_order3_plot_eps.C\n";
  file_unit << "%%Title: TEST \n";
  file_unit << "%%CreationDate: \n";

  file_unit << "%%Pages: 1\n";
  file_unit << "%%BoundingBox:  "
    << -100 << "  "
    << -100 << "  "
    << 100 << "  "
    << 100 << "\n";
  file_unit << "%%Document-Fonts: Times-Roman\n";
  file_unit << "%%LanguageLevel: 1\n";
  file_unit << "%%EndComments\n";
  file_unit << "%%BeginProlog\n";
  file_unit << "/inch {72 mul} def\n";
  file_unit << "%%EndProlog\n";
  file_unit << "%%Page:      1     1\n";
  file_unit << "save\n";
  file_unit << "%\n";
  file_unit << "% Set the RGB line color to very light gray.\n";
  file_unit << "%\n";
  file_unit << " 0.9000 0.9000 0.9000 setrgbcolor\n";
  file_unit << "%\n";
  file_unit << "% Draw a gray border around the page.\n";
  file_unit << "%\n";
  file_unit << "newpath  "
        << *ix << "  "
        << *iy << "  "
        <<  "3 0 360 arc closepath fill\n";

       ++ix;
       ++iy;
    }

    std::cout << "Integer Hello World" << std::endl;
    file_unit.close ( );
}

epsx::epsx(char *file_name)
{
   std::cout << "Hello World" << std::endl;
   std::cout << file_name << std::endl;
}
