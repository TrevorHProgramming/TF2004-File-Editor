#include "Headers/Main/mainwindow.h"

/*
The overall plan with VBIN files is to read through them section by section. The previous code looked for specific sections (positionarrays and indexarrays)
and used those to build the mesh, but this is a cheap shortcut that prevented progress for quite some time.
Reading the file in order, as you do with files, means it'll be easier to track down how mesh and node offsets are handled.
*/



void SceneNode::clear(){
    headerData.clear();
    mods.clear();
    sectionTypes.clear();
    sectionList.clear();
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
    fileData->currentPosition = headerData.sectionLocation + 25;
    hasVolume = fileData->readInt(1);
    type = fileData->readInt();
    center.setX(fileData->readFloat());
    center.setY(fileData->readFloat());
    center.setZ(fileData->readFloat());
    radius = fileData->readFloat();
    qDebug() << Q_FUNC_INFO << "location" << headerData.sectionLocation << "radius" << radius << "hasvolume" << hasVolume << "type" << type;
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
    base.printInfo(0);
    std::vector<Modifications> addedMods;
    base.modifyPosArrays(addedMods);

    qDebug() << Q_FUNC_INFO << "section list size" << base.sectionList.size() << "mesh list size" << base.meshList.size();

    qDebug() << Q_FUNC_INFO << "Successfully loaded the scene node tree.";

    return 0;
}

void FileSection::printInfo(int depth){
    //qDebug() << Q_FUNC_INFO << name << "section list size" << sectionList.size() << "mesh list size" << meshList.size();
    //qDebug() << Q_FUNC_INFO << "file name" << file->fileName;
    if(depth > 0){
        qDebug() << Q_FUNC_INFO << "depth" << depth << "section name" << headerData.name << "with parent" << parent->headerData.name;
    }
    for(int i = 0; i < sectionList.size(); i++){
        sectionList[i]->printInfo(depth+1);
    }
    for(int i = 0; i < meshList.size(); i++){
        //qDebug() << Q_FUNC_INFO << meshList[i]->name << "position list has" << meshList[i]->vertexSet.positionArray.positionList.size();
        meshList[i]->printInfo(depth+1);
    }
}

void FileSection::modifyPosArrays(std::vector<Modifications> addedMods){
    QMatrix3x3 rotMatrix;
    Mesh modifyMesh;
    SceneNode modifyNode;
    if(headerData.name != "vbinbase"){
        addedMods.push_back(mods);
        //qDebug() << Q_FUNC_INFO << "Getting modifications for " << name << " current scale: " << mods.scale << "passed mods" << addedMods[addedMods.size()-1].scale;
    }

    for (int i = 0; i < int(meshList.size()); i++) {
        meshList[i]->modifyPosArrays(addedMods);
        meshList[i]->modify(addedMods);
    }
    for(int i = 0; i < int(sectionList.size()); i++){
        sectionList[i]->modifyPosArrays(addedMods);
    }
    return;
}

const void BoundingVolume::operator=(BoundingVolume input){
    headerData = input.headerData;
    hasVolume = input.hasVolume;
    type = input.type;
    center = input.center;
    radius = input.radius;
    fileData = input.fileData;
}

const void FileSection::operator=(FileSection input){
    file = input.file;
    headerData = input.headerData;
    boundVol = input.boundVol;
    mods = input.mods;
    sectionList = input.sectionList;
    sectionTypes = input.sectionTypes;
}


void VBIN::readAnimationPrototype(){
    SectionHeader signature;
    int testProto = parent->fileData.readInt();
    parent->fileData.currentPosition -= 4;
    if(testProto > 20){
        float unknownFloat1 = parent->fileData.readFloat();
        int unknown4Byte4 = parent->fileData.readInt();
        int unknown4Byte5 = parent->fileData.readInt();

        fileData->signature(&signature);
        if(signature.type == "~NeutralData"){
            parent->fileData.currentPosition = signature.sectionLength + signature.sectionLocation;
        }
    }
}

void VBIN::readBoundingVolume(SectionHeader* signature){
    //qDebug() << Q_FUNC_INFO << "current position 1" << parent->fileData.currentPosition;
    parent->fileData.currentPosition = signature->sectionLength + signature->sectionLocation;
    //qDebug() << Q_FUNC_INFO << "current position 2" << parent->fileData.currentPosition;

    fileData->signature(signature);
    //this is such a bad way to do this but w/e
    if(signature->type == "~BoundingVolume"){
        parent->fileData.currentPosition = signature->sectionLength + signature->sectionLocation;
    }
}

