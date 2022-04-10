#include "mainwindow.h"
#include "ui_mainwindow.h"

void ProgWindow::openVBIN(){
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select VBIN"), QDir::currentPath() + "/VBIN/", tr("Model Files (*.vbin)"));
    vbinFile->filePath = fileIn;
    fileData.clear();

    QFile inputFile(fileIn);
    inputFile.open(QIODevice::ReadOnly);
    fileData = inputFile.readAll();

    qDebug() << Q_FUNC_INFO << "File data loaded.";
    vbinFile->readData(*this);
    qDebug() << Q_FUNC_INFO << "File data read.";

    ListLODLevels->clear();
    //highestLOD = this->vbinFile->nodeList[1].meshList[0].lodInfo.levels;
    //qDebug() << Q_FUNC_INFO << "highest LOD: " << highestLOD;
    if (highestLOD == 0){
        ListLODLevels->insertItem(0, "1");
    } else {
        for(int i=0; i<highestLOD; ++i){
            ListLODLevels->insertItem(i, QString::number(i+1));
        }
    }


}

/*void ProgWindow::openITF(){
    QByteArray txtrString = "TXTR";
    QByteArray ps2String = "PS2";
    QByteArrayMatcher matcher(txtrString);
    QByteArrayMatcher ps2Matcher(ps2String);
    int location = 0;
    int currentPos = 0;
    int ps2Header = 0;
    QString fileIn;
    fileIn = QFileDialog::getOpenFileName(this, tr("Select ITF to Open"), QDir::currentPath() + "/ITF/", tr("Texture Files (*.itf)"));
    QFile itfIn(fileIn);
    fileData.clear();
    QFile inputFile(fileIn);
    inputFile.open(QIODevice::ReadOnly);
    fileData = inputFile.readAll();
    fileLength = inputFile.size()-6;
    inputFile.close();
    qDebug() << Q_FUNC_INFO << fileLength;

    currentPos = ps2Matcher.indexIn(fileData, 0);
    qDebug() << Q_FUNC_INFO << "PS2 header found at: " << currentPos;
    ps2Header = reverse_input(fileData.mid(currentPos+3, 4).toHex(), 2).toInt(nullptr, 16);
    qDebug() << Q_FUNC_INFO << "PS2 header: " << ps2Header;
    if (ps2Header == 394){
        numColors = 16;
    } else {
        numColors = 256;
    }

    TablePalette = new QTableWidget(numColors, 5, this);
    TablePalette -> setGeometry(QRect(QPoint(50,300), QSize(600,300)));
    QStringList columnNames = {"Palette Index", "Red", "Blue", "Green", "Alpha"};
    TablePalette -> setHorizontalHeaderLabels(columnNames);
    TablePalette->show();

    connect(TablePalette, SIGNAL(cellChanged(int,int)), this, SLOT(editPalette(int,int)));

    location = matcher.indexIn(fileData, location);
    qDebug() << Q_FUNC_INFO << "TXTR section found at: " << location;
    currentPos = location + 8;
    for (int i = 0; i < numColors; i++){
        textureColors[i].R = fileData.mid(currentPos+(i*4), 1).toHex().toInt(nullptr, 16); //red
        textureColors[i].G = fileData.mid(currentPos+1+(i*4), 1).toHex().toInt(nullptr, 16); //green
        textureColors[i].B = fileData.mid(currentPos+2+(i*4), 1).toHex().toInt(nullptr, 16); //blue
        textureColors[i].A = fileData.mid(currentPos+3+(i*4), 1).toHex().toInt(nullptr, 16); //alpha
        //qDebug() << Q_FUNC_INFO << "Colors at palette index " << i << " are: " << textureColors[i].R << textureColors[i].G << textureColors[i].B << textureColors[i].A;
    }
    populatePalette(textureColors, numColors);
}*/
