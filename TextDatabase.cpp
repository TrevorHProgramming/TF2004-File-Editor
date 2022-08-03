#include "mainwindow.h"

/*read TMD file into memory
search for { and matching } locations, use to determine start and end points for each dictionary
at each {, search backwards for ~ to get section name
within each section, std::vector of dictionaries, each is a std::vector of dictitems
Type, "False", Name, default value, potential values.
for potential values, "" means any value of the given type is valid


TMD edit ideas:
change an entry to be sure these are actually used
add a valid-syntax entry and rebuild

add an extra button that edits the selected instance
also have to make it so the final list shows the instance details instead of just some random text

*/

/*See the dictitem section in database.h

*/

void dictItem::clear(){
    index = 0;
    type.clear();
    active = false;
    name.clear();
    value.clear();
    valueList.clear();
}

void TMDFile::clear(){
    classList.clear();
}

QString TMDFile::getNextLine(){
    QByteArrayMatcher findBreak;
    QByteArrayMatcher findSpace;
    findBreak.setPattern(QByteArray::fromHex(QString("0D").toUtf8()));
    findSpace.setPattern(QByteArray::fromHex(QString("20").toUtf8()));

    long spaceLocation =0;
    long startLine = 0;

    parent->fileData.currentPosition = findBreak.indexIn(parent->fileData.dataBytes, parent->fileData.currentPosition+1);
    startLine = findBreak.indexIn(parent->fileData.dataBytes, parent->fileData.currentPosition)+1; //get start of a line
    spaceLocation = findSpace.indexIn(parent->fileData.dataBytes, startLine)+1; //find the line type, using the space
    parent->fileData.currentPosition = startLine;
    return parent->fileData.mid(startLine, spaceLocation-startLine).trimmed();

}

QString TMDFile::getName(){
    QByteArrayMatcher findTilde;
    QByteArrayMatcher findSpace;
    QByteArrayMatcher findStartBrackets;
    findTilde.setPattern(QByteArray::fromHex(QString("7E").toUtf8()));
    findSpace.setPattern(QByteArray::fromHex(QString("20").toUtf8()));
    findStartBrackets.setPattern(QByteArray::fromHex(QString("7B").toUtf8()));

    long nameLocation = 0;
    long spaceLocation = 0;

    nameLocation = findTilde.indexIn(parent->fileData.dataBytes, parent->fileData.currentPosition)+1;
    spaceLocation = findSpace.indexIn(parent->fileData.dataBytes, nameLocation);
    //qDebug() << Q_FUNC_INFO << "searching for name at" << parent->fileData.currentPosition << "with name location" << nameLocation << "and space location" << spaceLocation;
    parent->fileData.currentPosition = findStartBrackets.indexIn(parent->fileData.dataBytes, parent->fileData.currentPosition+1);
    //qDebug() << Q_FUNC_INFO << "Name read as: " << parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
    return parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
}

QString BMDFile::getName(){
    QByteArrayMatcher findTilde;
    QByteArrayMatcher findStringEnd;
    findTilde.setPattern(QByteArray::fromHex(QString("7E").toUtf8()));
    findStringEnd.setPattern(QByteArray::fromHex(QString("0000").toUtf8()));

    long nameLocation = 0;
    long nameEndLocation = 0;

    //qDebug() <<Q_FUNC_INFO << "looking for name at" << parent->fileData.currentPosition;

    nameLocation = findTilde.indexIn(parent->fileData.dataBytes, parent->fileData.currentPosition);
    if(nameLocation == -1 or parent->fileData.currentPosition == -1){
        parent->fileData.currentPosition = -1;
        return "";
    } else {
        nameLocation += 1;
    }
    nameEndLocation = findStringEnd.indexIn(parent->fileData.dataBytes, nameLocation);
    parent->fileData.currentPosition = nameEndLocation+2;
    //parent->binChanger.reverse_input(parent->fileData.mid(nameEndLocation+2, 4).toHex(),2).toLong(nullptr, 16);
    qDebug() << Q_FUNC_INFO << "Name read as: " << parent->fileData.mid(nameLocation, nameEndLocation-nameLocation) << "and new location" << parent->fileData.currentPosition;
    return parent->fileData.mid(nameLocation, nameEndLocation-nameLocation);
}

