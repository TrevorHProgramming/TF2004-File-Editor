/*I have relegated all of the switch statements to one source file because I really want them gone.*/

#include "mainwindow.h"

QMap<int, QStringList> TMDFile::bdbTypeLength = {
    {0, {"Bool"}},
    {1, {"String", "Integer", "Float"}},
    {3, {"Point"}},
    {4, {"Quaternion", "Color"}}
};

dictItem BMDFile::addItem(dictItem itemDetails){
    QList<QStringList> mapValues = bdbTypeLength.values();
    QList<int> mapItems = bdbTypeLength.keys();

    int nameLength = 0;

    if(multiTypes.contains(itemDetails.type)){
        switch(multiTypes.indexOf(itemDetails.type)){
        case 0:
            //enum
            break;
        case 1:
            //Point
            break;
        }
    } else if (singleTypes.contains(itemDetails.type)){
        switch(singleTypes.indexOf(itemDetails.type)){
        case 0:
            //enum
            break;
        case 1:
            //float
            itemDetails.value = QString::number(parent->binChanger.hex_to_float(parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(), 2)));
            break;
        case 2:
            //bool
            if(parent->fileData.mid(location, 1).toHex().toInt(nullptr, 16) == 1){
                itemDetails.value = "true";
            } else {
                itemDetails.value = "false";
            }
            location += 1;
            break;
        case 3:
            nameLength = parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toInt(nullptr, 16);
            location += 4;
            itemDetails.value = parent->fileData.mid(location, nameLength);
            location += nameLength;
            break;
        }
    } else {
        parent->messageError("Unknown type " + itemDetails.type);
    }

    for(int i = 0; i < mapValues.size();i++){
        if(mapValues[i].contains(itemDetails.type)){
            //read that many items, put into either value or valuelist
            if(mapItems[i] == 1){
                //read a single-value item
                if(itemDetails.type == "String"){

                } else {
                    itemDetails.value = QString::number(parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toInt(nullptr, 16));
                    location +=4;
                }
            } else {
                if(arrayTypes.contains(itemDetails.type)){
                    //read one item, then read that many items
                    itemDetails.value = QString::number(parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(),2).toInt(nullptr, 16));
                    location +=4;
                } else {
                    //read however many items we're supposed to read
                    for(int j = 0; j < mapItems[i]; j++){
                        itemDetails.valueList.push_back(QString::number(parent->binChanger.hex_to_float(parent->binChanger.reverse_input(parent->fileData.mid(location, 4).toHex(), 2))));
                        location +=4;
                    }
                }
            }
        }
    }

    return itemDetails;
}


