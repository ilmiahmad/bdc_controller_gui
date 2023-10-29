#ifndef CONSOLELOGWIDGET_H
#define CONSOLELOGWIDGET_H
#include <QPlainTextEdit>
#include <QFile>

class consoleLogWidget : public QPlainTextEdit
{
    Q_OBJECT
    /* snip */
public:
    //explicit consoleLogWidget(QPlainTextEdit *parent);
    //virtual ~consoleLogWidget();
    void appendMessage(const QByteArray& text);

private:
    QFile m_logFile;
};


//consoleLogWidget::consoleLogWidget(QPlainTextEdit *parent)
//{

//}
//consoleLogWidget::~consoleLogWidget()
//{

//}

void consoleLogWidget::appendMessage(const QByteArray& text)
{
    this->appendMessage(text); // Adds the message to the widget
    //this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom
    m_logFile.write(text,text.size()); // Logs to file
    // optional if you want to see the changes
    // after appendPlainText() immediately
    // useful if you use this->appendMessage() in a loop
    //QApplication::processEvents();

}
#endif // CONSOLELOGWIDGET_H