const void dictItem::operator=(dictItem input){
    name = input.name;
    type = input.type;
    active = input.active;
    value = input.value;
    valueList = input.valueList;
    isDefault = input.isDefault;
    file = input.file;
}

int TMDFile::indexIn(QString searchName){
    //get index of the given dict in the TMD type list
    for (int i = 0; i < classList.size(); i++) {
        if (classList[i].name == searchName){
            return i;
        }
    }
    return -1;
}

int TMDFile::dictItemIndex(int dictIndex, QString searchName){
    //get index of the given dictItem in the given TMD dict's details
    for (int i = 0; i < classList[dictIndex].itemList.size(); i++) {
        if (classList[dictIndex].itemList[i].name == searchName) {
            return i;
        }
    }
    return -1;
}

int TDBFile::instanceIndexIn(int searchIndex){
    //get index of the given instanceIndex in the given TDB's instanceList
    for (int i = 0; i < instanceList.size(); i++) {
        if (instanceList[i].instanceIndex == searchIndex) {
            return i;
        }
    }
    return -1;
}

int BMDFile::readData(){
    QByteArrayMatcher findStartBrackets;
    QByteArrayMatcher findEndBrackets;
    QByteArrayMatcher findTilde;
    QByteArrayMatcher findBreak;
    QByteArrayMatcher findEndSpace;
    QByteArrayMatcher findQuote;
    QByteArrayMatcher findSectionEnd;
    findStartBrackets.setPattern(QByteArray::fromHex(QString("7B").toUtf8()));
    findEndBrackets.setPattern(QByteArray::fromHex(QString("7D").toUtf8()));
    findTilde.setPattern(QByteArray::fromHex(QString("7E").toUtf8()));
    findBreak.setPattern(QByteArray::fromHex(QString("0D").toUtf8()));
    findEndSpace.setPattern(QByteArray::fromHex(QString("00000000").toUtf8()));
    findQuote.setPattern(QByteArray::fromHex(QString("22").toUtf8()));
    static QRegularExpression quoteRemover = QRegularExpression("[\[\"\\]]");
    static QRegularExpression pathRemover = QRegularExpression("../");
    QByteArray fullRead;
    QByteArrayList fullSplit;
    long endLocation = 0;
    int foundSections =0;
    long endSection = 0;
    int nameLength = 0;
    QByteArray tempRead;
    QString checkDefault;
    dictItem itemDetails;
    itemDetails.file = this;

    QString name;
    /*rewrite this to work with the newly-structured text database reading
    might be able to just rewrite the individual read sections */

    parent->fileData.currentPosition = 0;
    while (parent->fileData.currentPosition != -1){
        name = getName();
        endSection = parent->fileData.currentPosition + parent->fileData.readLong();
        qDebug() << Q_FUNC_INFO << "section name: " << name << "and endSection" << endSection;
        if (name == "IncludedFiles") {
            //read TMD file name, verify that we're using the right TMD
            //since some TMD files need other TMD files, we'll need to have a list of files instead of just one.
            if(parent->fileData.currentPosition != endSection){
                endLocation = parent->fileData.currentPosition + parent->fileData.readLong();
                fullRead = parent->fileData.readHex(endLocation-parent->fileData.currentPosition).trimmed();
                //fullRead = fullRead.removeIf(quoteRemover).removeIf(pathRemover).trimmed();
                includedFile = fullRead;
                if (fullRead != "") {
                    for(int i = 0; i < parent->tmdFile.size(); i++){
                        if(fullRead.toUpper() == parent->tmdFile[i].fileName.toUpper()){
                            qDebug() << "The TDB file includes the loaded TMD file. We can continue.";
                            inheritedFileIndex = i;
                        } else {
                            parent->messageError("The file does not include the loaded TMD file. Please verify that the correct files are loaded."
                                                 "Currently loaded TMD file:" + parent->tmdFile[0].fileName + " | TDB file includes:" + fullRead);
                            return -1;
                        }
                    }
                }
            }

        } else if (name == "Dictionary"){
            name = getName();
            foundSections = 0;
            while(parent->fileData.currentPosition != -1 and parent->fileData.currentPosition < endSection){
                //name = parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
                classList.resize(foundSections+1);
                classList[foundSections].name = name;
                //qDebug() << Q_FUNC_INFO << location << nameLocation << "seciton name: " << name;
                endLocation = parent->fileData.currentPosition + parent->fileData.readLong(); // parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toLong(nullptr, 16);
                //tempRead = getNextLine();
                while(parent->fileData.currentPosition > 0 and parent->fileData.currentPosition < endLocation){
                    //read the type
                    itemDetails.clear();
                    nameLength = parent->fileData.readLong();
                    itemDetails.type = parent->fileData.readHex(nameLength);

                    //read the activation
                    nameLength = parent->fileData.readLong();
                    if (parent->fileData.readHex(nameLength) == "False") {
                        itemDetails.active = false;
                    } else {
                        itemDetails.active = true;
                    }

                    //read the name
                    nameLength = parent->fileData.readLong();
                    itemDetails.name = parent->fileData.readHex(nameLength);

                    itemDetails = addItem(itemDetails);

                    //then we have to read a length based on what type we're dealing with

                    classList[foundSections].itemList.push_back(itemDetails);
                }
                //location = -1;
                name = getName();
                foundSections++;

            }
            parent->fileData.currentPosition = -1;
        }

        for(int i = 0; i<classList.size();i++){
            //qDebug() << Q_FUNC_INFO << "class name" << i << "is" << classList[i].name;
            for (int j = 0; j < classList[i].itemList.size();j++) {
                //qDebug() << Q_FUNC_INFO << "item" << j << "in class" << i << "is" << classList[i].itemList[j].name << "type" << classList[i].itemList[j].type << "with value" << classList[i].itemList[j].value << "and value list" << classList[i].itemList[j].valueList;
            }
        }
    }

    for(int i = 0; i < classList.size();i++){
        //parent->DatabaseEdit->addItem(classList[i].name);
        //qDebug() << Q_FUNC_INFO << "Section" << i << "name: " << classList[i].name;
        for (int j = 0; j<classList[i].itemList.size();j++){
            //qDebug() << Q_FUNC_INFO << "index" << j << "Item name: " << classList[i].itemList[j].name << "item type: " << classList[i].itemList[j].type;
        }
    }

    for(int i = 0; i < instanceList.size();i++){
        //qDebug() << Q_FUNC_INFO << "Section" << i << "name: " << instanceList[i].name << "index:" << instanceList[i].instanceIndex;
        for (int j = 0; j<instanceList[i].itemList.size();j++){
            //qDebug() << Q_FUNC_INFO << "Item name: " << instanceList[i].itemList[j].name << "item type: " << instanceList[i].itemList[j].type << "float value" << instanceList[i].itemList[j].floatValue;
        }
    }

    return 0;
}