void FileSection::readModifications(){
    int offsets = 0;
    float m_value = 0;
    //offset properties
    offsets = fileData->readInt(1);
    //qDebug() << Q_FUNC_INFO << "offsets read as" << offsets;
    if (!(offsets & 1)) {
        //position offset
        mods.offset = fileData->read3DVector();
    } else {
        mods.offset = QVector3D(0,0,0);
    }
    if (!(offsets & 2)) {
        //rotation offset
        mods.rotation = fileData->readQuaternion();
    } else {
        mods.rotation = QQuaternion(1,0,0,0);
    }
    if (!(offsets & 4)) {
        //scale offset
        m_value = fileData->readFloat();
        mods.scale = m_value;
    } else {
        mods.scale = 1;
    }
}

void VBIN::readModifications(){
    //This only exists for the AnimationPrototype section
    //eventually AnimationPrototype needs to be a filesection like the rest
    int offsets = 0;
    //offset properties
    offsets = parent->fileData.readInt(1);
    qDebug() << Q_FUNC_INFO << "offsets read as" << offsets;
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
    fileData = &parent->fileData;
    fileData->currentPosition = 4; //moving past FISH
    //not a huge fan of while(1)'s but I'll put a loopbreaker in just to be safe
    int loopBreaker = 0;
    SectionHeader signature;
    int unknown4Byte1 = 0; //appears to be a version number
    int unknown4Byte2 = 0;
    int unknown4Byte3 = 0;
    FileSection* currentBranch;
    FileSection* possibleBranch;

    base.headerData = signature;
    base.headerData.name = "vbinbase";
    currentBranch = &base;
    possibleBranch = &base;

    while(1){

        if (parent->fileData.currentPosition == base.sectionEnd){
            qDebug() << Q_FUNC_INFO << "Reached end of tree. Exiting now.";
            return 0;
        }

        //Checks if we've reached the end of a branch in the scene node tree. If we're at the root of the tree, there's nowhere to go.
        if(currentBranch->sectionEnd == parent->fileData.currentPosition && currentBranch->headerData.sectionLocation !=0){
            while(currentBranch->sectionEnd == currentBranch->parent->sectionEnd){
                currentBranch = currentBranch->parent;
                //qDebug() << Q_FUNC_INFO << "Branch completed, moving back up a layer";
            }
            currentBranch = currentBranch->parent;
        } else if (possibleBranch->sectionEnd != parent->fileData.currentPosition) {
            currentBranch = possibleBranch;
        }

        fileData->signature(&signature);


        //qDebug() << Q_FUNC_INFO << "name length " << sectionNameLength << " read as " << sectionName;


        if (base.headerData.sectionLength == 0) {
            base.headerData.sectionLength = signature.sectionLength;
            base.sectionEnd = parent->fileData.currentPosition -4 + signature.sectionLength;
        }
        
        if(signature.type == "~Instance"){
            Instance *instanceSection = new Instance();
            instanceSection->file = this;
            instanceSection->fileData = &parent->fileData;
            instanceSection->parent = currentBranch;
            instanceSection->headerData = signature;
            instanceSection->sectionEnd = signature.sectionLength + signature.sectionLocation;
            //this should bring us to the vbin name starting position
            parent->fileData.currentPosition = 12 + signature.sectionLocation;
            QString instance_name = fileData->ProtoName();
            //read the modifications to the vbin imported file
            int instanceoffsets = 0;
            instanceoffsets = parent->fileData.readInt(1);
            qDebug() << Q_FUNC_INFO << "offsets read as" << instanceoffsets;
            if (!(instanceoffsets & 1)) {
                //position offset - this looks right
                parent->fileData.currentPosition += 12;
            }
            if (!(instanceoffsets & 2)) {
                //rotation offset - I don't think this is correct
                parent->fileData.currentPosition += 16;
            }
            if (!(instanceoffsets & 4)) {
                //scale offset - I don't think this ever happens for instances?
                parent->fileData.currentPosition += 4;
            }
            //not sure why this is yet but it works for now. Maybe they added a few 0000s? 
            parent->fileData.currentPosition += 16;
        }
        
        if(signature.type == "~Mesh"){
            Mesh *meshSection = new Mesh();
            meshSection->file = this; //file will need to be reassigned later as the current VBIN object is temporary
            meshSection->fileData = &parent->fileData;
            meshSection->parent = currentBranch;
            meshSection->headerData = signature;
            meshSection->sectionEnd = signature.sectionLength + signature.sectionLocation;

            if (signature.name == "CollisionMesh") {
                parent->messageError("It looks like you're trying to read a Collision Mesh. "
                                     "These models have weird stuff going on and are not currently compatible with this program.");
                return 1;
            }

            if(meshSection->readMesh()){
                qDebug() << "Error while reading mesh";
                return 1;
            }
            unknown4Byte1 = parent->fileData.readInt();
            meshSection->readModifications();
            //qDebug() << Q_FUNC_INFO << "mesh offsets: scale" << meshSection->mods.scale << "offset" << meshSection->mods.offset << "rotation" << meshSection->mods.rotation;

            currentBranch->meshList.push_back(meshSection);
            possibleBranch = meshSection;
            //qDebug() << Q_FUNC_INFO << "name " << meshSection->name << "with parent" << meshSection->parent->name << "finished at" << fileData->currentPosition;
        }

        if(signature.type == "~SceneNode"){
            SceneNode *sceneSection = new SceneNode();
            sceneSection->file = this;
            sceneSection->fileData = &parent->fileData;
            sceneSection->parent = currentBranch;
            sceneSection->headerData = signature;
            sceneSection->sectionEnd = signature.sectionLength + signature.sectionLocation;
            unknown4Byte1 = parent->fileData.readInt(); //appears to be a version number of some kind
            sceneSection->readModifications();
            currentBranch->sectionList.push_back(sceneSection);
            possibleBranch = sceneSection;
            //qDebug() << Q_FUNC_INFO << "name " << sceneSection->name << "with parent" << sceneSection->parent->name;
        }

        if(signature.type == "~anAnimationPrototype"){
            readAnimationPrototype();
            unknown4Byte1 = parent->fileData.readInt();
            readModifications();
        }

        if(!knownSections.contains(signature.type)){
            /*unknown types include:
            taCybertronUnicronLocationManager
            taCybertronTowerManager
            InformationNode
            anAnimationActor
            vlLODSwitcher
            taOcclusionMap
            taOcclusionNodeMapMarker
            taCompressedShadowMap
            anAnimationSourceSet
            ecoCollection*/
            qDebug() << Q_FUNC_INFO << "Found" << signature.type << "at" << parent->fileData.currentPosition << "- skipping for now";
            parent->fileData.currentPosition = signature.sectionLength + signature.sectionLocation;
            qDebug() << Q_FUNC_INFO << "position after skip" << parent->fileData.currentPosition;
            continue;
        }

        if(signature.type == "~anAnimationSourceSet"){
            //qDebug() << Q_FUNC_INFO << "End of tree located - leaving at Animation Data";
            //break;
//            animationSet.file = this;
//            animationSet.fileLocation = parent->fileData.currentPosition-4;
//            animationSet.sectionLength = sectionLength;
//            animationSet.sectionEnd = animationSet.fileLocation + sectionLength;
//            animationSet.name = sectionName;
//            animationSet.readAnimationSet();
            qDebug() << Q_FUNC_INFO << "Found animation data at " << parent->fileData.currentPosition << "- skipping for now";
            parent->fileData.currentPosition = signature.sectionLength + signature.sectionLocation;
            qDebug() << Q_FUNC_INFO << "position after skip" << parent->fileData.currentPosition;
            continue;
        }


        if (signature.type != "~Instance"){
            unknown4Byte2 = parent->fileData.readInt();
            if (unknown4Byte1 > 3) {
                parent->fileData.currentPosition += 8;
                unknown4Byte3 = parent->fileData.readInt(); //this should take us to the end
            }
        }

        fileData->signature(&signature);

        if(signature.type == "~BoundingVolume"){
            readBoundingVolume(&signature);
        }
        //qDebug() << Q_FUNC_INFO << "Expected end of section at location: " << parent->fileData.currentPosition;

        loopBreaker++;
        if (loopBreaker > 900 or parent->fileData.currentPosition > base.sectionEnd){
            parent->messageError("Excessive looping detected or node tree exceeded for file " + fileName);
            qDebug() << Q_FUNC_INFO << "Excessive looping detected or node tree exceeded. loopbreaker:" << loopBreaker;
            return 1;
        }

        if (parent->fileData.currentPosition == base.sectionEnd){
            qDebug() << Q_FUNC_INFO << "Reached end of tree. Exiting now.";
            return 0;
        }

        //qDebug() << Q_FUNC_INFO << loopBreaker << "loops completed";

    }

    return 0; //read successfully
}

