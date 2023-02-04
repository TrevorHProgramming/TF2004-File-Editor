#include "mainwindow.h"

/*
The overall plan with VBIN files is to read through them section by section. The previous code looked for specific sections (positionarrays and indexarrays)
and used those to build the mesh, but this is a cheap shortcut that prevented progress for quite some time.
Reading the file in order, as you do with files, means it'll be easier to track down how mesh and node offsets are handled.
*/



void SceneNode::clear(){
    fileLocation = 0;

    mods.clear();
    sectionTypes.clear();
    sectionList.clear();
    name = "";
    sectionLength = 0;
    sectionEnd = 0;

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
    file->parent->fileData.currentPosition = fileLocation + 25;
    hasVolume = file->parent->fileData.readInt(1);
    type = file->parent->fileData.readInt();
    location.setX(file->parent->fileData.readFloat());
    location.setY(file->parent->fileData.readFloat());
    location.setZ(file->parent->fileData.readFloat());
    radius = file->parent->fileData.readFloat();
    qDebug() << Q_FUNC_INFO << "location" << fileLocation << "radius" << radius << "hasvolume" << hasVolume << "type" << type;
}

int VBIN::readData(){

    //gets all scene nodes in the file, populates their PositionArrays and IndexArrays, and gets modifications
    currentLocation = 0;
    base.file = this;
    //base.getSceneNodeTree(0, parent->fileData.dataBytes.length(), 0);
    if(getSceneNodeTree()){
        qDebug() << Q_FUNC_INFO << "Error while reading scene node tree";
        return 1;
    }
    //qDebug() << Q_FUNC_INFO << "section list size" << base.sectionList.size() << "mesh list size" << base.meshList.size();
    //base.printInfo(0);

    //qDebug() << Q_FUNC_INFO << "section list length" << base.sectionList.size() << "typelist length" << base.sectionTypes.size();

//    Modifications baseMods;
//    baseMods.clear();
//    base.mods = baseMods;
//    std::vector<Modifications> baseModList;
//    baseModList.push_back(baseMods);

    //qDebug() << Q_FUNC_INFO << "base mod values" << baseMods.offset << baseMods.rotation << baseMods.scale;

//    for(int i = 0; i < base.sectionList.size(); i++){
//        qDebug() << Q_FUNC_INFO << "layer 0," << i <<"is" << base.sectionList[i]->name << "with rotation" << base.sectionList[i]->mods.rotation << "and scale" << base.sectionList[i]->mods.scale;
//        base.sectionList[i]->printInfo(1);
//    }

    //base.modifyPosArrays(baseModList);


    qDebug() << Q_FUNC_INFO << "Successfully loaded the scene node tree.";

    //modify every position set based on the modifications that were read
    //qDebug() << Q_FUNC_INFO << "Node list size (read data): " << this->nodeList.size();
//    for (int i = 0; i < int(this->nodeList.size()); i++) {
//        this->nodeList[i].modifyPosArrays();
//    }
    //this->modifyPosArrays();

    //qDebug() << Q_FUNC_INFO << "Position arrays modified.";

    return 0;
}

void FileSection::printInfo(int depth){
    //qDebug() << Q_FUNC_INFO << name << "section list size" << sectionList.size() << "mesh list size" << meshList.size();
    //qDebug() << Q_FUNC_INFO << "file name" << file->fileName;
    if(depth > 0){
        //qDebug() << Q_FUNC_INFO << "depth" << depth << "section name" << name << "with parent" << parent->name;
    }
    for(int i = 0; i < sectionList.size(); i++){
        sectionList[i]->printInfo(depth+1);
    }
    for(int i = 0; i < meshList.size(); i++){
        meshList[i]->printInfo(depth+1);
    }
}

