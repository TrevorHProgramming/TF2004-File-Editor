#include "mainwindow.h"

/*THE PLAN
The original code looped through the file to find all position arrays and the associated LODinfo sections. Now that we know that SceneNodes are
more important, this will instead loop through and look for those. The issue with this is that a scene node can contain 0 positionarrays
but have a child scenenode, so we have to check for that. Organize these scene nodes and preserve the parent-child structure, since the offsets
for each scenenode will be applied to the points in the positionarrays.
*/

void SceneNode::clear(){
    fileLocation = 0;
    meshList.clear();

    offset = QVector3D();
    rotation = QQuaternion();
    scale = 1;

    return;
}

void BoundingVolume::populateData(){
    long currentPosition = fileLocation + 25;
    this->hasVolume = file->parent->fileData.mid(currentPosition, 1).toInt(nullptr, 16);
    currentPosition += 1;
    this->type = file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2).toInt(nullptr, 16);
    currentPosition += 4;
    this->location.setX(file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2)));
    this->location.setY(file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition+4, 4).toHex(), 2)));
    this->location.setZ(file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition+8, 4).toHex(), 2)));
    currentPosition += 12;
    this->radius = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition+8, 4).toHex(), 2));

    return;
}

void VBIN::readData(){

    //gets all scene nodes in the file, populates their PositionArrays and IndexArrays, and gets modifications
    this->getSceneNodeTree(0);

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

void VBIN::modifyPosArrays(){
//    QMatrix3x3 rotMatrix;
//    QMatrix4x4 rotMatrix4 = QMatrix4x4(rotMatrix);
//    QVector4D expandVector;

    for (int i = 0; i < int(this->meshList.size()); ++i) {
//        qDebug() << Q_FUNC_INFO << "Applying modifications for Mesh " << this->meshList[i].name << ": offset: " << this->meshList[i].offset;
//        qDebug() << Q_FUNC_INFO << "Offset: " << this->meshList[i].offset;
//        qDebug() << Q_FUNC_INFO << "Rotation: " << this->meshList[i].rotation;

        //case statement to force it to use the right scene node for spider tank

        for (int j = 0; j < this->meshList[i].posArray.vertexCount; ++j) {
            //rotMatrix = meshList[i].rotation.toRotationMatrix();
            //expandVector = QVector4D(meshList[i].posArray.positionList[i]);


            meshList[i].posArray.positionList[j] = meshList[i].posArray.positionList[j] + meshList[i].offset;
            //meshList[i].posArray.positionList[i] = QVector3D(expandVector*rotMatrix4);
            meshList[i].posArray.positionList[j] = meshList[i].rotation.rotatedVector(meshList[i].posArray.positionList[j]);
            meshList[i].posArray.positionList[j] = meshList[i].posArray.positionList[j] * meshList[i].scale;

        }
    }
    return;
}


void SceneNode::getModifications(){
    //initialize and set default values
    offset = QVector3D();
    rotation = QQuaternion();
    scale = 1;

    long currentPosition = this->fileLocation+12; //reads after 12 bytes, length of "~SceneNode" + 2
    QByteArray readData = file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2);
    int lengthOfName = readData.toUInt(nullptr, 16);
    currentPosition += 4;
    this->name = file->parent->fileData.mid(currentPosition, lengthOfName);
    currentPosition += lengthOfName+8;
    readData = file->parent->fileData.mid(currentPosition, 1).toHex();
    modifications = readData.toUInt(nullptr, 16);
    //qDebug() << Q_FUNC_INFO << "Reading at: " << currentPosition << " node: " << this->name << " with modifications: " << modifications << " from hex: " << readData;
    currentPosition += 1;
    if (!(modifications & 1)) {
        //not default position, read 3 sets of 4 bytes, convert to float, and make vector
        float x_offset = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float y_offset = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float z_offset = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        offset = QVector3D(x_offset, y_offset, z_offset);
        //qDebug() << Q_FUNC_INFO << this->offset;
    }
    if (!(modifications & 2)) {
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
        rotation = QQuaternion(scalar, x_rotation, y_rotation, z_rotation).normalized();
    }
    if (!(modifications & 4)) {
        //not default scale, read set of 4 bytes, convert to float
        scale = file->parent->binChanger.hex_to_float(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4; //currently unnecessary but I'm putting this here just in case the position needs to be used in this function at a later time.
    }

        //qDebug() << Q_FUNC_INFO << "Getting modifications for " << this->name << " rotation: " << this->rotation  << " scale: " << this->scale;
    //    qDebug() << Q_FUNC_INFO << "scale: " << this->modifications.scale;
    //    qDebug() << Q_FUNC_INFO << "Offset: " << this->modifications.offset;
    //    qDebug() << Q_FUNC_INFO << "Rotation: " << this->modifications.rotation;

    return;
}

void VBIN::getSceneNodeTree(long searchStart){
    SceneNode sceneNode;
    Mesh mesh;
    QByteArray meshStr = QByteArray::fromHex("7E4D6573680001");
    QStringList fileSections = {"~SceneNode", "~PositionArray", "~LODinfo", "~BoundingVolume"};
    QByteArrayMatcher searchFile;
    long sceneLocation = 0;
    long positionLocation = 0;
    long meshLocation = 0;
    long lodLocation = 0;
    long boundingLocation = 0;
    float x_position = 0;
    float y_position = 0;
    float z_position = 0;
    int nameLength = 0;
    std::vector<long> locationList;
    searchFile.setPattern(fileSections[0].toUtf8());
    sceneLocation = searchFile.indexIn(parent->fileData, searchStart);
    //qDebug() << Q_FUNC_INFO << "Found SceneNode at: " << sceneLocation;
    while (sceneLocation != -1){
        sceneNode.clear();
        sceneNode.fileLocation = sceneLocation;
        sceneNode.getModifications();
        searchFile.setPattern(fileSections[3].toUtf8());
        sceneNode.boundVol.fileLocation = searchFile.indexIn(parent->fileData, sceneNode.fileLocation);

        this->nodeList.push_back(sceneNode);

        searchFile.setPattern(fileSections[0].toUtf8());
        sceneLocation = searchFile.indexIn(parent->fileData, sceneNode.boundVol.fileLocation);

    }

    qDebug() << Q_FUNC_INFO << " Finished loading scene arrays.";

    searchFile.setPattern(meshStr);
    meshLocation = searchFile.indexIn(parent->fileData, searchStart);
    while (meshLocation != -1) {
        mesh.clear();
        mesh.fileLocation = meshLocation;
        nameLength = parent->binChanger.reverse_input(parent->fileData.mid(meshLocation + meshStr.length(), 4).toHex(), 2).toInt(nullptr, 16);
        mesh.name = parent->fileData.mid(meshLocation + meshStr.length()+4, nameLength);

        searchFile.setPattern(fileSections[1].toUtf8());
        positionLocation = searchFile.indexIn(parent->fileData, mesh.fileLocation);
        mesh.posArray.fileLocation = positionLocation;
        mesh.posArray.getIndexArrays();
        positionLocation += fileSections[1].length()+2;
        mesh.posArray.vertexCount = (parent->binChanger.reverse_input(parent->fileData.mid(positionLocation, 4).toHex(), 2).toInt(nullptr, 16)-4)/12; //-4 to exclude itself, /12 for 1 every vertex
        positionLocation += 4;
        mesh.posArray.positionList.resize(mesh.posArray.vertexCount);
        for (int i = 0; i < mesh.posArray.vertexCount; i++) {
            x_position = parent->binChanger.hex_to_float(parent->binChanger.reverse_input(parent->fileData.mid(positionLocation + (i*12), 4).toHex(), 2));
            //qDebug() << Q_FUNC_INFO << "X float: " << x_position << " from hex: " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(positionLocation + (i*12), 4).toHex(), 2);
            y_position = parent->binChanger.hex_to_float(parent->binChanger.reverse_input(parent->fileData.mid(positionLocation+4 + (i*12), 4).toHex(), 2));
            z_position = parent->binChanger.hex_to_float(parent->binChanger.reverse_input(parent->fileData.mid(positionLocation+8 + (i*12), 4).toHex(), 2));
            mesh.posArray.positionList[i] = QVector3D(x_position, y_position, z_position);
            //qDebug() << Q_FUNC_INFO << "Position floats: " << mesh.posArray.positionList[i];
        }

        searchFile.setPattern(fileSections[2].toUtf8());
        lodLocation = searchFile.indexIn(parent->fileData, positionLocation);
        if (lodLocation != -1) {
            mesh.lodInfo.fileLocation = lodLocation + fileSections[2].length()+2;
            mesh.lodInfo.populateLevels();
        } else {
            mesh.lodInfo.fileLocation = 0;
            mesh.lodInfo.levels = 1;
            mesh.lodInfo.targetIndecies.push_back({0,0});
            mesh.lodInfo.targetIndecies.push_back({0,0});
        }
        //qDebug() << Q_FUNC_INFO << "levels for mesh " << mesh.name << ": " << mesh.lodInfo.targetIndecies;

        searchFile.setPattern(fileSections[3].toUtf8());
        boundingLocation = searchFile.indexIn(parent->fileData, positionLocation);
        mesh.boundVol.fileLocation = boundingLocation;
        mesh.boundVol.populateData();

        mesh.getModifications();

        this->meshList.push_back(mesh);

        searchFile.setPattern(meshStr);
        meshLocation = searchFile.indexIn(parent->fileData, mesh.boundVol.fileLocation);

    }

    qDebug() << Q_FUNC_INFO << " Finished loading meshes.";


    for(int i = 0; i < this->nodeList.size(); i++){
        //qDebug() << "Scene node: " << nodeList[i].name << " is index " << i << " at file location " << nodeList[i].fileLocation;

    }
    for (int j = 0; j < this->meshList.size(); j++) {
        //qDebug() << Q_FUNC_INFO << "Mesh: " << meshList[j].name << " is index " << j << " at file location " << meshList[j].fileLocation << ". Bounding location: " << meshList[j].boundVol.fileLocation;
    }

    return;
}

void ProgWindow::convertVBINToSTL(){

    this->vbinFile->writeData();

    return;
}


void VBIN::writeData(){

    int index[3];
    std::vector<int> allowedMeshes = {1,2,3};
    QVector3D tempVec;

    std::vector<int> chosenLOD;

    if(parent->radioSingle->isChecked()){
        QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", parent->tr("Model Files (*.stl)"));
        QFile stlOut(fileOut);
        QFile file(fileOut);
        file.open(QFile::WriteOnly|QFile::Truncate);
        file.close();

        qDebug() << Q_FUNC_INFO << Q_FUNC_INFO << "node list size (write data): " << int(this->nodeList.size());
        if (stlOut.open(QIODevice::ReadWrite)){
            QTextStream stream(&stlOut);
            stream << "solid Default" << Qt::endl;
            //qDebug() << Q_FUNC_INFO << "SceneNodes: " << this->nodeList.size();
//            for (int p = 0; p<int(allowedMeshes.size()); p++) {
//                int i = allowedMeshes[p];
            for (int i = 0; i < int(this->meshList.size()); i++) {
                chosenLOD = meshList[i].lodInfo.targetIndecies[parent->ListLODLevels->currentText().toInt(nullptr, 10)-1];
                //qDebug() << Q_FUNC_INFO << "chosen LOD: " << chosenLOD << " for node " << i << " and mesh " << j;
                for (int n = chosenLOD[0]; n <= chosenLOD[1]; n++){
                    for (int k = 0; k < int(meshList[i].posArray.indexArrays[n].triangleStrips.size());k++){
                        for (int m = 0; m < int(meshList[i].posArray.indexArrays[n].triangleStrips[k].stripIndecies.size()-2); m++){
                            index[0] = meshList[i].posArray.indexArrays[n].triangleStrips[k].stripIndecies[m];
                            index[1] = meshList[i].posArray.indexArrays[n].triangleStrips[k].stripIndecies[m+1];
                            index[2] = meshList[i].posArray.indexArrays[n].triangleStrips[k].stripIndecies[m+2];
                            //qDebug() << Q_FUNC_INFO << "index set: " << index[0] << ", " << index[1] << ", " << index[2];
                            if (index[0] != index[1] and index[1] != index[2] and index[n] != index[2]) {
                                stream << "  facet normal 0 0 0" << Qt::endl;
                                stream << "    outer loop" << Qt::endl;
                                for (int n = 0; n < 3; ++n) {
                                    stream << "      vertex ";
                                    tempVec = this->meshList[i].posArray.positionList[index[n]]; //just for readability for the next line
                                    stream << QString::number(tempVec.x(), 'f', 3) << " " << QString::number(tempVec.y(), 'f', 3) << " " << QString::number(tempVec.z(), 'f', 3) << Qt::endl;
                                }
                                stream << "    endloop" << Qt::endl;
                                stream << "  endfacet" << Qt::endl;
                            }
                        }
                    }
                }
            }
            stream << "endsolid Default" << Qt::endl;
        }
    } else {
        QString fileOut = QFileDialog::getExistingDirectory(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", QFileDialog::ShowDirsOnly);
        QFile stlOut(fileOut);
        QFile file(fileOut);

        for (int j = 0; j < int(this->meshList.size()); j++) {
            QString multiFileOut = fileOut + "/" + meshList[j].name + ".stl";
            stlOut.setFileName(multiFileOut);
            file.setFileName(multiFileOut);
            file.open(QFile::WriteOnly|QFile::Truncate);
            file.close();

            if (stlOut.open(QIODevice::ReadWrite)){
                QTextStream stream(&stlOut);
                stream << "solid Default" << Qt::endl;
                //qDebug() << Q_FUNC_INFO << "Position Array length: " << nodeList[i].meshList[j].posArray.vertexCount;
                //qDebug() << Q_FUNC_INFO << "Index Array length: " << nodeList[i].meshList[j].posArray.indexArrays[0].arrayLength;
                chosenLOD = meshList[j].lodInfo.targetIndecies[parent->ListLODLevels->currentText().toInt(nullptr, 10)-1];
                //qDebug() << Q_FUNC_INFO << "chosen LOD: " << chosenLOD << " for node " << i << " and mesh " << j;
                for (int n = chosenLOD[0]; n <= chosenLOD[1]; n++){
                    for (int k = 0; k < int(meshList[j].posArray.indexArrays[n].triangleStrips.size());k++){
                        for (int m = 0; m < int(meshList[j].posArray.indexArrays[n].triangleStrips[k].stripIndecies.size()-2); m++){
                            index[0] = meshList[j].posArray.indexArrays[n].triangleStrips[k].stripIndecies[m];
                            index[1] = meshList[j].posArray.indexArrays[n].triangleStrips[k].stripIndecies[m+1];
                            index[2] = meshList[j].posArray.indexArrays[n].triangleStrips[k].stripIndecies[m+2];
                            //qDebug() << Q_FUNC_INFO << "index set: " << index[0] << ", " << index[1] << ", " << index[2];
                            if (index[0] != index[1] and index[1] != index[2] and index[n] != index[2]) {
                                stream << "  facet normal 0 0 0" << Qt::endl;
                                stream << "    outer loop" << Qt::endl;
                                for (int n = 0; n < 3; ++n) {
                                    stream << "      vertex ";
                                    tempVec = this->meshList[j].posArray.positionList[index[n]]; //just for readability for the next line
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
            }
        }


    }

    qDebug() << Q_FUNC_INFO << "Output complete.";

    return;
}
