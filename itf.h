#ifndef ITF_H
#define ITF_H

#include <QMatrix4x4>

#include <vector>

class MainWindow;

class Palette{
public:
    int32_t R;
    int32_t G;
    int32_t B;
    int32_t A;
};

class ITF{
    private:
    void populatePalette(Palette colors[], int numColors);
    void saveITFPalette();
    void editPalette(int row, int column);
    void convertBMPtoPNG(QString bmpPath);
};

#endif // ITF_H
