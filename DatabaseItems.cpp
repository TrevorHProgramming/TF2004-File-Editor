/*I have relegated all of the switch statements to one source file because I really want them gone.*/

#include "mainwindow.h"

QMap<int, QStringList> DefinitionFile::bdbTypeLength = {
    {0, {"Bool"}},
    {1, {"String", "Integer", "Float"}},
    {3, {"Point"}},
    {4, {"Quaternion", "Color"}}
};

void DefinitionFile::getFileLengths(){
    for(int i = 0; i < classList.size(); i++){
        classList[i].length = 0;
        //qDebug() << Q_FUNC_INFO << "class name:" << classList[i].name;
        classList[i].length += 4;
        for(int j = 0; j<classList[i].itemList.size();j++){
            //qDebug() << Q_FUNC_INFO << "item name:" << classList[i].itemList[j].name;
            if(!database){
                classList[i].length += classList[i].itemList[j].type.length() + 4;
                if(classList[i].itemList[j].active){
                    classList[i].length += 8; //lengthint + "True"
                } else {
                    classList[i].length += 9; //lengthint + "False"
                }
            }
            classList[i].length += classList[i].itemList[j].name.length() + 4;
            if(!database){
                classList[i].length += itemLength(classList[i].itemList[j]);
            }
        }
    }
    for(int i = 0; i < instanceList.size(); i++){
        //repeat for instances
        instanceList[i].length = 0;
        //qDebug() << Q_FUNC_INFO << "class name:" << instanceList[i].name;
        //instanceList[i].length += instanceList[i].name.length()+2;
        instanceList[i].length += 6; //section length, int, plus instance ID, short
        for(int j = 0; j < instanceList[i].itemList.size(); j++){
            instanceList[i].length += instanceLength(instanceList[i].itemList[j]);
        }
    }
}

int DefinitionFile::itemLength(dictItem itemDetails){
    itemDetails.length = 0;
    if(multiTypes.contains(itemDetails.type)){
        switch(multiTypes.indexOf(itemDetails.type)){
        case 0:{
            //enum. stored as a single int in bdb files.
            itemDetails.length += 4;
            break;}
        case 1:{
            //Point. stored as 3 floats
            itemDetails.length += 12;
            break;}
        case 2:{
            //Quaternion. stored as 4 floats
            itemDetails.length += 16;
            break;}
        case 7:{
            //Link Array. stored as list of shorts + int indicator of list count
            itemDetails.length += 4 + (itemDetails.valueList.size()*2);
            break;}
        case 8:{
            //Vector Array. stored as list of floats + int indicator of list count
            qDebug() << Q_FUNC_INFO << "item name" << itemDetails.name << "list length" << itemDetails.valueList.size();
            itemDetails.length += 4 + (itemDetails.valueList.size()*12);
            break;}
        default:{
            parent->messageError("Unknown type " + itemDetails.type);
            break;}
        }
    } else if (singleTypes.contains(itemDetails.type)){
        switch(singleTypes.indexOf(itemDetails.type)){
        case 0:{
            //enum
            break;}
        case 1:{
            //float
            itemDetails.length += 4;
            break;}
        case 2:{
            //bool
            itemDetails.length += 1;
            break;}
        case 3:{
            //string
            itemDetails.length += itemDetails.value.length()+4;
            break;}
        case 4:{
            //integer
            itemDetails.length += 4;
            break;}
        case 5:{
            //Link, linked item IDs, stored as short
            itemDetails.length += 2;
            break;}
        case 6:{
            //Flag. seems like it's just an int
            itemDetails.length += 4;
            break;}
        default:{
            parent->messageError("Unknown type " + itemDetails.type);
            break;}
        }
    } else {
        parent->messageError("Unknown type " + itemDetails.type);
    }

    return itemDetails.length;
}

int DefinitionFile::instanceLength(dictItem itemDetails){
    int instanceLength = 1; //default notdefault bit

    if(!itemDetails.isDefault){
        instanceLength += itemLength(itemDetails);
    }

    return instanceLength;
}

