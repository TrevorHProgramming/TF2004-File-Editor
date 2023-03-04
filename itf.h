#ifndef ITF_H
#define ITF_H

#include <QMatrix4x4>

#include <vector>
#include <algorithm>

class ProgWindow;

class Color{
public:
    uint32_t R;
    uint32_t G;
    uint32_t B;
    uint32_t A;
};

class Palette{
public:
    std::vector<Color> paletteColors;
    int size;
};

class ITF{
public:
    ProgWindow *parent;
    QString filePath;
    QString fileName;
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
    std::vector<Palette> paletteList;
    std::vector<int> pixelList;
    std::vector<int> swizzledPixels;

    void readData();
    void writeITF();
    void writeBMP();
    void populatePalette();
    void editPalette(int row, int column);
    void bruteForce(int imageheight, int imagewidth, int blockheight, int blockwidth, int relativeAddress);

    private:
    void saveITFPalette();
    void unswizzle_8bit();
    void unswizzle_4bit();
    void unswizzle_GPT();
    void unswizzle();
    void swizzle();
    void convertBMPtoPNG(QString bmpPath);
};

#endif // ITF_H
