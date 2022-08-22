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
    mods.clear();
    sectionList.clear();
    sectionTypes.clear();
    file = nullptr;
}

const void Mesh::operator=(Mesh input){
    name = input.name;
    fileLocation = input.fileLocation;
    posArray = input.posArray;
    mods = input.mods;
    file = input.file;
    sectionList = input.sectionList;
    sectionTypes = input.sectionTypes;
    lodInfo = input.lodInfo;
    lodInfo.file = input.lodInfo.file;
    lodInfo.fileLocation = input.lodInfo.fileLocation;
    lodInfo.levels = input.lodInfo.levels;
    lodInfo.targetIndecies = input.lodInfo.targetIndecies;
}

const void PositionArray::operator=(PositionArray input){
    arrayID = input.arrayID;
    indexArrays = input.indexArrays;
    file = input.file;
    meshName = input.meshName;
    positionList = input.positionList;
    vertexCount = input.vertexCount;
    fileLocation = input.fileLocation;
}

void Mesh::readData(long meshLocation){
    int nameLength = 0;
    long lengthLocation =0;
    long positionLocation = 0;
    long lodLocation = 0;
    long boundingLocation = 0;
    QByteArrayMatcher searchFile;
    float x_position = 0;
    float y_position = 0;
    float z_position = 0;

    file->parent->fileData.currentPosition = meshLocation + file->meshStr.length();

    nameLength = file->parent->fileData.readInt();
    name = file->parent->fileData.readHex(nameLength);
    //qDebug() << Q_FUNC_INFO << "mesh name" << name;
    lengthLocation = meshLocation+file->meshStr.length() + 4 + nameLength;
    file->parent->fileData.currentPosition = meshLocation+file->meshStr.length() + 4 + nameLength;
    trueLength = lengthLocation + file->parent->fileData.readLong();

    searchFile.setPattern(file->sectionNames[1].toUtf8());
    positionLocation = searchFile.indexIn(file->parent->fileData.dataBytes, fileLocation);
    posArray.file = file;
    posArray.fileLocation = positionLocation;
    file->parent->fileData.currentPosition = positionLocation;
    //qDebug() << Q_FUNC_INFO << "Getting index arrays";
    posArray.getIndexArrays();
    file->parent->fileData.currentPosition = positionLocation + file->sectionNames[1].length()+2;
    posArray.vertexCount = (file->parent->fileData.readInt()-4)/12; //-4 to exclude itself, /12 for 1 every vertex
    posArray.positionList.resize(posArray.vertexCount);
    //qDebug() << Q_FUNC_INFO << "position array length " << posArray.vertexCount << "read at" << file->parent->fileData.currentPosition-4;
    for (int i = 0; i < posArray.vertexCount; i++) {
        x_position = file->parent->fileData.readFloat();
        //qDebug() << Q_FUNC_INFO << "X float: " << x_position << " from hex: " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(positionLocation + (i*12), 4).toHex(), 2);
        y_position = file->parent->fileData.readFloat();
        z_position = file->parent->fileData.readFloat();
        posArray.positionList[i] = QVector3D(x_position, y_position, z_position);
        //qDebug() << Q_FUNC_INFO << "Position floats: " << posArray.positionList[i];
    }

    searchFile.setPattern(file->sectionNames[2].toUtf8());
    lodLocation = searchFile.indexIn(file->parent->fileData.dataBytes, positionLocation);
    lodInfo.file = file;
    if (lodLocation != -1) {
        lodInfo.fileLocation = lodLocation + file->sectionNames[2].length()+2;
        //qDebug() << Q_FUNC_INFO << "Getting LOD Info";
        lodInfo.populateLevels();
    } else {
        lodInfo.fileLocation = 0;
        lodInfo.levels = 1;
        lodInfo.targetIndecies.push_back({0,0});
        lodInfo.targetIndecies.push_back({0,0});
    }

    searchFile.setPattern(file->sectionNames[3].toUtf8());
    boundingLocation = searchFile.indexIn(file->parent->fileData.dataBytes, lengthLocation);
    boundVol.file=file;
    boundVol.fileLocation = boundingLocation;
    //qDebug() << Q_FUNC_INFO << "getting bounding volume for" << name;
    boundVol.populateData();

    getModifications();
}

void Mesh::getModifications(){
    //initialize and set default values
    mods.offset = QVector3D(0,0,0);
    mods.rotation = QQuaternion(1,0,0,0);
    mods.scale = 1;

    file->parent->fileData.currentPosition = lodInfo.fileLocation;
    int lengthOfLod = file->parent->fileData.readUInt()-4;
    //qDebug() << Q_FUNC_INFO << "current location" << file->parent->fileData.currentPosition << "length of LOD: " << lengthOfLod;
    file->parent->fileData.currentPosition += lengthOfLod;
    mods.modByte = file->parent->fileData.readUInt(1);
    //qDebug() << Q_FUNC_INFO << "Reading at: " << file->parent->fileData.currentPosition << " node: " << name << " with modifications: " << mods.modByte;
    if (!(mods.modByte & 1)) {
        //not default position, read 3 sets of 4 bytes, convert to float, and make vector
        float x_offset = file->parent->fileData.readFloat();
        float y_offset = file->parent->fileData.readFloat();
        float z_offset = file->parent->fileData.readFloat();
        mods.offset = QVector3D(x_offset, y_offset, z_offset);
    }
    if (!(mods.modByte & 2)) {
        //not default rotation, read 4 sets of 4 bytes, convert to float, and make quaternion
        //these could be in the wrong order, we'll see
        float x_rotation = file->parent->fileData.readFloat();
        float y_rotation = file->parent->fileData.readFloat();
        float z_rotation = file->parent->fileData.readFloat();
        float scalar = file->parent->fileData.readFloat();
        mods.rotation = QQuaternion(scalar, x_rotation, y_rotation, z_rotation).normalized();
        //mods.rotation = QQuaternion(scalar, scalar, scalar, scalar).normalized();
    }
    if (!(mods.modByte & 4)) {
        //not default scale, read set of 4 bytes, convert to float
        mods.scale = file->parent->fileData.readFloat();
    }

    //qDebug() << Q_FUNC_INFO << "Getting modifications for " << name << " rotation: " << mods.rotation  << " scale: " << mods.scale << "offset" << mods.offset;
    //qDebug() << Q_FUNC_INFO << "Getting modifications for " << name << "scale:" << mods.scale;
    //qDebug() << Q_FUNC_INFO << "Offset: " << mods.offset;
    //qDebug() << Q_FUNC_INFO << "Rotation: " << mods.rotation;

    return;
}