void ProgWindow::convertVBINToSTL(){
    if(vbinFiles.empty()){
        messageError("No VBIN files currently loaded to export.");
        return;
    }

    vbinFiles[ListFiles->currentIndex()].outputDataSTL();

    return;
}

void ProgWindow::convertVBINToDAE(){
    if(vbinFiles.empty()){
        messageError("No VBIN files currently loaded to export.");
        return;
    }

    vbinFiles[ListFiles->currentIndex()].outputDataDAE();

    return;
}

void FileSection::writeSectionListSTL(QTextStream &fileOut){
    //for writing to a single file
    for (int i = 0; i < int(meshList.size()); i++) {
        meshList[i]->file = file;
        meshList[i]->writeDataSTL(fileOut);
        meshList[i]->writeSectionListSTL(fileOut);
    }
    for(int i = 0; i < int(sectionList.size()); i++){
        sectionList[i]->file = file;
        sectionList[i]->writeSectionListSTL(fileOut);
    }
}

void FileSection::writeSectionListSTL(QString path){
    //for writing to multiple files

    for (int i = 0; i < int(meshList.size()); i++) {
        QString multiFileOut = path + "/" + meshList[i]->headerData.name + ".stl";
        QFile stlOut(multiFileOut);
        QFile fileOut(multiFileOut);
        fileOut.open(QFile::WriteOnly|QFile::Truncate);
        fileOut.close();

        if (!stlOut.open(QIODevice::ReadWrite)){
            continue;
        }
        QTextStream stream(&stlOut);
        stream << "solid Default" << Qt::endl;

        meshList[i]->file = file;
        meshList[i]->writeDataSTL(stream);
        stream << "endsolid Default" << Qt::endl;
        meshList[i]->writeSectionListSTL(path);
    }
    for(int i = 0; i < int(sectionList.size()); i++){
        sectionList[i]->file = file;
        sectionList[i]->writeSectionListSTL(path);
    }
}

