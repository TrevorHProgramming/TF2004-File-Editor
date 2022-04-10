#include "mainwindow.h"

void LODInfo::clear(){
    levels= 0;
    targetIndecies.clear();
    fileLocation = 0;
}

void PositionArray::clear(){
    arrayID = 0;
    vertexCount = 0;
    fileLocation = 0;
    meshName = "";
    positionList.clear();
    indexArrays.clear();
}

void Mesh::clear(){
    fileLocation = 0;
    posArray.clear();
    lodInfo.clear();
    offset = QVector3D();
    rotation = QQuaternion();
    scale = 1;
}

void Mesh::getModifications(ProgWindow &ProgWindow){
    //initialize and set default values
    offset = QVector3D();
    rotation = QQuaternion();
    scale = 1;

    long currentPosition = this->lodInfo.fileLocation;
    QByteArray readData = ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2);
    int lengthOfLod = readData.toUInt(nullptr, 16)+4;
    //qDebug() << Q_FUNC_INFO << "length of LOD: " << lengthOfLod << " from hex " << readData;
    currentPosition += lengthOfLod;
    readData = ProgWindow.fileData.mid(currentPosition, 1).toHex();
    modifications = readData.toUInt(nullptr, 16);
    //qDebug() << Q_FUNC_INFO << "Reading at: " << currentPosition << " mesh: " << this->name << " with modifications: " << modifications << " from hex: " << readData;
    currentPosition += 1;
    if (!(modifications & 1)) {
        //not default position, read 3 sets of 4 bytes, convert to float, and make vector
        float x_offset = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float y_offset = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float z_offset = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        offset = QVector3D(x_offset, y_offset, z_offset);
        //qDebug() << Q_FUNC_INFO << this->offset;
    }
    if (!(modifications & 2)) {
        //not default rotation, read 4 sets of 4 bytes, convert to float, and make quaternion
        //these could be in the wrong order, we'll see
        float x_rotation = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float y_rotation = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float z_rotation = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        float scalar = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4;
        rotation = QQuaternion(scalar, x_rotation, y_rotation, z_rotation).normalized();
    }
    if (!(modifications & 4)) {
        //not default scale, read set of 4 bytes, convert to float
        scale = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2));
        currentPosition += 4; //currently unnecessary but I'm putting this here just in case the position needs to be used in this function at a later time.
    }

    //qDebug() << Q_FUNC_INFO << "Getting modifications for " << this->name << " rotation: " << this->rotation;
//    qDebug() << Q_FUNC_INFO << "scale: " << this->scale;
//    qDebug() << Q_FUNC_INFO << "Offset: " << this->offset;
//    qDebug() << Q_FUNC_INFO << "Rotation: " << this->rotation;


    return;
}

void LODInfo::populateLevels(ProgWindow &ProgWindow){
    long currentPosition = this->fileLocation+4;
    this->levels = ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2).toInt(nullptr, 16);
    if (levels > ProgWindow.highestLOD){
        ProgWindow.highestLOD = levels;
    }
    //qDebug() << Q_FUNC_INFO << "LOD levels: " << levels << " from hex: " << ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2);
    currentPosition += 4;
    this->targetIndecies.resize(levels);
    for (int i = 0; i<levels; i++){
        this->targetIndecies[i].push_back(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition, 4).toHex(), 2).toInt(nullptr, 16));
        this->targetIndecies[i].push_back(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition+4, 4).toHex(), 2).toInt(nullptr, 16));
        //third item is float, assuming distance to swap LOD levels
        currentPosition += 12;
    }

    return;
}

void IndexArray::populateTriangleStrips(ProgWindow &ProgWindow){
    int indexCount = 0;
    TriangleStrip triangleStrip;
    long currentLocation = this->fileLocation+4;

    //qDebug() << Q_FUNC_INFO << "array length: " << arrayLength;

    for (int i = 0; i < this->arrayLength; i++) {
        indexCount = ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentLocation, 2).toHex(),2).toInt(nullptr, 16);
        //qDebug() << Q_FUNC_INFO << "index count at " << currentLocation << " is " << indexCount;
        i += indexCount;
        //qDebug() << Q_FUNC_INFO << "first index read is " << ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(i + 2, 2).toHex(),2);
        currentLocation += 2;
        for (int j = 0; j < indexCount; j++){
            triangleStrip.stripIndecies.push_back(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentLocation, 2).toHex(),2).toInt(nullptr, 16));
            currentLocation += 2;
        }
        //qDebug() << Q_FUNC_INFO << "triangle strip: " << triangleStrip.stripIndecies;
        this->triangleStrips.push_back(triangleStrip);
        triangleStrip.stripIndecies.clear();
    }
    for(int i = 0; i < this->triangleStrips.size(); i++){
        //qDebug() << Q_FUNC_INFO << "Triangle Strip " << i << ": " << this->triangleStrips[i].stripIndecies;
    }
}

void PositionArray::getIndexArrays(ProgWindow &ProgWindow){
    QByteArrayMatcher searchFile("~BoundingVolume");
    long startSearch = this->fileLocation;
    long endSearch = searchFile.indexIn(ProgWindow.fileData, startSearch);
    long location = 0;
    int indexArrayIndex = 0; //not my proudest variable


    searchFile.setPattern("~IndexArray");
    location = searchFile.indexIn(ProgWindow.fileData, startSearch);
    while (location < endSearch & location != -1) {
        startSearch = location + 13; //increase by length of "~IndexArray" + 2
        this->indexArrays.resize(indexArrayIndex+1);
        this->indexArrays[indexArrayIndex].arrayID = indexArrayIndex;
        this->indexArrays[indexArrayIndex].fileLocation = startSearch;
        this->indexArrays[indexArrayIndex].arrayLength = (ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(startSearch, 4).toHex(), 2).toUInt(nullptr, 16)-4)/2; //-4 to remove itself, /2 for short length
        //qDebug() << Q_FUNC_INFO << "Array length: " << ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(startSearch, 4).toHex(), 2).toUInt(nullptr, 16) << " from hex: " << ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(startSearch, 4).toHex(), 2);

        startSearch += 4;
        this->indexArrays[indexArrayIndex].populateTriangleStrips(ProgWindow);

        location = searchFile.indexIn(ProgWindow.fileData, startSearch);
        ++indexArrayIndex;

    }

    return;

}
