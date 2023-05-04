#ifndef BINCHANGER_H
#define BINCHANGER_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QDebug>
#include <QMatrix3x3>
#include <QVector3D>
class ProgWindow;

class SectionHeader{
public:
    QString type;
    bool hasName;
    QString name;
    int sectionLocation;
    int sectionLength;

    SectionHeader();
    const void operator=(SectionHeader input);
    void clear();
};

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
    bool input;


    void readFile(QString filePath);
    long readLong(int length = 4, long location = 0);
    int readInt(int length = 4, long location = 0);
    int readUInt(int length = 4, long location = 0);
    bool readBool(int length = 1, long location = 0);
    float readFloat(int length = 4, long location = 0);
    float readMiniFloat(int length = 2, long location = 0);
    QVector3D read3DVector();
    QVector4D read4DVector();
    QQuaternion readQuaternion();
    QQuaternion readMiniQuaternion();
//    QByteArray readHex(int length, long location = 0);
//    void vector3DValue(QVector3D* value, long location = 0);
//    void vector4DValue(QVector4D* value, long location = 0);
//    void quaternionValue(QQuaternion* value, long location = 0);
//    void miniQuaternionValue(QQuaternion* value, long location = 0); //referred to as "packed quaternion" in game ELF
    void hexValue(QByteArray* value, int length, long location = 0);
    void hexValue(QString* value, int length, long location = 0);
    QByteArray mid(long location, int length);
    void signature(SectionHeader *signature);
    QString ProtoName();
};

class TFFile{
  public:
    QString filePath;
    ProgWindow *parent;
    FileData *fileData;
    QString fileName;
    QString fileWithoutExtension;
};

#endif // BINCHANGER_H
