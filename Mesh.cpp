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
    file = nullptr;
}

void Mesh::clear(){
    fileLocation = 0;
    vertexSet.positionArray.clear();
    elementArray.lodInfo.clear();
    mods.clear();
    sectionList.clear();
    sectionTypes.clear();
    file = nullptr;
}

const void Mesh::operator=(Mesh input){
    name = input.name;
    fileLocation = input.fileLocation;
    vertexSet.positionArray = input.vertexSet.positionArray;
    mods = input.mods;
    file = input.file;
    sectionList = input.sectionList;
    sectionTypes = input.sectionTypes;
    elementArray.lodInfo = input.elementArray.lodInfo;
    elementArray.lodInfo.file = input.elementArray.lodInfo.file;
    elementArray.lodInfo.fileLocation = input.elementArray.lodInfo.fileLocation;
    elementArray.lodInfo.levels = input.elementArray.lodInfo.levels;
    elementArray.lodInfo.targetIndecies = input.elementArray.lodInfo.targetIndecies;
}

const void PositionArray::operator=(PositionArray input){
    arrayID = input.arrayID;
    file = input.file;
    meshName = input.meshName;
    positionList = input.positionList;
    vertexCount = input.vertexCount;
    fileLocation = input.fileLocation;
}

int Mesh::readMesh(){
    QString signature;
    QString subSignature;
    int nameLength = 0;
    QString name;
    int subSectionLength = 0;
    int subSectionEnd = 0;
    int indexEnd = 0;
    float x_position = 0;
    float y_position = 0;
    float z_position = 0;
    float a_position = 0;
    //qDebug() << Q_FUNC_INFO << "Do mesh stuff";
    elementCount = file->parent->fileData.readInt();
    signature = file->parent->fileData.getSignature(); //get extended info
    if(signature == "~ExtendedInfo"){
        subSectionLength = file->parent->fileData.readInt();
        file->parent->fileData.currentPosition += subSectionLength-4;
        signature = file->parent->fileData.getSignature(); //then read vertex set
    }
    if (signature != "~VertexSet") {
        qDebug() << Q_FUNC_INFO << "Expected VertexSet. Unexpected section found at " << file->parent->fileData.currentPosition << ": " << signature;
        return 1;
    }

    subSectionLength = file->parent->fileData.readInt();
    //file->parent->fileData.currentPosition += subSectionLength - 4;
    vertexSet.vertexCount = file->parent->fileData.readInt();
    int unknown4Byte2 = file->parent->fileData.readInt();
    int unknown4Byte3 = file->parent->fileData.readInt();

    //Position Array
    signature = file->parent->fileData.getSignature();
    if (signature == "~PositionArray") {
        subSectionLength = file->parent->fileData.readInt();

        for(int position = 0; position < (subSectionLength-4)/12; position++){
            x_position = file->parent->fileData.readFloat();
            //qDebug() << Q_FUNC_INFO << "X float: " << x_position << " from hex: " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(positionLocation + (i*12), 4).toHex(), 2);
            y_position = file->parent->fileData.readFloat();
            z_position = file->parent->fileData.readFloat();
            vertexSet.positionArray.positionList.push_back(QVector3D(x_position, y_position, z_position));
        }

        signature = file->parent->fileData.getSignature();
    } else {
        qDebug() << Q_FUNC_INFO << "Model does not have PositionArray";
    }


    //Normal Array
    if (signature == "~NormalArray") {
        subSectionLength = file->parent->fileData.readInt();

        for(int position = 0; position < (subSectionLength-4)/12; position++){
            x_position = file->parent->fileData.readFloat();
            //qDebug() << Q_FUNC_INFO << "X float: " << x_position << " from hex: " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(positionLocation + (i*12), 4).toHex(), 2);
            y_position = file->parent->fileData.readFloat();
            z_position = file->parent->fileData.readFloat();
            vertexSet.normalArray.positionList.push_back(QVector3D(x_position, y_position, z_position));
        }

        signature = file->parent->fileData.getSignature();
    } else {
        qDebug() << Q_FUNC_INFO << "Model does not have NormalArray";
    }


    //Color Array
    if (signature == "~ColorArray") {
        subSectionLength = file->parent->fileData.readInt();

        for(int position = 0; position < (subSectionLength-4)/16; position++){
            x_position = file->parent->fileData.readFloat();
            //qDebug() << Q_FUNC_INFO << "X float: " << x_position << " from hex: " << file->parent->binChanger.reverse_input(file->parent->fileData.mid(positionLocation + (i*12), 4).toHex(), 2);
            y_position = file->parent->fileData.readFloat();
            z_position = file->parent->fileData.readFloat();
            a_position = file->parent->fileData.readFloat();
            vertexSet.colorArray.positionList.push_back(QVector4D(x_position, y_position, z_position, a_position));
        }

        signature = file->parent->fileData.getSignature();
    } else {
        qDebug() << Q_FUNC_INFO << "Model does not have ColorArray.";
    }


    //Texture Coords
    if (signature == "~TextureCoords") {
        subSectionLength = file->parent->fileData.readInt();

        for(int position = 0; position < (subSectionLength-4)/8; position++){
            x_position = file->parent->fileData.readFloat();
            y_position = file->parent->fileData.readFloat();
            vertexSet.textureCoords.positionList.push_back(QVector2D(x_position, y_position));
        }

        signature = file->parent->fileData.getSignature();
    } else {
        qDebug() << Q_FUNC_INFO << "Model does not have TextureCoords.";
    }

    //Element Array
    if (signature != "~ElementArray") {
        qDebug() << Q_FUNC_INFO << "Expected ElementArray. Unexpected section found at " << file->parent->fileData.currentPosition << ": " << signature;
        return 1;
    }
    //ElementArray *elementArray = new ElementArray;
    subSectionLength = file->parent->fileData.readInt();
    subSectionEnd = file->parent->fileData.currentPosition - 4 + subSectionLength;
    for(int readElement = 0; readElement < elementCount; readElement++){
        Element *element = new Element;
        signature = file->parent->fileData.getSignature();
        subSectionLength = file->parent->fileData.readInt();
        //need to check for LODINFO bit to see if we even read LODInfo
        if (signature == "~LODInfo") {
            qDebug() << Q_FUNC_INFO << "Reached LOD Info at " << file->parent->fileData.currentPosition << ": " << signature;
            break;
        }
        element->unknownProperty1 = file->parent->fileData.readInt();

        signature = file->parent->fileData.getSignature();
        if (signature != "~MeshFaceSet") {
            qDebug() << Q_FUNC_INFO << "Expected MeshFaceSet. Unexpected section found at " << file->parent->fileData.currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = file->parent->fileData.readInt();
        element->meshFaceSet.unknownProperty1 = file->parent->fileData.readInt();
        element->meshFaceSet.unknownProperty2 = file->parent->fileData.readInt();
        element->meshFaceSet.unknownProperty3 = file->parent->fileData.readInt();

        signature = file->parent->fileData.getSignature();
        if (signature != "~IndexArray") {
            qDebug() << Q_FUNC_INFO << "Expected IndexArray. Unexpected section found at " << file->parent->fileData.currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = file->parent->fileData.readInt();
        indexEnd = file->parent->fileData.currentPosition - 4 + subSectionLength;
        //skipping IndexArray for now
        //file->parent->fileData.currentPosition += subSectionLength - 4;
        while(file->parent->fileData.currentPosition < indexEnd){
            TriangleStrip *triangleStrip = new TriangleStrip;
            triangleStrip->stripIndecies.resize(file->parent->fileData.readInt(2));
            for (int triangle = 0; triangle<triangleStrip->stripIndecies.size();triangle++){
                triangleStrip->stripIndecies[triangle] = file->parent->fileData.readInt(2);
            }
            element->meshFaceSet.indexArray.triangleStrips.push_back(*triangleStrip);
        }

        //qDebug() << Q_FUNC_INFO << "Triangle strip count for this mesh is " << tristripcount;
        for(int i = 0; i < element->meshFaceSet.indexArray.triangleStrips.size(); i++){
            //qDebug() << Q_FUNC_INFO << "Triangle Strip " << i << ": " << element->meshFaceSet.indexArray.triangleStrips[i].stripIndecies.size();
        }

        signature = file->parent->fileData.getSignature();
        if (signature != "~SurfaceProperties") {
            qDebug() << Q_FUNC_INFO << "Expected SurfaceProperties. Unexpected section found at " << file->parent->fileData.currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = file->parent->fileData.readInt();
        element->surfaceProperties.materialRelated = file->parent->fileData.readInt();
        nameLength = file->parent->fileData.readInt();
        element->surfaceProperties.name = file->parent->fileData.readHex(nameLength);
        element->surfaceProperties.unknownProperty2 = file->parent->fileData.readInt();

        signature = file->parent->fileData.getSignature();
        if (signature != "~Material") {
            qDebug() << Q_FUNC_INFO << "Expected Material. Unexpected section found at " << file->parent->fileData.currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = file->parent->fileData.readInt();

        if(element->surfaceProperties.materialRelated == 5){
            element->surfaceProperties.material.unknownProperty1 = file->parent->fileData.readInt();

            element->surfaceProperties.material.nameLength = file->parent->fileData.readInt();
            element->surfaceProperties.material.name = file->parent->fileData.readHex(element->surfaceProperties.material.nameLength);
        }

        element->surfaceProperties.material.unknownFloat1 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat2 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat3 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat4 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat5 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat6 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat7 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat8 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat9 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat10 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat11 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat12 = file->parent->fileData.readFloat();
        element->surfaceProperties.material.unknownFloat13 = file->parent->fileData.readFloat();


        signature = file->parent->fileData.getSignature();
        if (signature != "~RenderStateGroup") {
            qDebug() << Q_FUNC_INFO << "Expected RenderStateGroup. Unexpected section found at " << file->parent->fileData.currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty1 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty2 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty3 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty4 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty5 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty6 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty7 = file->parent->fileData.readInt();

        signature = file->parent->fileData.getSignature();
        if (signature != "~RenderStateGroup") {
            qDebug() << Q_FUNC_INFO << "Expected RenderStateGroup. Unexpected section found at " << file->parent->fileData.currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup2.unknownProperty1 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup2.unknownProperty2 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup2.unknownProperty3 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup2.unknownProperty4 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup2.unknownProperty5 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup2.unknownProperty6 = file->parent->fileData.readInt();
        element->surfaceProperties.renderStateGroup2.unknownProperty7 = file->parent->fileData.readInt();

        element->unknownProperty2 = file->parent->fileData.readInt();
        element->unknownProperty3 = file->parent->fileData.readInt();

        elementArray.elementArray.push_back(*element);
    }

    bool hasLodInfo = file->parent->fileData.readBool();
    if (hasLodInfo) {
        signature = file->parent->fileData.getSignature();
        subSectionLength = file->parent->fileData.readInt();
        elementArray.lodInfo.levels = file->parent->fileData.readInt();
        std::vector<int> levelTargetArray;
        for (int readLevels = 0; readLevels < elementArray.lodInfo.levels; readLevels++) {
            levelTargetArray.push_back(file->parent->fileData.readInt());
            levelTargetArray.push_back(file->parent->fileData.readInt());
            elementArray.lodInfo.targetIndecies.push_back(levelTargetArray);
            levelTargetArray.clear();
            elementArray.lodInfo.levelDistance.push_back(file->parent->fileData.readFloat());
        }
        if (elementArray.lodInfo.levels > file->highestLOD){
            file->highestLOD = elementArray.lodInfo.levels;
        }
    }

    //qDebug() << Q_FUNC_INFO << "finished reading mesh at " << file->parent->fileData.currentPosition;
    return 0;

}

void Mesh::modify(std::vector<Modifications> addedMods){
    addedMods.push_back(mods);

//    for (int i = 0; i < addedMods.size(); i++) {
//        qDebug() << name << "mod" << i << "is scale" << addedMods[i].scale;
//        qDebug() << name << "mod" << i << "is rotation" << addedMods[i].rotation;
//        qDebug() << name << "mod" << i << "is offset" << addedMods[i].offset;
//    }

    for (int j = 0; j < vertexSet.positionArray.positionList.size(); ++j) {
        for(int k = addedMods.size()-1; k > -1; k--){
            vertexSet.positionArray.positionList[j] = vertexSet.positionArray.positionList[j] * addedMods[k].scale;
            vertexSet.positionArray.positionList[j] = addedMods[k].rotation.rotatedVector(vertexSet.positionArray.positionList[j]);
            vertexSet.positionArray.positionList[j] = vertexSet.positionArray.positionList[j] + addedMods[k].offset;
        }
    }
}


void Mesh::writeData(QTextStream &fileOut){
    std::vector<int> chosenLOD;
    int triangle[3];
    QVector3D tempVec;

    //qDebug() << Q_FUNC_INFO << "mesh file" << file->fileName;
    //qDebug() << Q_FUNC_INFO << "lodinfo list size" << elementArray.lodInfo.targetIndecies.size() << "chosen lod" << file->parent->ListLevels->currentIndex();
    if (elementArray.lodInfo.targetIndecies.size() <= file->parent->ListLevels->currentIndex()) {
        chosenLOD = {0,static_cast<int>(elementArray.elementArray.size()-1)};
    } else {
        chosenLOD = elementArray.lodInfo.targetIndecies[file->parent->ListLevels->currentText().toInt(nullptr, 10)-1];
    }
    //qDebug() << Q_FUNC_INFO << "chosen LOD index targets: " << chosenLOD << " for mesh " << name;
    //qDebug() << Q_FUNC_INFO << "position list size" << vertexSet.positionArray.positionList.size();

    for (int index = chosenLOD[0]; index <= chosenLOD[1]; index++){
        //qDebug() << Q_FUNC_INFO << "index target" << index << "has" << int(elementArray.elementArray[index].meshFaceSet.indexArray.triangleStrips.size()) << "triangle strips";
        for (int strip = 0; strip < int(elementArray.elementArray[index].meshFaceSet.indexArray.triangleStrips.size());strip++){
            //qDebug() << Q_FUNC_INFO << "index" << index << "strip" << strip << "has" << int(elementArray.elementArray[index].meshFaceSet.indexArray.triangleStrips[strip].stripIndecies.size()) << "triangles";
            for (int m = 0; m < int(elementArray.elementArray[index].meshFaceSet.indexArray.triangleStrips[strip].stripIndecies.size()-2); m++){
                triangle[0] = elementArray.elementArray[index].meshFaceSet.indexArray.triangleStrips[strip].stripIndecies[m];
                triangle[1] = elementArray.elementArray[index].meshFaceSet.indexArray.triangleStrips[strip].stripIndecies[m+1];
                triangle[2] = elementArray.elementArray[index].meshFaceSet.indexArray.triangleStrips[strip].stripIndecies[m+2];
                if(triangle[0] == triangle[1] or triangle[1] == triangle[2] or triangle[0] == triangle[2]){
                    continue;
                }
                fileOut << "  facet normal 0 0 0" << Qt::endl;
                fileOut << "    outer loop" << Qt::endl;
                for (int n = 0; n < 3; ++n) {
                    fileOut << "      vertex ";
                    tempVec = vertexSet.positionArray.positionList[triangle[n]]; //just for readability for the next line
                    fileOut << QString::number(tempVec.x(), 'f', 3) << " " << QString::number(tempVec.y(), 'f', 3) << " " << QString::number(tempVec.z(), 'f', 3) << Qt::endl;
                }
                fileOut << "    endloop" << Qt::endl;
                fileOut << "  endfacet" << Qt::endl;
            }
        }
    }
}
