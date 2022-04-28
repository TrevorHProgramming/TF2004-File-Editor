#include "mainwindow.h"
#include "ui_mainwindow.h"

void ITF::readData(){
    //TODO: add capability to change which palette is displayed.


    QByteArray txtrString = "TXTR";
    QByteArrayMatcher matcher(txtrString);
    QTableWidgetItem currentItem;
    long location = 0;
    long startLocation = 0;
    long currentPos = 0;
    long contentLength = 0;
    int colorCount = 0;
    std::tuple <int8_t, int8_t> nibTup;
    parent->fileData.clear();
    QFile inputFile(this->filePath);
    inputFile.open(QIODevice::ReadOnly);
    parent->fileData = inputFile.readAll();
    fileLength = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+4, 4).toHex(),2).toInt(nullptr, 16);
    inputFile.close();
    qDebug() << Q_FUNC_INFO << fileLength;

    /*Load header data*/
    currentPos = 15;
    unknownByte1 = parent->fileData.mid(currentPos, 1).toHex().toInt(nullptr, 16);
    headerLength = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+1, 4).toHex(),2).toInt(nullptr, 16);
    currentPos += 8;
    propertyByte = parent->fileData.mid(currentPos, 1).toHex().toInt(nullptr, 16);
    unknown4Byte1 = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+1, 4).toHex(),2).toInt(nullptr, 16);
    width = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+5, 4).toHex(),2).toInt(nullptr, 16);
    height = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+9, 4).toHex(),2).toInt(nullptr, 16);
    unknown4Byte2 = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+13, 4).toHex(),2).toInt(nullptr, 16);
    paletteCount = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+17, 4).toHex(),2).toInt(nullptr, 16);
    unknown4Byte3 = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+21, 4).toHex(),2).toInt(nullptr, 16);
    unknown4Byte4 = parent->binChanger.reverse_input(parent->fileData.mid(currentPos+25, 4).toHex(),2).toInt(nullptr, 16);
    /*End header data. Now we can remake the file with any edits.*/

    paletteList.resize(paletteCount);
    parent->createDropdown(paletteCount);
    if (propertyByte & 1){
        colorCount = 256;
    } else {
        colorCount = 16;
    }
    //qDebug() << Q_FUNC_INFO << "Color count: " << colorCount;
    location = matcher.indexIn(parent->fileData, 0)+4;
    startLocation = location; //this will be used later to remove the palette from the content
    dataLength = parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toInt(nullptr, 16);
    contentLength = dataLength;
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


    if (propertyByte & 1){
        //256 palette case. nice and easy since each pixel uses 1 byte to refer to the palette
        contentLength -= (paletteCount*1024); //remove the length of the palette section before getting to the pixels
        pixelList.resize(contentLength);
        for (int i = 0; i < contentLength; i++){
            pixelList[i] = parent->fileData.mid(location, 1).toHex().toInt(nullptr, 16);
            location += 1;
        }
    } else {
        //16 palette case. this is tougher since each pixel is only half a byte (nibble?) and we can only refer to whole bytes.
        //however every image should be an even number of pixels so we can just grab them in pairs.
        //byte_to_nib here to get a tuple of both nibbles
        contentLength -= (paletteCount*64); //remove the length of the palette section before getting to the pixels
        pixelList.resize(contentLength*2);
        int pixelIndex = 0;
        for(int i = 0; i < contentLength; i++){
            nibTup = parent->binChanger.byte_to_nib(parent->fileData.mid(location+i, 1));
            pixelList[pixelIndex] = std::get<0>(nibTup);
            pixelIndex += 1;
            pixelList[pixelIndex] = std::get<1>(nibTup);
            pixelIndex += 1;
        }
    }

    qDebug() << Q_FUNC_INFO << "Pixel list length: " << pixelList.size() << "vs content length" << contentLength;

    populatePalette();

    //that should be it for loading data.
    //after this, have a function catching the table cell changed slot
    //needs to check if value is between 0 and 255 to be a valid color
    //then only change the associated value if valid

    //the only issue I see with this is turning the data BACK into an ITF file.
    //We don't currently know what all of the header data stands for, which could be an issue

}

