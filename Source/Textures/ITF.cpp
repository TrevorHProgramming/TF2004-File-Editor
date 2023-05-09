#include "Headers/Main/mainwindow.h"
#include "ui_mainwindow.h"

//https://ps2linux.no-ip.info/playstation2-linux.com/docs/howto/display_docef7c.html?docid=75

const void Color::operator=(Color input){
    R = input.R;
    G = input.G;
    B = input.B;
    A = input.A;

};

void ITF::load(QString fromType){
    int failedRead = 0;
    if(fromType == "ITF"){
        failedRead = readDataITF();
    } else {
        failedRead = 1;
    }
    if(failedRead){
        parent->messageError("There was an error reading " + fileName);
        return;
    }
}

void ITF::save(QString toType){
    if(toType == "ITF"){
        writeITF();
    } else if (toType == "BMP"){
        writeBMP();
    }
}

void ITF::updateCenter(){
    parent->clearWindow();

    listPalettes = new QComboBox(parent->centralContainer);
    listPalettes -> setGeometry(QRect(QPoint(250,50), QSize(150,30)));
    if (paletteCount <= 0){
        listPalettes->insertItem(0, "1");
    } else {
        for(int i=0; i<paletteCount; ++i){
            listPalettes->insertItem(i, QString::number(i+1));
        }
    }
    //QAbstractButton::connect(ListPalettes, &QComboBox::currentIndexChanged, parent, [parent = this->parent]() {parent->levelSelectChange();});
    QAbstractButton::connect(listPalettes, &QComboBox::currentIndexChanged, parent, [this](int index) {selectPalette(index);});
    QAbstractButton::connect(listPalettes, &QComboBox::currentIndexChanged, parent, [this] {populatePalette();});
    listPalettes->show();
    parent->currentModeWidgets.push_back(listPalettes);

    paletteTable = new QTableWidget(paletteList[0].paletteColors.size(), 7, parent->centralContainer);
    paletteTable->setGeometry(QRect(QPoint(50,250), QSize(125*7,300)));
    QAbstractButton::connect(paletteTable, &QTableWidget::cellChanged, parent, [this](int row, int column) {editPalette(row, column);});
    paletteTable->show();
    parent->currentModeWidgets.push_back(paletteTable);

    currentPalette = 0;
    populatePalette();

}

void ITF::selectPalette(int palette){
    currentPalette = palette;
}

