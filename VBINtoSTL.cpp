#include "mainwindow.h"

/*Each SceneNode and Mesh stand on equal footing - they're the major elements of the VBIN tree. 
Modifications for SceneNodes and Meshes are passed down to other nodes and meshes as the tree goes on. The exact details for this are still being worked on.
There's likely some conflicts between worldspace-oriented transformations and parent-oriented transformations but I haven't seen any indication of when to use one or the other.
*/

void SceneNode::clear(){
    fileLocation = 0;

    mods.clear();
    sectionTypes.clear();
    sectionList.clear();
    name = "";
    trueLength = 0;

    return;
}

void Modifications::clear(){
    modByte = 0;
    offset = QVector3D();
    rotation = QQuaternion();
    scale = 1;
    return;
}

void BoundingVolume::populateData(){
    long currentPosition = fileLocation + 25;
    hasVolume = file->parent->fileData.mid(currentPosition, 1).toInt(nullptr, 16);
    currentPosition += 1;
    type = file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2).toInt(nullptr, 16);
    currentPosition += 4;
    location.setX(file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2)));
    location.setY(file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition+4, 4).toHex(), 2)));
    location.setZ(file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition+8, 4).toHex(), 2)));
    currentPosition += 12;
    radius = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
    qDebug() << Q_FUNC_INFO << "location" << fileLocation << "radius" << radius << "hasvolume" << hasVolume << "type" << type;
}

void VBIN::readData(){

    //gets all scene nodes in the file, populates their PositionArrays and IndexArrays, and gets modifications
    currentLocation = 0;
    base.file = this;
    base.getSceneNodeTree(0, parent->fileData.length(), 0);

    qDebug() << Q_FUNC_INFO << "section list length" << base.sectionList.size() << "typelist length" << base.sectionTypes.size();


    Modifications baseMods;
    baseMods.clear();
    base.mods = baseMods;
    std::vector<Modifications> baseModList;
    baseModList.push_back(baseMods);

    qDebug() << Q_FUNC_INFO << "base mod values" << baseMods.offset << baseMods.rotation << baseMods.scale;

    for(int i = 0; i < base.sectionList.size(); i++){
        qDebug() << Q_FUNC_INFO << "item" << i << "list type" << base.sectionTypes[i];
        if(base.sectionTypes[i] == "Mesh"){
            qDebug() << Q_FUNC_INFO << "layer 0," << i <<"is" << std::get<Mesh>(base.sectionList[i]).name << "with rotation" << std::get<Mesh>(base.sectionList[i]).mods.rotation;
            std::get<Mesh>(base.sectionList[i]).printInfo(1);
        } else {
            qDebug() << Q_FUNC_INFO << "layer 0," << i <<"is" << std::get<SceneNode>(base.sectionList[i]).name << "with rotation" << std::get<SceneNode>(base.sectionList[i]).mods.rotation;
            std::get<SceneNode>(base.sectionList[i]).printInfo(1);
        }
    }

    base.modifyPosArrays(baseModList);


    //qDebug() << Q_FUNC_INFO << "Successfully loaded the scene node tree.";

    //modify every position set based on the modifications that were read
    //qDebug() << Q_FUNC_INFO << "Node list size (read data): " << this->nodeList.size();
//    for (int i = 0; i < int(this->nodeList.size()); i++) {
//        this->nodeList[i].modifyPosArrays();
//    }
    //this->modifyPosArrays();

    //qDebug() << Q_FUNC_INFO << "Position arrays modified.";

    return;
}

void FileSection::printInfo(int depth){
    for(int i = 0; i < sectionList.size(); i++){
        if(sectionTypes[i] == "Mesh"){
            qDebug() << Q_FUNC_INFO << "layer" << depth << ","<< i <<"is" << std::get<Mesh>(sectionList[i]).name << "with rotation" << std::get<Mesh>(sectionList[i]).mods.rotation;
            if(std::get<Mesh>(sectionList[i]).sectionList.size() > 0){
                std::get<Mesh>(sectionList[i]).printInfo(depth+1);
            }
        } else {
            qDebug() << Q_FUNC_INFO << "layer" << depth << ","<< i <<"is" << std::get<SceneNode>(sectionList[i]).name << "with rotation" << std::get<SceneNode>(sectionList[i]).mods.rotation;
            if(std::get<SceneNode>(sectionList[i]).sectionList.size() > 0){
                std::get<SceneNode>(sectionList[i]).printInfo(depth+1);
            }
        }
    }
}

