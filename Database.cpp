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

/*Documentation outdated, whole system has been rewritten. TO-DO: rewrite that ^ to match

*/

void dictItem::clear(){
    index = 0;
    type.clear();
    active = false;
    name.clear();
    value.clear();
    valueList.clear();
    inherited = false;
    comment.clear();
}

void DefinitionFile::clear(){
    classList.clear();
}

QString DefinitionFile::getName(){
    if(binary){
        QByteArrayMatcher findTilde;
        QByteArrayMatcher findStringEnd;
        findTilde.setPattern(QByteArray::fromHex(QString("7E").toUtf8()));
        findStringEnd.setPattern(QByteArray::fromHex(QString("0000").toUtf8()));

        long nameLocation = 0;
        long nameEndLocation = 0;

        //qDebug() <<Q_FUNC_INFO << "looking for name at" << parent->fileData.currentPosition;

        nameLocation = findTilde.indexIn(parent->fileData.dataBytes, parent->fileData.currentPosition);
        nameEndLocation = findStringEnd.indexIn(parent->fileData.dataBytes, nameLocation);
        if(nameLocation == -1 or parent->fileData.currentPosition == -1){
            parent->fileData.currentPosition = -1;
            return "";
        } else {
            nameLocation += 1;
        }
        parent->fileData.currentPosition = nameEndLocation+2;
        //parent->binChanger.reverse_input(parent->fileData.mid(nameEndLocation+2, 4).toHex(),2).toLong(nullptr, 16);
        //qDebug() << Q_FUNC_INFO << "Name read as: " << parent->fileData.mid(nameLocation, nameEndLocation-nameLocation) << "and new location" << parent->fileData.currentPosition;
        return parent->fileData.mid(nameLocation, nameEndLocation-nameLocation);
    } else {
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
}

const void dictItem::operator=(dictItem input){
    name = input.name;
    type = input.type;
    active = input.active;
    value = input.value;
    valueList = input.valueList;
    isDefault = input.isDefault;
    file = input.file;
    comment = input.comment;
}

int DefinitionFile::indexIn(QString searchName){
    //get index of the given dict in the TMD type list
    for (int i = 0; i < classList.size(); i++) {
        if (classList[i].name == searchName){
            return i;
        }
    }
    return -1;
}

int DefinitionFile::dictItemIndex(int dictIndex, QString searchName){
    //get index of the given dictItem in the given TMD dict's details
    for (int i = 0; i < classList[dictIndex].itemList.size(); i++) {
        //qDebug() << Q_FUNC_INFO << "searching for name" << searchName << "comparing to" << classList[dictIndex].name << "'s" << classList[dictIndex].itemList[i].name;
        if (classList[dictIndex].itemList[i].name == searchName) {
            return i;
        }
    }
    qDebug() << Q_FUNC_INFO << "item" << searchName << "not found in" << classList[dictIndex].name;
    return -1;
}

int DatabaseFile::instanceIndexIn(int searchIndex){
    //get index of the given instanceIndex in the given TDB's instanceList
    for (int i = 0; i < instanceList.size(); i++) {
        if (instanceList[i].instanceIndex == searchIndex) {
            return i;
        }
    }
    return -1;
}

int DefinitionFile::readData(){
    int passed = 0;
    if(binary){
        passed = readBinary();
    } else {
        passed = readText();
    }
    return passed;
}

void DefinitionFile::readDictionary(QByteArray splitLine, int sectionIndex, QString instanceName){
    FileData tempRead;
    dictItem itemDetails;
    itemDetails.file = this;
    tempRead.dataBytes = splitLine;
    tempRead.parent = this->parent;

    classList.resize(sectionIndex+1);
    classList[sectionIndex].name = instanceName;

    while(tempRead.currentPosition < tempRead.dataBytes.size()){
        //qDebug() << Q_FUNC_INFO << "current position:" << tempRead.currentPosition;
        itemDetails.clear();
        int typeLength = tempRead.readUInt();
        itemDetails.type = tempRead.readHex(typeLength);
        //qDebug() << Q_FUNC_INFO << "type:" << itemDetails.type << "type length" << typeLength;
        int activeLength = tempRead.readUInt();
        if(tempRead.readHex(activeLength) == "False"){
            itemDetails.active = false;
        } else {
            itemDetails.active = true;
        }
        //qDebug() << Q_FUNC_INFO << "activation:" << itemDetails.active << "active length" << activeLength;
        int nameLength = tempRead.readUInt();
        itemDetails.name = tempRead.readHex(nameLength);
        //qDebug() << Q_FUNC_INFO << "name:" << itemDetails.name << "name length" << nameLength;
        itemDetails = addItem(itemDetails, &tempRead);

        classList[sectionIndex].itemList.push_back(itemDetails);
    }

}

void DefinitionFile::readFileDictionary(QByteArray splitLine, int sectionIndex, QString instanceName){
    FileData tempRead;
    dictItem itemDetails;
    itemDetails.file = this;
    tempRead.dataBytes = splitLine;
    tempRead.parent = this->parent;

    classList.resize(sectionIndex+1);
    classList[sectionIndex].name = instanceName;

    while(tempRead.currentPosition < tempRead.dataBytes.size()){
        //qDebug() << Q_FUNC_INFO << "current position:" << tempRead.currentPosition;
        itemDetails.clear();
        //don't have to worry about inheritance - a limitation of the format.
        int nameLength = tempRead.readUInt();
        itemDetails.name = tempRead.readHex(nameLength);
        //qDebug() << Q_FUNC_INFO << "name:" << itemDetails.name << "name length" << nameLength;
        classList[sectionIndex].itemList.push_back(itemDetails);
    }
}

void DefinitionFile::readInstances(QByteArray splitLine, int sectionIndex, QString instanceName){
    dictItem itemDetails;
    FileData tempRead;
    itemDetails.file = this;
    tempRead.dataBytes = splitLine;
    tempRead.parent = this->parent;

    int itemIndexBMD = 0;
    int typeIndexBMD = 0;
    int typeIndexBDB = 0;
    int currentItem = 0;

    instanceList.resize(sectionIndex + 1);
    instanceList[sectionIndex].name = instanceName;
    typeIndexBMD = parent->definitions[inheritedFileIndex].indexIn(instanceName); //get index in TMD list for current instance type
    typeIndexBDB = indexIn(instanceName);
    //qDebug() << Q_FUNC_INFO << "type index bmd:" << typeIndexBMD << "type Index BDB" << typeIndexBDB << "for name" << instanceName;
    if(typeIndexBMD == -1){
        //qDebug() << Q_FUNC_INFO << "Item " << instanceName << " does not exist in " << parent->definitions[inheritedFileIndex].filePath <<". Verify that the BMD and BDB are both correct.";
        parent->messageError("Item " + instanceName + " does not exist in " + parent->definitions[inheritedFileIndex].fileName + ". Verify that both this file and the definition file are correct.");
    }

    instanceList[sectionIndex].instanceIndex = tempRead.readUInt(2);

    //qDebug() << "checking item" << instanceList[sectionIndex].name << "at index" << instanceList[sectionIndex].instanceIndex;
    while(tempRead.currentPosition < tempRead.dataBytes.size()){
        itemDetails.clear();
        bool checkDefault = tempRead.readBool();
        //qDebug() << Q_FUNC_INFO << "searching tdb index" << typeIndexBDB << "named" << classList[typeIndexBDB].name << "with item list size" << classList[typeIndexBDB].itemList.size();
        if(checkDefault){
            itemIndexBMD = parent->definitions[inheritedFileIndex].dictItemIndex(typeIndexBMD, classList[typeIndexBDB].itemList[currentItem].name);
            itemDetails = parent->definitions[inheritedFileIndex].classList[typeIndexBMD].itemList[itemIndexBMD];
            itemDetails.isDefault = true;
        } else {
            itemIndexBMD = parent->definitions[inheritedFileIndex].dictItemIndex(typeIndexBMD, classList[typeIndexBDB].itemList[currentItem].name);
            itemDetails = parent->definitions[inheritedFileIndex].classList[typeIndexBMD].itemList[itemIndexBMD];
            itemDetails.isDefault = false;
            itemDetails = addItem(itemDetails, &tempRead);
        }
        itemDetails.index = instanceList[sectionIndex].instanceIndex;
        instanceList[sectionIndex].itemList.push_back(itemDetails);
        //qDebug() << Q_FUNC_INFO << "current position" << tempRead.currentPosition;
        currentItem += 1;
    }

}

int DefinitionFile::readBinary(){
    QList<QByteArray> byteSplit;
    QByteArray byteItem;
    FileData tempRead;
    int passed = 0;
    int cutLocation = 0;
    long startSection = 0;
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
    //qDebug() << Q_FUNC_INFO << "THE FUNCTION RUNS. file length" << parent->fileData.dataBytes.size();

    parent->fileData.currentPosition = 4;
    versionNumber = parent->fileData.readUInt();
    tempRead.currentPosition = 0;
    majorName = getName();
    while (parent->fileData.currentPosition != -1){
        majorSections.push_back(majorName);
        long sectionLength = parent->fileData.readUInt()-4;
        //parent->fileData.currentPosition = sectionStart;
        //endSection = findSectionEnd.indexIn(parent->fileData.dataBytes, parent->fileData.currentPosition);
        startSection = parent->fileData.currentPosition;
        endSection = parent->fileData.currentPosition + sectionLength;
        tempRead.dataBytes = parent->fileData.mid(parent->fileData.currentPosition, sectionLength);
        tempRead.currentPosition = 0;
        //qDebug() << Q_FUNC_INFO << "section name: " << majorName << "current position" << parent->fileData.currentPosition << "section end:" << endSection;
        if (majorName == "IncludedFiles") {
            int pathLength = tempRead.readUInt();
            QString realPath = tempRead.readHex(pathLength);
            //qDebug() << Q_FUNC_INFO << "current read:" << tempRead.dataBytes << "name length" << pathLength << "test path" << realPath;
            passed = readIncludedFiles(realPath);
            tempRead.dataBytes = tempRead.mid(pathLength, tempRead.dataBytes.length()-pathLength);
            if (passed == -1) {
                return -1;
            }
        }
        //qDebug() << Q_FUNC_INFO << "temp read position" << tempRead.currentPosition << "databytes length" << tempRead.dataBytes.length();
        foundSections = 0;
        while(tempRead.currentPosition < tempRead.dataBytes.length()){
            parent->fileData.currentPosition = startSection + tempRead.currentPosition;
            QString sectionName = getName();
            tempRead.currentPosition = parent->fileData.currentPosition - startSection;
            sectionLength = tempRead.readUInt();
            byteItem = tempRead.readHex(sectionLength-4);
            //qDebug() << Q_FUNC_INFO << "found name" << sectionName << "section length" << sectionLength << "byteitem" << byteItem.toHex();
            if (majorName == "Dictionary"){
                //qDebug() << Q_FUNC_INFO << "byte split line" << i << ":" << byteSplit[i];
                readDictionary(byteItem, foundSections, sectionName);
            } else if (majorName == "FileDictionary"){
                readFileDictionary(byteItem, foundSections, sectionName);
            } else if (majorName == "Instances") {
                //qDebug() << Q_FUNC_INFO << "reading instances. cut name:" << sectionName;
                readInstances(byteItem, foundSections, sectionName);
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

    return 0;
}

int DefinitionFile::readIncludedFiles(QString fullRead){
    //read Def file name, verify that we're using the right Def
    static QRegularExpression quoteRemover = QRegularExpression("[\[\"\\]]");
    static QRegularExpression pathRemover = QRegularExpression("../");
    int passed = -1;
    QString nameList; //for the error message

    includedFile = fullRead.remove(quoteRemover);

    fullRead = fullRead.remove(quoteRemover).remove(pathRemover);
    qDebug() << Q_FUNC_INFO << fullRead << "included file" << includedFile;
    if(fullRead == ""){
        includedFile = "";
        inheritedFileIndex = 0;
        passed = 0;
        return passed;
    }

    for(int i = 0; i < parent->definitions.size();i++){
        nameList += parent->definitions[i].fileName + ", ";
        if(fullRead.toUpper() == parent->definitions[i].fileName.toUpper()){
            qDebug() << "The Database file includes the loaded Definition file. We can continue.";
            inheritedFileIndex = i;
            passed = 0;
        }
    }

    if(passed == -1){
        parent->messageError("The file does not include a loaded TMD/BMD file. Please verify that the correct files are loaded."
                             "Currently loaded TMD/BMD files:" + nameList + " | TDB/BDB file includes:" + includedFile);
    }
    return passed;

}

void DefinitionFile::readDictionary(QStringList partSplit, int sectionIndex){
    QString tempRead;
    int typeIndexTMD = 0;
    dictItem itemDetails;
    bool inherits = false;
    bool updatedValue = false;
    static QRegularExpression quoteRemover = QRegularExpression("[\[\"\\]]");
    itemDetails.file = this;

    classList.resize(sectionIndex+1);
    classList[sectionIndex].name = getName();

    for(int j = 0; j<partSplit.length(); j++){
        //qDebug() << Q_FUNC_INFO << "section" << classList[sectionIndex].name << "line" << j << ":" << partSplit[j];
        itemDetails.clear();
        updatedValue = false;
        if (partSplit[j].contains(":")){
            inherits = true;
            //handle definition inheritence here
            //qDebug() << Q_FUNC_INFO << "section" << i << "line" << j << ":" << partSplit[j];
            classList[sectionIndex].inheritedClass = partSplit[j].mid(1, partSplit[j].length()-1);
            typeIndexTMD = indexIn(classList[sectionIndex].inheritedClass);
            //qDebug() << "inherits type " << tempRead.mid(1, tempRead.length()-1) << "at" << typeIndexTMD;
            for (int j = 0; j < classList[typeIndexTMD].itemList.size();j++) {
                itemDetails = classList[typeIndexTMD].itemList[j];
                itemDetails.inherited = true;
                classList[sectionIndex].itemList.push_back(itemDetails);
                //qDebug() << "item at " <<i << " by "  << j << "is" << itemList[typeIndexTMD][j].name;
            }
            continue;
        }
        itemDetails.type = partSplit[j].mid(0, partSplit[j].indexOf(" "));
        tempRead = partSplit[j].mid(partSplit[j].indexOf(" ")+1, partSplit[j].length() - partSplit[j].indexOf(" "));
        if (tempRead.mid(0, tempRead.indexOf(" ")).remove(quoteRemover) == "False") {
            itemDetails.active = false;
        } else {
            itemDetails.active = true;
        }
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        itemDetails.name = tempRead.mid(0, tempRead.indexOf(" ")).remove(quoteRemover);

        if (tempRead.indexOf(" ") != -1) {
            tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        } else {
            tempRead = "";
        }
        //qDebug() << Q_FUNC_INFO << itemDetails.type << itemDetails.name << tempRead;

        itemDetails = addItem(itemDetails, tempRead);
        if(itemDetails.value.contains("\"")){
            itemDetails.value = itemDetails.value.remove(quoteRemover);
            int spaceLocation = itemDetails.value.indexOf(" ");
            itemDetails.comment = itemDetails.value.mid(spaceLocation+1, itemDetails.value.length() - spaceLocation - 1);
            itemDetails.value = itemDetails.value.mid(0, spaceLocation);
        }
        if(inherits){
            for(int k = 0; k < classList[sectionIndex].itemList.size(); k++){
                if(itemDetails.name == classList[sectionIndex].itemList[k].name){
                    classList[sectionIndex].itemList[k] = itemDetails;
                    classList[sectionIndex].itemList[k].inherited = false;
                    updatedValue = true;
                }
            }
            if(!updatedValue){
                classList[sectionIndex].itemList.push_back(itemDetails);
            }
        } else {
            classList[sectionIndex].itemList.push_back(itemDetails);
        }
    }

}

void DefinitionFile::readFileDictionary(QStringList partSplit, int sectionIndex){
    //read section and affected attributes
    dictItem itemDetails;
    static QRegularExpression quoteRemover = QRegularExpression("[\[\"\\]]");
    itemDetails.file = this;
    int typeIndexTDB = 0;

    classList.resize(sectionIndex+1);
    classList[sectionIndex].name = getName();

    for(int j = 0; j<partSplit.length(); j++){
        //qDebug() << Q_FUNC_INFO << "line" << j << ":" << partSplit[j];
        partSplit[j] = partSplit[j].remove(quoteRemover);
        itemDetails.clear();
        if (partSplit[j].mid(0,1) == ":"){
            //handle definition inheritence here
            typeIndexTDB = indexIn(partSplit[j].mid(1, partSplit[j].length()-1));
            classList[sectionIndex].inheritedClass = partSplit[j].mid(1, partSplit[j].length()-1);
            qDebug() << Q_FUNC_INFO << "inherits type " << partSplit[j].mid(1, partSplit[j].length()-1) << "at" << typeIndexTDB;
            for (int k = 0; k < classList[typeIndexTDB].itemList.size();k++) {
                itemDetails = classList[typeIndexTDB].itemList[k];
                itemDetails.inherited = true;
                itemDetails.file = this;
                classList[sectionIndex].itemList.push_back(itemDetails);
                //classList[sectionIndex].itemList.push_back(classList[typeIndexTDB].itemList[k]);
            }
            continue;
        }
        itemDetails.name = partSplit[j];
        classList[sectionIndex].itemList.push_back(itemDetails);
    }
}

void DefinitionFile::readInstances(QStringList partSplit, int sectionIndex, QString instanceName){
    dictItem itemDetails;
    static QRegularExpression quoteRemover = QRegularExpression("[\[\"\\]]");
    itemDetails.file = this;

    QString tempRead;
    QString checkDefault;
    int itemIndexTMD = 0;
    int typeIndexTMD = 0;
    int typeIndexTDB = 0;

    itemDetails.clear();
    instanceList.resize(sectionIndex + 1);
    instanceList[sectionIndex].name = instanceName;
    typeIndexTMD = parent->definitions[inheritedFileIndex].indexIn(instanceName); //get index in TMD list for current instance type
    typeIndexTDB = indexIn(instanceName);
    //qDebug() << Q_FUNC_INFO << "type index tmd:" << typeIndexTMD << "type Index TDB" << typeIndexTDB << "for name" << instanceName;
    //qDebug() << Q_FUNC_INFO << "line read" << partSplit;
    if(typeIndexTMD == -1){
        qDebug() << Q_FUNC_INFO << "Item " << instanceName << " does not exist in " << parent->definitions[inheritedFileIndex].filePath <<". Verify that the TMD and TDB are both correct.";
    }

    for(int i = 0; i<partSplit.length(); i++){
        partSplit[i] = partSplit[i].remove(quoteRemover);
        checkDefault = partSplit[i].mid(0, partSplit[i].indexOf(" "));
        tempRead = partSplit[i].mid(partSplit[i].indexOf(" ")+1, partSplit[i].length() - partSplit[i].indexOf(" "));

        if (checkDefault == "ObjectId:") {
            //qDebug() << Q_FUNC_INFO << "found instance index: " << tempRead.trimmed();
            instanceList[sectionIndex].instanceIndex = tempRead.trimmed().toInt(nullptr);
            continue;
        } else if (checkDefault == "NotDefault") {
            //qDebug() << Q_FUNC_INFO << "searching tdb index" << typeIndexTDB << "named" << classList[typeIndexTDB].name << "with item list size" << classList[typeIndexTDB].itemList.size();
            itemIndexTMD = parent->definitions[inheritedFileIndex].dictItemIndex(typeIndexTMD, classList[typeIndexTDB].itemList[i-1].name);
            //qDebug() << Q_FUNC_INFO << "item index" << itemIndexTMD << "for item" << i << classList[typeIndexTDB].itemList[i-1].name;
            itemDetails = parent->definitions[inheritedFileIndex].classList[typeIndexTMD].itemList[itemIndexTMD];
            itemDetails.isDefault = false;
            itemDetails = addItem(itemDetails, tempRead);
        } else {
            itemIndexTMD = parent->definitions[inheritedFileIndex].dictItemIndex(typeIndexTMD, classList[typeIndexTDB].itemList[i-1].name);
            itemDetails = parent->definitions[inheritedFileIndex].classList[typeIndexTMD].itemList[itemIndexTMD];
            itemDetails.isDefault = true;
        }
        //qDebug() << instanceNames[foundSections] << "item" << i << checkDefault << itemDetails.index;
        itemDetails.index = instanceList[sectionIndex].instanceIndex;
        instanceList[sectionIndex].itemList.push_back(itemDetails);
        //classList[sectionIndex].itemList.push_back(itemDetails);
    }
}

void DatabaseFile::createItem(){
    bool cancelled;
    QStringList options;
    for(int i = 0; i < classList.size(); i++){
        options.append(classList[i].name);
    }

    QString chosenClass = QInputDialog::getItem(parent, parent->tr("Select TDB File:"), parent->tr("File Name:"), options, 0, false, &cancelled);
    //then a dialog box for each value in that class prompting for user input
    //give an option for "default"
}


int DefinitionFile::readText(){
    QByteArrayMatcher findEndBrackets;
    QByteArrayMatcher findSectionEnd;
    QByteArrayMatcher findStartBrackets;
    findStartBrackets.setPattern(QByteArray::fromHex(QString("7B").toUtf8()));
    findEndBrackets.setPattern(QByteArray::fromHex(QString("7D").toUtf8()));
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
    //qDebug() << Q_FUNC_INFO << "THE FUNCTION RUNS. file length" << parent->fileData.dataBytes.size();

    parent->fileData.currentPosition = 0;
    versionNumber = parent->fileData.mid(0, 4).toInt(nullptr);
    majorName = getName();
    while (parent->fileData.currentPosition != -1){
        majorSections.push_back(majorName);
        findSectionEnd.setPattern(majorName.toUtf8());
        endSection = findSectionEnd.indexIn(parent->fileData.dataBytes, parent->fileData.currentPosition);
        fullRead = parent->fileData.mid(parent->fileData.currentPosition+1, endSection-parent->fileData.currentPosition-6);
        //qDebug() << Q_FUNC_INFO << "section name: " << majorName << "current position" << parent->fileData.currentPosition << "section end:" << endSection;
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
            fullSplit[i-1] = fullSplit[i-1].mid(cutLocation+1).trimmed();
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
                partSplit[j-1] = partSplit[j-1].trimmed();
                if (partSplit[j-1] == "" or partSplit[j-1].contains(" // ")) {
                    partSplit.remove(j-1);
                }
            }

            if (majorName == "Dictionary"){
                //qDebug() << Q_FUNC_INFO << fullRead;
                readDictionary(partSplit, foundSections);
            } else if (majorName == "FileDictionary"){
                readFileDictionary(partSplit, foundSections);
            } else if (majorName == "Instances") {
                //qDebug() << Q_FUNC_INFO << "reading instances. cut name:" << cutNames[i];
                readInstances(partSplit, foundSections, cutNames[i]);
            }
            foundSections++;
        }
        for(int k = 0; k < classList.size();k++){
            //qDebug() << Q_FUNC_INFO << "Section" << k << "name: " << classList[k].name;
            for (int m = 0; m<classList[k].itemList.size();m++){
                //qDebug() << Q_FUNC_INFO << "index" << m << "Item name: " << classList[k].itemList[m].name << "item type: " << classList[k].itemList[m].type;
            }
        }
        parent->fileData.currentPosition = endSection;
        majorName = getName();
    }



    return 0;
}


void DatabaseFile::writeText(){
    if(parent->databases.empty()){
        parent->messageError("No loaded database files to save.");
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

        tdbOut.write(QString::number(versionNumber).toUtf8());
        tdbOut.write(" \r\n");
        tdbOut.write("~IncludedFiles \r\n{\r\n");
        tdbOut.write("	");
        if(includedFile.trimmed().size() != 0){
            //qDebug() << Q_FUNC_INFO << "includedFile value" << includedFile.trimmed().toUtf8();
            tdbOut.write("\"" + includedFile.trimmed().toUtf8() + "\" \r\n");
        }
        tdbOut.write("	\r\n} // IncludedFiles\r\n");
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
                //typeIndexTMD = indexIn(classList[i].inheritedClass);
                //startPoint = classList[typeIndexTMD].itemList.size();
                //qDebug() << Q_FUNC_INFO << "class name" << classList[i].inheritedClass << "/" << classList[typeIndexTMD].name << "type index" << typeIndexTMD << "start point" << startPoint;
            }
            for(int j = startPoint; j < classList[i].itemList.size(); j++){
                if(!classList[i].itemList[j].inherited){
                    tdbOut.write("\r\n		");
                    tdbOut.write("\"" + classList[i].itemList[j].name.toUtf8() + "\" ");
                }
            }
            tdbOut.write("\r\n		\r\n	} // " + classList[i].name.toUtf8());
        }
        tdbOut.write("\r\n	\r\n} // FileDictionary\r\n");
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
                    tdbOut.write("Default ");
                } else {
                    value = outputValue(instanceList[i].itemList[j]);
                    tdbOut.write("NotDefault " + value.toUtf8());
                }
            }
            tdbOut.write("\r\n		\r\n	} // " + instanceList[i].name.toUtf8());
        }
        tdbOut.write("\r\n	\r\n} // Instances\r\n");
    }
}