dictItem DefinitionFile::addItem(dictItem itemDetails, FileData *tempRead){
    QList<QStringList> mapValues = bdbTypeLength.values();
    QList<int> mapItems = bdbTypeLength.keys();

    int nameLength = 0;
    //qDebug() << Q_FUNC_INFO << "type" << itemDetails.type << "name" << itemDetails.name << tempRead->currentPosition;

    if(multiTypes.contains(itemDetails.type)){
        switch(multiTypes.indexOf(itemDetails.type)){
        case 0:{
            //enum, not present in existing bmd files, but present in BDB files
            //itemDetails.value = QString::number(tempRead->readInt());
            int enumIndex = tempRead->readUInt();
            itemDetails.value = itemDetails.valueList[enumIndex];
            //would be more fitting in "singlevalues" but this works too.
            break;}
        case 1:{
            //Point
            itemDetails.valueList.clear();
            itemDetails.valueList.push_back(QString::number(tempRead->readFloat()));
            itemDetails.valueList.push_back(QString::number(tempRead->readFloat()));
            itemDetails.valueList.push_back(QString::number(tempRead->readFloat()));
            break;}
        case 2:{
            //Quaternion
            itemDetails.valueList.clear();
            //qDebug() << Q_FUNC_INFO << "reading" << itemDetails.name << "index" << itemDetails.index << "quaternion at location" << tempRead->currentPosition;
            itemDetails.valueList.push_back(QString::number(tempRead->readFloat()));
            itemDetails.valueList.push_back(QString::number(tempRead->readFloat()));
            itemDetails.valueList.push_back(QString::number(tempRead->readFloat()));
            itemDetails.valueList.push_back(QString::number(tempRead->readFloat()));
            //qDebug() << Q_FUNC_INFO << "after reading" << tempRead->currentPosition << "with value" << itemDetails.valueList;
            break;}
        case 7:{
            //LinkArray
            itemDetails.valueList.clear();
            int arrayItems = tempRead->readUInt();
            //qDebug() << Q_FUNC_INFO << "vector items" << vectorItems;
            for (int i = 0; i<arrayItems; i++) {
                //qDebug() << Q_FUNC_INFO << "current position" << tempRead->currentPosition;
                itemDetails.valueList.push_back(QString::number(tempRead->readInt(2)));
            }
            break;}
        case 8:{
            //VectorArray
            itemDetails.valueList.clear();
            int arrayItems = tempRead->readUInt();
            //qDebug() << Q_FUNC_INFO << "vector items" << vectorItems;
            for (int i = 0; i<arrayItems; i++) {
                //qDebug() << Q_FUNC_INFO << "current position" << tempRead->currentPosition;
                QString item1 = QString::number(tempRead->readFloat());
                QString item2 = QString::number(tempRead->readFloat());
                QString item3 = QString::number(tempRead->readFloat());
                itemDetails.valueList.push_back(item1 + ", " + item2 + ", " + item3 + "; ");
            }
            break;}
        default:{
            parent->messageError("Unknown type " + itemDetails.type);
            break;}
        }
    } else if (singleTypes.contains(itemDetails.type)){
        switch(singleTypes.indexOf(itemDetails.type)){
        case 0:{
            //enum
            break;}
        case 1:{
            //float
            itemDetails.value = QString::number(tempRead->readFloat());
            break;}
        case 2:{
            //bool
            if(tempRead->readUInt(1) == 1){
                itemDetails.value = "true";
            } else {
                itemDetails.value = "false";
            }
            break;}
        case 3:{
            //string
            nameLength = tempRead->readUInt();
            //itemDetails.value = tempRead.readHex(nameLength);
            itemDetails.value = QString(tempRead->readHex(nameLength));
            //qDebug() << Q_FUNC_INFO << "value read as:" << itemDetails.value << "with current position" << tempRead->currentPosition;
            break;}
        case 4:{
            //integer
            itemDetails.value = QString::number(tempRead->readInt());
            break;}
        case 5:{
            //Link
            itemDetails.value = QString::number(tempRead->readUInt(2));
            break;}
        case 6:{
            //Flag
            itemDetails.value = QString::number(tempRead->readUInt());
            break;}
        default:{
            parent->messageError("Unknown type " + itemDetails.type);
            break;}
        }
    } else {
        parent->messageError("Unknown type " + itemDetails.type);
    }

    //swap back to this at some point
    /*for(int i = 0; i < mapValues.size();i++){
        if(mapValues[i].contains(itemDetails.type)){
            //read that many items, put into either value or valuelist
            if(mapItems[i] == 1){
                //read a single-value item
                if(itemDetails.type == "String"){

                } else {
                    itemDetails.value = QString::number(parent->fileData.readInt());
                }
            } else {
                if(arrayTypes.contains(itemDetails.type)){
                    //read one item, then read that many items
                    itemDetails.value = QString::number(parent->fileData.readInt());
                } else {
                    //read however many items we're supposed to read
                    for(int j = 0; j < mapItems[i]; j++){
                        itemDetails.valueList.push_back(QString::number(parent->fileData.readFloat()));
                    }
                }
            }
        }
    }*/

    return itemDetails;
}