int TMDFile::readIncludedFiles(QString fullRead){
    //read TMD file name, verify that we're using the right TMD
    //since some TMD files need other TMD files, we'll need to have a list of files instead of just one.
    static QRegularExpression quoteRemover = QRegularExpression("[\[\"\\]]");
    static QRegularExpression pathRemover = QRegularExpression("../");
    int passed = -1;
    QString nameList; //for the error message

    fullRead.remove(quoteRemover).remove(pathRemover);
    qDebug() << Q_FUNC_INFO << fullRead;
    if(fullRead == ""){
        includedFile = "";
        includedFileIndex = 0;
        passed = 0;
    }
    includedFile = fullRead;

    for(int i = 0; i < parent->tmdFile.size();i++){
        nameList += parent->tmdFile[i].fileName + ", ";
        if(fullRead.toUpper() == parent->tmdFile[i].fileName.toUpper()){
            qDebug() << "The TDB file includes the loaded TMD file. We can continue.";
            inheritedFileIndex = i;
            passed = 0;
        }
    }

    if(passed == -1){
        parent->messageError("The file does not include a loaded TMD file. Please verify that the correct files are loaded."
                             "Currently loaded TMD files:" + nameList + " | TDB file includes:" + includedFile);
    }
    return passed;

}

void TMDFile::readDictionary(QStringList partSplit, int sectionIndex){
    QString tempRead;
    int typeIndexTMD = 0;
    dictItem itemDetails;
    itemDetails.file = this;

    classList.resize(sectionIndex+1);
    classList[sectionIndex].name = getName();

    for(int j = 0; j<partSplit.length(); j++){
        //qDebug() << Q_FUNC_INFO << "section" << i << "line" << j << ":" << partSplit[j];
        itemDetails.clear();
        if (partSplit[j].contains(":")){
            //handle definition inheritence here
            //qDebug() << Q_FUNC_INFO << "section" << i << "line" << j << ":" << partSplit[j];
            classList[sectionIndex].inheritedClass = partSplit[j].mid(1, partSplit[j].length()-1);
            typeIndexTMD = indexIn(classList[sectionIndex].inheritedClass);
            //qDebug() << "inherits type " << tempRead.mid(1, tempRead.length()-1) << "at" << typeIndexTMD;
            for (int j = 0; j < classList[typeIndexTMD].itemList.size();j++) {
                classList[sectionIndex].itemList.push_back(classList[typeIndexTMD].itemList[j]);
                //qDebug() << "item at " <<i << " by "  << j << "is" << itemList[typeIndexTMD][j].name;
            }
            continue;
        }
        itemDetails.type = partSplit[j].mid(0, partSplit[j].indexOf(" "));
        tempRead = partSplit[j].mid(partSplit[j].indexOf(" ")+1, partSplit[j].length() - partSplit[j].indexOf(" "));
        if (tempRead.mid(0, tempRead.indexOf(" ")) == "False") {
            itemDetails.active = false;
        } else {
            itemDetails.active = true;
        }
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        itemDetails.name = tempRead.mid(0, tempRead.indexOf(" "));
        if (tempRead.indexOf(" ") != -1) {
            tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        } else {
            tempRead = "";
        }
        //qDebug() << Q_FUNC_INFO << itemDetails.type << itemDetails.name << tempRead;

        itemDetails = addItem(itemDetails, tempRead);

        classList[sectionIndex].itemList.push_back(itemDetails);
    }

}

