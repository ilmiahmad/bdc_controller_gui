#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QtMath>


#include <QtEndian>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , motor(new Motor)
    , serial(new QSerialPort(this))
    , stimer(new QTimer)
    , calibTimer(new QTimer)
{
    ui->setupUi(this);

    connect(serial, SIGNAL(readyRead()),
            this, SLOT(recvHandler()));

    connect(this, SIGNAL(dataParamChanged()),
            this, SLOT(updateGUIParamData()));

    connect(this, SIGNAL(dataPlotChanged()),
            this, SLOT(updatePlotData()));


    connect(this, SIGNAL(dataReady(QByteArray)),
            this, SLOT(processData(QByteArray)));

    connect(stimer, SIGNAL(timeout()), this, SLOT(targetcallback()));

    connect(this, SIGNAL(calibrationDone()), this, SLOT(calibrationProcessData()));

}

MainWindow::~MainWindow()
{
    delete stimer;
    delete serial;
    delete motor;
    delete ui;
}

void MainWindow::getFromGUIParam()
{
    // Hardware Parameter
    motor->devId = ui->devid_spinbox->value();
    motor->gearBox = ui->gearbox_spinbox->value();
    motor->encoderCpr = ui->encodercpr_spinbox->value();

    QString encoderdir = ui->encoderdir_combobox->currentText();

    if(encoderdir == "FORWARD")
    {
        motor->encoderDir = 1;
    }
    else if(encoderdir == "REVERSE")
    {
        motor->encoderDir = -1;
    }

    motor->freqMode = ui->freqmod_combobox->currentIndex();

    // Control Parameter
    motor->controlMode = ui->controlMode_combobox->currentIndex();
    motor->posKp = ui->kppos_spinbox->value();
    motor->speedKp = ui->speedkp_spinbox->value();
    motor->ki = ui->ki_spinbox->value();
    motor->maxPwm = ui->maxpwm_spinbox->value();
    motor->maxSpeed = ui->maxspeed_spinbox->value();
}

void MainWindow::sendParam()
{

    QByteArray full_out;
    QByteArray data_out;

    QDataStream s(&data_out, QDataStream::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);

    s << motor->devId;
    s << motor->gearBox;
    s << motor->encoderCpr;
    s << motor->freqMode;
    s << motor->encoderDir;
    s << motor->controlMode;
    s << motor->maxPwm;
    s << motor->maxSpeed;
    s << motor->speedKp;
    s << motor->ki;
    s << motor->posKp;

    full_out.append(0x69);
    full_out.append(0x74);
    full_out.append(0x73);
    full_out.append(0x02);
    full_out.append(0x1D);
    full_out.append(data_out);
    full_out.append(calculateCRC8(data_out));

    qDebug() << "send Param: " << full_out.toHex(':');
    qsizetype written = serial->write(full_out);

    if(written == full_out.size())
    {
        qDebug() << "write operation success";
    }
}

void MainWindow::requestParam()
{
    QByteArray full_out;

    QDataStream b(&full_out, QIODevice::WriteOnly);
    b.setByteOrder(QDataStream::LittleEndian);
    b << quint8(0x69);
    b << quint8(0x74);
    b << quint8(0x73);
    b << quint8(0x01);
    b << quint8(0x02);
    b << quint8(0x00);
    b << quint8(0x00);
    qDebug() << full_out;

    qsizetype written = serial->write(full_out);
    if(written == full_out.size())
    {
        qDebug() << "write operation success";
    }
}

bool MainWindow::saveConfiguration()
{   
    QByteArray full_out;
    QDataStream b(&full_out, QIODevice::WriteOnly);
    b.setByteOrder(QDataStream::LittleEndian);
    b << quint8(0x69);
    b << quint8(0x74);
    b << quint8(0x73);
    b << quint8(0x03);
    b << quint8(0x02);
    b << quint8(0x00);
    b << quint8(0x00);

    qDebug() << full_out;

    qsizetype written = serial->write(full_out);
    if(written == full_out.size())
    {
        qDebug() << "write operation success";
        return true;
    }

    return false;
}

void MainWindow::updatePlotData()
{

    double key = t*dt;

    t++;

    ui->speedplot->graph(0)->addData(key, (double)motor->m_speed);
    ui->speedplot->graph(0)->rescaleValueAxis(false,true);

    ui->positionplot->graph(0)->addData(key, (double)motor->m_position);
    ui->positionplot->graph(0)->rescaleValueAxis(false,true);

    ui->speedplot->xAxis->setRange(key, 20, Qt::AlignRight);
    ui->positionplot->xAxis->setRange(key, 20, Qt::AlignRight);

    ui->positionplot->replot();
    ui->speedplot->replot();
}