void DefinitionFile::removeItem(QString className, int itemIndex){
    int dictIndex = indexIn(className);
    classList[dictIndex].itemList.erase(classList[dictIndex].itemList.begin() + itemIndex);
}

void DatabaseFile::removeItem(int instanceIndex, int itemIndex){
    instanceList[instanceIndex].itemList.erase(instanceList[instanceIndex].itemList.begin() + itemIndex);
}

void DefinitionFile::removeClass(int classIndex){
    classList.erase(classList.begin() + classIndex);
}

void DatabaseFile::removeInstance(int instanceIndex){
    instanceList.erase(instanceList.begin() + instanceIndex);
}


QStringList DefinitionFile::editItem(QString className, int itemIndex){
    dictItem itemDetails;
    bool cancelled;
    int dictIndex = indexIn(className);
    QStringList changedValue;
    qDebug()<<Q_FUNC_INFO << "name: " << className << "index" << dictIndex << "item: " << itemIndex<<"value type" << classList[dictIndex].itemList[itemIndex].type;
    if (classList[dictIndex].itemList[itemIndex].type == "Enum"){
        qDebug() << Q_FUNC_INFO << "item" << classList[dictIndex].itemList[itemIndex].name << "is an enum";
        QString text = classList[dictIndex].itemList[itemIndex].valueList.join("\n");
        text = QInputDialog::getMultiLineText(parent, parent->tr("Enter New Values:"), parent->tr("Value:"), text, &cancelled);
        QStringList textList = text.split('\n');
        qDebug() << Q_FUNC_INFO << "new value" << textList;
        classList[dictIndex].itemList[itemIndex].valueList = textList;
        changedValue = textList;
        text = QInputDialog::getItem(parent, parent->tr("Enter selected value:"), parent->tr("Value:"), textList);
        classList[dictIndex].itemList[itemIndex].value = QString::number(textList.indexOf(text));
        changedValue.prepend(text);
        changedValue.prepend("ENUM");
    } else {
        if (itemDetails.file->multiTypes.contains(classList[dictIndex].itemList[itemIndex].type)) {
            qDebug() << Q_FUNC_INFO << "type" << classList[dictIndex].itemList[itemIndex].type << "is a multi-value type";
            QString text = QInputDialog::getMultiLineText(parent, parent->tr("Enter New Values:"), parent->tr("Value:"), "", &cancelled);
            QStringList textList = text.split('\n');
            qDebug() << Q_FUNC_INFO << "new value" << textList;
            classList[dictIndex].itemList[itemIndex].valueList = textList;

            changedValue = textList;
        }
        if(itemDetails.file->singleTypes.contains(classList[dictIndex].itemList[itemIndex].type)){
            QString text = QInputDialog::getText(parent, parent->tr("Enter New Value:"), parent->tr("Value:"), QLineEdit::Normal, "", &cancelled);
            qDebug() << Q_FUNC_INFO << "type" << classList[dictIndex].itemList[itemIndex].type << "is a single-value type with new value" << text;
            classList[dictIndex].itemList[itemIndex].value = text;
            changedValue.append(text);
            changedValue.append("SINGLEVALUE"); //this is so hacky but it'll work
        }
    }

    return changedValue;
}

