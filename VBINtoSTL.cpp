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
    fileData->currentPosition = fileLocation + 25;
    hasVolume = fileData->readInt(1);
    type = fileData->readInt();
    location.setX(fileData->readFloat());
    location.setY(fileData->readFloat());
    location.setZ(fileData->readFloat());
    radius = fileData->readFloat();
    qDebug() << Q_FUNC_INFO << "location" << fileLocation << "radius" << radius << "hasvolume" << hasVolume << "type" << type;
}

int VBIN::readData(){

    //gets all scene nodes in the file, populates their PositionArrays and IndexArrays, and gets modifications
    currentLocation = 0;
    base.file = this;
    base.name = "vbinbase";
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
        qDebug() << Q_FUNC_INFO << "depth" << depth << "section name" << name << "with parent" << parent->name;
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
    if(name != "vbinbase"){
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
    fileLocation = input.fileLocation;
    hasVolume = input.hasVolume;
    type = input.type;
    location = input.location;
    radius = input.radius;
    fileData = input.fileData;
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

void FileSection::readModifications(){
    int offsets = 0;
    float x_value = 0;
    float y_value = 0;
    float z_value = 0;
    float m_value = 0;
    //offset properties
    offsets = fileData->readInt(1);
    //qDebug() << Q_FUNC_INFO << "offsets read as" << offsets;
    if (!(offsets & 1)) {
        //position offset
        x_value = fileData->readFloat();
        y_value = fileData->readFloat();
        z_value = fileData->readFloat();
        mods.offset = QVector3D(x_value, y_value, z_value);
    } else {
        mods.offset = QVector3D(0,0,0);
    }
    if (!(offsets & 2)) {
        //rotation offset
//        x_value = fileData->readFloat();
//        y_value = fileData->readFloat();
//        z_value = fileData->readFloat();
//        m_value = fileData->readFloat();
//        mods.rotation = QQuaternion(m_value, x_value, y_value, z_value);
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
    QString signature;
    int sectionNameLength = 0;
    QString sectionName;
    int sectionLength = 0;
    int unknown4Byte1 = 0;
    int unknown4Byte2 = 0;
    int unknown4Byte3 = 0;
    FileSection* currentBranch;
    FileSection* possibleBranch;

    //signature = parent->fileData.getSignature();

    //sectionNameLength = parent->fileData.readInt();
    //sectionName = parent->fileData.readHex(sectionNameLength);
    //qDebug() << Q_FUNC_INFO << "name length " << sectionNameLength << " read as " << sectionName;

    //sectionLength = parent->fileData.readInt();
    //qDebug() << Q_FUNC_INFO << "end of tree excpeted at" << base.sectionEnd;
    base.sectionLength = 0;
    base.fileLocation = 0;
    currentBranch = &base;
    possibleBranch = &base;

    while(1){

        if (parent->fileData.currentPosition == base.sectionEnd){
            qDebug() << Q_FUNC_INFO << "Reached end of tree. Exiting now.";
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

        if (base.sectionLength == 0) {
            base.sectionLength = sectionLength;
            base.sectionEnd = parent->fileData.currentPosition -4 + sectionLength;
        }

        if(signature == "~anAnimationSourceSet"){
            //qDebug() << Q_FUNC_INFO << "End of tree located - leaving at Animation Data";
            //break;
//            animationSet.file = this;
//            animationSet.fileLocation = parent->fileData.currentPosition-4;
//            animationSet.sectionLength = sectionLength;
//            animationSet.sectionEnd = animationSet.fileLocation + sectionLength;
//            animationSet.name = sectionName;
//            animationSet.readAnimationSet();
            qDebug() << Q_FUNC_INFO << "Found animation data at " << parent->fileData.currentPosition << "- skipping for now";
            parent->fileData.currentPosition += sectionLength-4;
            qDebug() << Q_FUNC_INFO << "position after skip" << parent->fileData.currentPosition;
            continue;
        }

        if(signature == "~Mesh"){
            Mesh *meshSection = new Mesh();
            meshSection->file = this; //file will need to be reassigned later as the current VBIN object is temporary
            meshSection->fileData = &parent->fileData;
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
            unknown4Byte1 = parent->fileData.readInt();
            meshSection->readModifications();
            //qDebug() << Q_FUNC_INFO << "mesh offsets: scale" << meshSection->mods.scale << "offset" << meshSection->mods.offset << "rotation" << meshSection->mods.rotation;

            currentBranch->meshList.push_back(meshSection);
            possibleBranch = meshSection;
            qDebug() << Q_FUNC_INFO << "name " << meshSection->name << "with parent" << meshSection->parent->name << "finished at" << fileData->currentPosition;
        }

        if(signature == "~SceneNode"){
            SceneNode *sceneSection = new SceneNode();
            sceneSection->file = this;
            sceneSection->fileData = &parent->fileData;
            sceneSection->parent = currentBranch;
            sceneSection->fileLocation = parent->fileData.currentPosition-4;
            sceneSection->sectionLength = sectionLength;
            sceneSection->sectionEnd = sceneSection->fileLocation + sectionLength;
            sceneSection->name = sectionName;
            unknown4Byte1 = parent->fileData.readInt(); //appears to be a version number of some kind
            sceneSection->readModifications();
            currentBranch->sectionList.push_back(sceneSection);
            possibleBranch = sceneSection;
            //qDebug() << Q_FUNC_INFO << "name " << sceneSection->name << "with parent" << sceneSection->parent->name;
        }

        if(signature == "~anAnimationPrototype"){
            readAnimationPrototype();
            unknown4Byte1 = parent->fileData.readInt();
            readModifications();
        }

        //unknown4Byte1 = parent->fileData.readInt(); //looks like this value is always 4? I haven't found an exception to this
            //unfortunately means it's really hard to figure out what this is for.
        //qDebug() << Q_FUNC_INFO << "section name" << sectionName << "with unknown value" << unknown4Byte1;

        //readModifications();
        unknown4Byte2 = parent->fileData.readInt();
//        parent->fileData.currentPosition += 8;
//        unknown4Byte3 = parent->fileData.readInt(); //this should take us to the end
        if (unknown4Byte1 > 3) {
            parent->fileData.currentPosition += 8;
            unknown4Byte3 = parent->fileData.readInt(); //this should take us to the end
        }

        signature = parent->fileData.getSignature();

        if(signature == "~BoundingVolume"){
            readBoundingVolume();
        }
        //qDebug() << Q_FUNC_INFO << "Expected end of section at location: " << parent->fileData.currentPosition;

        loopBreaker++;
        if (loopBreaker > 100 or parent->fileData.currentPosition > base.sectionEnd){
            parent->messageError("Excessive looping detected or node tree exceeded for file " + fileName);
            qDebug() << Q_FUNC_INFO << "Excessive looping detected or node tree exceeded.";
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
        QString multiFileOut = path + "/" + meshList[i]->name + ".stl";
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

void FileSection::writeSectionListDAE(QTextStream &fileOut){
    for (int i = 0; i < meshList.size(); i++) {
        meshList[i]->file = file;
        meshList[i]->writeDataDAE(fileOut);
        meshList[i]->writeSectionListDAE(fileOut);
    }
    for (int i = 0; i < sectionList.size(); i++) {
        sectionList[i]->file = file;
        sectionList[i]->writeSectionListDAE(fileOut);
    }
}

void FileSection::writeSceneListDAE(QTextStream &fileOut){
    for (int i = 0; i < meshList.size(); i++) {
        meshList[i]->file = file;
        meshList[i]->writeNodesDAE(fileOut);
        meshList[i]->writeSceneListDAE(fileOut);
    }
    for (int i = 0; i < sectionList.size(); i++) {
        sectionList[i]->file = file;
        sectionList[i]->writeSceneListDAE(fileOut);
    }
}

void FileSection::writeEffectListDAE(QTextStream &fileOut){
    for (int i = 0; i < meshList.size(); i++) {
        meshList[i]->file = file;
        meshList[i]->writeEffectsDAE(fileOut);
        meshList[i]->writeEffectListDAE(fileOut);
    }
    for (int i = 0; i < sectionList.size(); i++) {
        sectionList[i]->file = file;
        sectionList[i]->writeEffectListDAE(fileOut);
    }
}

void FileSection::writeImageListDAE(QTextStream &fileOut){
    for (int i = 0; i < meshList.size(); i++) {
        meshList[i]->file = file;
        meshList[i]->writeImagesDAE(fileOut);
        meshList[i]->writeImageListDAE(fileOut);
    }
    for (int i = 0; i < sectionList.size(); i++) {
        sectionList[i]->file = file;
        sectionList[i]->writeImageListDAE(fileOut);
    }
}

void FileSection::writeMaterialListDAE(QTextStream &fileOut){
    for (int i = 0; i < meshList.size(); i++) {
        meshList[i]->file = file;
        meshList[i]->writeMaterialsDAE(fileOut);
        meshList[i]->writeMaterialListDAE(fileOut);
    }
    for (int i = 0; i < sectionList.size(); i++) {
        sectionList[i]->file = file;
        sectionList[i]->writeMaterialListDAE(fileOut);
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
    base.writeEffectListDAE(stream);
    stream << "  </library_effects>" << Qt::endl;


    stream << "  <library_images>" << Qt::endl;
    base.writeImageListDAE(stream);
    stream << "  </library_images>" << Qt::endl;

    stream << "  <library_materials>" << Qt::endl;
    base.writeMaterialListDAE(stream);
    stream << "  </library_materials>" << Qt::endl;

    stream << "  <library_geometries>" << Qt::endl;
    base.writeSectionListDAE(stream);
    stream << "  </library_geometries>" << Qt::endl;

    stream << "  <library_visual_scenes>" << Qt::endl;
    stream << "    <visual_scene id=\"Scene\" name=\"Scene\">" << Qt::endl;
    base.writeSceneListDAE(stream);
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
        if(meshList[mesh]->name == channelName){
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
        if(meshList[mesh]->name == channelName){
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