int ITF::readDataITF(){

    swizzled = true; //if loading an ITF, the data will be swizzled

    QByteArray txtrString = "TXTR";
    QByteArrayMatcher matcher(txtrString);
    QTableWidgetItem currentItem;
    long location = 0;
    long startLocation = 0;
    long contentLength = 0;
    int colorCount = 0;
    std::tuple <int8_t, int8_t> nibTup;
    fileLength = parent->fileData.readInt(4, 4);
    qDebug() << Q_FUNC_INFO << "file length read as" << fileLength;

    /*Load header data*/
    parent->fileData.currentPosition = 15;
    versionNum = parent->fileData.readInt(1);
    headerLength = parent->fileData.readInt();
    parent->fileData.currentPosition += 3; //skip the "PS2" label
    propertyByte = parent->fileData.readInt(1);
    unknown4Byte1 = parent->fileData.readInt();
    width = parent->fileData.readInt();
    height = parent->fileData.readInt();
    qDebug() << Q_FUNC_INFO << "image height:" << height << "width:" << width;
    unknown4Byte2 = parent->fileData.readInt();
    paletteCount = std::max(1, parent->fileData.readInt()); //some textures say 0 palettes, this catches those. possibly older ITF file version?
    qDebug() << Q_FUNC_INFO << "palette count:" << paletteCount << " found at " << parent->fileData.currentPosition;
    unknown4Byte3 = parent->fileData.readInt();
    unknown4Byte4 = parent->fileData.readInt();
    /*End header data. Now we should be able to remake the file with any edits.*/

    if(paletteCount > 16){
        //this catch is for the Sarge textures, which claim to have 23 palettes (they don't).
        paletteCount = 1;
    }

    paletteList.resize(paletteCount);
    if (propertyByte & 1){
        colorCount = 256;
    } else {
        colorCount = 16;
    }
    qDebug() << Q_FUNC_INFO << "Color count: " << colorCount;
    parent->fileData.currentPosition = matcher.indexIn(parent->fileData.dataBytes, 0)+4;
    startLocation = parent->fileData.currentPosition; //this will be used later to remove the palette from the content
    dataLength = parent->fileData.readInt();
    contentLength = dataLength;
    qDebug() << Q_FUNC_INFO << "content length: " << contentLength;
    for (int i = 0; i<paletteCount;i++){
        paletteList[i].size = colorCount;
        paletteList[i].paletteColors.resize(colorCount);
        for (int j = 0; j<colorCount; j++){
            paletteList[i].paletteColors[j].R = parent->fileData.readUInt(1);
            paletteList[i].paletteColors[j].G = parent->fileData.readUInt(1);
            paletteList[i].paletteColors[j].B = parent->fileData.readUInt(1);
            paletteList[i].paletteColors[j].A = parent->fileData.readUInt(1);
        }
    }


    int pixelIndex = 0;
    if (propertyByte & 1){
        //256 palette case. nice and easy since each pixel uses 1 byte to refer to the palette
        contentLength -= (paletteCount*1024); //remove the length of the palette section before getting to the pixels
        swizzledPixels.resize(contentLength);
        for (int i = parent->fileData.currentPosition; i < startLocation + 4 + dataLength; i++){
            swizzledPixels[pixelIndex] = parent->fileData.readUInt(1);
            pixelIndex += 1;
        }
    } else {
        //16 palette case. this is tougher since each pixel is only half a byte (nibble?) and we can only refer to whole bytes.
        //however every image should be an even number of pixels so we can just grab them in pairs.
        //byte_to_nib here to get a tuple of both nibbles
        contentLength -= (paletteCount*64); //remove the length of the palette section before getting to the pixels
        swizzledPixels.resize(contentLength*2);
        for(int i = parent->fileData.currentPosition; i < startLocation + 4 + dataLength; i++){
            nibTup = parent->binChanger.byte_to_nib(parent->fileData.mid(location+i, 1));
            swizzledPixels[pixelIndex] = std::get<0>(nibTup);
            pixelIndex += 1;
            swizzledPixels[pixelIndex] = std::get<1>(nibTup);
            pixelIndex += 1;
        }
    }

    for(int i = 0; i < paletteList[0].paletteColors.size(); i++){
        qDebug() << Q_FUNC_INFO << "color" << i << "is" << paletteList[0].paletteColors[i].R << paletteList[0].paletteColors[i].G << paletteList[0].paletteColors[i].B;
    }

    //qDebug() << Q_FUNC_INFO << "Pixel list length: " << pixelList.size() << "vs content length" << contentLength;

    //the only issue I see with this is turning the data BACK into an ITF file.
    //We don't currently know what all of the header data stands for, which could be an issue

    return 0;
}