void ITF::populatePalette(){
    int paletteIndex = parent->ListLevels->currentIndex();
    if(paletteIndex == -1){
        paletteIndex = 0;
    }
    qDebug() << Q_FUNC_INFO << "Function called. Palette index: " << paletteIndex;
    qDebug() << Q_FUNC_INFO << "Palette colors: " << paletteList[paletteIndex].paletteColors.size();
    parent->createTable(paletteList[paletteIndex].paletteColors.size(), 5);
    QStringList columnNames = {"Palette Index", "Red", "Blue", "Green", "Alpha"};
    parent->PaletteTable->setHorizontalHeaderLabels(columnNames);
    parent->PaletteTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    for(int i = 0; i < paletteList[paletteIndex].paletteColors.size(); i++){
        parent->PaletteTable->blockSignals(1);
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
        cellText->setText(QString::number(paletteList[paletteIndex].paletteColors[i].R));
        QTableWidgetItem *cellText2 = parent->PaletteTable->item(i,2);
        if (!cellText2){
            cellText2 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,2,cellText2);
        }
        cellText2->setText(QString::number(paletteList[paletteIndex].paletteColors[i].G));
        QTableWidgetItem *cellText3 = parent->PaletteTable->item(i,3);
        if (!cellText3){
            cellText3 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,3,cellText3);
        }
        cellText3->setText(QString::number(paletteList[paletteIndex].paletteColors[i].B));
        QTableWidgetItem *cellText4 = parent->PaletteTable->item(i,4);
        if (!cellText4){
            cellText4 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,4,cellText4);
        }
        cellText4->setText(QString::number(paletteList[paletteIndex].paletteColors[i].A));
        parent->PaletteTable->blockSignals(0);
    }
}

void ITF::editPalette(int row, int column){
    int changedValue = parent->PaletteTable->item(row, column)->text().toInt(nullptr, 10);
    int paletteIndex = parent->ListLevels->currentIndex();
    qDebug() << Q_FUNC_INFO << "Changed value: " << parent->PaletteTable->item(row, column)->text();
    qDebug() << Q_FUNC_INFO << "Row: " << row << " Column " << column;
    if (changedValue < 256 and changedValue >= 0 ){
        switch (column){
        case 1: paletteList[paletteIndex].paletteColors[row].R = changedValue; break;
        case 2: paletteList[paletteIndex].paletteColors[row].G = changedValue; break;
        case 3: paletteList[paletteIndex].paletteColors[row].B = changedValue; break;
        case 4: paletteList[paletteIndex].paletteColors[row].A = changedValue; break;
        }
    } else {
        qDebug() << Q_FUNC_INFO << "Not a valid color value.";
        switch (column){
        case 1: parent->PaletteTable->item(row,column)->text() = QString::number(paletteList[paletteIndex].paletteColors[row].R); break;
        case 2: parent->PaletteTable->item(row,column)->text() = QString::number(paletteList[paletteIndex].paletteColors[row].G); break;
        case 3: parent->PaletteTable->item(row,column)->text() = QString::number(paletteList[paletteIndex].paletteColors[row].B); break;
        case 4: parent->PaletteTable->item(row,column)->text() = QString::number(paletteList[paletteIndex].paletteColors[row].A); break;
        }
    }
}

