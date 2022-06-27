#ifndef BINCHANGER_H
#define BINCHANGER_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QDebug>
#include <QMatrix3x3>
#include <QVector3D>
class ProgWindow;

class BinChanger{
  public:
    ProgWindow *parent;

    QString reverse_input(QString input, int unitLength);
    QByteArray reverse_input(QByteArray input, int unitLength);
    QString hex_to_bin(QByteArray arrhex);
    int twosCompConv(QString input, int length);
    QString twosCompConv(int intput, int length);
    QString signExtend(QString input, int length);
    QByteArray remakeImm(QString tempRead, long immediate);
    std::tuple<int8_t,int8_t> byte_to_nib(QByteArray input);
    int nib_to_byte(std::tuple<int8_t, int8_t> input);
    float hex_to_float(QByteArray array);
    qint64 byteWrite(QFile& file, int8_t var);
    qint64 shortWrite(QFile& file, int16_t var);
    qint64 intWrite(QFile& file, int32_t var);
    qint64 longWrite(QFile& file, int64_t var);
    QVector3D forcedRotate(QMatrix3x3 rotMatrix, QVector3D offset, QVector3D point);
};

#endif // BINCHANGER_H
