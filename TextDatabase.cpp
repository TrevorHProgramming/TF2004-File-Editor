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

*/

void dictItem::clear(){
    index = 0;
    type.clear();
    active = false;
    name.clear();
    intValue = 0;
    floatValue = 0;
    pointValue = QVector3D(0,0,0);
    quatValue = QQuaternion(1,0,0,0);
    boolValue = false;
    integerArray = 0;
    stringArray = 0;
    floatArray = 0;
    link.clear();
    linkArray = 0;
    colorValue.clear();
    stringValue.clear();
    enumOptions.clear();
}

void TMDFile::clear(){
    itemList.clear();
    itemNames.clear();
}

void TDBFile::readData(){
    QByteArrayMatcher findStartBrackets ;
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
    QString fullRead;
    QStringList fullSplit;
    long location = 0;
    long nextLocation = 0;
    long endSection = 0;
    long endLocation = 0;
    long nameLocation =0;
    long spaceLocation =0;
    long startLine = 0;
    long endLine = 0;
    int foundSections =0;
    QString checkDefault;
    QString tempRead;
    int typeIndexTMD = 0;
    int typeIndexTDB = 0;
    int itemIndexTMD = 0;
    int itemIndexTDB = 0;
    int instanceIndex = 0;
    dictItem itemDetails;
    /*loop here until end of file
    find open bracket, get name

    */
    QString name;
    while (location != -1){
        nameLocation = findTilde.indexIn(parent->fileData, location)+1;
        spaceLocation = findSpace.indexIn(parent->fileData, nameLocation);
        location = findStartBrackets.indexIn(parent->fileData, location+1);
        name = parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
        findSectionEnd.setPattern(parent->fileData.mid(nameLocation, spaceLocation-nameLocation));
        endSection = findSectionEnd.indexIn(parent->fileData, location);
        foundSections = 0;
        qDebug() << endSection << name;
        if (name == "IncludedFiles") {
            //read TMD file name, verify that we're using the right TMD
        } else if (name == "FileDictionary") {
            //read section and affected attributes
            nameLocation = findTilde.indexIn(parent->fileData, location)+1;
            spaceLocation = findSpace.indexIn(parent->fileData, nameLocation);
            nextLocation = findStartBrackets.indexIn(parent->fileData, location+1);
            while(nextLocation != -1 and nextLocation < endSection){
                location = nextLocation;
                name = parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
                itemNames.push_back(name);
                itemList.resize(foundSections+1);
                //qDebug() << Q_FUNC_INFO << location << nameLocation << "seciton name: " << name;
                endLocation = findEndBrackets.indexIn(parent->fileData, location);
                location = findBreak.indexIn(parent->fileData, location+1);
                startLine = findBreak.indexIn(parent->fileData, location)+1; //get start of a line
                endLine = findBreak.indexIn(parent->fileData, startLine); //get end of that line
                spaceLocation = findSpace.indexIn(parent->fileData, startLine)+1; //find the line type, using the space
                tempRead = parent->fileData.mid(startLine, spaceLocation-startLine).trimmed();
                location = startLine;
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
                        //qDebug() << "inherits type " << fullSplit[i].mid(1, fullSplit[i].length()-1) << "at" << typeIndexTDB;
                        for (int j = 0; j < itemList[typeIndexTDB].size();j++) {
                            //qDebug() << "item at " << typeIndexTDB << " by "  << j << "is" << itemList[typeIndexTDB][j].name;
                            itemList[foundSections].push_back(itemList[typeIndexTDB][j]);
                        }
                        continue;
                    }
                    //qDebug() << Q_FUNC_INFO << fullSplit[i];
                    itemDetails.name = fullSplit[i];
                    itemList[foundSections].push_back(itemDetails);
                }
                nameLocation = findTilde.indexIn(parent->fileData, location)+1; //find next dict section
                spaceLocation = findSpace.indexIn(parent->fileData, nameLocation); //each dict section name ends with a space. convenient for this method.
                nextLocation = findStartBrackets.indexIn(parent->fileData, location+1); //finds next line break after dict section name
                foundSections++;
            }

        } else if (name == "Instances") {
            //check that FileDictionary has already been read
            //then find NotDefault values and change them in the instance
            //read section and affected attributes
            nameLocation = findTilde.indexIn(parent->fileData, location)+1;
            spaceLocation = findSpace.indexIn(parent->fileData, nameLocation);
            nextLocation = findStartBrackets.indexIn(parent->fileData, location+1);
            while(nextLocation != -1 and nextLocation < endSection){
                location = nextLocation;
                name = parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
                instanceNames.push_back(name);
                instances.resize(foundSections+1);
                typeIndexTMD = parent->tmdFile->indexIn(name); //get index in TMD list for current instance type
                typeIndexTDB = indexIn(name);
                if(typeIndexTMD == -1){
                    qDebug() << Q_FUNC_INFO << "Item " << name << " does not exist in " << parent->tmdFile->filePath <<". Verify that the TMD and TDB are both correct.";
                }
                qDebug() << Q_FUNC_INFO << "seciton name: " << name << " index " << typeIndexTMD << "TDB index" << typeIndexTDB;
                endLocation = findEndBrackets.indexIn(parent->fileData, location);
                //location = findBreak.indexIn(parent->fileData, location);
                startLine = findBreak.indexIn(parent->fileData, location)+1; //get start of a line
                endLine = findBreak.indexIn(parent->fileData, startLine); //get end of that line
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
                        instanceIndex = tempRead.toInt(nullptr);
                        continue;
                    } else if (checkDefault == "NotDefault") {
                        //switch statement here
                        itemIndexTMD = parent->tmdFile->dictItemIndex(typeIndexTMD, itemList[typeIndexTDB][i-1].name);
                        //qDebug() << Q_FUNC_INFO << "Item " << i-1 << itemList[typeIndexTDB][i-1].name << " found at " << itemIndexTMD << "remaining:" << tempRead;
                        itemDetails = parent->tmdFile->itemList[typeIndexTMD][i-1];
                        //qDebug() << Q_FUNC_INFO << "Item " << i-1 << itemList[typeIndexTDB][i-1].name << " found at " << itemIndexTMD << "type:" << itemDetails.type;
                        itemDetails = addItem(itemDetails, tempRead);
                    } else {
                        itemIndexTMD = parent->tmdFile->dictItemIndex(typeIndexTMD, itemList[typeIndexTDB][i-1].name);
                        //qDebug() << Q_FUNC_INFO << "Item" << i << parent->tmdFile->itemList[typeIndexTMD][itemIndexTMD].name;
                        itemDetails = parent->tmdFile->itemList[typeIndexTMD][itemIndexTMD];
                    }
                    //qDebug() << instanceNames[foundSections] << "item" << i << checkDefault << itemDetails.index;
                    itemDetails.index = instanceIndex;
                    instances[foundSections].push_back(itemDetails);
                }
                nameLocation = findTilde.indexIn(parent->fileData, location)+1; //find next dict section
                spaceLocation = findSpace.indexIn(parent->fileData, nameLocation); //each dict section name ends with a space. convenient for this method.
                nextLocation = findStartBrackets.indexIn(parent->fileData, location+1); //finds next line break after dict section name
                foundSections++;
            }
        }
    }

    for(int i = 0; i < instances.size();i++){
        //qDebug() << Q_FUNC_INFO << "Section name: " << itemNames[i];
        for (int j = 0; j<instances[i].size();j++){
            //qDebug() << Q_FUNC_INFO << "Item name: " << instances[i][j].name << "item type: " << instances[i][j].type << "float value" << instances[i][j].floatValue;
        }
    }
}