void ITF::writeITF(){
    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output ITF"), QDir::currentPath() + "/ITF/", parent->tr("Texture Files (*.itf)"));
    QFile itfOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    std::tuple<int8_t, int8_t> nibtup;

    if (itfOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&itfOut);

        itfOut.write("FORM");
        parent->binChanger.intWrite(itfOut, fileLength);
        itfOut.write("ITF0HDR");
        parent->binChanger.byteWrite(itfOut, unknownByte1);
        parent->binChanger.intWrite(itfOut, headerLength);
        itfOut.write("PS2");
        parent->binChanger.byteWrite(itfOut, propertyByte);
        parent->binChanger.intWrite(itfOut, unknown4Byte1);
        parent->binChanger.intWrite(itfOut, height);
        parent->binChanger.intWrite(itfOut, width);
        parent->binChanger.intWrite(itfOut, paletteCount);
        parent->binChanger.intWrite(itfOut, unknown4Byte2);
        parent->binChanger.intWrite(itfOut, unknown4Byte3);
        parent->binChanger.intWrite(itfOut, unknown4Byte4);
        itfOut.write("TXTR");
        parent->binChanger.intWrite(itfOut, dataLength);
        for(int i = 0; i<paletteCount;i++){
            for(int j = 0; j < paletteList[i].paletteColors.size(); j++){
                parent->binChanger.byteWrite(itfOut, paletteList[i].paletteColors[j].R);
                parent->binChanger.byteWrite(itfOut, paletteList[i].paletteColors[j].G);
                parent->binChanger.byteWrite(itfOut, paletteList[i].paletteColors[j].B);
                parent->binChanger.byteWrite(itfOut, paletteList[i].paletteColors[j].A);
            }
        }
        if(propertyByte&1){
            //256 color
            for (int i = 0; i<pixelList.size();i++){
                parent->binChanger.byteWrite(itfOut, pixelList[i]);
            }
        } else {
            //16 color
            //combine both nibbles into a byte, then write that byte
            for (int i=0; i<pixelList.size();i+=2){
                std::get<0>(nibtup) = pixelList[i];
                std::get<1>(nibtup) = pixelList[i+1];
                parent->binChanger.byteWrite(itfOut, parent->binChanger.nib_to_byte(nibtup));
            }
        }

        //and that should be it

    }

}

void ITF::writeBMP(){
    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output BMP"), QDir::currentPath() + "/BMP/", parent->tr("Texture Files (*.bmp)"));
    QFile bmpOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    std::tuple<int8_t, int8_t> nibtup;

    /*Swizzling currently commented out to make sure the rest works right in the first place.*/
    //swizzle();
    //swizzle3();



    int dataOffset = 0; //this will be where the pixel data starts in the BMP
    //calculate by adding header length to palette length

    int numColors = 0;

    if (propertyByte&1){
        numColors = 256;
        dataOffset = 54 + (paletteCount*1024);
    } else {
        numColors = 16;
        dataOffset = 54 + (paletteCount*64);
    }

    std::vector<int> reversePixels = pixelList;
    //::reverse(reversePixels.begin(), reversePixels.end());

    if (bmpOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&bmpOut);

        bmpOut.write("BM");
        parent->binChanger.intWrite(bmpOut, fileLength);
        parent->binChanger.intWrite(bmpOut, 0); //reserved
        parent->binChanger.intWrite(bmpOut, dataOffset);
        parent->binChanger.intWrite(bmpOut, 40);    //size of info header
        parent->binChanger.intWrite(bmpOut, width);
        parent->binChanger.intWrite(bmpOut, height);
        parent->binChanger.shortWrite(bmpOut, 1);   //number of planes

        if (propertyByte&1){
            parent->binChanger.shortWrite(bmpOut, 8);
        } else {
            parent->binChanger.shortWrite(bmpOut, 4);
        }

        parent->binChanger.intWrite(bmpOut, 0); //compression type
        parent->binChanger.intWrite(bmpOut, height*width/2);
        parent->binChanger.intWrite(bmpOut, 0); //pixels per meter, x
        parent->binChanger.intWrite(bmpOut, 0); //pixels per meter, y
        parent->binChanger.intWrite(bmpOut, numColors);
        parent->binChanger.intWrite(bmpOut, 0); //important colors. 0 for all.

        for (int i = 0; i<numColors; i++){
            parent->binChanger.byteWrite(bmpOut, paletteList[parent->ListLevels->currentIndex()].paletteColors[i].B);
            parent->binChanger.byteWrite(bmpOut, paletteList[parent->ListLevels->currentIndex()].paletteColors[i].G);
            parent->binChanger.byteWrite(bmpOut, paletteList[parent->ListLevels->currentIndex()].paletteColors[i].R);
            parent->binChanger.byteWrite(bmpOut, 0); // no alpha in bmp.
        }

        if (propertyByte&1){
            for (int i = 0; i<reversePixels.size(); i++) {
                parent->binChanger.byteWrite(bmpOut, reversePixels[i]);
            }
        } else {
            for (int i = 0; i<reversePixels.size(); i+=2) {
                std::get<1>(nibtup) = reversePixels[i];
                std::get<0>(nibtup) = reversePixels[i+1];
                parent->binChanger.byteWrite(bmpOut, parent->binChanger.nib_to_byte(nibtup));
            }
        }
    }
}

