
#include "gui.h"
#include "ui_gui.h"

#include "qp_port.h"
#include "pelican.h"

QT_USE_NAMESPACE

Q_DEFINE_THIS_FILE

//............................................................................
Gui *Gui::instance;

Gui::Gui(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Gui)
{
  instance = this;

  ui->setupUi(this);

  foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
      ui->serialPortComboBox->addItem(info.portName());
  }
}

Gui::~Gui()
{
  delete ui;
}

QString Gui::portName()
{
  return ui->serialPortComboBox->currentText();
}

qint32 Gui::baudrate()
{
  return ui->buadRateComboBox->currentText().toInt();
}

QSerialPort::DataBits Gui::data_bits()
{
  qint32 data_bits = ui->dataBitsComboBox->currentText().toInt();
  switch (data_bits) {
    case QSerialPort::Data5:
      return QSerialPort::Data5;
    case QSerialPort::Data6:
      return QSerialPort::Data6;
    case QSerialPort::Data7:
      return QSerialPort::Data7;
    case QSerialPort::Data8:
      return QSerialPort::Data8;
    default:
      return QSerialPort::UnknownDataBits;
  }
}

QSerialPort::StopBits Gui::stop_bits()
{
  qint32 stop_bits = ui->stopBitsComboBox->currentText().toInt();
  switch (stop_bits) {
    case QSerialPort::OneStop:
      return QSerialPort::OneStop;
    case QSerialPort::OneAndHalfStop:
      return QSerialPort::OneAndHalfStop;
    case QSerialPort::TwoStop:
      return QSerialPort::TwoStop;
    defualt:
      return QSerialPort::UnknownStopBits;
  }
}

QSerialPort::Parity Gui::parity()
{
  qint32 parity = ui->parityComboBox->currentText().toInt();
  switch (parity) {
    case QSerialPort::NoParity:
      return QSerialPort::NoParity;
    case QSerialPort::EvenParity:
      return QSerialPort::EvenParity;
    case QSerialPort::OddParity:
      return QSerialPort::OddParity;
    default:
      return QSerialPort::UnknownParity;
  }
}

void Gui::setPortState(int state)
{
  QPalette pal = ui->serialPortPushButton->palette();
  pal.setColor(QPalette::Button, QColor(Qt::green));
  ui->serialPortPushButton->setText("关闭串口");
  if (state) {
    pal.setColor(QPalette::Button, QColor(Qt::red));
    ui->serialPortPushButton->setText("打开串口");
  }
  ui->serialPortPushButton->setAutoFillBackground(true);
  ui->serialPortPushButton->setPalette(pal);
  ui->serialPortPushButton->update();
}

void Gui::on_pushButton_2_clicked()
{

    qDebug("onQuit");

    static QP::QEvt const e(PELICAN::TERMINATE_SIG);
    QP::QF::PUBLISH(&e, (void *)0);
}

void Gui::closeEvent(QCloseEvent *event)
{
    static QP::QEvt const e(PELICAN::TERMINATE_SIG);
    QP::QF::PUBLISH(&e, (void *)0);
    qDebug("onQuit");
}

void Gui::on_serialPortPushButton_clicked()
{
  static QP::QEvt const e(PELICAN::PORT_INIT_SIG);
  QP::QF::PUBLISH(&e, (void *)0);
}

void Gui::on_plainTextEdit_textChanged()
{
  qDebug("wwwww");
  QString text = ui->sendTextEdit->toPlainText().toUpper();
  text.replace(QRegExp("[^A-F]"), "");

  QStringList tokens;
  for (int i = 0; i < text.length(); i+= 2) {
      tokens << text.mid(i,2);
    }
  ui->sendTextEdit->blockSignals(true);
  ui->sendTextEdit->setPlainText(tokens.join(" "));
  ui->sendTextEdit->moveCursor(QTextCursor::EndOfBlock);
  ui->sendTextEdit->blockSignals(false);
}

void Gui::on_sendTextEdit_textChanged()
{
  qDebug("wwwww");
  QString text = ui->sendTextEdit->toPlainText().toUpper();
  text.replace(QRegExp("[^a-fA-F0-9]"), "");

  QStringList tokens;
  for (int i = 0; i < text.length(); i+= 2) {
      tokens << text.mid(i,2);
    }
  ui->sendTextEdit->blockSignals(true);
  ui->sendTextEdit->setPlainText(tokens.join(" "));
  ui->sendTextEdit->moveCursor(QTextCursor::EndOfBlock);
  ui->sendTextEdit->blockSignals(false);
}