const void dictItem::operator=(dictItem input){
    name = input.name;
    type = input.type;
    active = input.active;
    intValue = input.intValue;
    floatValue = input.floatValue;
    pointValue = input.pointValue;
    quatValue = input.quatValue;
    boolValue = input.boolValue;
    integerArray = input.integerArray;
    stringArray = input.stringArray;
    floatArray = input.floatArray;
    link = input.link;
    linkArray = input.linkArray;
    colorValue = input.colorValue;
    stringValue = input.stringValue;
    enumOptions = input.enumOptions;
}

int TMDFile::indexIn(QString searchName){
    //get index of the given dict in the TMD type list
    for (int i = 0; i < itemNames.size(); i++) {
        if (itemNames[i] == searchName){
            return i;
        }
    }
    return -1;
}

int TMDFile::dictItemIndex(int dictIndex, QString searchName){
    //get index of the given dictItem in the given TMD dict's details
    for (int i = 0; i < itemList[dictIndex].size(); i++) {
        if (itemList[dictIndex][i].name == searchName) {
            return i;
        }
    }
    return -1;
}

void TMDFile::readData(){
    QByteArrayMatcher findStartBrackets;
    QByteArrayMatcher findEndBrackets;
    QByteArrayMatcher findTilde;
    QByteArrayMatcher findBreak;
    QByteArrayMatcher findSpace;
    QByteArrayMatcher findQuote;
    findStartBrackets.setPattern(QByteArray::fromHex(QString("7B").toUtf8()));
    findEndBrackets.setPattern(QByteArray::fromHex(QString("7D").toUtf8()));
    findTilde.setPattern(QByteArray::fromHex(QString("7E").toUtf8()));
    findBreak.setPattern(QByteArray::fromHex(QString("0D").toUtf8()));
    findSpace.setPattern(QByteArray::fromHex(QString("20").toUtf8()));
    findQuote.setPattern(QByteArray::fromHex(QString("22").toUtf8()));
    static QRegularExpression quoteRemover = QRegularExpression("[\[\"\\]]");
    QString fullRead;
    QStringList fullSplit;
    long location = 0;
    long endLocation = 0;
    long nameLocation =0;
    long spaceLocation =0;
    long startLine = 0;
    long endLine = 0;
    int foundSections =0;
    int typeIndexTMD = 0;
    QString tempRead;
    dictItem itemDetails;
    /*loop here until end of file
    find open bracket, get name
    find close bracket and next open bracket
    if next open bracket is sooner than next close bracket, we're in a subgroup. start recording objects

    ↑ not currently doing things that way, but that should be the plan.
    afind a subgroup that can be recorded, read the whole group into a QString
    Split that QString into a QStringList along /n and iterate through that
    */
    QString name;
    while (location != -1){
        nameLocation = findTilde.indexIn(parent->fileData, location)+1;
        spaceLocation = findSpace.indexIn(parent->fileData, nameLocation);
        location = findStartBrackets.indexIn(parent->fileData, location+1);
        name = parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
        if (name == "Dictionary"){
            nameLocation = findTilde.indexIn(parent->fileData, location)+1;
            spaceLocation = findSpace.indexIn(parent->fileData, nameLocation);
            location = findStartBrackets.indexIn(parent->fileData, location+1);
            while(location != -1){
                name = parent->fileData.mid(nameLocation, spaceLocation-nameLocation);
                itemNames.push_back(name);
                itemList.resize(foundSections+1);
                //qDebug() << Q_FUNC_INFO << location << nameLocation << "seciton name: " << name;
                endLocation = findEndBrackets.indexIn(parent->fileData, location);
                location = findBreak.indexIn(parent->fileData, location+1);
                startLine = findBreak.indexIn(parent->fileData, location)+1; //get start of a line
                endLine = findBreak.indexIn(parent->fileData, startLine); //get end of that line
                spaceLocation = findSpace.indexIn(parent->fileData, startLine)+1; //find the line type, using the space
                tempRead = parent->fileData.mid(startLine, spaceLocation-startLine).trimmed();
                location = startLine;
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
                        typeIndexTMD = indexIn(tempRead.mid(1, tempRead.length()-1));
                        //qDebug() << "inherits type " << tempRead.mid(1, tempRead.length()-1) << "at" << typeIndexTMD;
                        for (int j = 0; j < itemList[typeIndexTMD].size();j++) {
                            itemList[foundSections].push_back(itemList[typeIndexTMD][j]);
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
                    tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
                    //qDebug() << itemDetails.type << itemDetails.active << itemDetails.name;

                    itemDetails = addItem(itemDetails, tempRead);

                    itemList[foundSections].push_back(itemDetails);
                }

                nameLocation = findTilde.indexIn(parent->fileData, location)+1; //find next dict section
                spaceLocation = findSpace.indexIn(parent->fileData, nameLocation); //each dict section name ends with a space. convenient for this method.
                location = findStartBrackets.indexIn(parent->fileData, location+1); //finds next line break after dict section name
                foundSections++;

            }
            location = -1;
        }
    }

    for(int i = 0; i < itemList.size();i++){
        //qDebug() << Q_FUNC_INFO << "Section name: " << itemNames[i];
        for (int j = 0; j<itemList[i].size();j++){
            //qDebug() << Q_FUNC_INFO << "index" << j << "Item name: " << itemList[i][j].name << "item type: " << itemList[i][j].type;
        }
    }
    /*confirmed - it works
    now all the needs to be done is read a TDB file (should be fairly similar to this) and compare the information stored there to the TMD
    for TDB files: the "includedfiles" header indicates what TMD file is associated with it. use this to verify correct info.
    after that, check the "filedictionary" section to find the relevant data for the tdb and its detault values
    then using that info, we can load the information from the "instances" section with relative ease.
    */

}

