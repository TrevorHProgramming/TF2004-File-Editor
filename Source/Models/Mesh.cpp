#include "Headers/Main/mainwindow.h"

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
    headerData.clear();
    vertexSet.positionArray.clear();
    elementArray.lodInfo.clear();
    mods.clear();
    sectionList.clear();
    sectionTypes.clear();
    file = nullptr;
    fileData = nullptr;
}

const void Mesh::operator=(Mesh input){
    headerData = input.headerData;
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

void Material::debug(){
//    qDebug() << Q_FUNC_INFO << "sectionLength" << sectionLength;
//    qDebug() << Q_FUNC_INFO << "version" << version;
//    qDebug() << Q_FUNC_INFO << "nameLength" << nameLength;
//    qDebug() << Q_FUNC_INFO << "name" << name;
//    qDebug() << Q_FUNC_INFO << "diffuse" << diffuse;
//    qDebug() << Q_FUNC_INFO << "specular" << specular;
//    qDebug() << Q_FUNC_INFO << "ambient" << ambient;
//    qDebug() << Q_FUNC_INFO << "specularPower" << specularPower;
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
    SectionHeader signature;
    int indexEnd = 0;
    float x_position = 0;
    float y_position = 0;
    float z_position = 0;
    float a_position = 0;
    //qDebug() << Q_FUNC_INFO << "Do mesh stuff";
    elementCount = fileData->readInt();
    fileData->signature(&signature); //get extended info
    if(signature.type == "~ExtendedInfo"){
        fileData->currentPosition = signature.sectionLength + signature.sectionLocation;
        fileData->signature(&signature); //then read vertex set
    }
    if (signature.type == "~VertexSet") {
        //fileData->currentPosition += subSectionLength - 4;
        vertexSet.vertexCount = fileData->readInt();
        //qDebug() << Q_FUNC_INFO << "reading unknown values at" << fileData->currentPosition;
        int unknown4Byte1 = fileData->readInt();
        int unknown4Byte2 = fileData->readInt();
        //qDebug() << Q_FUNC_INFO << "unknown1" << unknown4Byte1 << "unknown2" << unknown4Byte2;

        //Position Array
        fileData->signature(&signature);
        if (signature.type == "~PositionArray") {
            for(int position = 0; position < (signature.sectionLength-4)/12; position++){
                x_position = fileData->readFloat();
                //qDebug() << Q_FUNC_INFO << "X float: " << x_position << " from hex: " << file->parent->binChanger.reverse_input(fileData->mid(positionLocation + (i*12), 4).toHex(), 2);
                y_position = fileData->readFloat();
                z_position = fileData->readFloat();
                vertexSet.positionArray.positionList.push_back(QVector3D(x_position, y_position, z_position));
            }

            fileData->signature(&signature);
        } else {
            //qDebug() << Q_FUNC_INFO << "Model does not have PositionArray";
        }


        //Normal Array
        if (signature.type == "~NormalArray") {
            for(int position = 0; position < (signature.sectionLength-4)/12; position++){
                vertexSet.normalArray.positionList.push_back(fileData->read3DVector());
            }

            fileData->signature(&signature);
        } else {
            //qDebug() << Q_FUNC_INFO << "Model does not have NormalArray";
        }


        //Color Array
        if (signature.type == "~ColorArray") {
            for(int position = 0; position < (signature.sectionLength-4)/16; position++){
                vertexSet.colorArray.positionList.push_back(fileData->read4DVector());
            }

            fileData->signature(&signature);
        } else {
            //qDebug() << Q_FUNC_INFO << "Model does not have ColorArray.";
        }


        //Texture Coords
        if (signature.type == "~TextureCoords") {
            //qDebug() << Q_FUNC_INFO << "getting texture coords for" << headerData.name;
            for(int position = 0; position < (signature.sectionLength-4)/8; position++){
                x_position = fileData->readFloat();
                y_position = 1.0 - fileData->readFloat();
                vertexSet.textureCoords.positionList.push_back(QVector2D(x_position, y_position));
            }
            //qDebug() << Q_FUNC_INFO << vertexSet.textureCoords.positionList;
            fileData->signature(&signature);
        } else {
            //qDebug() << Q_FUNC_INFO << "Model does not have TextureCoords.";
        }
    } else {
        qDebug() << Q_FUNC_INFO << "Model does not have VertexSet. Expected at " << fileData->currentPosition << ": " << signature.type << ". Moving on.";
    }



    //Element Array
    if (signature.type != "~ElementArray") {
        qDebug() << Q_FUNC_INFO << "Expected ElementArray. Unexpected section found at " << fileData->currentPosition << ": " << signature.type;
        return 1;
    }
    //ElementArray *elementArray = new ElementArray;
    for(int readElement = 0; readElement < elementCount; readElement++){
        Element *element = new Element;
        fileData->signature(&signature);
        element->meshFaceSet.indexArray.triangleCount = 0;
        //need to check for LODINFO bit to see if we even read LODInfo
        if (signature.type == "~LODInfo") {
            qDebug() << Q_FUNC_INFO << "Reached LOD Info at " << fileData->currentPosition << ": " << signature.type;
            break;
        }
        element->version = fileData->readInt();

        fileData->signature(&signature);
        if (signature.type == "~MeshFaceSet") {
            element->meshFaceSet.version = fileData->readInt();
            element->meshFaceSet.indexCount = fileData->readInt();
            element->meshFaceSet.primitiveType = fileData->readInt();

            fileData->signature(&signature);
        } else {
            qDebug() << Q_FUNC_INFO << "Model does not have MeshFaceSet. Unexpected section found at " << fileData->currentPosition << ": " << signature.type;
        }

        if (signature.type == "~IndexArray") {
            indexEnd = signature.sectionLength + signature.sectionLocation;
            //qDebug() << Q_FUNC_INFO << "subsectionlength" << subSectionLength << "indexEnd" << indexEnd;
            //skipping IndexArray for now
            //fileData->currentPosition += subSectionLength - 4;
            while(fileData->currentPosition < indexEnd){
                TriangleStrip *triangleStrip = new TriangleStrip;
                if(element->meshFaceSet.primitiveType == 1){
                    triangleStrip->stripIndecies.resize(fileData->readInt(2));
                    for (int triangle = 0; triangle<triangleStrip->stripIndecies.size();triangle++){
                        element->meshFaceSet.indexArray.triangleCount += 1;
                        triangleStrip->stripIndecies[triangle] = fileData->readInt(2);
                    }
                } else {
                    triangleStrip->stripIndecies.resize(element->meshFaceSet.indexCount);
                    for (int triangle = 0; triangle<triangleStrip->stripIndecies.size();triangle++){
                        element->meshFaceSet.indexArray.triangleCount += 1;
                        triangleStrip->stripIndecies[triangle] = fileData->readInt(2);
                    }
                }
                element->meshFaceSet.indexArray.triangleStrips.push_back(*triangleStrip);
            }

            //qDebug() << Q_FUNC_INFO << "Triangle strip count for this mesh is " << element->meshFaceSet.indexArray.triangleStrips.size();
//            for(int i = 0; i < element->meshFaceSet.indexArray.triangleStrips.size(); i++){
//                qDebug() << Q_FUNC_INFO << "Triangle Strip " << i << ": " << element->meshFaceSet.indexArray.triangleStrips[i].stripIndecies.size();
//                for(int j = 0; j < element->meshFaceSet.indexArray.triangleStrips[i].stripIndecies.size(); j++){
//                    qDebug() << Q_FUNC_INFO << "index" << j << ":" << vertexSet.positionArray.positionList[element->meshFaceSet.indexArray.triangleStrips[i].stripIndecies[j]];
//                }
//            }

            fileData->signature(&signature);
        } else {
            qDebug() << Q_FUNC_INFO << "Expected IndexArray. Unexpected section found at " << fileData->currentPosition << ": " << signature.type;
        }

        if (signature.type != "~SurfaceProperties") {
            qDebug() << Q_FUNC_INFO << "Expected SurfaceProperties. Unexpected section found at " << fileData->currentPosition << ": " << signature.type;
            return 1;
        }
        element->surfaceProperties.version = fileData->readInt(); //seems like a version number
        int nameLength = fileData->readInt();
        fileData->hexValue(&element->surfaceProperties.textureName, nameLength);
        //element->surfaceProperties.textureName = fileData->readHex(nameLength);
        nameLength = fileData->readInt();
        fileData->hexValue(&element->surfaceProperties.texture2Name, nameLength);
        //element->surfaceProperties.texture2Name = fileData->readHex(nameLength);
        //qDebug() << Q_FUNC_INFO << "texture for mesh" << name << "read as" << element->surfaceProperties.textureName;

        fileData->signature(&signature);
        if (signature.type != "~Material") {
            qDebug() << Q_FUNC_INFO << "Expected Material. Unexpected section found at " << fileData->currentPosition << ": " << signature.type;
            return 1;
        }
        element->surfaceProperties.material.sectionLength = signature.sectionLength;

        if(element->surfaceProperties.version == 5){
            //materialRelated 3 is seen on TFA/PICKUPS models, 4 is seen on LightningRod
            element->surfaceProperties.material.version = fileData->readInt();

            element->surfaceProperties.material.nameLength = fileData->readInt();
            fileData->hexValue(&element->surfaceProperties.material.name, element->surfaceProperties.material.nameLength);
            //element->surfaceProperties.material.name = fileData->readHex(element->surfaceProperties.material.nameLength);
        }

        element->surfaceProperties.material.diffuse = fileData->read4DVector();
        element->surfaceProperties.material.specular = fileData->read4DVector();
        element->surfaceProperties.material.ambient = fileData->read4DVector();
        element->surfaceProperties.material.specularPower = fileData->readFloat();

        element->surfaceProperties.material.debug();

        fileData->signature(&signature);
        if (signature.type != "~RenderStateGroup") {
            qDebug() << Q_FUNC_INFO << "Expected RenderStateGroup. Unexpected section found at " << fileData->currentPosition << ": " << signature.type;
            return 1;
        }
        element->surfaceProperties.renderStateGroup1.version = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.pixelBlend = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.textureBlend = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.textureClamp = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.textureFlip = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.textureFilter = fileData->readInt();
        element->surfaceProperties.renderStateGroup1.cullMode = fileData->readInt();

        //qDebug() << Q_FUNC_INFO << "material version number" << element->surfaceProperties.materialRelated;
        if(element->surfaceProperties.version > 2){
            //materialRelated 3 is seen on TFA/PICKUPS models
            fileData->signature(&signature);
            if (signature.type != "~RenderStateGroup") {
                qDebug() << Q_FUNC_INFO << "Expected RenderStateGroup. Unexpected section found at " << fileData->currentPosition << ": " << signature.type;
                return 1;
            }
            element->surfaceProperties.renderStateGroup2.version = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.pixelBlend = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.textureBlend = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.textureClamp = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.textureFlip = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.textureFilter = fileData->readInt();
            element->surfaceProperties.renderStateGroup2.cullMode = fileData->readInt();
        }

        element->attributes = fileData->readInt();

        if(element->surfaceProperties.version > 3){
            element->attributes = fileData->readInt();
        }

        elementArray.elementArray.push_back(*element);
    }

    bool hasLodInfo = false;
    //qDebug() << Q_FUNC_INFO << "checking for lod info at" << fileData->currentPosition;
    if(elementArray.elementArray[0].surfaceProperties.version > 2){
        //the use of materialrelated here is absolutely horrible. Find another version number of property byte that can show whether there are LODs or not.
        hasLodInfo = fileData->readBool();
    }
    //qDebug() << Q_FUNC_INFO << "has lod info:" << hasLodInfo;
    if (hasLodInfo) {
        fileData->signature(&signature);
        elementArray.lodInfo.levels = fileData->readInt();
        std::vector<int> levelTargetArray;
        for (int readLevels = 0; readLevels < elementArray.lodInfo.levels; readLevels++) {
            levelTargetArray.push_back(std::max(fileData->readInt(), 0));
            levelTargetArray.push_back(std::max(fileData->readInt(), 0));
            elementArray.lodInfo.targetIndecies.push_back(levelTargetArray);
            levelTargetArray.clear();
            elementArray.lodInfo.levelDistance.push_back(fileData->readFloat());
        }
        if (elementArray.lodInfo.levels > file->highestLOD){
            file->highestLOD = elementArray.lodInfo.levels;
        }
    }

    qDebug() << Q_FUNC_INFO << "finished reading mesh at " << fileData->currentPosition;
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

//template<typename WriteType>
void FileSection::searchListsWriteDAE(QTextStream &fileOut, void (Mesh::*write)(QTextStream&)) {
    for (int i = 0; i < meshList.size(); i++) {
        meshList[i]->file = file;
        (meshList[i]->*write)(fileOut);
        //write(meshList[i], fileOut);
        meshList[i]->searchListsWriteDAE(fileOut, write);
    }
    for (int i = 0; i < sectionList.size(); i++) {
        sectionList[i]->file = file;
        sectionList[i]->searchListsWriteDAE(fileOut, write);
    }
}

void Mesh::writeMaterialsDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD = getChosenElements();
    QString textureName;
    for (int element = chosenLOD[0]; element <= chosenLOD[1]; element++) {
        textureName = elementArray.elementArray[element].surfaceProperties.textureName;
        fileOut << "    <material id=\"" + textureName +"Texture-material\" name=\"" + textureName + "Texture\">" << Qt::endl;
        fileOut << "      <instance_effect url=\"#" + textureName + "Texture-effect\"/>" << Qt::endl;
        fileOut << "    </material>" << Qt::endl;
    }
}

void Mesh::writeEffectsDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD = getChosenElements();
    QString textureName;
    QString meshName = file->fileName + "-" + headerData.name;
    meshName = meshName.right(meshName.length() - (meshName.indexOf(".")+1));
    for (int element = chosenLOD[0]; element <= chosenLOD[1]; element++) {
        //qDebug() << Q_FUNC_INFO << "element array";
        textureName = elementArray.elementArray[element].surfaceProperties.textureName;
        fileOut << "    <effect id=\"" + textureName +"Texture-effect\">" << Qt::endl;
        fileOut << "      <profile_COMMON>" << Qt::endl;
        fileOut << "        <newparam sid=\"" + textureName +"_bmp-surface\">" << Qt::endl;
        fileOut << "          <surface type=\"2D\">" << Qt::endl;
        fileOut << "            <init_from>" + textureName +"_bmp</init_from>" << Qt::endl;
        fileOut << "          </surface>" << Qt::endl;
        fileOut << "        </newparam>" << Qt::endl;
        fileOut << "        <newparam sid=\"" + textureName +"_bmp-sampler\">" << Qt::endl;
        fileOut << "          <sampler2D>" << Qt::endl;
        fileOut << "            <source>" + textureName +"_bmp-surface</source>" << Qt::endl;
        fileOut << "          </sampler2D>" << Qt::endl;
        fileOut << "        </newparam>" << Qt::endl;
        fileOut << "        <technique sid=\"common\">" << Qt::endl;
        fileOut << "          <lambert>" << Qt::endl;
        fileOut << "            <emission>" << Qt::endl;
        fileOut << "              <color sid=\"emission\">0 0 0 1</color>" << Qt::endl;
        fileOut << "            </emission>" << Qt::endl;
        fileOut << "            <diffuse>" << Qt::endl;
        fileOut << "              <texture texture=\"" + textureName +"_bmp-sampler\" texcoord=\""+meshName+"-mesh-texcoords\"/>" << Qt::endl;
        fileOut << "            </diffuse>" << Qt::endl;
        fileOut << "            <index_of_refraction>" << Qt::endl;
        fileOut << "              <float sid=\"ior\">1.45</float>" << Qt::endl;
        fileOut << "            </index_of_refraction>" << Qt::endl;
        fileOut << "          </lambert>" << Qt::endl;
        fileOut << "        </technique>" << Qt::endl;
        fileOut << "      </profile_COMMON>" << Qt::endl;
        fileOut << "    </effect>" << Qt::endl;
    }
}

