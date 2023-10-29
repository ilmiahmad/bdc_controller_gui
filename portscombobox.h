#ifndef PORTSCOMBOBOX_H
#define PORTSCOMBOBOX_H
#include <QComboBox>
#include <QSerialPortInfo>

class portsComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit portsComboBox(QWidget *parent = nullptr);
    virtual ~portsComboBox();

public:
    virtual void showPopup()
    {
        while (count() > 0) { removeItem(0); }
        const auto infos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo &info : infos)
            addItem(info.portName());
        QComboBox::showPopup();
    }
};
#endif // PORTSCOMBOBOX_H
