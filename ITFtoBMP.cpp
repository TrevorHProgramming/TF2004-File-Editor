#include "mainwindow.h"
#include "ui_mainwindow.h"

void ITF::readData(){
    //TODO: add capability to change which palette is displayed.


    QByteArray txtrString = "TXTR";
    QByteArray ps2String = "PS2";
    QByteArrayMatcher matcher(txtrString);
    QByteArrayMatcher ps2Matcher(ps2String);
    QTableWidgetItem currentItem;
    long location = 0;
    long startLocation = 0;
    long currentPos = 0;
    int ps2Header = 0;
    int paletteCount = 0;
    int colorCount = 0;
    long contentLength = 0;
    std::tuple <int8_t, int8_t> nibTup;
    parent->fileData.clear();
    QFile inputFile(this->filePath);
    inputFile.open(QIODevice::ReadOnly);
    parent->fileData = inputFile.readAll();
    inputFile.close();
    //qDebug() << Q_FUNC_INFO << fileLength;

    currentPos = ps2Matcher.indexIn(parent->fileData, 0);
    //qDebug() << Q_FUNC_INFO << "PS2 header found at: " << currentPos;
    ps2Header = parent->fileData.mid(currentPos+3, 1).toHex().toInt(nullptr, 16);
    //qDebug() << Q_FUNC_INFO << "PS2 header: " << ps2Header;
    paletteCount = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+20, 4).toHex(),2).toInt(nullptr, 16);
    //qDebug() << Q_FUNC_INFO << "Palette count: " << paletteCount;
    this->paletteList.resize(paletteCount);
    if (ps2Header & 1){
        colorCount = 256;
    } else {
        colorCount = 16;
    }
    //qDebug() << Q_FUNC_INFO << "Color count: " << colorCount;
    location = matcher.indexIn(parent->fileData, 0)+4;
    startLocation = location; //this will be used later to remove the palette from the content
    contentLength = parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toInt(nullptr, 16);
    location += 4;
    //qDebug() << Q_FUNC_INFO << "content length: " << contentLength;
    for (int i = 0; i<paletteCount;i++){
        paletteList[i].size = colorCount;
        paletteList[i].paletteColors.resize(colorCount);
        for (int j = 0; j<colorCount; j++){
            paletteList[i].paletteColors[j].R = parent->fileData.mid(location, 1).toHex().toInt(nullptr, 16);
            paletteList[i].paletteColors[j].G = parent->fileData.mid(location+1, 1).toHex().toInt(nullptr, 16);
            paletteList[i].paletteColors[j].B = parent->fileData.mid(location+2, 1).toHex().toInt(nullptr, 16);
            paletteList[i].paletteColors[j].A = parent->fileData.mid(location+3, 1).toHex().toInt(nullptr, 16);
            location += 4;
        }
    }
    contentLength -= (location-startLocation); //remove the length of the palette section before getting to the pixels

    if (ps2Header & 1){
        //256 palette case. nice and easy since each pixel uses 1 byte to refer to the palette
        pixelList.resize(contentLength);
        for (int i = 0; i < contentLength; i++){
            pixelList[i] = parent->fileData.mid(location, 1).toHex().toInt(nullptr, 16);
            location += 1;
        }
    } else {
        //16 palette case. this is tougher since each pixel is only half a byte (nibble?) and we can only refer to whole bytes.
        //however every image should be an even number of pixels so we can just grab them in pairs.
        //byte_to_nib here to get a tuple of both nibbles
        int pixelIndex = 0;
        pixelList.resize(contentLength*2);
        for(int i = 0; i < contentLength; i++){
            nibTup = parent->binChanger.byte_to_nib(parent->fileData.mid(location, 1));
            pixelList[pixelIndex] = std::get<0>(nibTup);
            pixelIndex += 1;
            pixelList[pixelIndex] = std::get<1>(nibTup);
            pixelIndex += 1;
        }
    }

    qDebug() << Q_FUNC_INFO << "Palette length: " << paletteList[0].paletteColors.size();

    //this will need to be moved to its own function to be called when changing palettes

    parent->createTable(paletteList[0].paletteColors.size(), 5);
    QStringList columnNames = {"Palette Index", "Red", "Blue", "Green", "Alpha"};
    parent->PaletteTable->setHorizontalHeaderLabels(columnNames);
    parent->PaletteTable->show();
    parent->PaletteTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    for(int i = 0; i < paletteList[0].paletteColors.size(); i++){
        //I actually hate this part. Needing to make an item for every single cell feels so overcomplicated but that's how tables work, I guess.
        QTableWidgetItem *cellText0 = parent->PaletteTable->item(i,0);
        if (!cellText0){
            cellText0 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,0,cellText0);
        }
        cellText0->setText(QString::number(i));
        QTableWidgetItem *cellText = parent->PaletteTable->item(i,1);
        if (!cellText){
            cellText = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,1,cellText);
        }
        cellText->setText(QString::number(paletteList[0].paletteColors[i].R));
        QTableWidgetItem *cellText2 = parent->PaletteTable->item(i,2);
        if (!cellText2){
            cellText2 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,2,cellText2);
        }
        cellText2->setText(QString::number(paletteList[0].paletteColors[i].G));
        QTableWidgetItem *cellText3 = parent->PaletteTable->item(i,3);
        if (!cellText3){
            cellText3 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,3,cellText3);
        }
        cellText3->setText(QString::number(paletteList[0].paletteColors[i].B));
        QTableWidgetItem *cellText4 = parent->PaletteTable->item(i,4);
        if (!cellText4){
            cellText4 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,4,cellText4);
        }
        cellText4->setText(QString::number(paletteList[0].paletteColors[i].A));

    }

    //that should be it for loading data.
    //after this, have a function catching the table cell changed slot
    //needs to check if value is between 0 and 255 to be a valid color
    //then only change the associated value if valid

    //the only issue I see with this is turning the data BACK into an ITF file.
    //We don't currently know what all of the header data stands for, which could be an issue

}

