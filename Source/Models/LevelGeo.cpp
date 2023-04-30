#include "Headers/Main/mainwindow.h"



void MeshVBIN::save(QString toType){
    if(toType == "STL"){
        outputDataSTL();
    } /*else if (toType == "DAE"){
        outputDataDAE();
    }*/
}

void MeshVBIN::load(QString fromType){
    int failedRead = 0;
    if(fromType == "MESH.VBIN"){
        failedRead = readData();
    } else {
        failedRead = 1;
    }
    if(failedRead){
        parent->messageError("There was an error reading " + fileName);
        return;
    }
    updateCenter();
}

void GeometrySet::getVerticies(){

    float u_val = 0;
    float v_val = 0;
    int loopcounter = 0;
    int endSection = 0;
    int headerLength = 0;
    bool recognizedSection = false;
    bool first6E = true;
    int geoSetSections = 0;
    int possibleSectionEnd = 0;

    version = fileData->readInt();
    unknownValue1 = fileData->readInt();
    vertexAttributeGroup = fileData->readInt();
    vertexFormat = fileData->readInt();
    unknownValue2 = fileData->readInt();
    unknownValue3 = fileData->readInt();
    unknownValue4 = fileData->readInt();
    unknownValue5 = fileData->readInt();
    unknownValue6 = fileData->readInt();
    possibleSectionEnd = (unknownValue6*16) + fileData->currentPosition;
    qDebug() << Q_FUNC_INFO << "position after first part header read" << fileData->currentPosition;
    fileData->currentPosition += 12;
    headerLength = fileData->readInt(1);
    if(headerLength & 2){
        fileData->currentPosition += 88;
    } else {
        fileData->currentPosition += 120;
    }
    unknownValue7 = fileData->readInt(2);
    unknownValue8 = fileData->readInt(2);
    unknownValue9 = fileData->readInt();
    unknownValue10 = fileData->readInt(1);
    unknownValue11 = fileData->readInt(2);

    //qDebug() << Q_FUNC_INFO << "unknown values" << unknownValue2 << unknownValue3 << unknownValue4 << unknownValue5 << unknownValue6;
    //qDebug() << Q_FUNC_INFO << "header check:" << unknownValue7 << unknownValue8 << unknownValue9 << unknownValue10 << unknownValue11;

    if(unknownValue7 != 128 || /*skip 8*/ unknownValue9 != 0 || unknownValue10 != 0 || unknownValue11 != 17793){
        qDebug() << Q_FUNC_INFO << "header check was not passed. Last read at" << fileData->currentPosition;
        return;
    }

    //while(fileData->currentPosition < headerData.sectionLocation + headerData.sectionLength){
    while(!endSection){
    //for(int section = 0; section < possibleItemCount; section++){
        //qDebug() << Q_FUNC_INFO << "current position" << fileData->currentPosition << "vs end of section" << headerData.sectionLength + headerData.sectionLocation;
        int header1 = fileData->readUInt(1);
        int header2 = fileData->readUInt(1);
        int vertexCount = fileData->readUInt(1);
        int properties = fileData->readUInt(1);
        recognizedSection = false;

//        qDebug().noquote() << Q_FUNC_INFO << "header property value" << QString::number(header1, 16) << QString::number(header2, 16)
//                 << QString::number(vertexCount, 16) << QString::number(properties, 16) << "at" << fileData->currentPosition;

        if(properties == 117){
            qDebug().noquote() << Q_FUNC_INFO << "header property value" << QString::number(header1, 16) << QString::number(header2, 16)
                     << QString::number(vertexCount, 16) << QString::number(properties, 16) << "at" << fileData->currentPosition;
        }

        //qDebug() << Q_FUNC_INFO << "vertex count for item" << loopcounter << "is" << vertexCount << ", property is" << QString::number(properties, 16) << "at" << fileData->currentPosition;

        if(properties == 104){ //0x68
            recognizedSection = true;
            for (int i = 0; i < vertexCount-1; i++){
                geoSetVerticies.push_back(fileData->read3DVector());
            }
            fileData->currentPosition += 12;
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
            //qDebug() << Q_FUNC_INFO << "6E ending" << fileData->currentPosition << "is first 6E" << first6E;
            if(first6E){
                fileData->currentPosition += 12;
                first6E = false;
            } else {
                fileData->currentPosition += 4;
            }
        }

        if (properties == 98) { //0x62
            /*This is probably the worst way to handle these sections, but I haven't been able to find ANY conssitency with the
            post-data footers. Some have 8 bytes, some have 9, and the properties are exactly the same for both.*/
            recognizedSection = true;

            //qDebug() << Q_FUNC_INFO << "check count" << std::min(80, 32 + (((vertexCount-1)/4) * 8));
            QByteArray skipRead;
            fileData->hexValue(&skipRead, std::min(80, 32 + (((vertexCount-1)/4) * 8)));
            //qDebug() << Q_FUNC_INFO << skipRead.toHex(' ');
            //fileData->currentPosition += std::min(80, 32 + (((vertexCount-1)/4) * 8));

            first6E = false;
        }

        if(properties == 117){ //0x75
            recognizedSection = true;
            for (int i = 0; i < vertexCount; i++){
//                u_val = fileData->readMiniFloat();
//                v_val = fileData->readMiniFloat();
                u_val = float(fileData->readInt(2))/2048;
                v_val = float(fileData->readInt(2))/2048;
                //qDebug() << Q_FUNC_INFO << "0x75 data" << u_val << v_val;
                geoSetTexCoords.push_back(QVector2D(u_val, v_val));
            }
            //qDebug() << Q_FUNC_INFO << "finished reading 0x75 data at" << fileData->currentPosition;

            fileData->currentPosition += 12;

            if(fileData->currentPosition >= possibleSectionEnd){
                qDebug() << Q_FUNC_INFO << "passing expected section end (" << possibleSectionEnd << ") at" << fileData->currentPosition;
            }
            fileData->currentPosition += 7;
            int checkEnd = fileData->readInt(1);
            if(checkEnd == 17){
                //qDebug() << Q_FUNC_INFO << "checking for end of section marker at" << fileData->currentPosition;
                endSection = 1;
                int loopbreaker = 0;
                checkEnd = 0;
                while(checkEnd == 0 && loopbreaker < 50){
                    checkEnd = fileData->readUInt(1);
                    //qDebug() << Q_FUNC_INFO << checkEnd << "at" << fileData->currentPosition;
                    loopbreaker++;
                }
                //qDebug() << Q_FUNC_INFO << "end of section marker found at" << fileData->currentPosition << "with" << loopbreaker << "items checked";
                fileData->currentPosition += 31;
            }
        }

        if(properties == 114){ //0x72
            recognizedSection = true;
            for(int i = 0; i < vertexCount; i++){
                qDebug() << Q_FUNC_INFO << "section 72 value:" << i << "is:" << fileData->readInt(1);
            }
            //fileData->currentPosition += vertexCount;
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
        geoSetSections++;
    }

//    int vertexRepeats = 0;
//    for(int i = 0; i < geoSetVerticies.size(); i++){
//        qDebug() << Q_FUNC_INFO << "point index" << i << "has value" << geoSetVerticies[i];
//    }
    //qDebug() << Q_FUNC_INFO << "geo set had" << geoSetSections << "sections";
    //qDebug() << Q_FUNC_INFO << "Next Geometry set.";

    //qDebug() << Q_FUNC_INFO << "successfully exited the loop at" << fileData->currentPosition;
    //fileData->currentPosition += 20;

    position1 = fileData->read3DVector();
    position2 = fileData->read3DVector();

    fileData->currentPosition += 1;


}

int MeshVBIN::readData(){
    parent->fileData.currentPosition = 4;
    int geoSetID = 0;
    int totalVerts = 0;

    SectionHeader signature;
    parent->fileData.input = true;

    //only reading the first geo set for now
    parent->fileData.signature(&signature);
    qDebug() << Q_FUNC_INFO << "starting signature:" << signature.type;
    while(signature.type == "~GeometrySet"){
        GeometrySet geoSet;
        geoSet.headerData = signature;
        geoSet.fileData = &parent->fileData;
        geoSet.getVerticies();

        geoSets.push_back(geoSet);
        //qDebug() << Q_FUNC_INFO << "geo set" << geoSetID << "has" << geoSets[geoSetID].geoSetVerticies.size() << "vertecies";
        totalVerts += geoSets[geoSetID].geoSetVerticies.size();
        geoSetID++;

        qDebug() << Q_FUNC_INFO << "starting next geo set at" << parent->fileData.currentPosition;
        parent->fileData.signature(&signature);
    }
    qDebug() << Q_FUNC_INFO << "total geo sets:" << geoSets.size();
    qDebug() << Q_FUNC_INFO << "total geo set verts:" << totalVerts;

    qDebug() << Q_FUNC_INFO << "next signature:" << signature.type;

    return 0;
}

void GeometrySet::writeDataSTL(QTextStream &fileOut){
    QVector3D tempVec;
    QVector3D triangle[3];

    for(int vertex = 0; vertex < geoSetVerticies.size()-2; vertex++){
        triangle[0] = geoSetVerticies[vertex];
        triangle[1] = geoSetVerticies[vertex+1];
        triangle[2] = geoSetVerticies[vertex+2];
        if(triangle[0] == triangle[1] or triangle[1] == triangle[2] or triangle[0] == triangle[2]){
            vertex+=1;
            continue;
        }
        fileOut << "  facet normal 0 0 0" << Qt::endl;
        fileOut << "    outer loop" << Qt::endl;
        for (int n = 0; n < 3; ++n) {
            fileOut << "      vertex ";
            tempVec = geoSetVerticies[vertex+n]; //just for readability for the next line
            fileOut << QString::number(tempVec.x(), 'f', 3) << " " << QString::number(tempVec.y(), 'f', 3) << " " << QString::number(tempVec.z(), 'f', 3) << Qt::endl;
        }
        fileOut << "    endloop" << Qt::endl;
        fileOut << "  endfacet" << Qt::endl;

    }
}

int MeshVBIN::outputDataSTL(){

    std::vector<int> allowedMeshes = {1,2,3};
    std::vector<int> chosenLOD;

    //applyKeyframe();

    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", parent->tr("Model Files (*.stl)"));
    if(fileOut.isEmpty()){
        parent->messageError("STL export cancelled.");
    }
    QFile stlOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    if(!stlOut.open(QIODevice::ReadWrite)){
        return 1;
    }
    QTextStream stream(&stlOut);
    stream << "solid Default" << Qt::endl;
    qDebug() << Q_FUNC_INFO << "sections:" << geoSets.size();
    geoSets[0].writeDataSTL(stream);
//    for(int i = 0; i < geoSets.size(); i++){
//        geoSets[i].writeDataSTL(stream);
//    }
    stream << "endsolid Default" << Qt::endl;

    parent->messageSuccess("STL file saved.");
    qDebug() << Q_FUNC_INFO << "STL output complete.";

    return 0;
}