void FileSection::modifyPosArrays(std::vector<Modifications> addedMods){
    QMatrix3x3 rotMatrix;
    //QMatrix4x4 rotMatrix4;
    //QVector4D expandVector;
    //stackedMods.rotation = mods.rotation * addedMods.rotation;
    //stackedMods.scale = mods.scale * addedMods.scale;
    //qDebug() << Q_FUNC_INFO << "Getting modifications for " << name << " current scale: " << mods.scale << "passed mods:" << addedMods.scale << "new scale:" << stackedMods.scale;
    addedMods.push_back(mods);

    for(int i = 0; i < sectionList.size(); i++){
        if(sectionTypes[i] == "Mesh"){
            if(std::get<Mesh>(sectionList[i]).sectionList.size() > 0){
                std::get<Mesh>(sectionList[i]).modifyPosArrays(addedMods);
            }
            for (int j = 0; j < std::get<Mesh>(sectionList[i]).posArray.vertexCount; ++j) {
                for(int k = 0; k < addedMods.size(); k++){
                    //rotMatrix = std::get<Mesh>(sectionList[i]).mods.rotation.toRotationMatrix();
                    //expandVector = QVector4D(std::get<Mesh>(sectionList[i]).posArray.positionList[i]);
                    //rotMatrix4 = QMatrix4x4(rotMatrix);

                    std::get<Mesh>(sectionList[i]).posArray.positionList[j] = std::get<Mesh>(sectionList[i]).posArray.positionList[j] * addedMods[k].scale;
                    std::get<Mesh>(sectionList[i]).posArray.positionList[j] = std::get<Mesh>(sectionList[i]).posArray.positionList[j] + addedMods[k].offset;
                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = file->parent->binChanger.forcedRotate(rotMatrix, addedMods[k].offset, std::get<Mesh>(sectionList[i]).posArray.positionList[j]);
                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = QVector3D(expandVector*rotMatrix4);
                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = mods.rotation.rotatedVector(std::get<Mesh>(sectionList[i]).posArray.positionList[j]);
                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = addedMods[k].rotation * std::get<Mesh>(sectionList[i]).posArray.positionList[j];
                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = mods.rotation * std::get<Mesh>(sectionList[i]).posArray.positionList[j];
                }
                std::get<Mesh>(sectionList[i]).posArray.positionList[j] = std::get<Mesh>(sectionList[i]).posArray.positionList[j] * std::get<Mesh>(sectionList[i]).mods.scale;
                std::get<Mesh>(sectionList[i]).posArray.positionList[j] = std::get<Mesh>(sectionList[i]).posArray.positionList[j] + std::get<Mesh>(sectionList[i]).mods.offset;
                //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = std::get<Mesh>(sectionList[i]).mods.rotation * std::get<Mesh>(sectionList[i]).posArray.positionList[j];
                //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = std::get<Mesh>(sectionList[i]).mods.rotation.rotatedVector(std::get<Mesh>(sectionList[i]).posArray.positionList[j]);
            }

        } else {
            if(std::get<SceneNode>(sectionList[i]).sectionList.size() > 0){
                std::get<SceneNode>(sectionList[i]).modifyPosArrays(addedMods);
            }
        }
    }

    return;
}

const void BoundingVolume::operator=(BoundingVolume input){
    fileLocation = input.fileLocation;
    hasVolume = input.hasVolume;
    type = input.type;
    location = input.location;
    radius = input.radius;
    file = input.file;
}

const void FileSection::operator=(FileSection input){
    file = input.file;
    fileLocation = input.fileLocation;
    boundVol = input.boundVol;
    mods = input.mods;
    sectionList = input.sectionList;
    sectionTypes = input.sectionTypes;
    name = input.name;
    trueLength = input.trueLength;
}