void TMDFile::readFileDictionary(QStringList partSplit, int sectionIndex){
    //read section and affected attributes
    dictItem itemDetails;
    itemDetails.file = this;
    int typeIndexTDB = 0;

    classList.resize(sectionIndex+1);
    classList[sectionIndex].name = getName();

    for(int j = 0; j<partSplit.length(); j++){
        //qDebug() << Q_FUNC_INFO << "line" << j << ":" << partSplit[j];
        itemDetails.clear();
        if (partSplit[j].mid(0,1) == ":"){
            //handle definition inheritence here
            typeIndexTDB = indexIn(partSplit[j].mid(1, partSplit[j].length()-1));
            classList[sectionIndex].inheritedClass = partSplit[j].mid(1, partSplit[j].length()-1);
            qDebug() << Q_FUNC_INFO << "inherits type " << partSplit[j].mid(1, partSplit[j].length()-1) << "at" << typeIndexTDB;
            for (int k = 0; k < classList[typeIndexTDB].itemList.size();k++) {
                classList[sectionIndex].itemList.push_back(classList[typeIndexTDB].itemList[k]);
            }
            continue;
        }
        itemDetails.name = partSplit[j];
        classList[sectionIndex].itemList.push_back(itemDetails);
    }
}

void TMDFile::readInstances(QStringList partSplit, int sectionIndex, QString instanceName){
    dictItem itemDetails;
    itemDetails.file = this;

    QString tempRead;
    QString checkDefault;
    int itemIndexTMD = 0;
    int typeIndexTMD = 0;
    int typeIndexTDB = 0;

    itemDetails.clear();
    instanceList.resize(sectionIndex + 1);
    instanceList[sectionIndex].name = instanceName;
    typeIndexTMD = parent->tmdFile[inheritedFileIndex].indexIn(instanceName); //get index in TMD list for current instance type
    typeIndexTDB = indexIn(instanceName);
    //qDebug() << Q_FUNC_INFO << "type index tmd:" << typeIndexTMD << "type Index TDB" << typeIndexTDB << "for name" << instanceName;
    if(typeIndexTMD == -1){
        qDebug() << Q_FUNC_INFO << "Item " << instanceName << " does not exist in " << parent->tmdFile[inheritedFileIndex].filePath <<". Verify that the TMD and TDB are both correct.";
    }

    for(int i = 0; i<partSplit.length(); i++){
        checkDefault = partSplit[i].mid(0, partSplit[i].indexOf(" "));
        tempRead = partSplit[i].mid(partSplit[i].indexOf(" ")+1, partSplit[i].length() - partSplit[i].indexOf(" "));

        if (checkDefault == "ObjectId:") {
            //qDebug() << Q_FUNC_INFO << "found instance index: " << tempRead.trimmed();
            instanceList[sectionIndex].instanceIndex = tempRead.trimmed().toInt(nullptr);
            continue;
        } else if (checkDefault == "NotDefault") {
            //qDebug() << Q_FUNC_INFO << "searching tdb index" << typeIndexTDB << "named" << classList[typeIndexTDB].name << "with item list size" << classList[typeIndexTDB].itemList.size();
            itemIndexTMD = parent->tmdFile[inheritedFileIndex].dictItemIndex(typeIndexTMD, classList[typeIndexTDB].itemList[i-1].name);
            itemDetails = parent->tmdFile[inheritedFileIndex].classList[typeIndexTMD].itemList[itemIndexTMD];
            itemDetails.isDefault = false;
            itemDetails = addItem(itemDetails, tempRead);
        } else {
            itemIndexTMD = parent->tmdFile[inheritedFileIndex].dictItemIndex(typeIndexTMD, classList[typeIndexTDB].itemList[i-1].name);
            itemDetails = parent->tmdFile[inheritedFileIndex].classList[typeIndexTMD].itemList[itemIndexTMD];
            itemDetails.isDefault = true;
        }
        //qDebug() << instanceNames[foundSections] << "item" << i << checkDefault << itemDetails.index;
        itemDetails.index = instanceList[sectionIndex].instanceIndex;
        instanceList[sectionIndex].itemList.push_back(itemDetails);
        //classList[sectionIndex].itemList.push_back(itemDetails);
    }
}