void LODInfo::populateLevels(){
    file->parent->fileData.currentPosition = fileLocation+4;
    levels = file->parent->fileData.readInt();
    //qDebug() << Q_FUNC_INFO << "current LOD levels" << levels << "current highest LOD " << file->highestLOD;
    if (levels > file->highestLOD){
        //qDebug() << Q_FUNC_INFO << "yeah this says that was lower. new highest LOD is " << levels;
        file->highestLOD = levels;
    }
    //qDebug() << Q_FUNC_INFO << "LOD levels: " << levels << "at location" << fileLocation << " from hex: " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentPosition, 4).toHex(), 2);
    targetIndecies.resize(levels);
    for (int i = 0; i<levels; i++){
        targetIndecies[i].push_back(file->parent->fileData.readInt());
        targetIndecies[i].push_back(file->parent->fileData.readInt());
        //third item is float, assuming distance to swap LOD levels. skipping that with the next line
        file->parent->fileData.currentPosition += 4;
    }

    return;
}

void IndexArray::populateTriangleStrips(){
    int indexCount = 0;
    TriangleStrip triangleStrip;
    int tristripcount = 0;

    //qDebug() << Q_FUNC_INFO << "array length: " << arrayLength;

    //some models have index arrays that are just one big triangle strip. not sure how to tell yet though.

    for (int i = 0; i < arrayLength; i++) {
        indexCount = file->parent->fileData.readInt(2);
        //qDebug() << Q_FUNC_INFO << "index count at " << file->parent->fileData.currentPosition << " is " << indexCount;
        //i += arrayLength;
        i += indexCount;
        //qDebug() << Q_FUNC_INFO << "first index read at " << currentLocation + 2 << " is " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentLocation+2, 2).toHex(),2);

        for (int j = 0; j<indexCount;j++){
        //for (int j = 0; j < arrayLength-1; j++){
            triangleStrip.stripIndecies.push_back(file->parent->fileData.readInt(2));
        }
        //qDebug() << Q_FUNC_INFO << "last index read at " << currentLocation - 2 << " is " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(currentLocation-2, 2).toHex(),2);
        //qDebug() << Q_FUNC_INFO << "triangle strip: " << triangleStrip.stripIndecies;
        tristripcount ++;
        triangleStrips.push_back(triangleStrip);
        triangleStrip.stripIndecies.clear();
    }
    //qDebug() << Q_FUNC_INFO << "Triangle strip count for this mesh is " << tristripcount;
    for(int i = 0; i < triangleStrips.size(); i++){
        //qDebug() << Q_FUNC_INFO << "Triangle Strip " << i << ": " << triangleStrips[i].stripIndecies;
    }
}

void PositionArray::getIndexArrays(){
    QByteArrayMatcher searchFile("~BoundingVolume");
    file->parent->fileData.currentPosition = fileLocation;
    //qDebug() << Q_FUNC_INFO << "original index array search start" << fileLocation << file->parent->fileData.currentPosition;
    long endSearch = searchFile.indexIn(file->parent->fileData.dataBytes, file->parent->fileData.currentPosition);
    long location = 0;
    int indexArrayIndex = 0; //not my proudest variable

    searchFile.setPattern("~IndexArray");
    location = searchFile.indexIn(file->parent->fileData.dataBytes, 0);
    //qDebug() << Q_FUNC_INFO << "test location indexarray" << location;



    location = searchFile.indexIn(file->parent->fileData.dataBytes, file->parent->fileData.currentPosition);
    //qDebug() << Q_FUNC_INFO << "file data read length:" << file->parent->fileData.dataBytes.size();
    //qDebug() << Q_FUNC_INFO << "Index array search start" << file->parent->fileData.currentPosition << "found location" << location << "end search" << endSearch;
    while (location < endSearch & location != -1) {
        file->parent->fileData.currentPosition = location + 13; //increase by length of "~IndexArray" + 2
        indexArrays.resize(indexArrayIndex+1);
        indexArrays[indexArrayIndex].file=file;
        indexArrays[indexArrayIndex].arrayID = indexArrayIndex;
        indexArrays[indexArrayIndex].fileLocation = file->parent->fileData.currentPosition;
        indexArrays[indexArrayIndex].arrayLength = (file->parent->fileData.readUInt()-4)/2; //-4 to remove itself, /2 for short length
        //qDebug() << Q_FUNC_INFO << "Array length: " << indexArrays[indexArrayIndex].arrayLength;

        indexArrays[indexArrayIndex].populateTriangleStrips();

        location = searchFile.indexIn(file->parent->fileData.dataBytes, file->parent->fileData.currentPosition);
        //qDebug() << Q_FUNC_INFO << "next location" << location << "with search start" << file->parent->fileData.currentPosition;
        ++indexArrayIndex;

    }

    return;

}