void ITF::populatePalette(){

}

//void ProgWindow::convertITFToBMP(){
//    QByteArray txtrString = "TXTR";
//    QByteArray ps2String = "PS2";
//    QByteArrayMatcher matcher(txtrString);
//    QByteArrayMatcher ps2Matcher(ps2String);
//    QByteArray imageData;
//    QString fileOut;
//    int currentPos = 0;
//    int dataOffset = 0;
//    std::tuple <int8_t, int8_t> nibTup;
//    int currentPixel = 0;
//    int imageWidth = 0;
//    int imageHeight = 0;
//    int ps2Header = 0;

//    currentPos = ps2Matcher.indexIn(fileData, 0);
//    qDebug() << Q_FUNC_INFO << "PS2 header found at: " << currentPos;
//    ps2Header = reverse_input(fileData.mid(currentPos+3, 4).toHex(), 2).toInt(nullptr, 16);
//    imageHeight = reverse_input(fileData.mid(currentPos+8, 4).toHex(), 2).toInt(nullptr, 16);
//    imageWidth = reverse_input(fileData.mid(currentPos+12, 4).toHex(), 2).toInt(nullptr, 16);
//    qDebug() << Q_FUNC_INFO << "Image height: " << imageHeight << " image width: " << imageWidth;
//    if (ps2Header == 394){
//        currentPos = matcher.indexIn(fileData, 0) + 66;
//    } else {
//        currentPos = matcher.indexIn(fileData, 0) + 1026;
//    }

//    dataOffset = currentPos;

//    int maxPixel = imageWidth*imageHeight;
//    int8_t pixels[maxPixel];

//    qDebug() << Q_FUNC_INFO << "Position after palette read: " << currentPos;
//    if(ps2Header == 394){
//        for (int i = 0; i < maxPixel/2; i++){
//            //split byte data into list of nibbles
//            //method: take byte, mask upper 4 bits to clear
//            //then mask lower 4 bits to clear and shift upper bits right by 4
//            //return values from byte_to_nib as tuple
//            nibTup = byte_to_nib(fileData.mid(currentPos+i, 1));
//            pixels[currentPixel] = std::get<0>(nibTup);
//            pixels[currentPixel + 1] = std::get<1>(nibTup);
//            currentPixel += 2;
//        }
//    }
//    else {
//        for (int i = 0; i < maxPixel; i++){
//            //for images that use 8bpp, just put a byte per pixel
//            pixels[i] = fileData.mid(currentPos+i, 1).toHex().toInt(nullptr, 16);
//        }
//    }


//        //do whatever you have to do to make this a valid BMP image
//    fileOut = QFileDialog::getSaveFileName(this, tr("Select Output BMP"), QDir::currentPath() + "/BMP/", tr("Bitmap Files (*.bmp)"));
//    QFile bmpOut(fileOut);
//    QFile file(fileOut);
//    file.open(QFile::WriteOnly|QFile::Truncate);
//    file.close();

//    if(bmpOut.open(QIODevice::ReadWrite)){
//        //http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
//        //probably have to define all static variables here so they output as the proper length
//        qDebug() << Q_FUNC_INFO << "Length of file: " << fileLength;
//        QDataStream stream( &bmpOut );

//        bmpOut.write("BM"); //bitmap signiture
//        intWrite(bmpOut, fileLength); //length of the file
//        intWrite(bmpOut, 0); //reserved byte
//        intWrite(bmpOut, dataOffset); //offset from SoF to bitmap data
//        intWrite(bmpOut, 40); //size of info header
//        intWrite(bmpOut, imageWidth); //width of image
//        intWrite(bmpOut, imageHeight); //height of image
//        shortWrite(bmpOut, 1); //number of planes

//        imageData.append("BM");
//        imageData.append(sizeof(int), fileLength);
//        imageData.append(sizeof(int), 0);
//        imageData.append(sizeof(int), dataOffset);
//        imageData.append(sizeof(int), 40);
//        imageData.append(sizeof(int), imageWidth);
//        imageData.append(sizeof(int), imageHeight);
//        imageData.append(sizeof(short), 1);