int TMDFile::readData(){
    QByteArrayMatcher findEndBrackets;
    QByteArrayMatcher findSectionEnd;
    QByteArrayMatcher findStartBrackets;
    findStartBrackets.setPattern(QByteArray::fromHex(QString("7B").toUtf8()));
    findEndBrackets.setPattern(QByteArray::fromHex(QString("7D").toUtf8()));
    static QRegularExpression quoteRemover = QRegularExpression("[\[\"\\]]");
    static QRegularExpression pathRemover = QRegularExpression("../");
    QString fullRead;
    QStringList fullSplit;
    QStringList partSplit;
    int passed = 0;
    int cutLocation = 0;
    long endSection = 0;
    int foundSections = 0;
    dictItem itemDetails;
    itemDetails.file = this;
    /*Loops through file to find major sections, then handle the information in that section based on what type of section it is
     IncludedFiles needs to be outside the inner loop since it doesn't have subsections of its own

     Ideally I'd like to get this out of a while loop but this works well enough for now.
    */
    QString majorName;
    QStringList cutNames;
    qDebug() << Q_FUNC_INFO << "THE FUNCTION RUNS. file length" << parent->fileData.dataBytes.size();

    parent->fileData.currentPosition = 0;
    majorName = getName();
    while (parent->fileData.currentPosition != -1){
        findSectionEnd.setPattern(majorName.toUtf8());
        endSection = findSectionEnd.indexIn(parent->fileData.dataBytes, parent->fileData.currentPosition);
        fullRead = parent->fileData.mid(parent->fileData.currentPosition+1, endSection-parent->fileData.currentPosition-6);
        qDebug() << Q_FUNC_INFO << "section name: " << majorName << "current position" << parent->fileData.currentPosition << "section end:" << endSection;
        if (majorName == "IncludedFiles") {
            passed = readIncludedFiles(fullRead.trimmed());
            if (passed == -1) {
                return -1;
            }
        }

        fullSplit = fullRead.split("~");
        cutNames.resize(fullSplit.size());

        //loop to remove blank lines, extra symbols, and leading/trailing whitespace
        for (int i = fullSplit.length(); i>0; i--) {
            cutLocation = findStartBrackets.indexIn(fullSplit[i-1].toUtf8(), 0);
            cutNames[i-1] = fullSplit[i-1].mid(0, cutLocation).trimmed();
            fullSplit[i-1] = fullSplit[i-1].mid(cutLocation+1).remove(quoteRemover).trimmed();
            if (fullSplit[i-1].trimmed() == "") {
                fullSplit.remove(i-1);
                cutNames.remove(i-1);
            }
        }

        foundSections = 0;
        for(int i = 0; i < fullSplit.length(); i++){

            partSplit = fullSplit[i].split("\n");

            //loop to remove blank lines, extra symbols, and leading/trailing whitespace
            for (int j = partSplit.length(); j>0; j--) {
                partSplit[j-1] = partSplit[j-1].remove(quoteRemover).trimmed();
                if (partSplit[j-1] == "" or partSplit[j-1].contains("/")) {
                    partSplit.remove(j-1);
                }
            }

            if (majorName == "Dictionary"){
                //qDebug() << Q_FUNC_INFO << fullRead;
                readDictionary(partSplit, foundSections);
            } else if (majorName == "FileDictionary"){
                readFileDictionary(partSplit, foundSections);
            } else if (majorName == "Instances") {
                qDebug() << Q_FUNC_INFO << "reading instances. cut name:" << cutNames[i];
                readInstances(partSplit, foundSections, cutNames[i]);
            }
            foundSections++;
        }
        parent->fileData.currentPosition = endSection;
        majorName = getName();
    }

    for(int i = 0; i < classList.size();i++){
        //qDebug() << Q_FUNC_INFO << "Section" << i << "name: " << classList[i].name;
        for (int j = 0; j<classList[i].itemList.size();j++){
            //qDebug() << Q_FUNC_INFO << "index" << j << "Item name: " << classList[i].itemList[j].name << "item type: " << classList[i].itemList[j].type;
        }
    }

    for(int i = 0; i < instanceList.size();i++){
        //qDebug() << Q_FUNC_INFO << "Section" << i << "name: " << instanceList[i].name << "index:" << instanceList[i].instanceIndex;
        for (int j = 0; j<instanceList[i].itemList.size();j++){
            //qDebug() << Q_FUNC_INFO << "Item name: " << instanceList[i].itemList[j].name << "item type: " << instanceList[i].itemList[j].type;
        }
    }

    return 0;
}