void ITF::populatePalette(){
    int paletteIndex = currentPalette;
    if(paletteIndex == -1){
        paletteIndex = 0;
    }
    qDebug() << Q_FUNC_INFO << "Function called. Palette index: " << paletteIndex;
    qDebug() << Q_FUNC_INFO << "Palette colors: " << paletteList[paletteIndex].paletteColors.size();
    //parent->createTable(paletteList[paletteIndex].paletteColors.size(), 7);
    QStringList columnNames = {"Palette Index", "Red", "Green", "Blue", "Alpha", "Original", "Current"};
    paletteTable->setHorizontalHeaderLabels(columnNames);
    paletteTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    for(int i = 0; i < paletteList[paletteIndex].paletteColors.size(); i++){
        paletteTable->blockSignals(1);
        //I actually hate this part. Needing to make an item for every single cell feels so overcomplicated but that's how tables work, I guess.
        QTableWidgetItem *cellText0 = paletteTable->item(i,0);
        if (!cellText0){
            cellText0 = new QTableWidgetItem;
            paletteTable->setItem(i,0,cellText0);
        }
        cellText0->setText(QString::number(i));
        QTableWidgetItem *cellText = paletteTable->item(i,1);
        if (!cellText){
            cellText = new QTableWidgetItem;
            paletteTable->setItem(i,1,cellText);
        }
        cellText->setText(QString::number(paletteList[paletteIndex].paletteColors[i].R));
        QTableWidgetItem *cellText2 = paletteTable->item(i,2);
        if (!cellText2){
            cellText2 = new QTableWidgetItem;
            paletteTable->setItem(i,2,cellText2);
        }
        cellText2->setText(QString::number(paletteList[paletteIndex].paletteColors[i].G));
        QTableWidgetItem *cellText3 = paletteTable->item(i,3);
        if (!cellText3){
            cellText3 = new QTableWidgetItem;
            paletteTable->setItem(i,3,cellText3);
        }
        cellText3->setText(QString::number(paletteList[paletteIndex].paletteColors[i].B));
        QTableWidgetItem *cellText4 = paletteTable->item(i,4);
        if (!cellText4){
            cellText4 = new QTableWidgetItem;
            paletteTable->setItem(i,4,cellText4);
        }
        cellText4->setText(QString::number(paletteList[paletteIndex].paletteColors[i].A));
        QTableWidgetItem *cellText5 = paletteTable->item(i,5);
        if (!cellText5){
            cellText5 = new QTableWidgetItem;
            paletteTable->setItem(i,5,cellText5);
        }
        cellText5->setBackground(QColor::fromRgb(paletteList[paletteIndex].paletteColors[i].R,paletteList[paletteIndex].paletteColors[i].G,paletteList[paletteIndex].paletteColors[i].B));
        QTableWidgetItem *cellText6 = paletteTable->item(i,6);
        if (!cellText6){
            cellText6 = new QTableWidgetItem;
            paletteTable->setItem(i,6,cellText6);
        }
        cellText6->setBackground(QColor::fromRgb(paletteList[paletteIndex].paletteColors[i].R,paletteList[paletteIndex].paletteColors[i].G,paletteList[paletteIndex].paletteColors[i].B));
        paletteTable->blockSignals(0);
    }
}

void ITF::editPalette(int row, int column){
    int changedValue = paletteTable->item(row, column)->text().toInt(nullptr, 10);
    qDebug() << Q_FUNC_INFO << "Changed value: " << paletteTable->item(row, column)->text();
    qDebug() << Q_FUNC_INFO << "Row: " << row << " Column " << column;
    if (changedValue < 256 and changedValue >= 0 ){
        qDebug() << Q_FUNC_INFO << "Valid color value";
        switch (column){
        case 1: paletteList[currentPalette].paletteColors[row].R = changedValue; break;
        case 2: paletteList[currentPalette].paletteColors[row].G = changedValue; break;
        case 3: paletteList[currentPalette].paletteColors[row].B = changedValue; break;
        case 4: paletteList[currentPalette].paletteColors[row].A = changedValue; break;
        }
        QTableWidgetItem *cellText5 = paletteTable->item(row, 6);
        cellText5->setBackground(QColor::fromRgb(paletteList[currentPalette].paletteColors[row].R,paletteList[currentPalette].paletteColors[row].G,paletteList[currentPalette].paletteColors[row].B));
        qDebug() << Q_FUNC_INFO << "cell text" << cellText5->text();
    } else {
        qDebug() << Q_FUNC_INFO << "Not a valid color value.";
        switch (column){
        case 1: paletteTable->item(row,column)->text() = QString::number(paletteList[currentPalette].paletteColors[row].R); break;
        case 2: paletteTable->item(row,column)->text() = QString::number(paletteList[currentPalette].paletteColors[row].G); break;
        case 3: paletteTable->item(row,column)->text() = QString::number(paletteList[currentPalette].paletteColors[row].B); break;
        case 4: paletteTable->item(row,column)->text() = QString::number(paletteList[currentPalette].paletteColors[row].A); break;
        }
    }
}