void DefinitionFile::writeText(){
    if(parent->definitions.empty()){
        parent->messageError("No loaded definition files to save.");
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

        tmdOut.write(QString::number(versionNumber).toUtf8());
        tmdOut.write(" \r\n");
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
                //typeIndexTMD = indexIn(classList[i].inheritedClass);
                //startPoint = classList[typeIndexTMD].itemList.size();
                //qDebug() << Q_FUNC_INFO << "type index" << typeIndexTMD << "start point" << startPoint;
            }
            for(int j = startPoint; j < classList[i].itemList.size(); j++){
                if(!classList[i].itemList[j].inherited){
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
            }
            tmdOut.write("\r\n		\r\n	} // " + classList[i].name.toUtf8());
        }
        tmdOut.write("\r\n	\r\n} // Dictionary\r\n");
    }
}

void DefinitionFile::writeBinary(){
    if(parent->definitions.empty()){
        parent->messageError("No loaded definition files to save.");
        return;
    }
    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output BMD"), QDir::currentPath() + "/BMD/", parent->tr("Definition Files (*.bmd)"));
    QFile bmdOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    QString value;
    int startPoint = 0; //used for inherited classes
    int typeIndexTMD = 0;
    long sectionLength = 4;

    if (bmdOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&bmdOut);
        getFileLengths();

        bmdOut.write("FISH");
        parent->binChanger.intWrite(bmdOut, versionNumber);
        bmdOut.write("~IncludedFiles");
        parent->binChanger.shortWrite(bmdOut, 0);
        parent->binChanger.intWrite(bmdOut, 4+includedFile.length()); //file length could be an issue. needs to be handled in "edit item"
        if(includedFile.trimmed().size() != 0){
            qDebug() << Q_FUNC_INFO << "includedFile value" << includedFile.trimmed().toUtf8() << "length" << 4+includedFile.length();
            bmdOut.write(includedFile.trimmed().toUtf8());
        }
        bmdOut.write("~Dictionary");
        parent->binChanger.shortWrite(bmdOut, 0);

        for(int i = 0; i < classList.size(); i++){
            sectionLength += classList[i].length;
            sectionLength += classList[i].name.length()+3; //2 for spacing, 1 for tilde
        }

        parent->binChanger.intWrite(bmdOut, sectionLength);

        for(int i = 0; i < classList.size(); i++){
            bmdOut.write("~" + classList[i].name.toUtf8());
            parent->binChanger.shortWrite(bmdOut, 0);
             parent->binChanger.intWrite(bmdOut, classList[i].length);
             for(int j = 0; j < classList[i].itemList.size(); j++){
                 parent->binChanger.intWrite(bmdOut, classList[i].itemList[j].type.length());
                 bmdOut.write(classList[i].itemList[j].type.toUtf8());
                 if(classList[i].itemList[j].active){
                     parent->binChanger.intWrite(bmdOut, 4);
                     bmdOut.write("True");
                 } else {
                     parent->binChanger.intWrite(bmdOut, 5);
                     bmdOut.write("False");
                 }
                 parent->binChanger.intWrite(bmdOut, classList[i].itemList[j].name.length());
                 bmdOut.write(classList[i].itemList[j].name.toUtf8());
                 binaryOutput(bmdOut, classList[i].itemList[j]);
             }
        }
    }
}

