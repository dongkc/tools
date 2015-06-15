#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include<QtSerialPort/QtSerialPort>

namespace Ui {
  class Gui;
}

class Gui : public QMainWindow
{
  Q_OBJECT

public:
  explicit Gui(QWidget *parent = 0);
  static Gui *instance;
  ~Gui();

  QString portName();

  void setPortState(int state);

  qint32 baudrate();

  QSerialPort::DataBits data_bits();

  QSerialPort::StopBits stop_bits();

  QSerialPort::Parity parity();

protected:
     void closeEvent(QCloseEvent *event);

private slots:
  void on_pushButton_2_clicked();

  void on_serialPortPushButton_clicked();

  void on_plainTextEdit_textChanged();

  void on_sendTextEdit_textChanged();

public:
  Ui::Gui *ui;
};

#endif // GUI_H