void MainWindow::updateGUIParamData()
{
    // Hardware Parameter
    ui->devid_spinbox->setValue(motor->devId);
    ui->gearbox_spinbox->setValue(motor->gearBox);
    ui->encodercpr_spinbox->setValue(motor->encoderCpr);

    if(motor->encoderDir == -1)
    {
        ui->encoderdir_combobox->setCurrentIndex(0);
    }
    else if(motor->encoderDir == 1)
    {
        ui->encoderdir_combobox->setCurrentIndex(1);
    }
    ui->freqmod_combobox->setCurrentIndex(motor->freqMode);

    // Control Parameter
    ui->controlMode_combobox->setCurrentIndex(motor->controlMode);
    ui->kppos_spinbox->setValue(motor->posKp);
    ui->speedkp_spinbox->setValue(motor->speedKp);
    ui->ki_spinbox->setValue(motor->ki);
    ui->maxpwm_spinbox->setValue(motor->maxPwm);
    ui->maxspeed_spinbox->setValue(motor->maxSpeed);
}

void MainWindow::setupPlot()
{

    ui->speedplot->addGraph();
    ui->speedplot->graph(0)->setPen(QPen(Qt::blue));
    ui->speedplot->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->speedplot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

    //ui->speedplot->setOpenGl(true);

    ui->speedplot->xAxis->setLabel("time(s)");
    ui->speedplot->yAxis->setLabel("speed");
    ui->speedplot->yAxis->setScaleType(QCPAxis::stLinear);

    ui->speedplot->axisRect()->setupFullAxesBox(true);
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    ui->speedplot->xAxis->setTicker(timeTicker);

    // position plot
    ui->positionplot->addGraph();
    ui->positionplot->graph(0)->setPen(QPen(Qt::red));
    ui->positionplot->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->positionplot->graph(0)->setBrush(QBrush(QColor(255, 0, 0, 20)));
    ui->positionplot->setOpenGl(true);

    ui->positionplot->xAxis->setLabel("time(s)");
    ui->positionplot->yAxis->setLabel("position");

    ui->positionplot->axisRect()->setupFullAxesBox(true);
    ui->positionplot->xAxis->setTicker(timeTicker);

    connect(stimer, SIGNAL(timeout()),this, SLOT(updatePlotData()));
    // Interval 0 means to refresh as fast as possible
}

void MainWindow::showStatusMessage(const QString &message)
{
    QMessageBox::warning(this, tr("Status"), message);
}

void MainWindow::showWriteError(const QString &message)
{
    QMessageBox::warning(this, tr("Warning"), message);
}

void MainWindow::targetcallback()
{
    quint8 mode = motor->controlMode;
    float targetValue = ui->applytarget_spinbox->value();
    float roundinput;

    if(calibrationState && calibTimer->isActive())
    {
        mode = PWM;
        targetValue = 50;
    }


    switch(mode)
    {
    case 0:
        roundinput = qRound(targetValue);
        sendTarget(mode,roundinput);
        break;
    case 1:
        sendTarget(mode,targetValue);
        break;
    case 2:
        sendTarget(mode,targetValue);
        break;
    default:
        break;
    }
}

void MainWindow::sendTarget(quint8 mode, float data)
{

    QByteArray full_out;
    QByteArray data_out;

    QDataStream s(&data_out, QDataStream::WriteOnly);
    s.setByteOrder(QDataStream::LittleEndian);
    s.setFloatingPointPrecision(QDataStream::SinglePrecision);

    switch(mode)
    {
    case 0:
        s << qint16(data);
        s << float(0);
        s << float(0);
        break;
    case 1:
        s << qint16(0);
        s << data;
        s << float(0);
        break;
    case 2:
        s << qint16(0);
        s << float(0);
        s << float(data);
        break;
    default:
        break;
    }
    s << qint16(0);

    full_out.append(0x69);
    full_out.append(0x74);
    full_out.append(0x73);
    full_out.append(char(0x00));
    full_out.append(0x0D);
    full_out.append(data_out);
    full_out.append(calculateCRC8(data_out));

    qDebug() << "send Param: " << full_out.toHex(':');
    qsizetype written = serial->write(full_out);

    if(written == full_out.size())
    {
        qDebug() << "write operation success";
    }
}

quint8 MainWindow::calculateCRC8(QByteArray &data) {
    quint8 crc = 0x00;
    for (qsizetype i = 0; i < data.size(); i++) {
        quint8 t = (quint8)(data[i] ^ crc);
        crc = (quint8)(mxCRC8_table[t]);
    }

    return crc;
}

void MainWindow::recvHandler()
{
    static QByteArray package;
    static bool isHeaderRead = false;
    static quint8 dataSize = 0;


    //! Is there whole header appears?
    if (serial->bytesAvailable() < header_size) {
        //! If not, waiting for other bytes
        return;
    }

    if (!isHeaderRead) {
        //! Read fixed size header.
        package.append(serial->read(header_size));
        //! Check is it actually beginning of our package?
        if(!(package.startsWith("its")))
        {
            return;
        }
        //! read the package datasize at header index 4
        dataSize = qFromLittleEndian<quint8>(package.sliced(4,1));

        isHeaderRead = true;
    }

    //! Check is there whole package available?
    if (dataSize > serial->bytesAvailable()) {
        //! If not, waiting for other bytes.
        return;
    }
    //! Read rest.
    package.append(serial->read(dataSize));
    //qDebug() << "readvalue: " << package;
    //! And notify about it.
    emit dataReady(package);
    package.clear();
    isHeaderRead = false;

}

