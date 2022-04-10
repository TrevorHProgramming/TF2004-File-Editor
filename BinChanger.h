#ifndef BINCHANGER_H
#define BINCHANGER_H

#include <QString>
#include <QByteArray>
class MainWindow;

class BinChanger{
  public:
    QString reverse_input(QString input, int unitLength);
    QByteArray reverse_input(QByteArray input, int unitLength);
    QString hex_to_bin(QByteArray arrhex);
    int twosCompConv(QString input, int length);
    QString twosCompConv(int intput, int length);
    QString signExtend(QString input, int length);
    QByteArray remakeImm(QString tempRead, long immediate);
    std::tuple<int8_t,int8_t> byte_to_nib(QByteArray input);
    float hex_to_float(QByteArray array);
};

#endif // BINCHANGER_H
