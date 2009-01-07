/****************************************************************
**
** Implementation PaperField class
**
****************************************************************/

#include "PaperField.h"
#include "BasicStatistics.h"
#include <qapplication.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qfont.h>
#include <qlayout.h>
#include <qprinter.h>
#include <iostream>
#include "scone.h"



PaperField::PaperField( QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    xl=-200;
    xu=600;
    yl=-200;
    yu=600;

    QButtonGroup *bgroup = new QButtonGroup( this );
    bgroup->resize( 110, 100 );
    setPalette( QPalette( QColor( 250, 250, 200) ) );
    //QPushButton *quit = new QPushButton( "Quit", this, "quit" );
    QPushButton *quit = new QPushButton( "Quit", bgroup, "quit" );
    //quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()),qApp, SLOT(quit()) );
#ifndef QT_NO_PRINTER
    printer = new QPrinter;
    // Create and setup the print button
    //QPushButton *print = new QPushButton( "Print...", this, "print" );
    QPushButton *print = new QPushButton( "Print", bgroup, "print" );
    connect( print, SIGNAL(clicked()), SLOT(printIt()) );
#endif
    //QGridLayout *grid = new QGridLayout( bgroup, 2, 2, 10 );
    QGridLayout *grid = new QGridLayout( bgroup, 2, 1, 10 );
    grid->addWidget( quit, 0, 0 );
    grid->addWidget( print, 1, 0 );
    //grid->setColStretch( 1, 10 );
}

PaperField::PaperField(int x0, int y0, int x1, int y1, QWidget *parent, const char *name )
        : QWidget( parent, name )
{
    xl=x0;
    xu=x1;
    yl=y0;
    yu=y1;

    QButtonGroup *bgroup = new QButtonGroup( this );
    bgroup->resize( 110, 100 );
    setPalette( QPalette( QColor( 250, 250, 200) ) );
    //QPushButton *quit = new QPushButton( "Quit", this, "quit" );
    QPushButton *quit = new QPushButton( "Quit", bgroup, "quit" );
    //quit->setFont( QFont( "Times", 18, QFont::Bold ) );
    connect( quit, SIGNAL(clicked()),qApp, SLOT(quit()) );
#ifndef QT_NO_PRINTER
    printer = new QPrinter;
    // Create and setup the print button
    //QPushButton *print = new QPushButton( "Print...", this, "print" );
    QPushButton *print = new QPushButton( "Print", bgroup, "print" );
    connect( print, SIGNAL(clicked()), SLOT(printIt()) );
#endif
    //QGridLayout *grid = new QGridLayout( bgroup, 2, 2, 10 );
    QGridLayout *grid = new QGridLayout( bgroup, 2, 1, 10 );
    grid->addWidget( quit, 0, 0 );
    grid->addWidget( print, 1, 0 );
    //grid->setColStretch( 1, 10 );
}

void PaperField::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    paintIt(&p);
}


void PaperField::AddPoint( int x, int y )
{
    xps.push_back(x);
    yps.push_back(y);
}

void PaperField::printIt()
{
    if ( printer->setup( this ) ) {
        QPainter paint;
        if( !paint.begin( printer ) )
            return;
        paintIt( &paint );
    }
}

void PaperField::paintIt( QPainter *p )
{
    int limit=yu-yl;
    int xorigin=-xl;
    int yorigin=-yl;
    int itic;

    int step=(int)(limit/10);
    int small_step=(int)(limit/100);

    QString s = "+";

    // rough axes
    p->drawLine(xl+xorigin,limit-yorigin,xu+xorigin,limit-yorigin);
    p->drawLine(xorigin,limit,-xl,0);

    // plot the data
    std::vector<int>::iterator xv = xps.begin();
    std::vector<int>::iterator yv = yps.begin();
    while( xv != xps.end() ) {
         //p.drawText( *xv,*yv, s );
         //p.drawText( *xv+origin,(*yv+origin), s );
         p->drawPoint( (*xv+xorigin),limit-(*yv+yorigin) );
         p->drawPoint( (*xv+xorigin)-1,limit-(*yv+yorigin) );
         p->drawPoint( (*xv+xorigin)+1,limit-(*yv+yorigin) );
         p->drawPoint( (*xv+xorigin),limit-(*yv+yorigin)-1 );
         p->drawPoint( (*xv+xorigin),limit-(*yv+yorigin)+1 );
         xv++;
         yv++;
    }
    for (itic=0;itic<(int)(limit/step)+1;itic++){
         p->drawLine(itic*step,limit-yorigin-3,itic*step,limit-yorigin+3);
         p->drawText(itic*step-10,limit-yorigin+20,StrmConvert(step*itic-xorigin));

         p->drawLine(xorigin-3,itic*step,xorigin+3,itic*step);
         p->drawText(xorigin-30,itic*step+5,StrmConvert(limit-step*itic-yorigin));
    }
    for (itic=0;itic<(int)(limit/small_step)+1;itic++){
         step=small_step;
         p->drawLine(itic*step,limit-yorigin-1,itic*step,limit-yorigin+1);
         p->drawLine(xorigin-1,itic*step,xorigin+1,itic*step);
    }
    float chi2=linearChi2();
    p->drawText(150,100,"Mean variance from expected = "+StrmConvert(chi2));
    p->drawText(150,80,"Number of points = "+StrmConvert( xps.size() ));
    std::cout << xl << " " << xu << " " << yl << " " << yu << std::endl;
    p->drawLine(xorigin,limit-yorigin, xu + xorigin, limit - xu - yorigin);
}


         //p->drawPoint( (*xv+xorigin),limit-(*yv+yorigin) );


QSizePolicy PaperField::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}


float PaperField::linearChi2()
{
    float Chi2=0.0;
    double sum, mean, var, dev, skew, kurt;

    computeStats(yps.begin( ), yps.end( ), sum, mean, var, dev, skew, kurt);

    std::cout << " " << yps.size( );
    std::cout << " " << sum;
    std::cout << " " << mean;
    std::cout << " " << var;
    std::cout << " " << dev;
    std::cout << " " << skew;
    std::cout << " " << kurt;
    std::cout << endl;

    std::vector<int>::iterator xv = xps.begin();
    std::vector<int>::iterator yv = yps.begin();
    while( xv != xps.end() ) {
         std::cout<<"Plot Data ("<<*xv <<","<<*yv<<")"<<std::endl;
         // This is a simple Chi-squared calculation to see
         // how close the observed value "y" is equal to the expected "x"
         // using this in the test where we have similated data based on summed actual
         // originals.
         Chi2=(float)((*yv-*xv)*(*yv-*xv)) + Chi2;
         xv++;
         yv++;
    }
 //return Chi2/(var*yps.size());
 return Chi2/(yps.size());       // This is sigma squared
}

