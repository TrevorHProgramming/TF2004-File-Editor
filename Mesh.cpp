#include "mainwindow.h"

void LODInfo::clear(){
    levels= 0;
    targetIndecies.clear();
    fileLocation = 0;
    file = nullptr;
}

void PositionArray::clear(){
    arrayID = 0;
    vertexCount = 0;
    fileLocation = 0;
    meshName = "";
    positionList.clear();
    indexArrays.clear();
    file = nullptr;
}

void Mesh::clear(){
    fileLocation = 0;
    posArray.clear();
    lodInfo.clear();
    offset = QVector3D();
    rotation = QQuaternion();
    scale = 1;
    file = nullptr;
}

void Mesh::getModifications(){
    //initialize and set default values
    offset = QVector3D();
    rotation = QQuaternion();
    scale = 1;

    long currentPosition = this->lodInfo.fileLocation;
    QByteArray readData = file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2);
    int lengthOfLod = readData.toUInt(nullptr, 16)+4;
    //qDebug() << Q_FUNC_INFO << "length of LOD: " << lengthOfLod << " from hex " << readData;
    currentPosition += lengthOfLod;
    readData = file->parent->fileData.mid(currentPosition, 1).toHex();
    modifications = readData.toUInt(nullptr, 16);
    //qDebug() << Q_FUNC_INFO << "Reading at: " << currentPosition << " mesh: " << this->name << " with modifications: " << modifications << " from hex: " << readData;
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

    //qDebug() << Q_FUNC_INFO << "Getting modifications for " << this->name << " rotation: " << this->rotation;
//    qDebug() << Q_FUNC_INFO << "scale: " << this->scale;
//    qDebug() << Q_FUNC_INFO << "Offset: " << this->offset;
//    qDebug() << Q_FUNC_INFO << "Rotation: " << this->rotation;


    return;
}

void LODInfo::populateLevels(){
    long currentPosition = this->fileLocation+4;
    levels = file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2).toInt(nullptr, 16);
    if (levels > file->highestLOD){
        file->highestLOD = levels;
    }
    //qDebug() << Q_FUNC_INFO << "LOD levels: " << levels << " from hex: " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2);
    currentPosition += 4;
    this->targetIndecies.resize(levels);
    for (int i = 0; i<levels; i++){
        this->targetIndecies[i].push_back(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2).toInt(nullptr, 16));
        this->targetIndecies[i].push_back(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition+4, 4).toHex(), 2).toInt(nullptr, 16));
        //third item is float, assuming distance to swap LOD levels
        currentPosition += 12;
    }

    return;
}

void IndexArray::populateTriangleStrips(){
    int indexCount = 0;
    TriangleStrip triangleStrip;
    long currentLocation = this->fileLocation+4;

    //qDebug() << Q_FUNC_INFO << "array length: " << arrayLength;

    for (int i = 0; i < this->arrayLength; i++) {
        indexCount = file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentLocation, 2).toHex(),2).toInt(nullptr, 16);
        //qDebug() << Q_FUNC_INFO << "index count at " << currentLocation << " is " << indexCount;
        i += indexCount;
        //qDebug() << Q_FUNC_INFO << "first index read is " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(i + 2, 2).toHex(),2);
        currentLocation += 2;
        for (int j = 0; j < indexCount; j++){
            triangleStrip.stripIndecies.push_back(file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentLocation, 2).toHex(),2).toInt(nullptr, 16));
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

void PositionArray::getIndexArrays(){
    QByteArrayMatcher searchFile("~BoundingVolume");
    long startSearch = this->fileLocation;
    long endSearch = searchFile.indexIn(file->parent->fileData, startSearch);
    long location = 0;
    int indexArrayIndex = 0; //not my proudest variable


    searchFile.setPattern("~IndexArray");
    location = searchFile.indexIn(file->parent->fileData, startSearch);
    while (location < endSearch & location != -1) {
        startSearch = location + 13; //increase by length of "~IndexArray" + 2
        indexArrays.resize(indexArrayIndex+1);
        indexArrays[indexArrayIndex].file=file;
        indexArrays[indexArrayIndex].arrayID = indexArrayIndex;
        indexArrays[indexArrayIndex].fileLocation = startSearch;
        indexArrays[indexArrayIndex].arrayLength = (file->parent->binChanger.reverse_input(file->parent->fileData.mid(startSearch, 4).toHex(), 2).toUInt(nullptr, 16)-4)/2; //-4 to remove itself, /2 for short length
        //qDebug() << Q_FUNC_INFO << "Array length: " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(startSearch, 4).toHex(), 2).toUInt(nullptr, 16) << " from hex: " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(startSearch, 4).toHex(), 2);

        startSearch += 4;
        indexArrays[indexArrayIndex].populateTriangleStrips();

        location = searchFile.indexIn(file->parent->fileData, startSearch);
        ++indexArrayIndex;

    }

    return;

}