void Mesh::writeImagesDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD = getChosenElements();
    QString textureName;
    for (int element = chosenLOD[0]; element <= chosenLOD[1]; element++) {
        textureName = elementArray.elementArray[element].surfaceProperties.textureName;
        fileOut << "    <image id=\"" + textureName +"_bmp\" name=\"" + textureName +"_bmp\">" << Qt::endl;
        fileOut << "      <init_from>" + textureName +".bmp</init_from>" << Qt::endl;
        fileOut << "    </image>" << Qt::endl;
    }
}

void Mesh::writeDataDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD = getChosenElements();
    QString meshName = file->fileName + "-" + headerData.name;
    meshName = meshName.right(meshName.length() - (meshName.indexOf(".")+1));
    qDebug() << Q_FUNC_INFO << meshName;
    QString textureName;
    int triangle[3];
    QVector3D tempVec;

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
//    fileOut << "          <float_array id=\"" + meshName + "-mesh-colors-array\" count = \"" + QString::number(mesh->vertexSet.colorArray.positionList.size()*4) + "\">";
//    for(int position = 0; position < mesh->vertexSet.colorArray.positionList.size(); position++){
//        fileOut << QString::number(mesh->vertexSet.colorArray.positionList[position].x()) << " ";
//        fileOut << QString::number(mesh->vertexSet.colorArray.positionList[position].y()) << " ";
//        fileOut << QString::number(mesh->vertexSet.colorArray.positionList[position].z()) << " ";
//        fileOut << QString::number(mesh->vertexSet.colorArray.positionList[position].w()) << " ";
//    }
//    fileOut << "</float_array>" << Qt::endl;
//    fileOut << "          <technique_common>" << Qt::endl;
//    fileOut << "            <accessor source=\"#"+ meshName +"-mesh-colors-array\" count=\"" + QString::number(mesh->vertexSet.colorArray.positionList.size()) + "\" stride=\"4\">" << Qt::endl;
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

    for (int element = chosenLOD[0]; element <= chosenLOD[1]; element++) {
        int triangleCount = 0;
        Element *currentElement = &elementArray.elementArray[element];
        textureName = currentElement->surfaceProperties.textureName;
        for(int strip = 0; strip < currentElement->meshFaceSet.indexArray.triangleStrips.size(); strip++){
            triangleCount += currentElement->meshFaceSet.indexArray.triangleStrips[strip].stripIndecies.size();
        }
        fileOut << "        <triangles material=\"" + textureName +"Texture-material\" count=\"" + QString::number(triangleCount) + "\">" << Qt::endl;
        fileOut << "          <input semantic=\"VERTEX\" source=\"#"+ meshName +"-mesh-vertices\" offset=\"0\"/>" << Qt::endl;
        fileOut << "          <input semantic=\"NORMAL\" source=\"#"+ meshName +"-mesh-normals\" offset=\"0\"/>" << Qt::endl;
        //fileOut << "          <input semantic=\"COLOR\" source=\"#"+ meshName +"-mesh-texcoords\" offset=\"0\"/>" << Qt::endl; //not sure if this is even an option in DAE
        fileOut << "          <input semantic=\"TEXCOORD\" source=\"#"+ meshName +"-mesh-map-0\" offset=\"0\" set=\"0\"/>" << Qt::endl;

        fileOut << "          <p>";
        for (int strip = 0; strip < currentElement->meshFaceSet.indexArray.triangleStrips.size(); strip++){
            for (int triangle = 0 ; triangle < currentElement->meshFaceSet.indexArray.triangleStrips[strip].stripIndecies.size()-2; triangle++ ){
                fileOut << QString::number(currentElement->meshFaceSet.indexArray.triangleStrips[strip].stripIndecies[triangle]) << " ";
                fileOut << QString::number(currentElement->meshFaceSet.indexArray.triangleStrips[strip].stripIndecies[triangle+1]) << " ";
                fileOut << QString::number(currentElement->meshFaceSet.indexArray.triangleStrips[strip].stripIndecies[triangle+2]) << " ";
            }
        }
        fileOut << "</p>" << Qt::endl;
        fileOut << "        </triangles>" << Qt::endl;
    }
    fileOut << "      </mesh>" << Qt::endl;
    fileOut << "    </geometry>" << Qt::endl;
}