void VBIN::outputDataSTL(){

    std::vector<int> allowedMeshes = {1,2,3};
    std::vector<int> chosenLOD;
    base.file = this;

    //applyKeyframe();

    if(parent->radioSingle->isChecked()){
        QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", parent->tr("Model Files (*.stl)"));
        if(fileOut.isEmpty()){
            parent->messageError("STL export cancelled.");
        }
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
        base.writeSectionListSTL(stream);
        stream << "endsolid Default" << Qt::endl;
    } else {
        QString fileOut = QFileDialog::getExistingDirectory(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", QFileDialog::ShowDirsOnly);
        if(fileOut.isEmpty()){
            parent->messageError("STL export cancelled.");
        }
        base.writeSectionListSTL(fileOut);
    }

    parent->messageSuccess("STL file saved.");
    qDebug() << Q_FUNC_INFO << "STL output complete.";

    return;
}

void VBIN::outputDataDAE(){
    base.file = this;

    //applyKeyframe();

    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output DAE"), QDir::currentPath() + "/DAE/", parent->tr("Model Files (*.dae)"));
    if(fileOut.isEmpty()){
        parent->messageError("DAE export cancelled.");
        return;
    }
    QFile stlOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    if(!stlOut.open(QIODevice::ReadWrite)){
        parent->messageError("DAE export failed, could not open output file.");
        return;
    }
    QTextStream stream(&stlOut);
    //return;

    stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << Qt::endl;
    stream << "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">" << Qt::endl;

    stream << "  <asset>" << Qt::endl;
    stream << "    <contributor>" << Qt::endl;
    stream << "      <author>PrincessTrevor</author>" << Qt::endl;
    stream << "      <authoring_tool>TF2004 File Converter v" << QString::number(parent->version) << "</authoring_tool>" << Qt::endl;
    stream << "    </contributor>" << Qt::endl;
    stream << "    <created>" << QDateTime::currentDateTime().toString("yyyy-MM-dd") + "T" + QDateTime::currentDateTime().toString("hh:mm:ss") << "</created>" << Qt::endl;
    stream << "    <modified>" << QDateTime::currentDateTime().toString("yyyy-MM-dd") + "T" + QDateTime::currentDateTime().toString("hh:mm:ss") << "</modified>" << Qt::endl;
    stream << "    <unit name=\"meter\" meter=\"1\"/>" << Qt::endl;
    stream << "    <up_axis>Z_UP</up_axis>" << Qt::endl;
    stream << "  </asset>" << Qt::endl;

    stream << "  <library_effects>" << Qt::endl;
    base.searchListsWriteDAE(stream, &Mesh::writeEffectsDAE);
    stream << "  </library_effects>" << Qt::endl;


    stream << "  <library_images>" << Qt::endl;
    base.searchListsWriteDAE(stream, &Mesh::writeImagesDAE);
    //base.writeImageListDAE(stream);
    stream << "  </library_images>" << Qt::endl;

    stream << "  <library_materials>" << Qt::endl;
    base.searchListsWriteDAE(stream, &Mesh::writeMaterialsDAE);
    //base.writeMaterialListDAE(stream);
    stream << "  </library_materials>" << Qt::endl;

    stream << "  <library_geometries>" << Qt::endl;
    base.searchListsWriteDAE(stream, &Mesh::writeDataDAE);
    //base.writeSectionListDAE(stream);
    stream << "  </library_geometries>" << Qt::endl;

    stream << "  <library_visual_scenes>" << Qt::endl;
    stream << "    <visual_scene id=\"Scene\" name=\"Scene\">" << Qt::endl;
    base.searchListsWriteDAE(stream, &Mesh::writeNodesDAE);
    //base.writeSceneListDAE(stream);
    stream << "    </visual_scene>" << Qt::endl;
    stream << "  </library_visual_scenes>" << Qt::endl;

    stream << "  <scene>" << Qt::endl;
    stream << "    <instance_visual_scene url=\"#Scene\"/>" << Qt::endl;
    stream << "  </scene>" << Qt::endl;

    stream << "</COLLADA>" << Qt::endl;

    parent->messageSuccess("DAE file saved.");
    qDebug() << Q_FUNC_INFO << "DAE output complete.";
}

void VBIN::applyKeyframe(){
    qDebug() << Q_FUNC_INFO << "current animation index" << parent->ListAnimation->currentIndex() << "with current frame index" << parent->ListFrame->currentIndex();
    QString chosenType;

    QVector3D translation;
    QQuaternion rotation;
    QString channelName;
    for (int channel = 0; channel < animationSet.streamArray[parent->ListAnimation->currentIndex()]->channelArray.size(); channel++) {
        chosenType = animationSet.streamArray[parent->ListAnimation->currentIndex()]->channelArray[channel]->animationType;
        if(chosenType == "~anAnimationTranslation"){
            translation = animationSet.streamArray[parent->ListAnimation->currentIndex()]->channelArray[channel]->vectorList[parent->ListFrame->currentIndex()];
            channelName = animationSet.streamArray[parent->ListAnimation->currentIndex()]->channelArray[channel]->name;
            base.sendKeyframe(translation, channelName);
        } else if (chosenType == "~anAnimationOrientation"){
            rotation = animationSet.streamArray[parent->ListAnimation->currentIndex()]->channelArray[channel]->rotationList[parent->ListFrame->currentIndex()];
            channelName = animationSet.streamArray[parent->ListAnimation->currentIndex()]->channelArray[channel]->name;
            base.sendKeyframe(rotation, channelName);
        }
        qDebug() << Q_FUNC_INFO << "animation type" << chosenType << "is not currently supported";
        return;
    }
}

void FileSection::sendKeyframe(QVector3D keyOffset, QString channelName){
    for(int mesh = 0; mesh < meshList.size(); mesh++){
        if(meshList[mesh]->headerData.name == channelName){
            meshList[mesh]->applyKeyframe(keyOffset);
        }
        meshList[mesh]->sendKeyframe(keyOffset, channelName);
    }
    for(int section = 0; section < sectionList.size(); section++){
        sectionList[section]->sendKeyframe(keyOffset, channelName);
    }
}

void FileSection::sendKeyframe(QQuaternion keyRotation, QString channelName){
    for(int mesh = 0; mesh < meshList.size(); mesh++){
        if(meshList[mesh]->headerData.name == channelName){
            meshList[mesh]->applyKeyframe(keyRotation);
        }
        meshList[mesh]->sendKeyframe(keyRotation, channelName);
    }
    for(int section = 0; section < sectionList.size(); section++){
        sectionList[section]->sendKeyframe(keyRotation, channelName);
    }
}

void FileSection::modify(std::vector<Modifications> addedMods){
    qDebug() << Q_FUNC_INFO << "THIS SHOULDN'T RUN";
}
