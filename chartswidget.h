#ifndef CHARTSWIDGET_H
#define CHARTSWIDGET_H
#include <QWidget>
#include <QtCharts>
#include <QApplication>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>

class chartsWidget : public QWidget {
    Q_OBJECT

public:
    chartsWidget(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);

        QLineSeries *series = new QLineSeries();
        series->append(0, 6);
        series->append(2, 4);
        series->append(3, 8);
        series->append(7, 4);
        series->append(10, 5);
        series->setUseOpenGL(true);

        *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

        QChart *chart = new QChart();
        chart->adjustSize();
        chart->addSeries(series);
        chart->createDefaultAxes();


        QChartView *chartView = new QChartView(chart);
        //chartView->setRenderHint(QPainter::Antialiasing);

        layout->addWidget(chartView);
    }
};

#endif // CHARTSWIDGET_H