void ITF::swizzle(){
    std::vector<int> swizzledImage;
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

            //swizzledImage[block_address+x+(y*blockwidth)] = pixelList[i+(j*width)];
            //swizzledImage[i+(j*width)] = pixelList[block_address+x+(y*blockwidth)];
        }
    }
    for (int i = 0; i<pixelList.size(); i++) {
        pixelList[i] = swizzledImage[i];
    }
}

void ITF::bruteForce(int imageheight, int imagewidth, int blockheight, int blockwidth, int relativeAddress){
    QString fileOut = "D:\\TF2_RevEngineer\\VBINConverter\\VBINConverter\\BMP\\bruteforce\\x" + QString::number(blockheight) + "_y" + QString::number(blockwidth) + ".bmp";
    QFile bmpOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    std::tuple<int8_t, int8_t> nibtup;

    swizzledPixels.resize(pixelList.size());

    unsigned int rowblocks = width/blockwidth;
    unsigned int blockx = 0;
    unsigned int blocky = 0;
    unsigned int x = 0;
    unsigned int y = 0;
    unsigned int block_index = 0;
    unsigned int block_address = 0;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            blockx = i/blockwidth;
            blocky = j/blockheight;

            x = (i-blockx*blockwidth);
            y = (j-blocky*blockheight);
            block_index = blockx+(blocky*rowblocks);
            block_address = block_index*blockheight*blockwidth;
            swizzledPixels[block_address+x+(y*blockwidth)] = pixelList[i+(j*width)];
        }
    }

    int originalheight = blockheight;
    while(blockheight/2>1){
        blockheight /=2;
        qDebug() << "writing file x " << blockheight << " y " << blockwidth;
        bruteForce(imageheight,imagewidth,blockheight,blockwidth,relativeAddress);
        blockwidth /=2;
        bruteForce(imageheight,imagewidth,blockheight,blockwidth,relativeAddress);
    }


    int dataOffset = 0; //this will be where the pixel data starts in the BMP
    //calculate by adding header length to palette length

    int numColors = 0;

    if (propertyByte&1){
        numColors = 256;
        dataOffset = 54 + (paletteCount*1024);
    } else {
        numColors = 16;
        dataOffset = 54 + (paletteCount*64);
    }

    std::vector<int> reversePixels = swizzledPixels;
    //::reverse(reversePixels.begin(), reversePixels.end());

    if (bmpOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&bmpOut);

        bmpOut.write("BM");
        parent->binChanger.intWrite(bmpOut, fileLength);
        parent->binChanger.intWrite(bmpOut, 0); //reserved
        parent->binChanger.intWrite(bmpOut, dataOffset);
        parent->binChanger.intWrite(bmpOut, 40);    //size of info header
        parent->binChanger.intWrite(bmpOut, width);
        parent->binChanger.intWrite(bmpOut, height);
        parent->binChanger.shortWrite(bmpOut, 1);   //number of planes

        if (propertyByte&1){
            parent->binChanger.shortWrite(bmpOut, 8);
        } else {
            parent->binChanger.shortWrite(bmpOut, 4);
        }

        parent->binChanger.intWrite(bmpOut, 0); //compression type
        parent->binChanger.intWrite(bmpOut, height*width/2);
        parent->binChanger.intWrite(bmpOut, 0); //pixels per meter, x
        parent->binChanger.intWrite(bmpOut, 0); //pixels per meter, y
        parent->binChanger.intWrite(bmpOut, numColors);
        parent->binChanger.intWrite(bmpOut, 0); //important colors. 0 for all.

        for (int i = 0; i<numColors; i++){
            parent->binChanger.byteWrite(bmpOut, paletteList[parent->ListLevels->currentIndex()].paletteColors[i].B);
            parent->binChanger.byteWrite(bmpOut, paletteList[parent->ListLevels->currentIndex()].paletteColors[i].G);
            parent->binChanger.byteWrite(bmpOut, paletteList[parent->ListLevels->currentIndex()].paletteColors[i].R);
            parent->binChanger.byteWrite(bmpOut, 0); // no alpha in bmp.
        }

        if (propertyByte&1){
            for (int i = 0; i<reversePixels.size(); i++) {
                parent->binChanger.byteWrite(bmpOut, reversePixels[i]);
            }
        } else {
            for (int i = 0; i<reversePixels.size(); i+=2) {
                std::get<1>(nibtup) = reversePixels[i];
                std::get<0>(nibtup) = reversePixels[i+1];
                parent->binChanger.byteWrite(bmpOut, parent->binChanger.nib_to_byte(nibtup));
            }
        }
    }

}