QStringList DatabaseFile::editItem(int searchIndex, int itemIndex){
    dictItem itemDetails;
    bool cancelled;
    bool setDefault;
    int instanceIndex = instanceIndexIn(searchIndex);
    QStringList changedValue;
    if(searchIndex == 0){
        //working with filedictionary section
        //QString text = QInputDialog::getText(parent, parent->tr("Enter New Value:"), parent->tr("Value:"), QLineEdit::Normal, "", &cancelled);
        parent->messageError("Please don't edit the FileDictionary section. I'm not even sure how you did that, that section shouldn't display. If you aren't trying to edit"
                             "the FileDictionary section and got this error, let Trevor know.");
    } else {
        QMessageBox msgBox;
        msgBox.setText("Set item to default value?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int choice = msgBox.exec();
        qDebug() << Q_FUNC_INFO << choice;
        switch(choice){
        case 16384: //yes
            qDebug() << Q_FUNC_INFO << "setting item to default value";
            itemDetails.isDefault = true;
            changedValue.push_back("SETDEFAULT");
            break;
        case 65536: //no
            qDebug() << Q_FUNC_INFO << "prompting for new value";
            if (instanceList[instanceIndex].itemList[itemIndex].type == "Enum"){
                qDebug() << Q_FUNC_INFO << "type" << instanceList[instanceIndex].itemList[itemIndex].type << "is an enum";
                QString text = instanceList[instanceIndex].itemList[itemIndex].valueList.join("\n");
                text = QInputDialog::getMultiLineText(parent, parent->tr("Enter New Values:"), parent->tr("Value:"), text, &cancelled);
                QStringList textList = text.split('\n');
                qDebug() << Q_FUNC_INFO << "new value" << textList;
                instanceList[instanceIndex].itemList[itemIndex].valueList = textList;
                changedValue = textList;
                text = QInputDialog::getItem(parent, parent->tr("Enter selected value:"), parent->tr("Value:"), textList);
                instanceList[instanceIndex].itemList[itemIndex].value = text;
                changedValue.prepend(text);
                changedValue.prepend("ENUM");
            } else {
                if (multiTypes.contains(instanceList[instanceIndex].itemList[itemIndex].type)) {
                    qDebug() << Q_FUNC_INFO << "type" << instanceList[instanceIndex].itemList[itemIndex].type << "is a multi-value type";
                    QString text = QInputDialog::getMultiLineText(parent, parent->tr("Enter New Values:"), parent->tr("Value:"), "", &cancelled);
                    QStringList textList = text.split('\n');
                    qDebug() << Q_FUNC_INFO << "new value" << textList;
                    instanceList[instanceIndex].itemList[itemIndex].valueList = textList;
                    changedValue = textList;
                }
                if(singleTypes.contains(instanceList[instanceIndex].itemList[itemIndex].type)){
                    QString text = QInputDialog::getText(parent, parent->tr("Enter New Value:"), parent->tr("Value:"), QLineEdit::Normal, "", &cancelled);
                    qDebug() << Q_FUNC_INFO << "type" << instanceList[instanceIndex].itemList[itemIndex].type << "is a single-value type with new value" << text;
                    instanceList[instanceIndex].itemList[itemIndex].value = text;
                    changedValue.append(text);
                    changedValue.prepend("SINGLEVALUE"); //this is so hacky but it'll work
                }
            }
            itemDetails.isDefault = false;
            break;
        }

    }


    return changedValue;
}

void DefinitionFile::binaryOutput(QFile &file, dictItem itemDetails){

    if(multiTypes.contains(itemDetails.type)){
        switch(multiTypes.indexOf(itemDetails.type)){
        case 0:{
            //enum. stored as a single int in bdb files.
            qDebug() << Q_FUNC_INFO << "enum value" << itemDetails.value << "index" << itemDetails.valueList.indexOf(itemDetails.value);
            parent->binChanger.intWrite(file, itemDetails.valueList.indexOf(itemDetails.value));
            break;}
        case 1:{
            //Point. stored as 3 floats
            for(int i = 0; i < itemDetails.valueList.size(); i++){
                //qDebug() << Q_FUNC_INFO << "writing point float" << itemDetails.valueList[i];
                file.write(parent->binChanger.float_to_hex(itemDetails.valueList[i].toFloat(nullptr)));
            }
            break;}
        case 2:{
            //Quaternion. stored as 4 floats
            for(int i = 0; i < itemDetails.valueList.size(); i++){
                file.write(parent->binChanger.float_to_hex(itemDetails.valueList[i].toFloat(nullptr)));
            }
            break;}
        case 7:{
            //Link Array. stored as list of shorts + int indicator of list count
            parent->binChanger.intWrite(file, itemDetails.valueList.size());
            for(int i = 0; i < itemDetails.valueList.size(); i++){
                parent->binChanger.shortWrite(file, itemDetails.valueList[i].toShort(nullptr));
            }
            break;}
        case 8:{
            //Vector Array. stored as list of floats + int indicator of list count
            parent->binChanger.intWrite(file, itemDetails.valueList.size());
            QStringList tempList;
            for(int i = 0; i < itemDetails.valueList.size(); i++){
                itemDetails.valueList[i].remove(";");
                tempList = itemDetails.valueList[i].split(",");
                for(int j = 0; j < tempList.size(); j++){
                    tempList[j] = tempList[j].trimmed();
                    //qDebug() << Q_FUNC_INFO << "writing float" << tempList[j];
                    file.write(parent->binChanger.float_to_hex(tempList[j].toFloat(nullptr)));
                }
            }
            break;}
        default:{
            parent->messageError("Unknown type " + itemDetails.type);
            break;}
        }
    } else if (singleTypes.contains(itemDetails.type)){
        switch(singleTypes.indexOf(itemDetails.type)){
        case 0:{
            //enum
            break;}
        case 1:{
            //float
            file.write(parent->binChanger.float_to_hex(itemDetails.value.toFloat(nullptr)));
            break;}
        case 2:{
            //bool
            if(itemDetails.value == "true" or itemDetails.value == "True"){
                parent->binChanger.byteWrite(file, 1);
            } else {
                parent->binChanger.byteWrite(file, 0);
            }
            break;}
        case 3:{
            //string
            parent->binChanger.intWrite(file, itemDetails.value.length());
            file.write(itemDetails.value.toUtf8());
            break;}
        case 4:{
            //integer
            parent->binChanger.intWrite(file, itemDetails.value.toInt(nullptr));
            break;}
        case 5:{
            //Link, linked item IDs, stored as short
            parent->binChanger.shortWrite(file, itemDetails.value.toShort(nullptr));
            break;}
        case 6:{
            //Flag. seems like it's just an int
            parent->binChanger.intWrite(file, itemDetails.value.toInt(nullptr));
            break;}
        default:{
            parent->messageError("Unknown type " + itemDetails.type);
            break;}
        }
    } else {
        parent->messageError("Unknown type " + itemDetails.type);
    }
}

QString DefinitionFile::outputValue(dictItem itemDetails){
    QString tempString;
    QString valueSpace; //hacky fix for double-spaces on null values, EX Link type
    if(itemDetails.value != "" or itemDetails.valueList.size() != 0){
        valueSpace = " ";
    }
    if(itemDetails.type == "Enum"){
        tempString = "\"" + itemDetails.value + "\" \"[" + itemDetails.valueList.join(',') +"]\" ";
    } else if (itemDetails.type == "String"){
        tempString = "\"" + itemDetails.value + itemDetails.valueList.join(" ") + "\" \"" + itemDetails.comment + "\" ";
    } else {
        tempString = itemDetails.value + itemDetails.valueList.join(" ") + valueSpace + "\"" + itemDetails.comment + "\" ";}
    return tempString;
}

QString DatabaseFile::outputValue(dictItem itemDetails){
    QString tempString;
    QString valueSpace;
    if(itemDetails.value != "" and !itemDetails.valueList.isEmpty()){
        valueSpace = " ";
    } else if (itemDetails.type == "LinkArray" and !itemDetails.valueList.isEmpty()){
        valueSpace = " ";
    }
    if(itemDetails.type == "Enum"){
        tempString = QString::number(itemDetails.valueList.indexOf(itemDetails.value)) + " ";
        //tempString = "\"" + itemDetails.value + "\" \"[" + itemDetails.valueList.join(',') +"]\"";
    } else if (itemDetails.type == "LinkArray") {
        tempString = QString::number(itemDetails.valueList.size()) + valueSpace + itemDetails.valueList.join(" ") + " ";
    } else if (stringTypes.contains(itemDetails.type)){
        if(multiTypes.contains(itemDetails.type)){
            tempString = itemDetails.value + valueSpace + "\"" + itemDetails.valueList.join("\" \"") + "\" ";
        } else {
            tempString = "\"" + itemDetails.value + "\" ";
        }
    } else {
        tempString = itemDetails.value + valueSpace + itemDetails.valueList.join(" ") + " ";
    }
    return tempString;
}


QString DefinitionFile::displayValue(dictItem itemDetails){
    return itemDetails.value;
}

dictItem DatabaseFile::addItem(dictItem itemDetails, QString tempRead){
    QStringList readListItems;

    //qDebug() << "name " << itemDetails.name << "type" << itemDetails.type << "tempread" << tempRead;
    if(multiTypes.contains(itemDetails.type)){
        if(itemDetails.type == "Enum"){
            itemDetails.value = itemDetails.valueList[tempRead.mid(0, tempRead.indexOf(" ")).toInt(nullptr, 10)];
        } else if (arrayTypes.contains(itemDetails.type)){
            itemDetails.valueList.clear();
            itemDetails.value = tempRead.mid(0, tempRead.indexOf(" "));
            if(itemDetails.value.toInt(nullptr) == 0){
                tempRead = "";
            } else {
                tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
                readListItems = tempRead.split(" ");
                for (int i = 0; i < readListItems.length(); i++) {
                    itemDetails.valueList.push_back(readListItems[i]);
                }
            }
        } else {
            itemDetails.valueList.clear();
            readListItems = tempRead.split(" ");
            for (int i = 0; i < readListItems.length(); i++) {
                itemDetails.valueList.push_back(readListItems[i]);
            }
        }
    } else if(singleTypes.contains(itemDetails.type)){
        itemDetails.value = tempRead;
        //itemDetails.value = tempRead.mid(0, tempRead.indexOf(" "));
        //qDebug() << Q_FUNC_INFO << "single type fed value" << tempRead << "and saved" << itemDetails.value;
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
    }
    itemDetails.value = itemDetails.value.trimmed();

    //qDebug() << Q_FUNC_INFO << "value: " << itemDetails.value << "valueList " << itemDetails.valueList;
    return itemDetails;
}

dictItem DefinitionFile::addItem(dictItem itemDetails, QString tempRead){
    QStringList readListItems;

    //qDebug() << Q_FUNC_INFO << "name " << itemDetails.name << "type" << itemDetails.type << "tempread" << tempRead;
    if(itemDetails.file->multiTypes.contains(itemDetails.type)){
        if(itemDetails.type == "Enum"){
            int spaceLocation = tempRead.indexOf(" ");
            itemDetails.value = tempRead.mid(1, spaceLocation-1);
            tempRead = tempRead.mid(spaceLocation+3, tempRead.length() - spaceLocation-5);
            readListItems = tempRead.split(",");
        } else {
            itemDetails.value = tempRead.mid(tempRead.indexOf("\"")-1, tempRead.length() - tempRead.indexOf("\""));
            readListItems = tempRead.mid(0, tempRead.indexOf("\"")-1).split(" ");
        }
        for (int i = 0; i < readListItems.length(); i++) {
            itemDetails.valueList.push_back(readListItems[i]);
        }
    } else if(itemDetails.file->singleTypes.contains(itemDetails.type)){
        itemDetails.value = tempRead;
        //itemDetails.value = tempRead.mid(0, tempRead.indexOf(" "));
        //qDebug() << Q_FUNC_INFO << "single type fed value" << tempRead << "and saved" << itemDetails.value;
        tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
    }
    //qDebug() << Q_FUNC_INFO << "value: " << itemDetails.value << "valueList " << itemDetails.valueList;
    return itemDetails;
}


