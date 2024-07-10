#ifndef BINCHANGER_H
#define BINCHANGER_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QDebug>
#include <QMatrix3x3>
#include <QVector3D>
class ProgWindow;
class FileVisitor;

class SectionHeader{
public:
    QString type;
    bool hasName;
    QString name;
    int sectionLocation;
    int sectionLength;
    int id;

    SectionHeader();
    const void operator=(SectionHeader input);
    void clear();
};

class TextProperty{
public:
    QString name;
    QString readValue;
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
    qint64 hexWrite(QFile& file, QByteArray var);
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
    int line = 0;


    void readFile(QString filePath);
    long readLong(int length = 4, long location = 0);
    int readInt(int length = 4, long location = 0);
    int readUInt(int length = 4, long location = 0);
    bool readBool(int length = 1, long location = 0);
    float readFloat(int length = 4, long location = 0);
    float readMiniFloat(int length = 2, long location = 0);
    uint32_t readSpecial(int length = 4, long location = 0);
    QVector3D read3DVector();
    QVector3D readMini3DVector();
    QVector4D read4DVector();
    QColor readColor(bool isFloat);
    QQuaternion readQuaternion();
    QQuaternion readMiniQuaternion();
    QByteArray readHex(int length, long location = 0);
//    void vector3DValue(QVector3D* value, long location = 0);
//    void vector4DValue(QVector4D* value, long location = 0);
//    void quaternionValue(QQuaternion* value, long location = 0);
//    void miniQuaternionValue(QQuaternion* value, long location = 0); //referred to as "packed quaternion" in game ELF
    void hexValue(QByteArray* value, int length, long location = 0);
    void hexValue(QString* value, int length, long location = 0);
    QByteArray mid(long location, int length);
    void signature(SectionHeader *signature);
    void textSignature(SectionHeader *signature);
    QString textWord();
    void nextLine();
    int skipLine(bool checkEmpty = false);
};

class TFFile{
  public:
    QString inputPath;
    QString outputPath;
    ProgWindow *parent;
    FileData *fileData;
    QString fileName;
    QString fileExtension;
    int duplicateFileCount = 0; //defaults to 0, incremented by 1 for each file with a matching name
    bool binary; //0 for text file, 1 for binary

    TFFile();

    virtual const QStringList validOutputs(){
        qDebug() << Q_FUNC_INFO << "The chosen file does not have a valid output list.";
        return QStringList("");
    };

    virtual const QString fileCategory(){
        qDebug() << Q_FUNC_INFO << "The chosen file does not have a valid category;";
        return "";
    };

    virtual const QString fullFileName(){
        if(duplicateFileCount > 0){
            return fileName + "." + fileExtension + "(" + QString::number(duplicateFileCount) + ")";
        } else {
            return fileName + "." + fileExtension;
        }
    }

    virtual void save(QString toType);
    virtual void save(QString toType, QTextStream &stream);
    virtual void load(QString fromType);
    virtual void acceptVisitor(ProgWindow& visitor);
    virtual void updateCenter();
};

#endif // BINCHANGER_H