void TDBFile::writeData(){
    if(parent->tdbFile.empty()){
        parent->messageError("No loaded TDB files to save.");
        return;
    }
    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output TDB"), QDir::currentPath() + "/TDB/", parent->tr("Definition Files (*.tdb)"));
    QFile tdbOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    QString value;
    int startPoint = 0; //used for inherited classes
    int typeIndexTMD = 0;

    if (tdbOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&tdbOut);

        tdbOut.write("3507 \r\n");
        tdbOut.write("~IncludedFiles \r\n{\r\n");
        tdbOut.write("	");
        if(includedFile.trimmed().size() != 0){
            //qDebug() << Q_FUNC_INFO << "includedFile value" << includedFile.trimmed().toUtf8();
            tdbOut.write(includedFile.trimmed().toUtf8() + "\r\n");
        }
        tdbOut.write("\r\n} // IncludedFiles\r\n");
        tdbOut.write("\r\n~FileDictionary \r\n{");
        for(int i = 0; i < classList.size(); i++){
            startPoint = 0;
            tdbOut.write("\r\n	~");
            tdbOut.write(classList[i].name.toUtf8());

            tdbOut.write(" \r\n	{");
            //qDebug() << Q_FUNC_INFO << "class" << classList[i].name << "inherits" << classList[i].inheritedClass;
            if (classList[i].inheritedClass != "") {
                tdbOut.write("\r\n		");
                tdbOut.write("\":" + classList[i].inheritedClass.toUtf8() + "\" ");
                typeIndexTMD = indexIn(classList[i].inheritedClass);
                startPoint = classList[typeIndexTMD].itemList.size();
                //qDebug() << Q_FUNC_INFO << "type index" << typeIndexTMD << "start point" << startPoint;
            }
            for(int j = startPoint; j < classList[i].itemList.size(); j++){
                tdbOut.write("\r\n		");
                tdbOut.write("\"" + classList[i].itemList[j].name.toUtf8() + "\" ");
            }
            tdbOut.write("\r\n		\r\n	} // " + classList[i].name.toUtf8());
        }
        tdbOut.write("\r\n		\r\n} // FileDictionary\r\n");
        tdbOut.write("~Instances \r\n{");
        for(int i = 0; i < instanceList.size(); i++){
            startPoint = 0;
            tdbOut.write("\r\n	~");
            tdbOut.write(instanceList[i].name.toUtf8());

            tdbOut.write(" \r\n	{");
            //qDebug() << Q_FUNC_INFO << "class" << classList[i].name << "inherits" << classList[i].inheritedClass;
            tdbOut.write("\r\n		");
            tdbOut.write("ObjectId:       " + QString::number(instanceList[i].instanceIndex).toUtf8() + " ");
            //qDebug() << Q_FUNC_INFO << "type index" << typeIndexTMD << "start point" << startPoint;
            for(int j = startPoint; j < instanceList[i].itemList.size(); j++){
                tdbOut.write("\r\n		");
                if(instanceList[i].itemList[j].isDefault){
                    tdbOut.write("Default");
                } else {
                    value = outputValue(instanceList[i].itemList[j]);
                    tdbOut.write("NotDefault " + value.toUtf8());
                }
            }
            tdbOut.write("\r\n		\r\n	} // " + instanceList[i].name.toUtf8());
        }
        tdbOut.write("\r\n		\r\n} // Instances\r\n");
    }
}