void ITF::writeITF(){
//    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output ITF"), QDir::currentPath() + "/ITF/", parent->tr("Texture Files (*.itf)"));
//    if(fileOut.isEmpty()){
//        parent->messageError("ITF export cancelled.");
//        return;
//    }
    QFile itfOut(outputPath);
    QFile file(outputPath);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    std::tuple<int8_t, int8_t> nibtup;

    if(swizzledPixels.size() == 0){
        qDebug() << Q_FUNC_INFO << "no swizzled data to write.";
        //if the user imported a BMP, we'll need to swizzle the texture
        //unfortunately we can't yet :)
        //shouldn't be too long though, now that normal swizzling works
        //swizzle();
    }

    if (itfOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&itfOut);

        qDebug() << Q_FUNC_INFO << "Writing ITF header info";
        itfOut.write("FORM");
        parent->binChanger.intWrite(itfOut, fileLength);
        itfOut.write("ITF0HDR");
        parent->binChanger.byteWrite(itfOut, versionNum);
        parent->binChanger.intWrite(itfOut, headerLength);
        itfOut.write("PS2");
        parent->binChanger.byteWrite(itfOut, propertyByte);
        parent->binChanger.intWrite(itfOut, unknown4Byte1);
        parent->binChanger.intWrite(itfOut, height);
        parent->binChanger.intWrite(itfOut, width);
        parent->binChanger.intWrite(itfOut, unknown4Byte2);
        parent->binChanger.intWrite(itfOut, paletteCount);
        parent->binChanger.intWrite(itfOut, unknown4Byte3);
        parent->binChanger.intWrite(itfOut, unknown4Byte4);
        itfOut.write("TXTR");
        parent->binChanger.intWrite(itfOut, dataLength);
        
        std::vector<Color> outputPalette = paletteList[currentPalette].paletteColors;
        // we also need to reorder the palette for all 8bpp ITF files. Fortunately the process is identical. We could change this to a function.
        if(propertyByte&1){
            //256 color
            int k = 0;
            for(int i = 0; i < 8; i++){
                for(int j = 0; j < 8; j++){
                    outputPalette[k] = paletteList[currentPalette].paletteColors[k+0];
                    k++;
                }
                for(int j = 0; j < 8; j++){
                    outputPalette[k] = paletteList[currentPalette].paletteColors[k+8];
                    k++;
                }
                for(int j = 0; j < 8; j++){
                    outputPalette[k] = paletteList[currentPalette].paletteColors[k-8];
                    k++;
                }
                for(int j = 0; j < 8; j++){
                    outputPalette[k] = paletteList[currentPalette].paletteColors[k+0];
                    k++;
                }
            else {
                outputPalette = paletteList[currentPalette].paletteColors
            }
        }
        for(int j = 0; j < paletteList[currentPalette].paletteColors.size(); j++){
            parent->binChanger.byteWrite(itfOut, outputPalette[i].B);
            parent->binChanger.byteWrite(itfOut, outputPalette[i].G);
            parent->binChanger.byteWrite(itfOut, outputPalette[i].R);
            parent->binChanger.byteWrite(itfOut, outputPalette[i].A);
        }
        
        if(propertyByte&1){
            //256 color
            for (int i = 0; i<swizzledPixels.size();i++){
                parent->binChanger.byteWrite(itfOut, swizzledPixels[i]);
            }
        } else {
            //16 color
            //combine both nibbles into a byte, then write that byte
            for (int i=0; i<swizzledPixels.size();i+=2){
                std::get<0>(nibtup) = swizzledPixels[i];
                std::get<1>(nibtup) = swizzledPixels[i+1];
                //qDebug() << Q_FUNC_INFO << parent->binChanger.nib_to_byte(nibtup);
                parent->binChanger.byteWrite(itfOut, parent->binChanger.nib_to_byte(nibtup));
            }
        }

        //and that should be it

    }

}