/*void ITF::swizzle3(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
    std::vector<int> swizzledImage;
    swizzledImage.resize(pixelList.size());
    long currentIndex = 0;
    int startBlockPos = dbp * 64;

    for(int y = dsay; y < dsay + rrh; y++)
    {
        for(int x = dsax; x < dsax + rrw; x++)
        {
            int pageX = x / 64;
            int pageY = y / 32;
            int page  = pageX + pageY * dbw;

            int px = x - (pageX * 64);
            int py = y - (pageY * 32);

            int blockX = px / 8;
            int blockY = py / 8;
            int block  = blockX + blockY * 8;

            int bx = px - blockX * 8;
            int by = py - blockY * 8;

            int column = by / 2;

            int cx = bx;
            int cy = by - column * 2;
            int cw = cx + cy * 8;

            swizzledImage[startBlockPos + page * 2048 + block * 64 + column * 16 + cw] = pixelList[currentIndex];
            currentIndex++;
        }
    }
    for (int i = 0; i<pixelList.size(); i++) {
        pixelList[i] = swizzledImage[i];
    }
}

void ITF::unswizzle(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data){
    std::vector<int> unswizzledImage;
    unswizzledImage.resize(pixelList.size());
    int startBlockPos = dbp * 64;
    long currentIndex = 0;

    for(int y = dsay; y < dsay + rrh; y++)
    {
        for(int x = dsax; x < dsax + rrw; x++)
        {
            int pageX = x / 64;
            int pageY = y / 32;
            int page  = pageX + pageY * dbw;

            int px = x - (pageX * 64);
            int py = y - (pageY * 32);

            int blockX = px / 8;
            int blockY = py / 8;
            int block  = blockX + blockY * 8;

            int bx = px - blockX * 8;
            int by = py - blockY * 8;

            int column = by / 2;

            int cx = bx;
            int cy = by - column * 2;
            int cw = cx + cy * 8;

            unswizzledImage[currentIndex] = pixelList[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
            currentIndex++;
        }
    }
    for (int i = 0; i<pixelList.size(); i++) {
        pixelList[i] = unswizzledImage[i];
    }
}
*/