void DatabaseFile::writeBinary(){
    if(parent->definitions.empty()){
        parent->messageError("No loaded definition files to save.");
        return;
    }
    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output BDB"), QDir::currentPath() + "/BDB/", parent->tr("Definition Files (*.bdb)"));
    QFile bmdOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    QString value;
    int startPoint = 0; //used for inherited classes
    int typeIndexTMD = 0;
    long sectionLength = 4;

    if (bmdOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&bmdOut);
        getFileLengths();

        bmdOut.write("FISH");
        parent->binChanger.intWrite(bmdOut, versionNumber);
        bmdOut.write("~IncludedFiles");
        parent->binChanger.shortWrite(bmdOut, 0);
        if(includedFile.trimmed().size() != 0){
            qDebug() << Q_FUNC_INFO << "includedFile value" << includedFile.trimmed().toUtf8() << "length" << 4+includedFile.length();
            parent->binChanger.intWrite(bmdOut, 8+includedFile.length());
            parent->binChanger.intWrite(bmdOut, includedFile.length());
            bmdOut.write(includedFile.trimmed().toUtf8());
        } else {
            parent->binChanger.intWrite(bmdOut, 4);
        }
        bmdOut.write("~FileDictionary");
        parent->binChanger.shortWrite(bmdOut, 0);

        for(int i = 0; i < classList.size(); i++){
            sectionLength += classList[i].length;
            sectionLength += classList[i].name.length()+3; //2 for spacing, 1 for tilde
        }

        parent->binChanger.intWrite(bmdOut, sectionLength);

        for(int i = 0; i < classList.size(); i++){
            bmdOut.write("~" + classList[i].name.toUtf8());
            parent->binChanger.shortWrite(bmdOut, 0);
             parent->binChanger.intWrite(bmdOut, classList[i].length);
             for(int j = 0; j < classList[i].itemList.size(); j++){
                 parent->binChanger.intWrite(bmdOut, classList[i].itemList[j].name.length());
                 bmdOut.write(classList[i].itemList[j].name.toUtf8());
             }
        }

        bmdOut.write("~Instances");
        parent->binChanger.shortWrite(bmdOut, 0);
        sectionLength = 4;

        for(int i = 0; i < instanceList.size(); i++){
            sectionLength += instanceList[i].length;
            sectionLength += instanceList[i].name.length()+3; //2 for spacing, 1 for tilde
        }

        parent->binChanger.intWrite(bmdOut, sectionLength);

        for(int i = 0; i < instanceList.size(); i++){
            bmdOut.write("~" + instanceList[i].name.toUtf8());
            parent->binChanger.shortWrite(bmdOut, 0);
            parent->binChanger.intWrite(bmdOut, instanceList[i].length);
            parent->binChanger.shortWrite(bmdOut, instanceList[i].instanceIndex);
            for(int j = 0; j < instanceList[i].itemList.size(); j++){
                if(instanceList[i].itemList[j].isDefault){
                    parent->binChanger.byteWrite(bmdOut, 1);
                } else {
                    parent->binChanger.byteWrite(bmdOut, 0);
                    binaryOutput(bmdOut, instanceList[i].itemList[j]);
                }
            }
        }
    }
}