void TMDFile::writeData(){
    if(parent->tmdFile.empty()){
        parent->messageError("No loaded TMD files to save.");
        return;
    }
    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output TMD"), QDir::currentPath() + "/TMD/", parent->tr("Definition Files (*.tmd)"));
    QFile tmdOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    QString value;
    int startPoint = 0; //used for inherited classes
    int typeIndexTMD = 0;

    if (tmdOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&tmdOut);

        tmdOut.write("3523 \r\n");
        tmdOut.write("~IncludedFiles \r\n{\r\n");
        tmdOut.write("	");
        if(includedFile.trimmed().size() != 0){
            qDebug() << Q_FUNC_INFO << "includedFile value" << includedFile.trimmed().toUtf8();
            tmdOut.write(includedFile.trimmed().toUtf8() + "\r\n");
        }
        tmdOut.write("\r\n} // IncludedFiles\r\n");
        tmdOut.write("\r\n~Dictionary \r\n{");
        for(int i = 0; i < classList.size(); i++){
            startPoint = 0;
            tmdOut.write("\r\n	~");
            tmdOut.write(classList[i].name.toUtf8());

            tmdOut.write(" \r\n	{");
            //qDebug() << Q_FUNC_INFO << "class" << classList[i].name << "inherits" << classList[i].inheritedClass;
            if (classList[i].inheritedClass != "") {
                tmdOut.write("\r\n		");
                tmdOut.write(":" + classList[i].inheritedClass.toUtf8() + " ");
                typeIndexTMD = indexIn(classList[i].inheritedClass);
                startPoint = classList[typeIndexTMD].itemList.size();
                //qDebug() << Q_FUNC_INFO << "type index" << typeIndexTMD << "start point" << startPoint;
            }
            for(int j = startPoint; j < classList[i].itemList.size(); j++){
                tmdOut.write("\r\n		");
                tmdOut.write(classList[i].itemList[j].type.toUtf8());
                if (classList[i].itemList[j].active) {
                    tmdOut.write(" \"True\"");
                } else {
                    tmdOut.write(" \"False\"");
                }
                tmdOut.write(" \"" + classList[i].itemList[j].name.toUtf8() + "\" ");
                value = outputValue(classList[i].itemList[j]);
                //function that returns qstring "value" based on item type
                tmdOut.write(value.toUtf8());

            }
            tmdOut.write("\r\n		\r\n	} // " + classList[i].name.toUtf8());
        }
        tmdOut.write("\r\n		\r\n} // Dictionary\r\n");
    }
}