void FileSection::modifyPosArrays(std::vector<Modifications> addedMods){
    QMatrix3x3 rotMatrix;
    Mesh modifyMesh;
    SceneNode modifyNode;
    //QMatrix4x4 rotMatrix4;
    //QVector4D expandVector;
    //stackedMods.rotation = mods.rotation * addedMods.rotation;
    //stackedMods.scale = mods.scale * addedMods.scale;
    //qDebug() << Q_FUNC_INFO << "Getting modifications for " << name << " current scale: " << mods.scale << "passed mods:" << addedMods.scale << "new scale:" << stackedMods.scale;
    addedMods.push_back(mods);

    for(int i = 0; i < sectionList.size(); i++){
        if(sectionTypes[i] == "Mesh"){
            sectionList[i]->modify(addedMods);
//            if(sectionList[i].sectionList.size() > 0){
//                sectionList[i].modifyPosArrays(addedMods);
//            }
//            for (int j = 0; j < sectionList[i].posArray.vertexCount; ++j) {
//                for(int k = 0; k < addedMods.size(); k++){
//                    //rotMatrix = std::get<Mesh>(sectionList[i]).mods.rotation.toRotationMatrix();
//                    //expandVector = QVector4D(std::get<Mesh>(sectionList[i]).posArray.positionList[i]);
//                    //rotMatrix4 = QMatrix4x4(rotMatrix);

//                    sectionList[i].posArray.positionList[j] = sectionList[i].posArray.positionList[j] * addedMods[k].scale;
//                    sectionList[i].posArray.positionList[j] = sectionList[i].posArray.positionList[j] + addedMods[k].offset;
//                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = file->parent->binChanger.forcedRotate(rotMatrix, addedMods[k].offset, std::get<Mesh>(sectionList[i]).posArray.positionList[j]);
//                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = QVector3D(expandVector*rotMatrix4);
//                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = mods.rotation.rotatedVector(std::get<Mesh>(sectionList[i]).posArray.positionList[j]);
//                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = addedMods[k].rotation * std::get<Mesh>(sectionList[i]).posArray.positionList[j];
//                    //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = mods.rotation * std::get<Mesh>(sectionList[i]).posArray.positionList[j];
//                }
//                sectionList[i].posArray.positionList[j] = sectionList[i].posArray.positionList[j] * sectionList[i].mods.scale;
//                sectionList[i].posArray.positionList[j] = sectionList[i].posArray.positionList[j] + sectionList[i].mods.offset;
//                //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = std::get<Mesh>(sectionList[i]).mods.rotation * std::get<Mesh>(sectionList[i]).posArray.positionList[j];
//                //std::get<Mesh>(sectionList[i]).posArray.positionList[j] = std::get<Mesh>(sectionList[i]).mods.rotation.rotatedVector(std::get<Mesh>(sectionList[i]).posArray.positionList[j]);
//            }

        } else {
            if(sectionList[i]->sectionList.size() > 0){
                sectionList[i]->modifyPosArrays(addedMods);
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
    sectionLength = input.sectionLength;
}


void VBIN::readAnimationPrototype(){
    QString signature;
    int subSectionLength = 0;
    float unknownFloat1 = parent->fileData.readFloat();
    int unknown4Byte4 = parent->fileData.readInt();
    int unknown4Byte5 = parent->fileData.readInt();

    signature = parent->fileData.getSignature();
    if(signature == "~NeutralData"){
        subSectionLength = parent->fileData.readInt();
        parent->fileData.currentPosition += subSectionLength-4;
    }
}

void VBIN::readBoundingVolume(){
    QString signature;
    int subSectionLength = 0;
    subSectionLength = parent->fileData.readInt();
    //qDebug() << Q_FUNC_INFO << "bounding 1 section length read as" << subSectionLength << "at " << file->parent->fileData.currentPosition - 4;
    parent->fileData.currentPosition += subSectionLength-4;

    signature = parent->fileData.getSignature();
    //this is such a bad way to do this but w/e
    if(signature == "~BoundingVolume"){
        subSectionLength = parent->fileData.readInt();
        //qDebug() << Q_FUNC_INFO << "bounding 2 section length read as" << subSectionLength << "at " << file->parent->fileData.currentPosition - 4;
        parent->fileData.currentPosition += subSectionLength-4;
    }
}

void VBIN::readModifications(){
    int offsets = 0;
    //offset properties
    offsets = parent->fileData.readInt(1);
    if (!(offsets & 1)) {
        //position offset
        parent->fileData.currentPosition += 12;
    }
    if (!(offsets & 2)) {
        //rotation offset
        parent->fileData.currentPosition += 16;
    }
    if (!(offsets & 4)) {
        //scale offset
        parent->fileData.currentPosition += 4;
    }
}

void FileSection::readNode(){

}

int VBIN::getSceneNodeTree(){
    parent->fileData.currentPosition = 4; //moving past FISH
    //not a huge fan of while(1)'s but I'll put a loopbreaker in just to be safe
    int loopBreaker = 0;
    QString signature;
    int sectionNameLength = 0;
    QString sectionName;
    int sectionLength = 0;
    int unknown4Byte1 = 0;
    int unknown4Byte2 = 0;
    int unknown4Byte3 = 0;
    FileSection* currentBranch;
    FileSection* possibleBranch;

    signature = parent->fileData.getSignature();

    sectionNameLength = parent->fileData.readInt();
    sectionName = parent->fileData.readHex(sectionNameLength);
    //qDebug() << Q_FUNC_INFO << "name length " << sectionNameLength << " read as " << sectionName;

    sectionLength = parent->fileData.readInt();
    base.sectionLength = sectionLength;
    base.sectionEnd = parent->fileData.currentPosition -4 + sectionLength;
    qDebug() << Q_FUNC_INFO << "end of tree excpeted at" << base.sectionEnd;
    base.fileLocation = 0;
    currentBranch = &base;

    while(1){
        if(signature == "~anAnimationSourceSet"){
            qDebug() << Q_FUNC_INFO << "End of tree located - leaving at Animation Data";
            break;
        }

        if(signature == "~Mesh"){
            Mesh *meshSection = new Mesh();
            meshSection->file = this; //file will need to be reassigned later as the current VBIN object is temporary
            meshSection->parent = currentBranch;
            meshSection->fileLocation = parent->fileData.currentPosition-4;
            meshSection->sectionLength = sectionLength;
            meshSection->sectionEnd = meshSection->fileLocation + sectionLength;
            meshSection->name = sectionName;

            if (sectionName == "CollisionMesh") {
                parent->messageError("It looks like you're trying to read a Collision Mesh. "
                                     "These models have weird stuff going on and are not currently compatible with this program.");
                return 1;
            }

            if(meshSection->readMesh()){
                qDebug() << "Error while reading mesh";
                return 1;
            }

            currentBranch->meshList.push_back(meshSection);
            possibleBranch = meshSection;
            //qDebug() << Q_FUNC_INFO << "name " << meshSection->name << "with parent" << meshSection->parent->name;
        }

        if(signature == "~SceneNode"){
            SceneNode *sceneSection = new SceneNode();
            sceneSection->file = this;
            sceneSection->parent = currentBranch;
            sceneSection->fileLocation = parent->fileData.currentPosition-4;
            sceneSection->sectionLength = sectionLength;
            sceneSection->sectionEnd = sceneSection->fileLocation + sectionLength;
            sceneSection->name = sectionName;
            currentBranch->sectionList.push_back(sceneSection);
            possibleBranch = sceneSection;
            //qDebug() << Q_FUNC_INFO << "name " << sceneSection->name << "with parent" << sceneSection->parent->name;
        }

        if(signature == "~anAnimationPrototype"){
            readAnimationPrototype();
        }

        unknown4Byte1 = parent->fileData.readInt(); //looks like this value is always 4? I haven't found an exception to this
            //unfortunately means it's really hard to figure out what this is for.
        qDebug() << Q_FUNC_INFO << "section name" << sectionName << "with unknown value" << unknown4Byte1;

        readModifications();

        unknown4Byte2 = parent->fileData.readInt();
        parent->fileData.currentPosition += 8;
        unknown4Byte3 = parent->fileData.readInt(); //this should take us to the end

        signature = parent->fileData.getSignature();

        if(signature == "~BoundingVolume"){
            readBoundingVolume();
        }
        qDebug() << Q_FUNC_INFO << "Expected end of section at location: " << parent->fileData.currentPosition;

        loopBreaker++;
        if (loopBreaker > 100 or parent->fileData.currentPosition > base.sectionEnd){
            parent->messageError("Excessive looping detected or node tree exceeded for file " + fileName);
            qDebug() << Q_FUNC_INFO << "Excessive looping detected or node tree exceeded.";
            return 1;
        }

        if (parent->fileData.currentPosition == base.sectionEnd){
            qDebug() << Q_FUNC_INFO << "File does not have animation data. Exiting now.";
            return 0;
        }

        //Checks if we've reached the end of a branch in the scene node tree. If we're at the root of the tree, there's nowhere to go.
        if(currentBranch->sectionEnd == parent->fileData.currentPosition && currentBranch->fileLocation !=0){
            while(currentBranch->sectionEnd == currentBranch->parent->sectionEnd){
                currentBranch = currentBranch->parent;
                //qDebug() << Q_FUNC_INFO << "Branch completed, moving back up a layer";
            }
            currentBranch = currentBranch->parent;
        } else if (possibleBranch->sectionEnd != parent->fileData.currentPosition) {
            currentBranch = possibleBranch;
        }

        signature = parent->fileData.getSignature();

        sectionNameLength = parent->fileData.readInt();
        sectionName = parent->fileData.readHex(sectionNameLength);
        //qDebug() << Q_FUNC_INFO << "name length " << sectionNameLength << " read as " << sectionName;

        sectionLength = parent->fileData.readInt();
    }

    return 0; //read successfully
}

void ProgWindow::convertVBINToSTL(){
    if(vbinFiles.empty()){
        messageError("No VBIN files currently loaded to export.");
        return;
    }

    vbinFiles[ListFiles->currentIndex()].outputData();

    return;
}

void FileSection::writeSectionList(QTextStream &fileOut){
    //for writing to a single file
    for (int i = 0; i < int(meshList.size()); i++) {
        //qDebug() << Q_FUNC_INFO << "section" << i << "is type:" << sectionTypes[i];
        meshList[i]->file = file;
        meshList[i]->writeData(fileOut);
        meshList[i]->writeSectionList(fileOut);
    }
    for(int i = 0; i < int(sectionList.size()); i++){
        sectionList[i]->file = file;
        sectionList[i]->writeSectionList(fileOut);
    }
}

void FileSection::writeSectionList(QString path){
    //for writing to multiple files

    for (int i = 0; i < int(meshList.size()); i++) {
        QString multiFileOut = path + "/" + meshList[i]->name + ".stl";
        QFile stlOut(multiFileOut);
        QFile fileOut(multiFileOut);
        fileOut.open(QFile::WriteOnly|QFile::Truncate);
        fileOut.close();

        if (!stlOut.open(QIODevice::ReadWrite)){
            continue;
        }
        QTextStream stream(&stlOut);

        meshList[i]->file = file;
        meshList[i]->writeData(stream);
        meshList[i]->writeSectionList(path);
    }
    for(int i = 0; i < int(sectionList.size()); i++){
        sectionList[i]->file = file;
        sectionList[i]->writeSectionList(path);
    }
}

void VBIN::outputData(){

    std::vector<int> allowedMeshes = {1,2,3};
    std::vector<int> chosenLOD;

    if(parent->radioSingle->isChecked()){
        QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", parent->tr("Model Files (*.stl)"));
        QFile stlOut(fileOut);
        QFile file(fileOut);
        file.open(QFile::WriteOnly|QFile::Truncate);
        file.close();

        if(!stlOut.open(QIODevice::ReadWrite)){
            return;
        }
        QTextStream stream(&stlOut);
        stream << "solid Default" << Qt::endl;
        qDebug() << Q_FUNC_INFO << "sections:" << base.sectionList.size();
        qDebug() << Q_FUNC_INFO << "meshes:" << base.meshList.size();
        for (int i = 0; i < int(base.sectionList.size()); i++) {
            base.sectionList[i]->file = this;
            qDebug() << Q_FUNC_INFO << "writing mesh" << i << "named" << base.sectionList[i]->name << "with file" << base.sectionList[i]->file->fileName;
            base.sectionList[i]->writeSectionList(stream);
        }
        stream << "endsolid Default" << Qt::endl;
    } else {
        QString fileOut = QFileDialog::getExistingDirectory(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", QFileDialog::ShowDirsOnly);
        for (int i = 0; i < int(base.sectionList.size()); i++) {
            base.sectionList[i]->file = this;
            //qDebug() << Q_FUNC_INFO << "section" << i << "is type:" << base.sectionTypes[i];
            base.sectionList[i]->writeSectionList(fileOut);
        }
    }

    qDebug() << Q_FUNC_INFO << "Output complete.";

    return;
}

void FileSection::modify(std::vector<Modifications> addedMods){
    qDebug() << Q_FUNC_INFO << "THIS SHOULDN'T RUN";
}

void FileSection::writeData(QTextStream &fileOut){
    qDebug() << Q_FUNC_INFO << "THIS SHOULDN'T RUN";
}