void MainWindow::processData(QByteArray data_in)
{

    qDebug() << "response value: " << data_in.toHex(':');

    switch(data_in.at(3))
    {
    case 0:
        motor->m_current = qFromLittleEndian<float>(data_in.sliced(5,4));
        motor->m_speed = qFromLittleEndian<float>(data_in.sliced(9,4));
        motor->m_position = qFromLittleEndian<float>(data_in.sliced(13,4));
        break;

    case 1:
        motor->devId = qFromLittleEndian<quint8>(data_in.sliced(5,1));
        motor->gearBox = qFromLittleEndian<float>(data_in.sliced(6,4));
        motor->encoderCpr = qFromLittleEndian<quint16>(data_in.sliced(10,2));
        motor->freqMode = qFromLittleEndian<quint8>(data_in.sliced(12,1));
        motor->encoderDir = qFromLittleEndian<qint8>(data_in.sliced(13,1));
        motor->controlMode = qFromLittleEndian<quint8>(data_in.sliced(14,1));
        motor->maxPwm = qFromLittleEndian<qint16>(data_in.sliced(15,2));
        motor->maxSpeed = qFromLittleEndian<float>(data_in.sliced(17,4));
        motor->speedKp = qFromLittleEndian<float>(data_in.sliced(21,4));
        motor->ki = qFromLittleEndian<float>(data_in.sliced(25,4));
        motor->posKp = qFromLittleEndian<float>(data_in.sliced(29.,4));
        emit dataParamChanged();

        break;
        // request parameter data to hardware
    case 2:

        qDebug() << "Send parameter success";
        break;

        // save hardware configuration
    case 3:
        if(data_in.at(4) == 0x01)
        {
            qDebug() << "save config success";
        }
        else
        {
            qDebug() << "save config failed";
        }
        break;
    default:
        break;
    }

    if(calibrationState && !calibTimer->isActive())
    {
        emit calibrationDone();
    }
}


void MainWindow::on_applytarget_button_clicked()
{

    if(serial->isOpen())
    {
        if(ui->applytarget_button->text() == "Apply Target")
        {
            stimer->start(50);
            ui->applytarget_button->setText("Stop Target");
            ui->consolelog->appendPlainText("Target applied");
            ui->calibration_button->setDisabled(true);

        }
        else if(ui->applytarget_button->text() == "Stop Target")
        {
            stimer->stop();
            ui->applytarget_button->setText("Apply Target");
            ui->consolelog->appendPlainText("Target stop");
            ui->calibration_button->setDisabled(false);
        }
    }
    else
    {
        showStatusMessage(tr("Serial Port closed"));
    }

}


void MainWindow::on_connect_button_clicked()
{
    if (ui->connect_button->text() == "Connect") {
        // Set a new label when the button is clicked

        serial->setPortName(ui->port_combobox->currentText());
        serial->setBaudRate(ui->baud_combobox->currentText().toInt());
        serial->setReadBufferSize(0);

        if (serial->open(QIODevice::ReadWrite)) {
            ui->connect_button->setText("Disconnect");
            setupPlot();
            requestParam();
            ui->consolelog->appendPlainText("connected to device");
        } else {
            showStatusMessage(tr("Open serial error"));
        }
    }
    else
    {
        if (serial->isOpen())
        {
            serial->close();
            ui->connect_button->setText("Connect");
            ui->consolelog->appendPlainText("disconect from device");
        }
        //showStatusMessage(tr("Disconnected"));
    }
}


void MainWindow::on_calibration_button_clicked()
{

    if(serial->isOpen())
    {
        calibrationState = true;
        stimer->start(50);
        calibTimer->setSingleShot(true);
        calibTimer->start(5000);

        ui->applytarget_button->setDisabled(true);
        ui->consolelog->appendPlainText("encoder calibration Started");
    }
    else
    {
        showStatusMessage(tr("Serial Port closed"));
    }

}


void MainWindow::on_save_button_clicked()
{
    if(serial->isOpen())
    {
        saveConfiguration();
        ui->consolelog->appendPlainText("configuration saved");
    }
    else
    {
        showStatusMessage(tr("Serial Port closed"));
    }

}


void MainWindow::on_apply_button_clicked()
{
    if(serial->isOpen())
    {
        getFromGUIParam();
        sendParam();
        ui->consolelog->appendPlainText("device parameter updated");
    }
    else
    {
        showStatusMessage(tr("Serial Port closed"));
    }

}

void MainWindow::calibrationProcessData()
{
    calibrationState = false;
    stimer->stop();
    calibTimer->stop();

    if(motor->m_speed >= 0)
    {
        motor->encoderDir = 1;
    }
    else
    {
        motor->encoderDir = -1;
    }

    updateGUIParamData();

    ui->consolelog->appendPlainText("encoder calibration Done");

    ui->applytarget_button->setDisabled(false);
}
