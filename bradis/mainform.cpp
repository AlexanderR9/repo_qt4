 #include "mainform.h"
 #include "lstatic.h"
 #include "lcommonsettings.h"

 #include <QDebug>
 #include <QtCore/qmath.h>
 #include <QSettings>



//////// MainForm ////////////////////////
MainForm::MainForm(QWidget *parent)
    :LMainWidget(parent),
    m_centralWidget(NULL)
{
    setObjectName("main_form");
    move(150, 50);
    resize(800, 500);

}
void MainForm::initActions()
{
    addAction(LMainWidget::atStart);
    addAction(LMainWidget::atSettings);
    addAction(LMainWidget::atExit);
}
void MainForm::initWidgets()
{
	m_centralWidget = new MainWidget(this);
	this->addWidget(m_centralWidget, 0, 0);
}
void MainForm::initCommonSettings()
{
    QStringList combo_list;
    for (int i=1; i<=6; i++)
        combo_list.append(QString::number(i));

    QString key = QString("precision");
    lCommonSettings.addParam(QString("Precision values"), LSimpleDialog::sdtIntCombo, key);
    lCommonSettings.setComboList(key, combo_list);
    lCommonSettings.setDefValue(key, 2);

}
void MainForm::load()
{
    LMainWidget::load();
    QSettings settings(companyName(), projectName());
    m_centralWidget->load(settings);
}
void MainForm::save()
{
    LMainWidget::save();
    QSettings settings(companyName(), projectName());
    m_centralWidget->save(settings);
}
void MainForm::slotAction(int type)
{
    switch (type)
    {
        case LMainWidget::atSettings: {actCommonSettings(); break;}
        case LMainWidget::atStart: {start(); break;}
        default: break;
    }
}
void MainForm::start()
{
	quint8 precision = lCommonSettings.paramValue("precision").toUInt();
	m_centralWidget->calc(precision);
}


//MainWidget
MainWidget::MainWidget(QWidget *parent)
    :QWidget(parent)
{
    setupUi(this);
    initTable();
}
void MainWidget::calc(quint8 precision)
{
    LStatic::removeAllRowsTable(tableWidget);
    double angle = startAngleComboBox->currentText().toDouble();
    double last_angle = lastAngleComboBox->currentText().toDouble();
    double step = stepAngleComboBox->currentText().toDouble();

    while (angle <= last_angle)
    {
    	addBradisRow(angle, precision);
    	angle += step;
    }

    tableWidget->resizeRowsToContents();
}
void MainWidget::addBradisRow(double angle, quint8 precision)
{
    QStringList row_data;
    double rad = angle*pi()/double(180);
    double s = qSin(rad);
    double c = qCos(rad);
    QString tn = ((qAbs(c)<0.0001) ? "---" : QString::number(s/c, 'f', precision));
    QString ct = ((qAbs(s)<0.0001) ? "---" : QString::number(c/s, 'f', precision));

    row_data << QString::number(angle, 'f', 0);
    row_data << QString::number(rad, 'f', precision);
    row_data << QString::number(s, 'f', precision);
    row_data << QString::number(c, 'f', precision);
    row_data << tn << ct;

    LStatic::addTableRow(tableWidget, row_data);
}
void MainWidget::initTable()
{
    LStatic::fullClearTable(tableWidget);
    QStringList headers(headerLabels());
    LStatic::setTableHeaders(tableWidget, headers, Qt::Horizontal);
}
QStringList MainWidget::headerLabels()
{
    QStringList list;
    list.append("Angle");
    list.append("Angle (rad.)");
    list.append("Sin");
    list.append("Cos");
    list.append("Tg");
    list.append("Ctg");
    return list;
}
void MainWidget::save(QSettings &settings)
{
    settings.setValue("mainform/start_angle", startAngleComboBox->currentIndex());
    settings.setValue("mainform/last_angle", lastAngleComboBox->currentIndex());
    settings.setValue("mainform/step_angle", stepAngleComboBox->currentIndex());
}						
void MainWidget::load(QSettings &settings)
{
    startAngleComboBox->setCurrentIndex(settings.value("mainform/start_angle", 0).toInt());
    lastAngleComboBox->setCurrentIndex(settings.value("mainform/last_angle", 5).toInt());
    stepAngleComboBox->setCurrentIndex(settings.value("mainform/step_angle", 1).toInt());
}





