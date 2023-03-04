#include "mainwindow.h"

void LODInfo::clear(){
    levels= 0;
    targetIndecies.clear();
    fileLocation = 0;
    fileData = nullptr;
}

void PositionArray::clear(){
    arrayID = 0;
    vertexCount = 0;
    fileLocation = 0;
    meshName = "";
    positionList.clear();
    fileData = nullptr;
}

void Mesh::clear(){
    fileLocation = 0;
    vertexSet.positionArray.clear();
    elementArray.lodInfo.clear();
    mods.clear();
    sectionList.clear();
    sectionTypes.clear();
    file = nullptr;
    fileData = nullptr;
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
    elementArray.lodInfo.fileData = input.elementArray.lodInfo.fileData;
    elementArray.lodInfo.fileLocation = input.elementArray.lodInfo.fileLocation;
    elementArray.lodInfo.levels = input.elementArray.lodInfo.levels;
    elementArray.lodInfo.targetIndecies = input.elementArray.lodInfo.targetIndecies;
}

const void PositionArray::operator=(PositionArray input){
    arrayID = input.arrayID;
    fileData = input.fileData;
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
    elementCount = fileData->readInt();
    signature = fileData->getSignature(); //get extended info
    if(signature == "~ExtendedInfo"){
        subSectionLength = fileData->readInt();
        fileData->currentPosition += subSectionLength-4;
        signature = fileData->getSignature(); //then read vertex set
    }
    if (signature != "~VertexSet") {
        qDebug() << Q_FUNC_INFO << "Expected VertexSet. Unexpected section found at " << fileData->currentPosition << ": " << signature;
        return 1;
    }

    subSectionLength = fileData->readInt();
    //fileData->currentPosition += subSectionLength - 4;
    vertexSet.vertexCount = fileData->readInt();
    //qDebug() << Q_FUNC_INFO << "reading unknown values at" << fileData->currentPosition;
    int unknown4Byte1 = fileData->readInt();
    int unknown4Byte2 = fileData->readInt();
    //qDebug() << Q_FUNC_INFO << "unknown1" << unknown4Byte1 << "unknown2" << unknown4Byte2;

    //Position Array
    signature = fileData->getSignature();
    if (signature == "~PositionArray") {
        subSectionLength = fileData->readInt();

        for(int position = 0; position < (subSectionLength-4)/12; position++){
            x_position = fileData->readFloat();
            //qDebug() << Q_FUNC_INFO << "X float: " << x_position << " from hex: " << file->parent->binChanger.reverse_input(fileData->mid(positionLocation + (i*12), 4).toHex(), 2);
            y_position = fileData->readFloat();
            z_position = fileData->readFloat();
            vertexSet.positionArray.positionList.push_back(QVector3D(x_position, y_position, z_position));
        }

        signature = fileData->getSignature();
    } else {
        qDebug() << Q_FUNC_INFO << "Model does not have PositionArray";
    }


    //Normal Array
    if (signature == "~NormalArray") {
        subSectionLength = fileData->readInt();

        for(int position = 0; position < (subSectionLength-4)/12; position++){
            x_position = fileData->readFloat();
            y_position = fileData->readFloat();
            z_position = fileData->readFloat();
            vertexSet.normalArray.positionList.push_back(QVector3D(x_position, y_position, z_position));
        }

        signature = fileData->getSignature();
    } else {
        qDebug() << Q_FUNC_INFO << "Model does not have NormalArray";
    }


    //Color Array
    if (signature == "~ColorArray") {
        subSectionLength = fileData->readInt();

        for(int position = 0; position < (subSectionLength-4)/16; position++){
            x_position = fileData->readFloat();
            //qDebug() << Q_FUNC_INFO << "X float: " << x_position << " from hex: " << file->parent->binChanger.reverse_input(fileData->mid(positionLocation + (i*12), 4).toHex(), 2);
            y_position = fileData->readFloat();
            z_position = fileData->readFloat();
            a_position = fileData->readFloat();
            vertexSet.colorArray.positionList.push_back(QVector4D(x_position, y_position, z_position, a_position));
        }

        signature = fileData->getSignature();
    } else {
        qDebug() << Q_FUNC_INFO << "Model does not have ColorArray.";
    }


    //Texture Coords
    if (signature == "~TextureCoords") {
        subSectionLength = fileData->readInt();

        for(int position = 0; position < (subSectionLength-4)/8; position++){
            x_position = fileData->readFloat();
            y_position = fileData->readFloat();
            vertexSet.textureCoords.positionList.push_back(QVector2D(x_position, y_position));
        }

        signature = fileData->getSignature();
    } else {
        qDebug() << Q_FUNC_INFO << "Model does not have TextureCoords.";
    }

    //Element Array
    if (signature != "~ElementArray") {
        qDebug() << Q_FUNC_INFO << "Expected ElementArray. Unexpected section found at " << fileData->currentPosition << ": " << signature;
        return 1;
    }
    //ElementArray *elementArray = new ElementArray;
    subSectionLength = fileData->readInt();
    subSectionEnd = fileData->currentPosition - 4 + subSectionLength;
    for(int readElement = 0; readElement < elementCount; readElement++){
        Element *element = new Element;
        signature = fileData->getSignature();
        subSectionLength = fileData->readInt();
        element->meshFaceSet.indexArray.triangleCount = 0;
        //need to check for LODINFO bit to see if we even read LODInfo
        if (signature == "~LODInfo") {
            qDebug() << Q_FUNC_INFO << "Reached LOD Info at " << fileData->currentPosition << ": " << signature;
            break;
        }
        element->unknownProperty1 = fileData->readInt();

        signature = fileData->getSignature();
        if (signature != "~MeshFaceSet") {
            qDebug() << Q_FUNC_INFO << "Expected MeshFaceSet. Unexpected section found at " << fileData->currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = fileData->readInt();
        element->meshFaceSet.unknownProperty1 = fileData->readInt();
        element->meshFaceSet.unknownProperty2 = fileData->readInt();
        element->meshFaceSet.unknownProperty3 = fileData->readInt();

        signature = fileData->getSignature();
        if (signature != "~IndexArray") {
            qDebug() << Q_FUNC_INFO << "Expected IndexArray. Unexpected section found at " << fileData->currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = fileData->readInt();
        indexEnd = fileData->currentPosition - 4 + subSectionLength;
        //skipping IndexArray for now
        //fileData->currentPosition += subSectionLength - 4;
        while(fileData->currentPosition < indexEnd){
            TriangleStrip *triangleStrip = new TriangleStrip;
            triangleStrip->stripIndecies.resize(fileData->readInt(2));
            for (int triangle = 0; triangle<triangleStrip->stripIndecies.size();triangle++){
                element->meshFaceSet.indexArray.triangleCount += 1;
                triangleStrip->stripIndecies[triangle] = fileData->readInt(2);
            }
            element->meshFaceSet.indexArray.triangleStrips.push_back(*triangleStrip);
        }

        //qDebug() << Q_FUNC_INFO << "Triangle strip count for this mesh is " << tristripcount;
        for(int i = 0; i < element->meshFaceSet.indexArray.triangleStrips.size(); i++){
            //qDebug() << Q_FUNC_INFO << "Triangle Strip " << i << ": " << element->meshFaceSet.indexArray.triangleStrips[i].stripIndecies.size();
        }

        signature = fileData->getSignature();
        if (signature != "~SurfaceProperties") {
            qDebug() << Q_FUNC_INFO << "Expected SurfaceProperties. Unexpected section found at " << fileData->currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = fileData->readInt();
        element->surfaceProperties.materialRelated = fileData->readInt(); //seems like a version number
        nameLength = fileData->readInt();
        element->surfaceProperties.textureName = fileData->readHex(nameLength);
        element->surfaceProperties.unknownProperty2 = fileData->readInt();

        signature = fileData->getSignature();
        if (signature != "~Material") {
            qDebug() << Q_FUNC_INFO << "Expected Material. Unexpected section found at " << fileData->currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = fileData->readInt();

        if(element->surfaceProperties.materialRelated == 5){
            //materialRelated 3 is seen on TFA/PICKUPS models, 4 is seen on LightningRod
            element->surfaceProperties.material.unknownProperty1 = fileData->readInt();

            element->surfaceProperties.material.nameLength = fileData->readInt();
            element->surfaceProperties.material.name = fileData->readHex(element->surfaceProperties.material.nameLength);
        }

        element->surfaceProperties.material.unknownFloat1 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat2 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat3 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat4 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat5 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat6 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat7 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat8 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat9 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat10 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat11 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat12 = fileData->readFloat();
        element->surfaceProperties.material.unknownFloat13 = fileData->readFloat();


        signature = fileData->getSignature();
        if (signature != "~RenderStateGroup") {
            qDebug() << Q_FUNC_INFO << "Expected RenderStateGroup. Unexpected section found at " << fileData->currentPosition << ": " << signature;
            return 1;
        }
        subSectionLength = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty1 = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty2 = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty3 = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty4 = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty5 = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty6 = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.unknownProperty7 = fileData->readInt();

        //qDebug() << Q_FUNC_INFO << "material version number" << element->surfaceProperties.materialRelated;
        if(element->surfaceProperties.materialRelated > 2){
            //materialRelated 3 is seen on TFA/PICKUPS models
            signature = fileData->getSignature();
            if (signature != "~RenderStateGroup") {
                qDebug() << Q_FUNC_INFO << "Expected RenderStateGroup. Unexpected section found at " << fileData->currentPosition << ": " << signature;
                return 1;
            }
            subSectionLength = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.unknownProperty1 = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.unknownProperty2 = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.unknownProperty3 = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.unknownProperty4 = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.unknownProperty5 = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.unknownProperty6 = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.unknownProperty7 = fileData->readInt();
        }

        element->unknownProperty2 = fileData->readInt();

        if(element->surfaceProperties.materialRelated > 3){
            element->unknownProperty3 = fileData->readInt();
        }

        elementArray.elementArray.push_back(*element);
    }

    bool hasLodInfo = false;
    //qDebug() << Q_FUNC_INFO << "checking for lod info at" << fileData->currentPosition;
    if(elementArray.elementArray[0].surfaceProperties.materialRelated > 2){
        //the use of materialrelated here is absolutely horrible. Find another version number of property byte that can show whether there are LODs or not.
        hasLodInfo = fileData->readBool();
    }
    //qDebug() << Q_FUNC_INFO << "has lod info:" << hasLodInfo;
    if (hasLodInfo) {
        signature = fileData->getSignature();
        subSectionLength = fileData->readInt();
        elementArray.lodInfo.levels = fileData->readInt();
        std::vector<int> levelTargetArray;
        for (int readLevels = 0; readLevels < elementArray.lodInfo.levels; readLevels++) {
            levelTargetArray.push_back(fileData->readInt());
            levelTargetArray.push_back(fileData->readInt());
            elementArray.lodInfo.targetIndecies.push_back(levelTargetArray);
            levelTargetArray.clear();
            elementArray.lodInfo.levelDistance.push_back(fileData->readFloat());
        }
        if (elementArray.lodInfo.levels > file->highestLOD){
            file->highestLOD = elementArray.lodInfo.levels;
        }
    }

    //qDebug() << Q_FUNC_INFO << "finished reading mesh at " << fileData->currentPosition;
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

void Mesh::writeEffectsDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD;
    if (elementArray.lodInfo.targetIndecies.size() <= file->parent->ListLevels->currentIndex()) {
        chosenLOD = {0,static_cast<int>(elementArray.elementArray.size()-1)};
    } else {
        chosenLOD = elementArray.lodInfo.targetIndecies[file->parent->ListLevels->currentText().toInt(nullptr, 10)-1];
    }

    QString meshName = file->fileWithoutExtension + "-" + name;

    QString checkTexture;
    for(int element = chosenLOD[0]; element <= chosenLOD[1]; element++){
        checkTexture = elementArray.elementArray[element].surfaceProperties.textureName;
    }
    if(checkTexture != elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName){
        qDebug() << Q_FUNC_INFO << "checktexture" << checkTexture << "element texture" << elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName;
        //file->parent->messageError("Mesh " + name + " has different textures for element " + QString::number(chosenLOD[0]) + " and " + QString::number(chosenLOD[1]) + ". Let Trevor know.");
        //return;
    }

    fileOut << "    <effect id=\"" + checkTexture +"Texture-effect\">" << Qt::endl;
    fileOut << "      <profile_COMMON>" << Qt::endl;
    fileOut << "        <newparam sid=\"" + checkTexture +"_bmp-surface\">" << Qt::endl;
    fileOut << "          <surface type=\"2D\">" << Qt::endl;
    fileOut << "            <init_from>" + checkTexture +"_bmp</init_from>" << Qt::endl;
    fileOut << "          </surface>" << Qt::endl;
    fileOut << "        </newparam>" << Qt::endl;
    fileOut << "        <newparam sid=\"" + checkTexture +"_bmp-sampler\">" << Qt::endl;
    fileOut << "          <sampler2D>" << Qt::endl;
    fileOut << "            <source>" + checkTexture +"_bmp-surface</source>" << Qt::endl;
    fileOut << "          </sampler2D>" << Qt::endl;
    fileOut << "        </newparam>" << Qt::endl;
    fileOut << "        <technique sid=\"common\">" << Qt::endl;
    fileOut << "          <lambert>" << Qt::endl;
    fileOut << "            <emission>" << Qt::endl;
    fileOut << "              <color sid=\"emission\">0 0 0 1</color>" << Qt::endl;
    fileOut << "            </emission>" << Qt::endl;
    fileOut << "            <diffuse>" << Qt::endl;
    fileOut << "              <texture texture=\"" + checkTexture +"_bmp-sampler\" texcoord=\""+meshName+"-mesh-texcoords\"/>" << Qt::endl;
    fileOut << "            </diffuse>" << Qt::endl;
    fileOut << "            <index_of_refraction>" << Qt::endl;
    fileOut << "              <float sid=\"ior\">1.45</float>" << Qt::endl;
    fileOut << "            </index_of_refraction>" << Qt::endl;
    fileOut << "          </lambert>" << Qt::endl;
    fileOut << "        </technique>" << Qt::endl;
    fileOut << "      </profile_COMMON>" << Qt::endl;
    fileOut << "    </effect>" << Qt::endl;
}

void Mesh::writeImagesDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD;
    if (elementArray.lodInfo.targetIndecies.size() <= file->parent->ListLevels->currentIndex()) {
        chosenLOD = {0,static_cast<int>(elementArray.elementArray.size()-1)};
    } else {
        chosenLOD = elementArray.lodInfo.targetIndecies[file->parent->ListLevels->currentText().toInt(nullptr, 10)-1];
    }

    QString meshName = file->fileWithoutExtension + "-" + name;

    QString checkTexture;
    for(int element = chosenLOD[0]; element <= chosenLOD[1]; element++){
        checkTexture = elementArray.elementArray[element].surfaceProperties.textureName;
    }
    if(checkTexture != elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName){
        qDebug() << Q_FUNC_INFO << "checktexture" << checkTexture << "element texture" << elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName;
        //file->parent->messageError("Mesh " + name + " has different textures for element " + QString::number(chosenLOD[0]) + " and " + QString::number(chosenLOD[1]) + ". Let Trevor know.");
        //return;
    }
    fileOut << "    <image id=\"" + checkTexture +"_bmp\" name=\"" + checkTexture +"_bmp\">" << Qt::endl;
    fileOut << "      <init_from>" + checkTexture +".bmp</init_from>" << Qt::endl;
    fileOut << "    </image>" << Qt::endl;
}

void Mesh::writeMaterialsDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD;
    if (elementArray.lodInfo.targetIndecies.size() <= file->parent->ListLevels->currentIndex()) {
        chosenLOD = {0,static_cast<int>(elementArray.elementArray.size()-1)};
    } else {
        chosenLOD = elementArray.lodInfo.targetIndecies[file->parent->ListLevels->currentText().toInt(nullptr, 10)-1];
    }

    QString meshName = file->fileWithoutExtension + "-" + name;

    QString checkTexture;
    for(int element = chosenLOD[0]; element <= chosenLOD[1]; element++){
        checkTexture = elementArray.elementArray[element].surfaceProperties.textureName;
    }
    if(checkTexture != elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName){
        qDebug() << Q_FUNC_INFO << "checktexture" << checkTexture << "element texture" << elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName;
        //file->parent->messageError("Mesh " + name + " has different textures for element " + QString::number(chosenLOD[0]) + " and " + QString::number(chosenLOD[1]) + ". Let Trevor know.");
        //return;
    }
    fileOut << "    <material id=\"" + checkTexture +"Texture-material\" name=\"" + checkTexture +"Texture\">" << Qt::endl;
    fileOut << "      <instance_effect url=\"#" + checkTexture +"Texture-effect\"/>" << Qt::endl;
    fileOut << "    </material>" << Qt::endl;
}

void Mesh::writeDataSTL(QTextStream &fileOut){
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

void Mesh::writeDataDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD;
    if (elementArray.lodInfo.targetIndecies.size() <= file->parent->ListLevels->currentIndex()) {
        chosenLOD = {0,static_cast<int>(elementArray.elementArray.size()-1)};
    } else {
        chosenLOD = elementArray.lodInfo.targetIndecies[file->parent->ListLevels->currentText().toInt(nullptr, 10)-1];
    }
    QString meshName = file->fileWithoutExtension + "-" + name;
    int triangleCount = 0;
    QString checkTexture;
    for(int element = chosenLOD[0]; element <= chosenLOD[1]; element++){
        triangleCount += elementArray.elementArray[element].meshFaceSet.indexArray.getDAETriangleCount();
        checkTexture = elementArray.elementArray[element].surfaceProperties.textureName;
    }
    if(checkTexture != elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName){
        qDebug() << Q_FUNC_INFO << "checktexture" << checkTexture << "element texture" << elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName;
        //file->parent->messageError("Mesh " + name + " has different textures for element " + QString::number(chosenLOD[0]) + " and " + QString::number(chosenLOD[1]) + ". Let Trevor know.");
        //return;
    }


    fileOut << "    <geometry id=\"" + meshName + "-mesh\" name=\"" + meshName + "\">" << Qt::endl;
    fileOut << "      <mesh>" << Qt::endl;

    fileOut << "        <source id=\"" + meshName + "-mesh-positions\">" << Qt::endl;
    fileOut << "          <float_array id=\"" + meshName + "-mesh-positions-array\" count = \"" + QString::number(vertexSet.positionArray.positionList.size()*3) + "\">";
    for(int position = 0; position < vertexSet.positionArray.positionList.size(); position++){
        fileOut << QString::number(vertexSet.positionArray.positionList[position].x()) << " ";
        fileOut << QString::number(vertexSet.positionArray.positionList[position].y()) << " ";
        fileOut << QString::number(vertexSet.positionArray.positionList[position].z()) << " ";
    }
    fileOut << "</float_array>" << Qt::endl;
    fileOut << "          <technique_common>" << Qt::endl;
    fileOut << "            <accessor source=\"#"+ meshName +"-mesh-positions-array\" count=\"" + QString::number(vertexSet.positionArray.positionList.size()) + "\" stride=\"3\">" << Qt::endl;
    fileOut << "              <param name=\"X\" type=\"float\"/>" << Qt::endl;
    fileOut << "              <param name=\"Y\" type=\"float\"/>" << Qt::endl;
    fileOut << "              <param name=\"Z\" type=\"float\"/>" << Qt::endl;
    fileOut << "            </accessor>" << Qt::endl;
    fileOut << "          </technique_common>" << Qt::endl;
    fileOut << "        </source>" << Qt::endl;

    fileOut << "        <source id=\"" + meshName + "-mesh-normals\">" << Qt::endl;
    fileOut << "          <float_array id=\"" + meshName + "-mesh-normals-array\" count = \"" + QString::number(vertexSet.normalArray.positionList.size()*3) + "\">";
    for(int position = 0; position < vertexSet.normalArray.positionList.size(); position++){
        fileOut << QString::number(vertexSet.normalArray.positionList[position].x()) << " ";
        fileOut << QString::number(vertexSet.normalArray.positionList[position].y()) << " ";
        fileOut << QString::number(vertexSet.normalArray.positionList[position].z()) << " ";
    }
    fileOut << "</float_array>" << Qt::endl;
    fileOut << "          <technique_common>" << Qt::endl;
    fileOut << "            <accessor source=\"#"+ meshName +"-mesh-normals-array\" count=\"" + QString::number(vertexSet.normalArray.positionList.size()) + "\" stride=\"3\">" << Qt::endl;
    fileOut << "              <param name=\"X\" type=\"float\"/>" << Qt::endl;
    fileOut << "              <param name=\"Y\" type=\"float\"/>" << Qt::endl;
    fileOut << "              <param name=\"Z\" type=\"float\"/>" << Qt::endl;
    fileOut << "            </accessor>" << Qt::endl;
    fileOut << "          </technique_common>" << Qt::endl;
    fileOut << "        </source>" << Qt::endl;

    fileOut << "        <source id=\"" + meshName + "-mesh-map-0\">" << Qt::endl;
    fileOut << "          <float_array id=\"" + meshName + "-mesh-map-0-array\" count = \"" + QString::number(vertexSet.textureCoords.positionList.size()*2) + "\">";
    for(int position = 0; position < vertexSet.textureCoords.positionList.size(); position++){
        fileOut << QString::number(vertexSet.textureCoords.positionList[position].x()) << " ";
        fileOut << QString::number(vertexSet.textureCoords.positionList[position].y()) << " ";
    }
    fileOut << "</float_array>" << Qt::endl;
    fileOut << "          <technique_common>" << Qt::endl;
    fileOut << "            <accessor source=\"#"+ meshName +"-mesh-map-0-array\" count=\"" + QString::number(vertexSet.textureCoords.positionList.size()) + "\" stride=\"2\">" << Qt::endl;
    fileOut << "              <param name=\"U\" type=\"float\"/>" << Qt::endl;
    fileOut << "              <param name=\"V\" type=\"float\"/>" << Qt::endl;
    fileOut << "            </accessor>" << Qt::endl;
    fileOut << "          </technique_common>" << Qt::endl;
    fileOut << "        </source>" << Qt::endl;

//    fileOut << "        <source id=\"" + meshName + "-mesh-colors\">" << Qt::endl;
//    fileOut << "          <float_array id=\"" + meshName + "-mesh-colors-array\" count = \"" + QString::number(vertexSet.colorArray.positionList.size()*4) + "\">";
//    for(int position = 0; position < vertexSet.colorArray.positionList.size(); position++){
//        fileOut << QString::number(vertexSet.colorArray.positionList[position].x()) << " ";
//        fileOut << QString::number(vertexSet.colorArray.positionList[position].y()) << " ";
//        fileOut << QString::number(vertexSet.colorArray.positionList[position].z()) << " ";
//        fileOut << QString::number(vertexSet.colorArray.positionList[position].w()) << " ";
//    }
//    fileOut << "</float_array>" << Qt::endl;
//    fileOut << "          <technique_common>" << Qt::endl;
//    fileOut << "            <accessor source=\"#"+ meshName +"-mesh-colors-array\" count=\"" + QString::number(vertexSet.colorArray.positionList.size()) + "\" stride=\"4\">" << Qt::endl;
//    fileOut << "              <param name=\"R\" type=\"float\"/>" << Qt::endl;
//    fileOut << "              <param name=\"G\" type=\"float\"/>" << Qt::endl;
//    fileOut << "              <param name=\"B\" type=\"float\"/>" << Qt::endl;
//    fileOut << "              <param name=\"A\" type=\"float\"/>" << Qt::endl;
//    fileOut << "            </accessor>" << Qt::endl;
//    fileOut << "          </technique_common>" << Qt::endl;
//    fileOut << "        </source>" << Qt::endl;

    fileOut << "        <vertices id=\"" + meshName + "-mesh-vertices\">" << Qt::endl;
    fileOut << "          <input semantic=\"POSITION\" source=\"#"+ meshName +"-mesh-positions\"/>" << Qt::endl;
    fileOut << "        </vertices>" << Qt::endl;
    fileOut << "        <triangles material=\"" + checkTexture +"Texture-material\" count=\"" + QString::number(triangleCount) + "\">" << Qt::endl;
    fileOut << "          <input semantic=\"VERTEX\" source=\"#"+ meshName +"-mesh-vertices\" offset=\"0\"/>" << Qt::endl;
    fileOut << "          <input semantic=\"NORMAL\" source=\"#"+ meshName +"-mesh-normals\" offset=\"0\"/>" << Qt::endl;
    //fileOut << "          <input semantic=\"COLOR\" source=\"#"+ meshName +"-mesh-texcoords\" offset=\"0\"/>" << Qt::endl; //not sure if this is even an option in DAE
    fileOut << "          <input semantic=\"TEXCOORD\" source=\"#"+ meshName +"-mesh-map-0\" offset=\"0\" set=\"0\"/>" << Qt::endl;

    fileOut << "          <p>";
    for(int element = chosenLOD[0]; element <= chosenLOD[1]; element++){
        for (int strip = 0; strip < elementArray.elementArray[element].meshFaceSet.indexArray.triangleStrips.size(); strip++){
            for (int triangle = 0 ; triangle < elementArray.elementArray[element].meshFaceSet.indexArray.triangleStrips[strip].stripIndecies.size()-2; triangle++ ){
                fileOut << QString::number(elementArray.elementArray[element].meshFaceSet.indexArray.triangleStrips[strip].stripIndecies[triangle]) << " ";
                fileOut << QString::number(elementArray.elementArray[element].meshFaceSet.indexArray.triangleStrips[strip].stripIndecies[triangle+1]) << " ";
                fileOut << QString::number(elementArray.elementArray[element].meshFaceSet.indexArray.triangleStrips[strip].stripIndecies[triangle+2]) << " ";
            }
        }
    }
    fileOut << "</p>" << Qt::endl;
    fileOut << "        </triangles>" << Qt::endl;
    fileOut << "      </mesh>" << Qt::endl;
    fileOut << "    </geometry>" << Qt::endl;
}

void Mesh::writeNodesDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD;
    if (elementArray.lodInfo.targetIndecies.size() <= file->parent->ListLevels->currentIndex()) {
        chosenLOD = {0,static_cast<int>(elementArray.elementArray.size()-1)};
    } else {
        chosenLOD = elementArray.lodInfo.targetIndecies[file->parent->ListLevels->currentText().toInt(nullptr, 10)-1];
    }

    QString meshName = file->fileWithoutExtension + "-" + name;

    QString checkTexture;
    for(int element = chosenLOD[0]; element <= chosenLOD[1]; element++){
        checkTexture = elementArray.elementArray[element].surfaceProperties.textureName;
    }
    if(checkTexture != elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName){
        qDebug() << Q_FUNC_INFO << "checktexture" << checkTexture << "element texture" << elementArray.elementArray[chosenLOD[0]].surfaceProperties.textureName;
        //file->parent->messageError("Mesh " + name + " has different textures for element " + QString::number(chosenLOD[0]) + " and " + QString::number(chosenLOD[1]) + ". Let Trevor know.");
        //return;
    }

    fileOut << "      <node id=\"" + meshName + "\" name=\"" + meshName + "\" type=\"NODE\">" << Qt::endl;
    fileOut << "        <matrix sid=\"transform\">1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1</matrix>" << Qt::endl;
    fileOut << "        <instance_geometry url=\"#" + meshName + "-mesh\" name=\"" + meshName + "\">" << Qt::endl;
    fileOut << "          <bind_material>" << Qt::endl;
    fileOut << "            <technique_common>" << Qt::endl;
    fileOut << "              <instance_material symbol=\"" + checkTexture +"Texture-material\" target=\"#" + checkTexture +"Texture-material\">" << Qt::endl;
    fileOut << "                <bind_vertex_input semantic=\"" + meshName + "-mesh-texcoords\" input_semantic=\"TEXCOORD\" input_set=\"0\"/>" << Qt::endl;
    fileOut << "              </instance_material>" << Qt::endl;
    fileOut << "            </technique_common>" << Qt::endl;
    fileOut << "          </bind_material>" << Qt::endl;
    fileOut << "        </instance_geometry>" << Qt::endl;
    fileOut << "      </node>" << Qt::endl;
}

int IndexArray::getDAETriangleCount(){
    int triangleCount = 0;
    for(int strip = 0; strip < triangleStrips.size(); strip++){
        triangleCount += triangleStrips[strip].stripIndecies.size();
    }
    return triangleCount;
    //return triangleStrips.size();
}

void Mesh::applyKeyframe(QVector3D keyOffset){
    for(int position = 0; position < vertexSet.positionArray.positionList.size(); position++){
        vertexSet.positionArray.positionList[position] += keyOffset;
    }
}

void Mesh::applyKeyframe(QQuaternion keyRotation){
    for(int position = 0; position < vertexSet.positionArray.positionList.size(); position++){
        vertexSet.positionArray.positionList[position] = keyRotation.rotatedVector(vertexSet.positionArray.positionList[position]);
    }
}
