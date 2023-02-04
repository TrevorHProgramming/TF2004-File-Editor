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
    QByteArray float_to_hex(float input);
    qint64 byteWrite(QFile& file, int8_t var);
    qint64 shortWrite(QFile& file, int16_t var);
    qint64 intWrite(QFile& file, int32_t var);
    qint64 longWrite(QFile& file, int64_t var);
    QVector3D forcedRotate(QMatrix3x3 rotMatrix, QVector3D offset, QVector3D point);
};

class FileData{
  public:
    QByteArray dataBytes;
    ProgWindow *parent;
    long currentPosition = 0;


    void readFile(QString filePath);
    long readLong(int length = 4, long location = 0);
    int readInt(int length = 4, long location = 0);
    int readUInt(int length = 4, long location = 0);
    bool readBool(int length = 1, long location = 0);
    float readFloat(int length = 4, long location = 0);
    QByteArray readHex(int length, long location = 0);
    QByteArray mid(long location, int length);
    QString getSignature();
};

#endif // BINCHANGER_H
