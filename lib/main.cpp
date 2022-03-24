    #include "qapplication.h"
    #include "lfile.h"
    #include "lmath.h"
    #include "qdebug.h"
    #include "lchart.h" 
    #include "complex.h" 
    #include "lmainwidget.h" 
    #include "lcommonsettings.h"

    #include <QIcon>
    #include <QAction>
    #include <QPushButton>
    
int main(int argc, char ** argv)
{
    QApplication app(argc,argv);

    LChartDialog d;
    d.setAxisPrecision(1, 3);
    d.setAxisColor(Qt::gray);
    d.setAxisVisible(false);
    d.setCrossColor(Qt::white);
    d.setAxisFixedMinMax(0, 20, 0, 10);
//    d.setAxisMarksInterval(200, 60);
//    d.setVisibleMouseCross(true);
//    d.setColorMouseCross(QColor(200, 200, 0));
//    d.setOnlyPoints(true);
    d.setPointSize(0);

    d.addChart(LChartParams(Qt::red, Qt::blue));
    QList<QPointF> points;
//    points << QPointF(1, 3.5) << QPointF(4, 8.5) << QPointF(8, 1.15) << QPointF(11, 2) << QPointF(6, 6) << QPointF(7, 9) << QPointF(16, 9);
//    d.addChartPoints(points, 0);


    d.resize(400, 300);
    d.exec();	
    d.updateChart();

    return app.exec();

};