void FileSection::getModifications(){
    //initialize and set default values
    mods.offset = QVector3D();
    mods.rotation = QQuaternion();
    mods.scale = 1;
    //qDebug() << Q_FUNC_INFO << file->parent->fileData;

    long currentPosition = fileLocation+12; //reads after 12 bytes, length of "~SceneNode" + 2
    QByteArray readData = file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2);
    int lengthOfName = readData.toUInt(nullptr, 16);
    currentPosition += 4;
    this->name = file->parent->fileData.mid(currentPosition, lengthOfName);
    currentPosition += lengthOfName+8;
    readData = file->parent->fileData.mid(currentPosition, 1).toHex();
    mods.modByte = readData.toUInt(nullptr, 16);
    //qDebug() << Q_FUNC_INFO << "Reading at: " << currentPosition << " node: " << this->name << " with modifications: " << modifications << " from hex: " << readData;
    currentPosition += 1;
    if (!(mods.modByte & 1)) {
        //not default position, read 3 sets of 4 bytes, convert to float, and make vector
        float x_offset = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float y_offset = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float z_offset = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        mods.offset = QVector3D(x_offset, y_offset, z_offset);
        //qDebug() << Q_FUNC_INFO << this->offset;
    }
    if (!(mods.modByte & 2)) {
        //not default rotation, read 4 sets of 4 bytes, convert to float, and make quaternion
        //these could be in the wrong order, we'll see
        float x_rotation = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float y_rotation = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float z_rotation = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float scalar = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        mods.rotation = QQuaternion(scalar, x_rotation, y_rotation, z_rotation).normalized();
        //mods.rotation = QQuaternion(scalar, scalar, scalar, scalar).normalized();
    }
    if (!(mods.modByte & 4)) {
        //not default scale, read set of 4 bytes, convert to float
        mods.scale = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4; //currently unnecessary but I'm putting this here just in case the position needs to be used in this function at a later time.
    }

    //qDebug() << Q_FUNC_INFO << "Getting modifications for " << name << " rotation: " << mods.rotation  << " scale: " << mods.scale << "offset" << mods.offset;
    //qDebug() << Q_FUNC_INFO << "Getting modifications for " << name << "scale: " << mods.scale;
    //qDebug() << Q_FUNC_INFO << "Offset: " << mods.offset;
    //qDebug() << Q_FUNC_INFO << "Rotation: " << mods.rotation;

    return;
}

void SceneNode::readData(long sceneLocation){
    long nameLength = 0;
    long lengthLocation = 0;
    QByteArrayMatcher searchFile;


    nameLength = file->parent->binChanger.reverse_input(file->parent->fileData.mid(sceneLocation + file->sectionNames[0].length()+2, 4).toHex(), 2).toInt(nullptr, 16);
    name = file->parent->fileData.mid(sceneLocation + file->sectionNames[0].length()+6, nameLength);
    lengthLocation = sceneLocation + file->sectionNames[0].length() + 6 + nameLength;
    trueLength = lengthLocation + file->parent->binChanger.reverse_input(file->parent->fileData.mid(lengthLocation, 4).toHex(),2).toLong(nullptr, 16);
    //qDebug() << Q_FUNC_INFO << "scene node" << sceneNode.name << "found at" << sceneLocation + fileSections[0].length()+6 << " with length " << sceneNode.trueLength << "and ending at " << sceneNode.fileLocation + sceneNode.trueLength;
    getModifications();
    searchFile.setPattern(file->sectionNames[3].toUtf8());
    boundVol.fileLocation = searchFile.indexIn(file->parent->fileData, fileLocation);
    boundVol.file = file;
    qDebug() << Q_FUNC_INFO << "getting bounding volume for" << name;
    boundVol.populateData();
}



void FileSection::getSceneNodeTree(long searchStart, long searchEnd, int depth){
    SceneNode sceneNode;
    Mesh mesh;
    QByteArrayMatcher searchFile;
    long sceneLocation = 0;
    //long positionLocation = 0;
    long meshLocation = 0;
    //long lodLocation = 0;
    //long boundingLocation = 0;
    //long lengthLocation = 0;
    //float x_position = 0;
    //float y_position = 0;
    //float z_position = 0;
    //int nameLength = 0;
    std::vector<long> locationList;

    //qDebug() << Q_FUNC_INFO << depth << "starting search for item" << name << "with search start" << searchStart << "and current location" << searchEnd;

    while(file->currentLocation < searchEnd and file->currentLocation != -1){
        searchFile.setPattern(file->sectionNames[0].toUtf8());
        sceneLocation = searchFile.indexIn(file->parent->fileData, searchStart);
        searchFile.setPattern(file->meshStr);
        meshLocation = searchFile.indexIn(file->parent->fileData, searchStart);
        //qDebug()<<Q_FUNC_INFO << "mesh location" << meshLocation << "scene location" << sceneLocation;
        if((sceneLocation < meshLocation or meshLocation == -1) and sceneLocation != -1){
            //found a scenenode before a mesh, so read the scene node and determine what to do from there
            file->currentLocation = sceneLocation;
            sceneNode.clear();
            sceneNode.file = file;
            sceneNode.fileLocation = sceneLocation;

            sceneNode.readData(sceneLocation);

            file->currentLocation = sceneNode.boundVol.fileLocation + (file->sectionNames[3].length()*2) + 58 + 4;
            //qDebug() << Q_FUNC_INFO << depth << "node" << sceneNode.name << "current location" << file->currentLocation << "with original length" << searchEnd << "and new length" << sceneNode.trueLength;
            if(file->currentLocation < sceneNode.trueLength){
                depth++;
                sceneNode.getSceneNodeTree(file->currentLocation, sceneNode.trueLength, depth);
                searchStart = file->currentLocation;
            } else {
                searchStart = file->currentLocation;
            }
            sectionTypes.push_back("node");
            sectionList.push_back(sceneNode);
        } else if ((meshLocation < sceneLocation or sceneLocation == -1) and meshLocation != -1) {
            file->currentLocation = meshLocation;
            mesh.clear();
            mesh.file = file;
            mesh.fileLocation = meshLocation;
            mesh.readData(meshLocation);

            //qDebug() << Q_FUNC_INFO << "mesh name" << mesh.name << "at location" << mesh.fileLocation << "and bounding" << mesh.boundVol.fileLocation;
            file->currentLocation = mesh.boundVol.fileLocation + (file->sectionNames[3].length()*2) + 58 + 4;
            //qDebug() << Q_FUNC_INFO << depth << "mesh" << mesh.name << "current location" << file->currentLocation << "with original length" << searchEnd << "and new length" << mesh.trueLength;
            if(file->currentLocation < mesh.trueLength){
                depth++;
                mesh.getSceneNodeTree(file->currentLocation, mesh.trueLength, depth);
                searchStart = file->currentLocation;
            } else {
                searchStart = file->currentLocation;
            }
            sectionTypes.push_back("Mesh");
            sectionList.push_back(mesh);
        } else {
            file->currentLocation = searchEnd + 1;
        }
    }
    return;
}