void ITF::writeBMP(){
//    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output BMP"), QDir::currentPath() + "/BMP/", parent->tr("Texture Files (*.bmp)"));
//    if(fileOut.isEmpty()){
//        parent->messageError("BMP export cancelled.");
//        return;
//    }
    QFile bmpOut(outputPath);
    QFile file(outputPath);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();


    std::tuple<uint8_t, uint8_t> nibtup;
    std::vector<Color> outputPalette = paletteList[currentPalette].paletteColors;

    if(pixelList.size() == 0){
        unswizzle();
    }

    int dataOffset = 0; //this will be where the pixel data starts in the BMP
    //calculate by adding header length to palette length

    int numColors = 0;

//    if (propertyByte&1){
//        numColors = 256;
//        dataOffset = 54 + (paletteCount*1024);
//    } else {
//        numColors = 16;
//        dataOffset = 54 + (paletteCount*64);
//    }
    if (propertyByte&1){
        numColors = 256;
        dataOffset = 1078;
        int k = 0;
        for(int i = 0; i < 8; i++){
            for(int j = 0; j < 8; j++){
                outputPalette[k] = paletteList[currentPalette].paletteColors[k+0];
                k++;
            }
            for(int j = 0; j < 8; j++){
                outputPalette[k] = paletteList[currentPalette].paletteColors[k+8];
                k++;
            }
            for(int j = 0; j < 8; j++){
                outputPalette[k] = paletteList[currentPalette].paletteColors[k-8];
                k++;
            }
            for(int j = 0; j < 8; j++){
                outputPalette[k] = paletteList[currentPalette].paletteColors[k+0];
                k++;
            }
        }
    } else {
        numColors = 16;
        dataOffset = 118;
    }

    std::vector<uint> reversePixels = pixelList;
    int currentPixel = 0;
    //::reverse(reversePixels.begin(), reversePixels.end());
    for(int i = height-1; i >= 0; i--){
        for(int j = 0; j < width; j++){
            reversePixels[currentPixel] = pixelList[(width*i) + j];
            currentPixel++;
        }
    }

    if (bmpOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&bmpOut);

        bmpOut.write("BM");
        parent->binChanger.intWrite(bmpOut, fileLength);
        qDebug() << Q_FUNC_INFO << "file length written as" << fileLength;
        parent->binChanger.intWrite(bmpOut, 0); //reserved
        parent->binChanger.intWrite(bmpOut, dataOffset);
        qDebug() << Q_FUNC_INFO << "data offset written as" << dataOffset;
        parent->binChanger.intWrite(bmpOut, 40);    //size of info header
        parent->binChanger.intWrite(bmpOut, width);
        parent->binChanger.intWrite(bmpOut, height);
        parent->binChanger.shortWrite(bmpOut, 1);   //number of planes

        if (propertyByte&1){
            parent->binChanger.shortWrite(bmpOut, 8);
            parent->binChanger.intWrite(bmpOut, 0); //compression type
            parent->binChanger.intWrite(bmpOut, height*width);
        } else {
            parent->binChanger.shortWrite(bmpOut, 4);
            parent->binChanger.intWrite(bmpOut, 0); //compression type
            parent->binChanger.intWrite(bmpOut, height*width/2);
        }

        parent->binChanger.intWrite(bmpOut, 0); //pixels per meter, x
        parent->binChanger.intWrite(bmpOut, 0); //pixels per meter, y
        parent->binChanger.intWrite(bmpOut, numColors);
        parent->binChanger.intWrite(bmpOut, 0); //important colors. 0 for all.

//        for(int i = 0; i<paletteCount;i++){
//            for(int j = 0; j < paletteList[i].paletteColors.size(); j++){
//                parent->binChanger.byteWrite(bmpOut, paletteList[i].paletteColors[j].R);
//                parent->binChanger.byteWrite(bmpOut, paletteList[i].paletteColors[j].G);
//                parent->binChanger.byteWrite(bmpOut, paletteList[i].paletteColors[j].B);
//                parent->binChanger.byteWrite(bmpOut, paletteList[i].paletteColors[j].A);
//            }
//        }
        for(int j = 0; j < paletteList[currentPalette].paletteColors.size(); j++){
            qDebug() << Q_FUNC_INFO << "color" << j << "is" << outputPalette[j].R << outputPalette[j].G << outputPalette[j].B;
            parent->binChanger.byteWrite(bmpOut, outputPalette[j].B);
            parent->binChanger.byteWrite(bmpOut, outputPalette[j].G);
            parent->binChanger.byteWrite(bmpOut, outputPalette[j].R);
            parent->binChanger.byteWrite(bmpOut, 0);
        }

        if (propertyByte&1){
            for (int i = 0; i<reversePixels.size(); i++) {
                //qDebug() << Q_FUNC_INFO << "writing pixel" << reversePixels[i];
                parent->binChanger.byteWrite(bmpOut, reversePixels[i]);
            }
        } else {
            for (int i = 0; i<reversePixels.size(); i+=2) {
                //qDebug() << Q_FUNC_INFO << "writing pixels" << reversePixels[i] << reversePixels[i+1];
                std::get<1>(nibtup) = reversePixels[i];
                std::get<0>(nibtup) = reversePixels[i+1];
                parent->binChanger.byteWrite(bmpOut, parent->binChanger.nib_to_byte(nibtup));
            }
        }
    }
}