QStringList TMDFile::editItem(QString className, int itemIndex){
    dictItem itemDetails;
    bool cancelled;
    int dictIndex = indexIn(className);
    QStringList changedValue;
    qDebug()<<Q_FUNC_INFO << "name: " << className << "index" << dictIndex << "item: " << itemIndex<<"value type" << classList[dictIndex].itemList[itemIndex].type;
    if (classList[dictIndex].itemList[itemIndex].type == "Enum"){
        qDebug() << Q_FUNC_INFO << "type" << classList[dictIndex].itemList[itemIndex].type << "is an enum";
        QString text = QInputDialog::getMultiLineText(parent, parent->tr("Enter New Values:"), parent->tr("Value:"), "", &cancelled);
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

QStringList TDBFile::editItem(int searchIndex, int itemIndex){
    dictItem itemDetails;
    bool cancelled;
    int instanceIndex = instanceIndexIn(searchIndex);
    QStringList changedValue;
    if(searchIndex == 0){
        //working with filedictionary section
        //QString text = QInputDialog::getText(parent, parent->tr("Enter New Value:"), parent->tr("Value:"), QLineEdit::Normal, "", &cancelled);
        parent->messageError("Please don't edit the FileDictionary section. I'm not even sure how you did that, that section shouldn't display. If you aren't trying to edit"
                             "the FileDictionary section and got this error, let Trevor know.");
    } else {
        if (instanceList[instanceIndex].itemList[itemIndex].type == "Enum"){
            qDebug() << Q_FUNC_INFO << "type" << instanceList[instanceIndex].itemList[itemIndex].type << "is an enum";
            QString text = QInputDialog::getMultiLineText(parent, parent->tr("Enter New Values:"), parent->tr("Value:"), "", &cancelled);
            QStringList textList = text.split('\n');
            qDebug() << Q_FUNC_INFO << "new value" << textList;
            instanceList[instanceIndex].itemList[itemIndex].valueList = textList;
            changedValue = textList;
            text = QInputDialog::getItem(parent, parent->tr("Enter selected value:"), parent->tr("Value:"), textList);
            instanceList[instanceIndex].itemList[itemIndex].value = text;
            changedValue.prepend(text);
            changedValue.prepend("ENUM");
        } else {
            if (itemDetails.file->multiTypes.contains(instanceList[instanceIndex].itemList[itemIndex].type)) {
                qDebug() << Q_FUNC_INFO << "type" << instanceList[instanceIndex].itemList[itemIndex].type << "is a multi-value type";
                QString text = QInputDialog::getMultiLineText(parent, parent->tr("Enter New Values:"), parent->tr("Value:"), "", &cancelled);
                QStringList textList = text.split('\n');
                qDebug() << Q_FUNC_INFO << "new value" << textList;
                instanceList[instanceIndex].itemList[itemIndex].valueList = textList;
                changedValue = textList;
            }
            if(itemDetails.file->singleTypes.contains(instanceList[instanceIndex].itemList[itemIndex].type)){
                QString text = QInputDialog::getText(parent, parent->tr("Enter New Value:"), parent->tr("Value:"), QLineEdit::Normal, "", &cancelled);
                qDebug() << Q_FUNC_INFO << "type" << instanceList[instanceIndex].itemList[itemIndex].type << "is a single-value type with new value" << text;
                instanceList[instanceIndex].itemList[itemIndex].value = text;
                changedValue.append(text);
                changedValue.append("SINGLEVALUE"); //this is so hacky but it'll work
            }
        }
    }


    return changedValue;
}

QString TMDFile::outputValue(dictItem itemDetails){
    QString tempString;
    if(itemDetails.type == "Enum"){
        tempString = "\"" + itemDetails.value + "\" \"[" + itemDetails.valueList.join(',') +"]\"";
    } else if (itemDetails.type == "String"){
        tempString = itemDetails.value + itemDetails.valueList.join(" ") + "\"\" \"\" ";
    } else {
        tempString = itemDetails.value + itemDetails.valueList.join(" ") + " \"\" ";}
    return tempString;
}

QString TDBFile::outputValue(dictItem itemDetails){
    QString tempString;
    if(itemDetails.type == "Enum"){
        tempString = QString::number(itemDetails.valueList.indexOf(itemDetails.value));
        //tempString = "\"" + itemDetails.value + "\" \"[" + itemDetails.valueList.join(',') +"]\"";
    } else if (itemDetails.file->stringTypes.contains(itemDetails.type)){
        if(itemDetails.file->multiTypes.contains(itemDetails.type)){
            tempString = itemDetails.value + " \"" + itemDetails.valueList.join("\" \"") + "\" ";
        } else {
            tempString = "\"" + itemDetails.value + "\"";
        }
    } else {
        tempString = itemDetails.value + " " + itemDetails.valueList.join(" ") + " ";
    }
    return tempString;
}


QString TMDFile::displayValue(dictItem itemDetails){
    return itemDetails.value;
}

dictItem TMDFile::addItem(dictItem itemDetails, QString tempRead){
    QStringList readListItems;

    //qDebug() << "name " << itemDetails.name << "type" << itemDetails.type << "tempread" << tempRead;
    if(itemDetails.file->multiTypes.contains(itemDetails.type)){
        if(itemDetails.type == "Enum"){
            if(fileType == "TDB"){
                itemDetails.value = itemDetails.valueList[tempRead.mid(0, tempRead.indexOf(" ")).toInt(nullptr, 10)];
            } else {
                itemDetails.value = tempRead.mid(0, tempRead.indexOf(" "));
                tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
                readListItems = tempRead.split(",");
                for (int i = 0; i < readListItems.length(); i++) {
                    itemDetails.valueList.push_back(readListItems[i]);
                }
            }
        } else {
            if(fileType == "TDB"){
                itemDetails.valueList.clear();
                itemDetails.value = tempRead.mid(0, tempRead.indexOf(" "));
                tempRead = tempRead.mid(tempRead.indexOf(" ")+1, tempRead.length() - tempRead.indexOf(" "));
            }
            readListItems = tempRead.split(" ");
            for (int i = 0; i < readListItems.length(); i++) {
                itemDetails.valueList.push_back(readListItems[i]);
            }
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


