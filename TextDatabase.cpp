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

    location = findBreak.indexIn(parent->fileData, location+1);
    startLine = findBreak.indexIn(parent->fileData, location)+1; //get start of a line
    spaceLocation = findSpace.indexIn(parent->fileData, startLine)+1; //find the line type, using the space
    location = startLine;
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

    nameLocation = findTilde.indexIn(parent->fileData, location)+1;
    spaceLocation = findSpace.indexIn(parent->fileData, nameLocation);
    location = findStartBrackets.indexIn(parent->fileData, location+1);
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

    qDebug() <<Q_FUNC_INFO << "looking for name at" << location;

    nameLocation = findTilde.indexIn(parent->fileData, location);
    if(nameLocation == -1 or location == -1){
        location = -1;
        return "";
    } else {
        nameLocation += 1;
    }
    nameEndLocation = findStringEnd.indexIn(parent->fileData, nameLocation);
    location = nameEndLocation+2;
    //parent->binChanger.reverse_input(parent->fileData.mid(nameEndLocation+2, 4).toHex(),2).toLong(nullptr, 16);
    qDebug() << Q_FUNC_INFO << "Name read as: " << parent->fileData.mid(nameLocation, nameEndLocation-nameLocation) << "and new location" << location;
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
    int typeIndexTMD = 0;
    int itemIndexTMD = 0;
    long nextLocation = 0;
    long spaceLocation = 0;
    long endSection = 0;
    int instanceIndex = 0;
    int typeIndexTDB = 0;
    long startLine = 0;
    int nameLength = 0;
    QByteArray tempRead;
    QString checkDefault;
    dictItem itemDetails;
    itemDetails.file = this;
    /*loop here until end of file
    find open bracket, get name
    find close bracket and next open bracket
    if next open bracket is sooner than next close bracket, we're in a subgroup. start recording objects

    ↑ not currently doing things that way, but that should be the plan.
    afind a subgroup that can be recorded, read the whole group into a QString
    Split that QString into a QStringList along /n and iterate through that
    */
    QString name;

    location = 0;
    while (location != -1){
        name = getName();
        endSection = location + parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toLong(nullptr, 16);
        qDebug() << Q_FUNC_INFO << "section name: " << name << "and endSection" << endSection;
        if (name == "IncludedFiles") {
            //read TMD file name, verify that we're using the right TMD
            //since some TMD files need other TMD files, we'll need to have a list of files instead of just one.
            location += 4;
            if(location != endSection){
                endLocation = location + parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toLong(nullptr, 16);
                location += 4;
                fullRead = parent->fileData.mid(location, endLocation-location).trimmed();
                //fullRead = fullRead.removeIf(quoteRemover).removeIf(pathRemover).trimmed();
                includedFile = fullRead;
                if (fullRead != "") {
                    if(fullRead.toUpper() == parent->tmdFile[0]->fileName.toUpper()){
                        qDebug() << "The TDB file includes the loaded TMD file. We can continue.";
                    } else {
                        qDebug() << Q_FUNC_INFO << "The file does not include the loaded TMD file. Please verify that the correct files are loaded.";
                        qDebug() << Q_FUNC_INFO << "Currently loaded TMD file:" << parent->tmdFile[0]->fileName << " | TDB file includes:" << fullRead;
                        return -1;
                    }
                }
            }

        } else if (name == "Dictionary"){
            name = getName();
            foundSections = 0;
            while(location != -1 and location < endSection){
                //name = parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
                classList.resize(foundSections+1);
                classList[foundSections].name = name;
                //qDebug() << Q_FUNC_INFO << location << nameLocation << "seciton name: " << name;
                endLocation = location + parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toLong(nullptr, 16);
                location += 4;
                //tempRead = getNextLine();
                while(location > 0 and location < endLocation){
                    //read the type
                    itemDetails.clear();
                    nameLength = parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toLong(nullptr, 16);
                    location += 4;
                    itemDetails.type = parent->fileData.mid(location, nameLength);
                    location += nameLength;

                    //read the activation
                    nameLength = parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toLong(nullptr, 16);
                    location += 4;
                    if (parent->fileData.mid(location, nameLength) == "False") {
                        itemDetails.active = false;
                    } else {
                        itemDetails.active = true;
                    }
                    location += nameLength;

                    //read the name
                    nameLength = parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toLong(nullptr, 16);
                    location += 4;
                    itemDetails.name = parent->fileData.mid(location, nameLength);
                    location += nameLength;

                    itemDetails = addItem(itemDetails);

                    //then we have to read a length based on what type we're dealing with

                    classList[foundSections].itemList.push_back(itemDetails);
                }
                //location = -1;
                name = getName();
                foundSections++;

            }
            location = -1;
        } /*else if (name == "FileDictionary") {
            //read section and affected attributes
            name = getName();
            nextLocation = findStartBrackets.indexIn(parent->fileData, location+1);
            foundSections = 0;
            endLocation = findEndBrackets.indexIn(parent->fileData, location);
            while(nextLocation != -1 and (nextLocation < endSection or endLocation < endSection)){
                classList.resize(foundSections+1);
                classList[foundSections].name = name;
                //qDebug() << Q_FUNC_INFO << location << nameLocation << "seciton name: " << name;
                //tempRead = getNextLine();
                //qDebug() << Q_FUNC_INFO << "endlocation: " << endLocation;
                instanceIndex = 0;
                fullRead = parent->fileData.mid(location, endLocation-location);
                //fullSplit = fullRead.split("\n");

                //loop to remove blank lines
                for (int i = fullSplit.length(); i>0; i--) {
                    if (fullSplit[i-1].trimmed() == "") {
                        fullSplit.remove(i-1);
                    }
                }

                for(int i = 0; i<fullSplit.length(); i++){
                    fullSplit[i] = fullSplit[i].removeIf(quoteRemover).trimmed(); //prep line by removing extra symbols and whitespace
                    //qDebug() << Q_FUNC_INFO << "line" << i << ":" << fullSplit[i];
                    itemDetails.clear();
                    if (fullSplit[i].mid(0,1) == ":"){
                        //handle definition inheritence here
                        typeIndexTDB = indexIn(fullSplit[i].mid(1, fullSplit[i].length()-1));
                        classList[foundSections].inheritedClass = fullSplit[i].mid(1, fullSplit[i].length()-1);
                        //qDebug() << "inherits type " << fullSplit[i].mid(1, fullSplit[i].length()-1) << "at" << typeIndexTDB;
                        for (int j = 0; j < classList[typeIndexTDB].itemList.size();j++) {
                            //qDebug() << "item at " << typeIndexTDB << " by "  << j << "is" << itemList[typeIndexTDB][j].name;
                            classList[foundSections].itemList.push_back(classList[typeIndexTDB].itemList[j]);
                        }
                        continue;
                    }
                    //qDebug() << Q_FUNC_INFO << fullSplit[i];
                    itemDetails.name = fullSplit[i];
                    classList[foundSections].itemList.push_back(itemDetails);
                }
                name = getName();
                nextLocation = findStartBrackets.indexIn(parent->fileData, location+1);
                endLocation = findEndBrackets.indexIn(parent->fileData, location);
                foundSections++;
            }
            location = endSection;

        } else if (name == "Instances") {
            //check that FileDictionary has already been read
            //then find NotDefault values and change them in the instance
            //read section and affected attributes
            name = getName();
            nextLocation = findStartBrackets.indexIn(parent->fileData, location+1);
            endLocation = findEndBrackets.indexIn(parent->fileData, location);
            foundSections = 0;
            while(location!= -1 and (nextLocation != -1 or endLocation !=-1) and (nextLocation < endSection or endLocation < endSection)){
                //location = nextLocation;
                instanceList.resize(foundSections + 1);
                instanceList[foundSections].name = name;
                typeIndexTMD = parent->tmdFile[0]->indexIn(name); //get index in TMD list for current instance type
                typeIndexTDB = indexIn(name);
                if(typeIndexTMD == -1){
                    qDebug() << Q_FUNC_INFO << "Item " << name << " does not exist in " << parent->tmdFile[0]->filePath <<". Verify that the TMD and TDB are both correct.";
                }
                //qDebug() << Q_FUNC_INFO << "seciton name: " << name << " index " << typeIndexTMD << "TDB index" << typeIndexTDB;
                //location = findBreak.indexIn(parent->fileData, location);
                startLine = findBreak.indexIn(parent->fileData, location)+1; //get start of a line
                spaceLocation = findEndSpace.indexIn(parent->fileData, startLine)+1; //find the line type, using the space
                tempRead = parent->fileData.mid(startLine, spaceLocation-startLine).trimmed();
                location = startLine;
                fullRead = parent->fileData.mid(location, endLocation-location);
                //fullSplit = fullRead.split("\n");

                //loop to remove blank lines
                for (int i = fullSplit.length(); i>0; i--) {
                    if (fullSplit[i-1].trimmed() == "") {
                        fullSplit.remove(i-1);
                    }
                }

                for(int i = 0; i<fullSplit.length(); i++){
                    fullSplit[i] = fullSplit[i].removeIf(quoteRemover).trimmed(); //prep line by removing extra symbols and whitespace
                    //qDebug() << Q_FUNC_INFO << "line" << i << ":" << fullSplit[i];
                    itemDetails.clear();
                    if (fullSplit[i].mid(0,1) == ":"){
                        //handle definition inheritence here
                        //instances shouldn't have any, leaving this here for now
                        //qDebug() << "inherits type " << tempRead.mid(1, tempRead.length()-1);
                        continue;
                    }
                    checkDefault = fullSplit[i].mid(0, fullSplit[i].indexOf(" "));
                    tempRead = fullSplit[i].mid(fullSplit[i].indexOf(" ")+1, fullSplit[i].length() - fullSplit[i].indexOf(" "));

                    if (checkDefault == "ObjectId:") {
                        //qDebug() << Q_FUNC_INFO << "found instance index: " << tempRead.trimmed();
                        instanceList[foundSections].instanceIndex = tempRead.trimmed().toInt(nullptr);
                        continue;
                    } else if (checkDefault == "NotDefault") {
                        //switch statement here
                        //qDebug() << Q_FUNC_INFO << "Item " << i-1 << classList[typeIndexTDB].itemList[i-1].name << " found at " << itemIndexTMD << "remaining:" << tempRead;
                        itemIndexTMD = parent->tmdFile[0]->dictItemIndex(typeIndexTMD, classList[typeIndexTDB].itemList[i-1].name);
                        itemDetails = parent->tmdFile[0]->classList[typeIndexTMD].itemList[itemIndexTMD];
                        //qDebug() << Q_FUNC_INFO << "Item " << i-1 << itemList[typeIndexTDB][i-1].name << " found at " << itemIndexTMD << "type:" << itemDetails.type;
                        itemDetails.isDefault = false;
                        itemDetails = addItem(itemDetails, tempRead);
                    } else {
                        itemIndexTMD = parent->tmdFile[0]->dictItemIndex(typeIndexTMD, classList[typeIndexTDB].itemList[i-1].name);
                        //qDebug() << Q_FUNC_INFO << "Item" << i << parent->tmdFile->itemList[typeIndexTMD][itemIndexTMD].name;
                        itemDetails = parent->tmdFile[0]->classList[typeIndexTMD].itemList[itemIndexTMD];
                        itemDetails.isDefault = true;
                    }
                    //qDebug() << instanceNames[foundSections] << "item" << i << checkDefault << itemDetails.index;
                    itemDetails.index = instanceIndex;
                    instanceList[foundSections].itemList.push_back(itemDetails);
                }

                name = getName();
                nextLocation = findStartBrackets.indexIn(parent->fileData, location+1); //finds next line break after dict section name
                endLocation = findEndBrackets.indexIn(parent->fileData, location);
                qDebug() << Q_FUNC_INFO << "found section" << name << "and next location" << nextLocation << "and endsection" << endSection << "found from location" << location << "and endlocation" << endLocation;
                foundSections++;
            }
            location = endSection;
        }*/

        for(int i = 0; i<classList.size();i++){
            qDebug() << Q_FUNC_INFO << "class name" << i << "is" << classList[i].name;
            for (int j = 0; j < classList[i].itemList.size();j++) {
                qDebug() << Q_FUNC_INFO << "item" << j << "in class" << i << "is" << classList[i].itemList[j].name << "type" << classList[i].itemList[j].type << "with value" << classList[i].itemList[j].value << "and value list" << classList[i].itemList[j].valueList;
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
    /*confirmed - it works
    now all the needs to be done is read a TDB file (should be fairly similar to this) and compare the information stored there to the TMD
    for TDB files: the "includedfiles" header indicates what TMD file is associated with it. use this to verify correct info.
    after that, check the "filedictionary" section to find the relevant data for the tdb and its detault values
    then using that info, we can load the information from the "instances" section with relative ease.
    */
    return 0;
}

int TMDFile::readData(){
    QByteArrayMatcher findStartBrackets;
    QByteArrayMatcher findEndBrackets;
    QByteArrayMatcher findTilde;
    QByteArrayMatcher findBreak;
    QByteArrayMatcher findSpace;
    QByteArrayMatcher findQuote;
    QByteArrayMatcher findSectionEnd;
    findStartBrackets.setPattern(QByteArray::fromHex(QString("7B").toUtf8()));
    findEndBrackets.setPattern(QByteArray::fromHex(QString("7D").toUtf8()));
    findTilde.setPattern(QByteArray::fromHex(QString("7E").toUtf8()));
    findBreak.setPattern(QByteArray::fromHex(QString("0D").toUtf8()));
    findSpace.setPattern(QByteArray::fromHex(QString("20").toUtf8()));
    findQuote.setPattern(QByteArray::fromHex(QString("22").toUtf8()));
    static QRegularExpression quoteRemover = QRegularExpression("[\[\"\\]]");
    static QRegularExpression pathRemover = QRegularExpression("../");
    QString fullRead;
    QStringList fullSplit;
    long endLocation = 0;
    int foundSections =0;
    int typeIndexTMD = 0;
    int itemIndexTMD = 0;
    long nextLocation = 0;
    long spaceLocation = 0;
    long endSection = 0;
    int instanceIndex = 0;
    int typeIndexTDB = 0;
    long startLine = 0;
    QString tempRead;
    QString checkDefault;
    dictItem itemDetails;
    itemDetails.file = this;
    /*loop here until end of file
    find open bracket, get name
    find close bracket and next open bracket
    if next open bracket is sooner than next close bracket, we're in a subgroup. start recording objects

    ↑ not currently doing things that way, but that should be the plan.
    afind a subgroup that can be recorded, read the whole group into a QString
    Split that QString into a QStringList along /n and iterate through that
    */
    QString name;

    location = 0;
    while (location != -1){
        name = getName();
        qDebug() << Q_FUNC_INFO << "section name: " << name;
        findSectionEnd.setPattern(name.toUtf8());
        endSection = findSectionEnd.indexIn(parent->fileData, location);
        if (name == "IncludedFiles") {
            //read TMD file name, verify that we're using the right TMD
            //since some TMD files need other TMD files, we'll need to have a list of files instead of just one.
            endLocation = findEndBrackets.indexIn(parent->fileData, location);
            fullRead = parent->fileData.mid(location+1, endLocation-location-1);
            includedFile = fullRead;
            fullRead = fullRead.remove(quoteRemover).remove(pathRemover).trimmed();
            if (fullRead != "") {
                if(fullRead.toUpper() == parent->tmdFile[0]->fileName.toUpper()){
                    qDebug() << "The TDB file includes the loaded TMD file. We can continue.";
                } else {
                    qDebug() << Q_FUNC_INFO << "The file does not include the loaded TMD file. Please verify that the correct files are loaded.";
                    qDebug() << Q_FUNC_INFO << "Currently loaded TMD file:" << parent->tmdFile[0]->fileName << " | TDB file includes:" << fullRead;
                    return -1;
                }
            }
        } else if (name == "Dictionary"){
            name = getName();
            foundSections = 0;
            while(location != -1){
                //name = parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
                classList.resize(foundSections+1);
                classList[foundSections].name = name;
                //qDebug() << Q_FUNC_INFO << location << nameLocation << "seciton name: " << name;
                endLocation = findEndBrackets.indexIn(parent->fileData, location);
                tempRead = getNextLine();
                //qDebug() << Q_FUNC_INFO << "endlocation: " << endLocation;
                fullRead = parent->fileData.mid(location, endLocation-location);
                fullSplit = fullRead.split("\n");

                //loop to remove blank lines
                for (int i = fullSplit.length(); i>0; i--) {
                    if (fullSplit[i-1].trimmed() == "") {
                        fullSplit.remove(i-1);
                    }
                }

                for(int i = 0; i<fullSplit.length(); i++){
                    fullSplit[i] = fullSplit[i].remove(quoteRemover).trimmed(); //prep line by removing extra symbols and whitespace
                    //qDebug() << Q_FUNC_INFO << "line" << i << ":" << fullSplit[i];
                    itemDetails.clear();
                    if (fullSplit[i].mid(0,1) == ":"){
                        //handle definition inheritence here
                        classList[foundSections].inheritedClass = tempRead.mid(1, tempRead.length()-1);
                        typeIndexTMD = indexIn(classList[foundSections].inheritedClass);
                        //qDebug() << "inherits type " << tempRead.mid(1, tempRead.length()-1) << "at" << typeIndexTMD;
                        for (int j = 0; j < classList[typeIndexTMD].itemList.size();j++) {
                            classList[foundSections].itemList.push_back(classList[typeIndexTMD].itemList[j]);
                            //qDebug() << "item at " <<i << " by "  << j << "is" << itemList[typeIndexTMD][j].name;
                        }
                        continue;
                    }
                    itemDetails.type = fullSplit[i].mid(0, fullSplit[i].indexOf(" "));
                    tempRead = fullSplit[i].mid(fullSplit[i].indexOf(" ")+1, fullSplit[i].length() - fullSplit[i].indexOf(" "));
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

                    classList[foundSections].itemList.push_back(itemDetails);
                }

                name = getName();
                foundSections++;

            }
            location = -1;
        } else if (name == "FileDictionary") {
            //read section and affected attributes
            name = getName();
            nextLocation = findStartBrackets.indexIn(parent->fileData, location+1);
            foundSections = 0;
            endLocation = findEndBrackets.indexIn(parent->fileData, location);
            while(nextLocation != -1 and (nextLocation < endSection or endLocation < endSection)){
                classList.resize(foundSections+1);
                classList[foundSections].name = name;
                //qDebug() << Q_FUNC_INFO << location << nameLocation << "seciton name: " << name;
                tempRead = getNextLine();
                //qDebug() << Q_FUNC_INFO << "endlocation: " << endLocation;
                instanceIndex = 0;
                fullRead = parent->fileData.mid(location, endLocation-location);
                fullSplit = fullRead.split("\n");

                //loop to remove blank lines
                for (int i = fullSplit.length(); i>0; i--) {
                    if (fullSplit[i-1].trimmed() == "") {
                        fullSplit.remove(i-1);
                    }
                }

                for(int i = 0; i<fullSplit.length(); i++){
                    fullSplit[i] = fullSplit[i].remove(quoteRemover).trimmed(); //prep line by removing extra symbols and whitespace
                    //qDebug() << Q_FUNC_INFO << "line" << i << ":" << fullSplit[i];
                    itemDetails.clear();
                    if (fullSplit[i].mid(0,1) == ":"){
                        //handle definition inheritence here
                        typeIndexTDB = indexIn(fullSplit[i].mid(1, fullSplit[i].length()-1));
                        classList[foundSections].inheritedClass = fullSplit[i].mid(1, fullSplit[i].length()-1);
                        //qDebug() << "inherits type " << fullSplit[i].mid(1, fullSplit[i].length()-1) << "at" << typeIndexTDB;
                        for (int j = 0; j < classList[typeIndexTDB].itemList.size();j++) {
                            //qDebug() << "item at " << typeIndexTDB << " by "  << j << "is" << itemList[typeIndexTDB][j].name;
                            classList[foundSections].itemList.push_back(classList[typeIndexTDB].itemList[j]);
                        }
                        continue;
                    }
                    //qDebug() << Q_FUNC_INFO << fullSplit[i];
                    itemDetails.name = fullSplit[i];
                    classList[foundSections].itemList.push_back(itemDetails);
                }
                name = getName();
                nextLocation = findStartBrackets.indexIn(parent->fileData, location+1);
                endLocation = findEndBrackets.indexIn(parent->fileData, location);
                foundSections++;
            }
            location = endSection;

        } else if (name == "Instances") {
            //check that FileDictionary has already been read
            //then find NotDefault values and change them in the instance
            //read section and affected attributes
            name = getName();
            nextLocation = findStartBrackets.indexIn(parent->fileData, location+1);
            endLocation = findEndBrackets.indexIn(parent->fileData, location);
            foundSections = 0;
            while(location!= -1 and (nextLocation != -1 or endLocation !=-1) and (nextLocation < endSection or endLocation < endSection)){
                //location = nextLocation;
                instanceList.resize(foundSections + 1);
                instanceList[foundSections].name = name;
                typeIndexTMD = parent->tmdFile[0]->indexIn(name); //get index in TMD list for current instance type
                typeIndexTDB = indexIn(name);
                if(typeIndexTMD == -1){
                    qDebug() << Q_FUNC_INFO << "Item " << name << " does not exist in " << parent->tmdFile[0]->filePath <<". Verify that the TMD and TDB are both correct.";
                }
                //qDebug() << Q_FUNC_INFO << "seciton name: " << name << " index " << typeIndexTMD << "TDB index" << typeIndexTDB;
                //location = findBreak.indexIn(parent->fileData, location);
                startLine = findBreak.indexIn(parent->fileData, location)+1; //get start of a line
                spaceLocation = findSpace.indexIn(parent->fileData, startLine)+1; //find the line type, using the space
                tempRead = parent->fileData.mid(startLine, spaceLocation-startLine).trimmed();
                location = startLine;
                fullRead = parent->fileData.mid(location, endLocation-location);
                fullSplit = fullRead.split("\n");

                //loop to remove blank lines
                for (int i = fullSplit.length(); i>0; i--) {
                    if (fullSplit[i-1].trimmed() == "") {
                        fullSplit.remove(i-1);
                    }
                }

                for(int i = 0; i<fullSplit.length(); i++){
                    fullSplit[i] = fullSplit[i].remove(quoteRemover).trimmed(); //prep line by removing extra symbols and whitespace
                    //qDebug() << Q_FUNC_INFO << "line" << i << ":" << fullSplit[i];
                    itemDetails.clear();
                    if (fullSplit[i].mid(0,1) == ":"){
                        //handle definition inheritence here
                        //instances shouldn't have any, leaving this here for now
                        //qDebug() << "inherits type " << tempRead.mid(1, tempRead.length()-1);
                        continue;
                    }
                    checkDefault = fullSplit[i].mid(0, fullSplit[i].indexOf(" "));
                    tempRead = fullSplit[i].mid(fullSplit[i].indexOf(" ")+1, fullSplit[i].length() - fullSplit[i].indexOf(" "));

                    if (checkDefault == "ObjectId:") {
                        //qDebug() << Q_FUNC_INFO << "found instance index: " << tempRead.trimmed();
                        instanceList[foundSections].instanceIndex = tempRead.trimmed().toInt(nullptr);
                        continue;
                    } else if (checkDefault == "NotDefault") {
                        //switch statement here
                        //qDebug() << Q_FUNC_INFO << "Item " << i-1 << classList[typeIndexTDB].itemList[i-1].name << " found at " << itemIndexTMD << "remaining:" << tempRead;
                        itemIndexTMD = parent->tmdFile[0]->dictItemIndex(typeIndexTMD, classList[typeIndexTDB].itemList[i-1].name);
                        itemDetails = parent->tmdFile[0]->classList[typeIndexTMD].itemList[itemIndexTMD];
                        //qDebug() << Q_FUNC_INFO << "Item " << i-1 << itemList[typeIndexTDB][i-1].name << " found at " << itemIndexTMD << "type:" << itemDetails.type;
                        itemDetails.isDefault = false;
                        itemDetails = addItem(itemDetails, tempRead);
                    } else {
                        itemIndexTMD = parent->tmdFile[0]->dictItemIndex(typeIndexTMD, classList[typeIndexTDB].itemList[i-1].name);
                        //qDebug() << Q_FUNC_INFO << "Item" << i << parent->tmdFile->itemList[typeIndexTMD][itemIndexTMD].name;
                        itemDetails = parent->tmdFile[0]->classList[typeIndexTMD].itemList[itemIndexTMD];
                        itemDetails.isDefault = true;
                    }
                    //qDebug() << instanceNames[foundSections] << "item" << i << checkDefault << itemDetails.index;
                    itemDetails.index = instanceIndex;
                    instanceList[foundSections].itemList.push_back(itemDetails);
                }

                name = getName();
                nextLocation = findStartBrackets.indexIn(parent->fileData, location+1); //finds next line break after dict section name
                endLocation = findEndBrackets.indexIn(parent->fileData, location);
                qDebug() << Q_FUNC_INFO << "found section" << name << "and next location" << nextLocation << "and endsection" << endSection << "found from location" << location << "and endlocation" << endLocation;
                foundSections++;
            }
            location = endSection;
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
    /*confirmed - it works
    now all the needs to be done is read a TDB file (should be fairly similar to this) and compare the information stored there to the TMD
    for TDB files: the "includedfiles" header indicates what TMD file is associated with it. use this to verify correct info.
    after that, check the "filedictionary" section to find the relevant data for the tdb and its detault values
    then using that info, we can load the information from the "instances" section with relative ease.
    */
    return 0;
}

void TDBFile::writeData(){
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