dictItem TMDFile::addItem(dictItem itemDetails, QString tempRead){
    QStringList enumItems;
    float x = 0;
    float y = 0;
    float z = 0;
    float m = 0;

    switch (itemDetails.typeList.indexOf(itemDetails.type)) {
    case 0: //enum
        itemDetails.stringValue = tempRead.mid(0, tempRead.indexOf(" "));
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        //pull the next item in quotes
        //then loop through to find all values in the enum list
        //qDebug() << Q_FUNC_INFO << "Type switch statement: enum read " << tempRead;
        enumItems = tempRead.split(",");
        for (int i = 0; i < enumItems.length(); i++) {
            itemDetails.enumOptions.push_back(enumItems[i]);
        }
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "default:"<<itemDetails.stringValue << "options:" << itemDetails.enumOptions; break;
    case 1: //Float
        itemDetails.floatValue = tempRead.toFloat(nullptr);
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.floatValue; break;
    case 2: //Point
        x = tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr);
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        y = tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr);
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        z = tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr);

        itemDetails.pointValue = QVector3D(x,y,z);
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.pointValue; break;
    case 3: //Quaternion
        x = tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr);
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        y = tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr);
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        z = tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr);
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        m = tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr);

        itemDetails.quatValue = QQuaternion(m,x,y,z);
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.quatValue; break;
    case 4: //Bool
        if(tempRead == "true"){
            itemDetails.boolValue = true;
        } else {
            itemDetails.boolValue = false;
        }
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.boolValue; break;
    case 5: //IntegerArray
        itemDetails.integerArray = tempRead.toInt(nullptr);
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.integerArray; break;
    case 6: //StringArray
        itemDetails.stringArray = tempRead.toInt(nullptr);
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.stringArray; break;
    case 7: //String
        itemDetails.stringValue = tempRead;
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.stringValue; break;
    case 8: //color
        itemDetails.colorValue.push_back(tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr));
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        itemDetails.colorValue.push_back(tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr));
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        itemDetails.colorValue.push_back(tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr));
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
        itemDetails.colorValue.push_back(tempRead.mid(0, tempRead.indexOf(" ")).toFloat(nullptr));

        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.colorValue; break;
    case 9: //integer
        itemDetails.intValue = tempRead.toInt(nullptr);
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.intValue; break;
    case 10: //float array
        itemDetails.floatArray = tempRead.toInt(nullptr);
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.floatArray; break;
    case 11: //link
        itemDetails.link = tempRead;
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.link; break;
    case 12: //link array
        itemDetails.linkArray = tempRead.toInt(nullptr);
        break;
        qDebug() << Q_FUNC_INFO << "Detail type: " << itemDetails.type << "output:"<<itemDetails.linkArray; break;
    default:
        qDebug() << Q_FUNC_INFO << "Detail type switch statement: unknown type " << itemDetails.type; break;
    }

    return itemDetails;
}