void Mesh::writeNodesDAE(QTextStream &fileOut){
    std::vector<int> chosenLOD = getChosenElements();
    QString textureName;
    QString meshName = file->fileName + "-" + headerData.name;
    meshName = meshName.right(meshName.length() - (meshName.indexOf(".")+1));
    fileOut << "      <node id=\"" + meshName + "\" name=\"" + meshName + "\" type=\"NODE\">" << Qt::endl;
    fileOut << "        <matrix sid=\"transform\">1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1</matrix>" << Qt::endl;
    fileOut << "        <instance_geometry url=\"#" + meshName + "-mesh\" name=\"" + meshName + "\">" << Qt::endl;

    for (int element = chosenLOD[0]; element <= chosenLOD[1]; element++) {
        textureName = elementArray.elementArray[element].surfaceProperties.textureName;
        fileOut << "          <bind_material>" << Qt::endl;
        fileOut << "            <technique_common>" << Qt::endl;
        fileOut << "              <instance_material symbol=\"" + textureName +"Texture-material\" target=\"#" + textureName +"Texture-material\">" << Qt::endl;
        fileOut << "                <bind_vertex_input semantic=\"" + meshName + "-mesh-texcoords\" input_semantic=\"TEXCOORD\" input_set=\"0\"/>" << Qt::endl;
        fileOut << "              </instance_material>" << Qt::endl;
        fileOut << "            </technique_common>" << Qt::endl;
        fileOut << "          </bind_material>" << Qt::endl;
    }
    fileOut << "        </instance_geometry>" << Qt::endl;
    fileOut << "      </node>" << Qt::endl;
}

std::vector<int> Mesh::getChosenElements(){
    std::vector<int> chosenLOD;
    if (elementArray.lodInfo.targetIndecies.size() <= file->selectedLOD) {
        chosenLOD = {0,static_cast<int>(elementArray.elementArray.size()-1)};
    } else {
        chosenLOD = elementArray.lodInfo.targetIndecies[file->selectedLOD];
    }

    return chosenLOD;
}

void Mesh::writeDataSTL(QTextStream &fileOut){
    std::vector<int> chosenLOD;
    int triangle[3];
    QVector3D tempVec;

    //qDebug() << Q_FUNC_INFO << "mesh file" << file->fileName;
    //qDebug() << Q_FUNC_INFO << "lodinfo list size" << elementArray.lodInfo.targetIndecies.size() << "chosen lod" << file->parent->ListLevels->currentIndex();
    if (elementArray.lodInfo.targetIndecies.size() <= file->selectedLOD) {
        chosenLOD = {0,static_cast<int>(elementArray.elementArray.size()-1)};
    } else {
        chosenLOD = elementArray.lodInfo.targetIndecies[file->selectedLOD];
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