void ITF::swizzle(){
    std::vector<uint> swizzledImage;
    swizzledImage.resize(pixelList.size());

    //block height and width must be powers of 2
    int blockwidth = 64;
    int blockheight = 32;
    int startBlockPos = width*blockwidth;

    int rowblocks = width/blockwidth;
    int pagex = 0;
    int pagey = 0;
    int px = 0;
    int py = 0;
    int blockx = 0;
    int blocky = 0;
    int block = 0;
    int bx = 0;
    int by = 0;
    int column = 0;
    int cx = 0;
    int cy = 0;
    int cw = 0;
    int page = 0;
    int block_address = 0;
    int pixelIndex = 0;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            pagex = i/blockwidth;
            pagey = j/blockheight;
            page = pagex+(pagey*rowblocks);

            px = i-(blockx*blockwidth);
            py = j-(blocky*blockheight);

            blockx = px/8;
            blocky = py/8;
            block = blockx + (blocky*8);

            bx = px - (blockx*8);
            by = py - (blocky*8);

            column = by/2;

            cx= bx;
            cy = by-column*2;
            cw = cx+(cy*8);

            swizzledImage[pixelIndex] = pixelList[startBlockPos+(page*blockwidth*blockheight)+(block*blockwidth)+(column*blockheight)+cw];

        }
    }
    pixelList = swizzledImage;
}

void ITF::unswizzle(){
    //https://gist.github.com/Fireboyd78/1546f5c86ebce52ce05e7837c697dc72
    pixelList.resize(swizzledPixels.size());
    qDebug() << Q_FUNC_INFO << "swizzled image size" << swizzledPixels.size();
    int InterlaceMatrix[] = {
        0x00, 0x10, 0x02, 0x12,
        0x11, 0x01, 0x13, 0x03,
    };

    int Matrix[]        = { 0, 1, -1, 0 };
    int TileMatrix[]    = { 4, -4 };


    int d = 0;
    int s = 0;

    /*for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < (width >> 1); x++)
        {
            int p = swizzledPixels[s++];

            pixelList[d++] = (p & 0xF);
            pixelList[d++] = (p >> 4);
        }
    }*/


    //to-do: this code seems to work, but the variables should be renamed to actually be useful.

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int oddRow = ((y & 1) != 0);

            int num1 = ((y / 4) & 1);
            int num2 = ((x / 4) & 1);
            int num3 = (y % 4);

            int num4 = ((x / 4) % 4);

            if (oddRow){
                num4 += 4;
            }

            int num5 = ((x * 4) % 16);
            int num6 = ((x / 16) * 32);

            int num7 = (oddRow) ? ((y - 1) * width) : (y * width);

            int xx = x + num1 * TileMatrix[num2];
            int yy = y + Matrix[num3];

            int i = InterlaceMatrix[num4] + num5 + num6 + num7;
            int j = yy * width + xx;

            //qDebug() << Q_FUNC_INFO << "x" << x << "y" << y << "i" << i << "j" << j;

            pixelList[j] = swizzledPixels[i];
        }
    }

    /*if(paletteList[parent->ListLevels->currentIndex()].size == 16){
        std::vector<int> result = pixelList;

        s = 0;
        d = 0;

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < (width >> 1); x++){
                result[d++] = (int)((pixelList[s++] & 0xF) | (pixelList[s++] << 4));
            }
        }
        pixelList = result;
    }*/
}