void ProgWindow::convertVBINToSTL(){

    this->vbinFile->writeData();

    return;
}

void FileSection::writeData(QTextStream &fileOut){
    std::vector<int> chosenLOD;
    int index[3];
    QVector3D tempVec;
    Mesh tempMesh;

    for (int i = 0; i < int(sectionList.size()); i++) {
        //qDebug() << Q_FUNC_INFO << "section" << i << "is type:" << sectionTypes[i];
        if(sectionTypes[i] == "Mesh"){
            tempMesh = std::get<Mesh>(sectionList[i]);
            chosenLOD = tempMesh.lodInfo.targetIndecies[file->parent->ListLevels->currentText().toInt(nullptr, 10)-1];
            //qDebug() << Q_FUNC_INFO << "chosen LOD: " << chosenLOD << " for mesh " << name;
            //if(allowedMeshes.indexOf(tempMesh.name) >= 0){
            if(true){
                for (int n = chosenLOD[0]; n <= chosenLOD[1]; n++){
                    for (int k = 0; k < int(tempMesh.posArray.indexArrays[n].triangleStrips.size());k++){
                        for (int m = 0; m < int(tempMesh.posArray.indexArrays[n].triangleStrips[k].stripIndecies.size()-2); m++){
                            index[0] = tempMesh.posArray.indexArrays[n].triangleStrips[k].stripIndecies[m];
                            index[1] = tempMesh.posArray.indexArrays[n].triangleStrips[k].stripIndecies[m+1];
                            index[2] = tempMesh.posArray.indexArrays[n].triangleStrips[k].stripIndecies[m+2];
                            //qDebug() << Q_FUNC_INFO << "index set: " << index[0] << ", " << index[1] << ", " << index[2];
                            if (index[0] != index[1] and index[1] != index[2] and index[n] != index[2]) {
                                fileOut << "  facet normal 0 0 0" << Qt::endl;
                                fileOut << "    outer loop" << Qt::endl;
                                for (int n = 0; n < 3; ++n) {
                                    fileOut << "      vertex ";
                                    tempVec = tempMesh.posArray.positionList[index[n]]; //just for readability for the next line
                                    fileOut << QString::number(tempVec.x(), 'f', 3) << " " << QString::number(tempVec.y(), 'f', 3) << " " << QString::number(tempVec.z(), 'f', 3) << Qt::endl;
                                }
                                fileOut << "    endloop" << Qt::endl;
                                fileOut << "  endfacet" << Qt::endl;
                            }
                        }
                    }
                }
            }
            std::get<Mesh>(sectionList[i]).writeData(fileOut);
        } else {
            std::get<SceneNode>(sectionList[i]).writeData(fileOut);
        }
    }
}

