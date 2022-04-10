//#include "mainwindow.h"
//#include "ui_mainwindow.h"

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
