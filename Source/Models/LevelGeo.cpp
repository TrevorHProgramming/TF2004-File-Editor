#include "Headers/Main/mainwindow.h"

void GeometrySet::getVerticies(){

    float u_val = 0;
    float v_val = 0;
    int loopcounter = 0;
    bool recognizedSection = false;

    version = fileData->readInt();
    unknownValue1 = fileData->readInt();
    vertexAttributeGroup = fileData->readInt();
    vertexFormat = fileData->readInt();
    unknownValue2 = fileData->readInt();
    unknownValue3 = fileData->readInt();
    unknownValue4 = fileData->readInt();
    unknownValue5 = fileData->readInt();
    unknownValue6 = fileData->readInt();
    fileData->currentPosition += 75;
    possibleItemCount = fileData->readInt(2);
    fileData->currentPosition += 24;
    unknownValue7 = fileData->readInt();
    unknownValue8 = fileData->readInt();
    unknownValue9 = fileData->readInt(1);
    unknownValue10 = fileData->readInt(2);

    qDebug() << Q_FUNC_INFO << "header check:" << unknownValue7 << unknownValue8 << unknownValue9 << unknownValue10;
    qDebug() << Q_FUNC_INFO << "item count" << possibleItemCount;

    if(unknownValue7 != 50331776 || unknownValue8 != 0 || unknownValue9 != 0 || unknownValue10 != 17793){
        qDebug() << Q_FUNC_INFO << "header check was not passed. Last read at" << fileData->currentPosition;
        return;
    }

    //while(fileData->currentPosition < headerData.sectionLocation + headerData.sectionLength){
    for(int section = 0; section < possibleItemCount; section++){
        //qDebug() << Q_FUNC_INFO << "current position" << fileData->currentPosition << "vs end of section" << headerData.sectionLength + headerData.sectionLocation;
        int header1 = fileData->readUInt(1);
        int header2 = fileData->readUInt(1);
        int vertexCount = fileData->readUInt(1);
        int properties = fileData->readUInt(1);
        recognizedSection = false;

        qDebug().noquote() << Q_FUNC_INFO << "header property value" << QString::number(header1, 16) << QString::number(header2, 16)
                 << QString::number(vertexCount, 16) << QString::number(properties, 16) << "at" << fileData->currentPosition;

        /*if(properties == 104 or properties == 110 or properties == 98){
            qDebug().noquote() << Q_FUNC_INFO << "header property value" << QString::number(header1, 16) << QString::number(header2, 16)
                     << QString::number(vertexCount, 16) << QString::number(properties, 16) << "at" << fileData->currentPosition;
        }*/

        //qDebug() << Q_FUNC_INFO << "vertex count for item" << loopcounter << "is" << vertexCount << ", property is" << QString::number(properties, 16) << "at" << fileData->currentPosition;

        if(properties == 104){ //0x68
            recognizedSection = true;
            for (int i = 0; i < vertexCount; i++){
                geoSetVerticies.push_back(fileData->read3DVector());
            }
        }

        if(properties == 110){ //0x6e
            recognizedSection = true;
            for(int i = 0 ; i < vertexCount; i++){
                Color possibleColor;
                possibleColor.R = fileData->readInt(1);
                possibleColor.G = fileData->readInt(1);
                possibleColor.B = fileData->readInt(1);
                possibleColor.A = fileData->readInt(1);
                geoSetColors.push_back(possibleColor);
            }
            if(header2 & 3){
                fileData->currentPosition += 4;
            } else {
                fileData->currentPosition += 12;
            }
        }

        if (properties == 98) { //0x62
            /*This is probably the worst way to handle these sections, but I haven't been able to find ANY conssitency with the
            post-data footers. Some have 8 bytes, some have 9, and the properties are exactly the same for both.*/
            recognizedSection = true;
            fileData->currentPosition += vertexCount;

            bool hackLooping = true;
            while (hackLooping) {
                int dataCheck = fileData->readUInt(1);
                if(dataCheck == header1){
                    dataCheck = fileData->readUInt(1);
                    if(dataCheck == header2){
                        fileData->currentPosition -= 2;
                        hackLooping = false;
                    }
                }
            }
        }

        if(properties == 117){ //0x75
            recognizedSection = true;
            for (int i = 0; i < vertexCount; i++){
                u_val = fileData->readMiniFloat();
                v_val = fileData->readMiniFloat();
                geoSetTexCoords.push_back(QVector2D(u_val, v_val));
            }

            fileData->currentPosition += 36;
        }

        if(properties == 114){ //0x72
            recognizedSection = true;
            fileData->currentPosition += vertexCount;
            //this causes me physical pain
            if(vertexCount == 26){
                fileData->currentPosition += 2;
            } else if (vertexCount == 15) {
                fileData->currentPosition += 1;
            }
            fileData->currentPosition += 12;
        }

        if (!recognizedSection) {
            qDebug() << Q_FUNC_INFO << "unrecognized section type" << QString::number(properties, 16) << "after" << loopcounter << "loops";
            return;
        }
        loopcounter++;

    }

    qDebug() << Q_FUNC_INFO << "successfully exited the loop at" << fileData->currentPosition;
    fileData->currentPosition += 20;

    position1 = fileData->read3DVector();
    position2 = fileData->read3DVector();

    fileData->currentPosition += 1;

}

void MeshVBIN::readData(){
    parent->fileData.currentPosition = 4;

    SectionHeader signature;

    //only reading the first geo set for now
    parent->fileData.signature(&signature);
    qDebug() << Q_FUNC_INFO << "starting signature:" << signature.type;
    while(signature.type == "~GeometrySet"){
        GeometrySet geoSet;
        geoSet.headerData = signature;
        geoSet.fileData = &parent->fileData;
        geoSet.getVerticies();

        geoSets.push_back(geoSet);

        parent->fileData.signature(&signature);
    }

    qDebug() << Q_FUNC_INFO << "next signature:" << signature.type;
}

void MeshVBIN::openMeshVBINFile(){

    QString fileIn = QFileDialog::getOpenFileName(parent, parent->tr("Select VBIN"), QDir::currentPath() + "/VBIN/", parent->tr("Model Files (*.vbin)"));
    filePath = fileIn;
    parent->fileData.readFile(fileIn);

    qDebug() << Q_FUNC_INFO << "File data loaded.";
    readData();
    qDebug() << Q_FUNC_INFO << "File data read.";


}