void TMDFile::createDBTree(){
    if(parent->testView == nullptr){
        parent->testView = new QTreeView(parent);
    }
    if(parent->testModel == nullptr){
        parent->testModel = new QStandardItemModel;
    }

    parent->testView->setGeometry(QRect(QPoint(250,250), QSize(800,300)));
    QStandardItem *item = parent->testModel->invisibleRootItem();
    QList<QStandardItem *> dictRow;
    QStandardItem *classRow;
    QList<QStandardItem *> instanceRow;
    QList<QStandardItem *> details;
    QString enumOptions;
    QStandardItemModel model2;

    dictRow = {new QStandardItem("Included Files")};
    item->appendRow(dictRow);

    //append items to matching dictrow

    dictRow = {new QStandardItem(fileName), new QStandardItem("Type"), new QStandardItem("Value"), new QStandardItem("Value List")};
    item->appendRow(dictRow);

    //classRow = new QStandardItem("Autobot");
    //dictRow->appendRow(classRow);
    //add columns "name", "type", "value", "allowed values"

    if (fileType == "TMD"){
        for (int i = 0; i < classList.size();i++) {
            classRow = new QStandardItem(classList[i].name);
            dictRow.first()->appendRow(classRow);
            for(int j = 0; j<classList[i].itemList.size();j++){
                //detailName = new QStandardItem(tmdFile->classList[i].itemList[j].name);
                //detailType = new QStandardItem(tmdFile->classList[i].itemList[j].type);
                enumOptions = "";
                //enumOptions = classList[i].itemList[j].enumOptions.join(", ");
                enumOptions = classList[i].itemList[j].valueList.join(", ");
                details = {new QStandardItem(classList[i].itemList[j].name),new QStandardItem(classList[i].itemList[j].type),
                           new QStandardItem(classList[i].itemList[j].value), new QStandardItem(enumOptions)};
                classRow->appendRow(details);
            }
        }
    } else if (fileType=="TDB") {
        for (int i = 0; i < instanceList.size();i++) {
            instanceRow = {new QStandardItem(instanceList[i].name), new QStandardItem(QString::number(instanceList[i].instanceIndex))};
            dictRow.first()->appendRow(instanceRow);
            for(int j = 0; j<instanceList[i].itemList.size();j++){
                //detailName = new QStandardItem(tmdFile->classList[i].itemList[j].name);
                //detailType = new QStandardItem(tmdFile->classList[i].itemList[j].type);
                enumOptions = "";
                //enumOptions = instances[i][j].enumOptions.join(", ");
                enumOptions = instanceList[i].itemList[j].valueList.join(", ");
                details = {new QStandardItem(instanceList[i].itemList[j].name),new QStandardItem(instanceList[i].itemList[j].type),
                           new QStandardItem(instanceList[i].itemList[j].value), new QStandardItem(enumOptions)};
                instanceRow.first()->appendRow(details);
            }
        }
    }




    parent->testView->setModel(parent->testModel);
    //testView->expandAll();
    parent->testView->show();
}