//        if (ps2Header == 394){
//            shortWrite(bmpOut, 4); //bits per pixel, 16 color
//            imageData.append(sizeof(short), 4);
//        } else {
//            shortWrite(bmpOut, 8); //bits per pixel, 16 color
//            imageData.append(sizeof(short), 8);
//        }

//        intWrite(bmpOut, 0); //compression type. 0 for none.
//        intWrite(bmpOut, imageHeight*imageWidth/2); //image size. hard-coded for now
//        intWrite(bmpOut, 0); //pixels per meter, X. 0 seems to work fine
//        intWrite(bmpOut, 0); //pixels per meter, Y.
//        intWrite(bmpOut, numColors); //bits per pixel, 16 color
//        intWrite(bmpOut, 0); //important colors. 0 for all.

//        imageData.append(sizeof(int), 0);
//        imageData.append(sizeof(int), imageHeight*imageWidth/2);
//        imageData.append(sizeof(int), 0);
//        imageData.append(sizeof(int), 0);
//        imageData.append(sizeof(int), numColors);
//        imageData.append(sizeof(int), 0);

//        for (int i = 0; i < numColors; i++){
//            //qDebug() << Q_FUNC_INFO << "Colors at palette index " << i << " are: " << textureColors[i].R << textureColors[i].G << textureColors[i].B << textureColors[i].A;
//            byteWrite(bmpOut, textureColors[i].B);
//            byteWrite(bmpOut, textureColors[i].G);
//            byteWrite(bmpOut, textureColors[i].R);
//            byteWrite(bmpOut, 0); //no alpha in bitmap

//            imageData.append(1, textureColors[i].B);
//            imageData.append(1, textureColors[i].G);
//            imageData.append(1, textureColors[i].R);
//            imageData.append(1, 0);

//        }
//        int8_t tempInt;
//        if (ps2Header == 394){
//            for (int i = 0; i < maxPixel; i+=2){
//                //so you can't just write half a byte (cringe) you have to recombine the nibbles in the order you want and then write a byte
//                //god that's so dumb
//                tempInt = pixels[maxPixel-i] << 4;
//                tempInt += pixels[maxPixel-i-1];
//                byteWrite(bmpOut, tempInt);
//                imageData.append(1, tempInt);
//            }
//        } else {
//            for (int i = 0; i < maxPixel; i++){
//                byteWrite(bmpOut, pixels[maxPixel-i]);
//                imageData.append(1, pixels[maxPixel-i]);
//            }
//        }
//    }
//    //change this from a file string to a bytearray
//    //write to bytearray at the same time as the bmp file
//    convertBMPtoPNG(fileOut);
//}

//void MainWindow::populatePalette(Palette colors[], int numColors){
//    for (int i = 0; i<numColors; i++){
//        TablePalette -> setItem(i, 0, new QTableWidgetItem(QString::number(i+1)));
//        TablePalette -> setItem(i, 1, new QTableWidgetItem(QString::number(colors[i].R)));
//        TablePalette -> setItem(i, 2, new QTableWidgetItem(QString::number(colors[i].G)));
//        TablePalette -> setItem(i, 3, new QTableWidgetItem(QString::number(colors[i].B)));
//        TablePalette -> setItem(i, 4, new QTableWidgetItem(QString::number(colors[i].A)));
//    }
//}

//void MainWindow::editPalette(int row, int column){
//    switch(column){
//        case 1: textureColors[row].R = (TablePalette->item(row, column)->text()).toInt();
//            break;
//        case 2: textureColors[row].G = (TablePalette->item(row, column)->text()).toInt();
//            break;
//        case 3: textureColors[row].B = (TablePalette->item(row, column)->text()).toInt();
//            break;
//        case 4: textureColors[row].A = (TablePalette->item(row, column)->text()).toInt();
//            break;
//    }
//    //qDebug() << Q_FUNC_INFO << "Colors at palette index " << row << " are: " << textureColors[row].R << textureColors[row].G << textureColors[row].B << textureColors[row].A;
//}


//void MainWindow::saveITFPalette(){ //fileOut here is fileIn elsewhere since we're writing to the ITF
//    int location = 0;
//    int currentPos = 0;
//    QByteArray txtrString = "TXTR";
//    QByteArrayMatcher matcher(txtrString);
//    currentPos = matcher.indexIn(fileData, location) + 8;
//    QString fileOut;
//    fileOut = QFileDialog::getSaveFileName(this, tr("Select Output ITF"), QDir::currentPath() + "/ITF/", tr("ITF Files (*.itf)"));
//    QFile itfOut(fileOut);
//    QFile file(fileOut);
//    file.open(QFile::WriteOnly|QFile::Truncate);
//    file.close();

//    for (int i = 0; i < numColors; i++){
//        fileData[currentPos+(i*4)] = textureColors[i].R;
//        fileData[currentPos+1+(i*4)] = textureColors[i].G;
//        fileData[currentPos+2+(i*4)] = textureColors[i].B;
//        fileData[currentPos+3+(i*4)] = textureColors[i].A;
//    }

//    if(itfOut.open(QIODevice::ReadWrite)){
//        itfOut.write(fileData);
//        //just write filedata back to the file
//    }
//}
