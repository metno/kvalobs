/****************************************************************
**
** Definition of PaperField class
**
****************************************************************/

#ifndef _PAPER_FIELD_
#define _PAPER_FIELD_

#include <qwidget.h>
#include <vector>

#include <qprinter.h>


class PaperField : public QWidget
{
   Q_OBJECT
public:
   PaperField( QWidget *parent=0, const char *name=0 );
   PaperField(int xl, int yl, int xu, int yu, QWidget *parent, const char *name );

   QSizePolicy sizePolicy() const;

   float linearChi2();

public slots:
   void AddPoint( int x, int y );
   void printIt();
   void paintIt( QPainter *p );

signals:
   void newPointDrawn( int, int );

protected:
   void paintEvent( QPaintEvent * );

private:
   std::vector<int> xps;
   std::vector<int> yps;
   QPrinter *printer;
   int xl, yl, xu, yu;

};


#endif // CANNON_H
