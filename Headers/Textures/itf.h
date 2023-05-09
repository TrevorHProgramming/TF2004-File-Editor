#ifndef ITF_H
#define ITF_H

#include <QMatrix4x4>
#include <QTableWidget>
#include <QComboBox>

#include <vector>
#include <algorithm>

#include "Headers/Main/BinChanger.h"

class ProgWindow;

class Color{
public:
    uint32_t redInt;
    uint32_t greenInt;
    uint32_t blueInt;
    uint32_t alphaInt;
    uint32_t paletteIndex;

    const void operator=(Color input);
};

class Palette{
public:
    std::vector<Color> paletteColors;
    int size;
};

class ITF : public TFFile {
public:
    const QStringList validOutputs(){
        return QStringList{"ITF", "BMP"};
    };
    virtual const QString fileCategory(){
        return "Texture";
    };
    int fileLength;
    int versionNum;
    int headerLength;
    int propertyByte;
    int unknown4Byte1;
    int height;
    int width;
    int unknown4Byte2;
    int paletteCount;
    int unknown4Byte3;
    int unknown4Byte4;
    int dataLength;
    bool swizzled;
    bool hasPalette;
    std::vector<Palette> paletteList;
    std::vector<Color> pixelList;
//    std::vector<float> pixelValues;
//    //std::vector<uint> pixelList;
//    std::vector<uint> pixelIndexes;
//    std::vector<uint> swizzledPixelIndex;
    int currentPalette;
    QComboBox *listPalettes;
    QTableWidget *paletteTable;


    void writeITF();
    void writeBMP();
    void readPalette();
    void readIndexedData();
    void readImageData();
    void writeIndexedData(QFile *fileOut);
    void writeImageData(QFile *fileOut);
    void standardizePixels();
    void populatePalette();
    void editPalette(int row, int column);
    void bruteForce(int imageheight, int imagewidth, int blockheight, int blockwidth, int relativeAddress);
    void save(QString toType);
    void load(QString fromType);
    void updateCenter();
    void selectPalette(int palette);
    int bytesPerPixel();

    private:
    void saveITFPalette();
    int readDataITF();
    void unswizzle_8bit();
    void unswizzle_4bit();
    void unswizzle_GPT();
    void unswizzle();
    void swizzle();
    void convertBMPtoPNG(QString bmpPath);
};

#endif // ITF_H
