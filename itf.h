#ifndef ITF_H
#define ITF_H

#include <QMatrix4x4>

#include <vector>

class ProgWindow;

class Color{
public:
    int32_t R;
    int32_t G;
    int32_t B;
    int32_t A;
};

class Palette{
public:
    std::vector<Color> paletteColors;
    int size;
};

class ITF{
public:
    QString filePath;
    std::vector<Palette> paletteList;
    std::vector<int> pixelList;
    int width;
    int height;
    ProgWindow *parent;

    void readData();
    void writeData();

    private:
    void populatePalette();
    void saveITFPalette();
    void editPalette(int row, int column);
    void convertBMPtoPNG(QString bmpPath);
};

#endif // ITF_H