void DefinitionFile::createDBTree(){
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
    QList<QStandardItem *> details;
    QString enumOptions;
    QStandardItemModel model2;

    //dictRow = {new QStandardItem("Included Files")};
    //item->appendRow(dictRow);

    //append items to matching dictrow

    dictRow = {new QStandardItem(fileName), new QStandardItem("Type"), new QStandardItem("Value"), new QStandardItem("Value List"), new QStandardItem("Comment")};
    item->appendRow(dictRow);

    //add columns "name", "type", "value", "allowed values"

    for (int i = 0; i < classList.size();i++) {
        classRow = new QStandardItem(classList[i].name);
        dictRow.first()->appendRow(classRow);
        for(int j = 0; j<classList[i].itemList.size();j++){
            enumOptions = "";
            enumOptions = classList[i].itemList[j].valueList.join(", ");
            details = {new QStandardItem(classList[i].itemList[j].name),new QStandardItem(classList[i].itemList[j].type),
                       new QStandardItem(classList[i].itemList[j].value), new QStandardItem(enumOptions), new QStandardItem(classList[i].itemList[j].comment)};
            classRow->appendRow(details);
        }
    }

    parent->testView->setModel(parent->testModel);
    //testView->expandAll();
    parent->testView->show();
}

void DatabaseFile::createDBTree(){

    if(parent->testView == nullptr){
        parent->testView = new QTreeView(parent);
    }
    parent->testView->setSortingEnabled(false);
    if(parent->testModel == nullptr){
        parent->testModel = new QStandardItemModel;
    }

    QHeaderView *headers = parent->testView->header();
    headers->setSectionsClickable(true);
    QObject::connect(headers, &QHeaderView::sectionClicked, [this](int logicalIndex){sortDBTree(logicalIndex);});

    parent->testView->setGeometry(QRect(QPoint(250,250), QSize(800,300)));
    QStandardItem *item = parent->testModel->invisibleRootItem();
    QList<QStandardItem *> dictRow;
    QList<QStandardItem *> instanceRow;
    QList<QStandardItem *> details;
    QString enumOptions;
    QStandardItemModel model2;

    //dictRow = {new QStandardItem("Included Files")};
    //item->appendRow(dictRow);

    //append items to matching dictrow

    dictRow = {new QStandardItem(fileName), new QStandardItem("Type"), new QStandardItem("Value"), new QStandardItem("Value List"), new QStandardItem("Default")};
    item->appendRow(dictRow);

    //add columns "name", "type", "value", "allowed values"


    for (int i = 0; i < instanceList.size();i++) {
        instanceRow = {new QStandardItem(instanceList[i].name), new QStandardItem(QString::number(instanceList[i].instanceIndex))};
        dictRow.first()->appendRow(instanceRow);
        for(int j = 0; j<instanceList[i].itemList.size();j++){
            enumOptions = "";
            enumOptions = instanceList[i].itemList[j].valueList.join(", ");
            details = {new QStandardItem(instanceList[i].itemList[j].name), new QStandardItem(instanceList[i].itemList[j].type)
                       , new QStandardItem(instanceList[i].itemList[j].value), new QStandardItem(enumOptions), new QStandardItem(instanceList[i].itemList[j].isDefault)};
            instanceRow.first()->appendRow(details);
        }
    }

    parent->testView->setModel(parent->testModel);
    parent->testView->setSortingEnabled(true);
    //testView->expandAll();
    parent->testView->show();
}

void DefinitionFile::sortDBTree(int column){
    parent->testView->sortByColumn(column, Qt::AscendingOrder);
}