void FileSection::writeData(QString path){
    std::vector<int> chosenLOD;
    int index[3];
    QVector3D tempVec;
    Mesh tempMesh;
    QFile stlOut(path);
    QFile fileOut(path);

    for (int i = 0; i < int(sectionList.size()); i++) {
        //() << Q_FUNC_INFO << "section" << i << "is type:" << sectionTypes[i];
        if(sectionTypes[i] == "Mesh"){
            tempMesh = std::get<Mesh>(sectionList[i]);
            //qDebug() << Q_FUNC_INFO << "mesh name is" << tempMesh.name;

            QString multiFileOut = path + "/" + tempMesh.name + ".stl";
            stlOut.setFileName(multiFileOut);
            fileOut.setFileName(multiFileOut);
            fileOut.open(QFile::WriteOnly|QFile::Truncate);
            fileOut.close();

            if (stlOut.open(QIODevice::ReadWrite)){
                QTextStream stream(&stlOut);
                stream << "solid Default" << Qt::endl;
                chosenLOD = tempMesh.lodInfo.targetIndecies[file->parent->ListLevels->currentText().toInt(nullptr, 10)-1];
                //qDebug() << Q_FUNC_INFO << "chosen LOD: " << chosenLOD << " for mesh " << name;
                for (int n = chosenLOD[0]; n <= chosenLOD[1]; n++){
                    for (int k = 0; k < int(tempMesh.posArray.indexArrays[n].triangleStrips.size());k++){
                        for (int m = 0; m < int(tempMesh.posArray.indexArrays[n].triangleStrips[k].stripIndecies.size()-2); m++){
                            index[0] = tempMesh.posArray.indexArrays[n].triangleStrips[k].stripIndecies[m];
                            index[1] = tempMesh.posArray.indexArrays[n].triangleStrips[k].stripIndecies[m+1];
                            index[2] = tempMesh.posArray.indexArrays[n].triangleStrips[k].stripIndecies[m+2];
                            //qDebug() << Q_FUNC_INFO << "index set: " << index[0] << ", " << index[1] << ", " << index[2];
                            if (index[0] != index[1] and index[1] != index[2] and index[n] != index[2]) {
                                stream << "  facet normal 0 0 0" << Qt::endl;
                                stream << "    outer loop" << Qt::endl;
                                for (int n = 0; n < 3; ++n) {
                                    stream << "      vertex ";
                                    tempVec = tempMesh.posArray.positionList[index[n]]; //just for readability for the next line
                                    stream << QString::number(tempVec.x(), 'f', 3) << " " << QString::number(tempVec.y(), 'f', 3) << " " << QString::number(tempVec.z(), 'f', 3) << Qt::endl;
                                }
                                stream << "    endloop" << Qt::endl;
                                stream << "  endfacet" << Qt::endl;
                            }
                        }
                    }
                }
                stream << "endsolid Default" << Qt::endl;
                stlOut.close();
                std::get<Mesh>(sectionList[i]).writeData(path);
            }
        } else {
            std::get<SceneNode>(sectionList[i]).writeData(path);
        }
    }
}

void VBIN::writeData(){

    std::vector<int> allowedMeshes = {1,2,3};
    std::vector<int> chosenLOD;

    if(parent->radioSingle->isChecked()){
        QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", parent->tr("Model Files (*.stl)"));
        QFile stlOut(fileOut);
        QFile file(fileOut);
        file.open(QFile::WriteOnly|QFile::Truncate);
        file.close();

        //qDebug() << Q_FUNC_INFO << Q_FUNC_INFO << "node list size (write data): " << int(this->nodeList.size());
        if (stlOut.open(QIODevice::ReadWrite)){
            QTextStream stream(&stlOut);
            stream << "solid Default" << Qt::endl;
            //qDebug() << Q_FUNC_INFO << "sections:" << base.sectionList.size();
            for (int i = 0; i < int(base.sectionList.size()); i++) {
                //qDebug() << Q_FUNC_INFO << "section" << i << "is type:" << base.sectionTypes[i];
                if(base.sectionTypes[i] == "Mesh"){
                    std::get<Mesh>(base.sectionList[i]).writeData(stream);
                } else {
                    std::get<SceneNode>(base.sectionList[i]).writeData(stream);
                }
            }
            stream << "endsolid Default" << Qt::endl;
        }
    } else {
        //qDebug() << Q_FUNC_INFO << "CAN'T DO THAT RIGHT NOW. FIX ME.";
        QString fileOut = QFileDialog::getExistingDirectory(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", QFileDialog::ShowDirsOnly);
        for (int i = 0; i < int(base.sectionList.size()); i++) {
            //qDebug() << Q_FUNC_INFO << "section" << i << "is type:" << base.sectionTypes[i];
            if(base.sectionTypes[i] == "Mesh"){
                std::get<Mesh>(base.sectionList[i]).writeData(fileOut);
            } else {
                std::get<SceneNode>(base.sectionList[i]).writeData(fileOut);
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "Output complete.";

    return;
}
