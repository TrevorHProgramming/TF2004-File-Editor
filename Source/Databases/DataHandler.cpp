#include "Headers/Main/mainwindow.h"

/*std::vector<Pickup> convertPickups(std::vector<dictItem> itemList){

}

std::vector<Minicon> convertMinicons(std::vector<dictItem> itemList){

}

std::vector<Warpgate> convertWarpgates(std::vector<dictItem> itemList){
    std::vector<Warpgate> warpgateList;
    for(int i = 0; i < itemList.size(); i++){
        if(itemList[i].name == "CreatureWarpGate"){
            Warpgate nextWarpgate = Warpgate(itemList[i]);
            warpgateList.push_back(nextWarpgate);
        }
    }
    return warpgateList;
}*/


Pickup::Pickup(dictItem copyItem){
    dataID = 99;
    this->name = copyItem.name;
    this->instanceIndex = copyItem.instanceIndex;
    //this->attributes = copyItem.attributes;
    for(int i = 0; i < copyItem.attributes.size(); i++){
        if(copyItem.attributes[i]->name == "PickupToSpawn"){
            this->pickupToSpawn = copyItem.attributes[i]->display();
            this->enumID = copyItem.attributes[i]->intValue();
            if(enumID > 3){
                this->isMinicon = true;
            } else {
                this->isMinicon = false;
            }
        }
        if(copyItem.attributes[i]->name == "ProductionArt" && !this->isMinicon){
            this->pickupToSpawn = copyItem.attributes[i]->display();
            this->dataID = copyItem.attributes[i]->intValue();
        }
    }
    //qDebug() << Q_FUNC_INFO << "adding pickup" << copyItem.instanceIndex << "name" << copyItem.name << "|" << pickupToSpawn << "has enumID" << enumID << "is minicon?" << isMinicon;
    placed = false;
}

Pickup::Pickup(){
    dataID = 99;
    isMinicon = false;
    isWeapon = false;
    setAttribute("Position", "0, 0, 0");
}


Minicon::Minicon(Pickup copyItem){
    this->name = copyItem.name;
    this->instanceIndex = copyItem.instanceIndex;
    this->isWeapon = copyItem.isWeapon;
    this->pickupToSpawn = copyItem.pickupToSpawn;
    this->enumID = copyItem.enumID;
    //QVector3D location = copyItem.searchAttributes<QVector3D>("Position");
    //this->setAttribute("Position", QString::number(location.x()) + ", " + QString::number(location.y()) + ", " + QString::number(location.z()));
    //this->attributes = copyItem.attributes;
    /*for(int i = 0; i < copyItem.attributes.size(); i++){
        if(copyItem.attributes[i]->name == "PickupToSpawn"){
            this->pickupToSpawn = copyItem.attributes[i]->display();
            this->enumID = copyItem.attributes[i]->intValue();
        }
    }*/
    placed = false;
}

Minicon::Minicon(){
    instanceIndex = 0;
    isWeapon = false;
    pickupToSpawn = "Default Shockpunch";
    enumID = 27;
    setAttribute("Position", "0, 0, 0");
}

Minicon* DataHandler::getMinicon(int searchID){
    if(searchID == 3){
        qDebug() << Q_FUNC_INFO << "Searching for a datacon instead of minicon. This shouldn't happen.";
        return nullptr;
    }
    for(int i = 0; i < miniconList.size(); i++){
        if(miniconList[i].enumID == searchID){
            return &miniconList[i];
        }
    }
    qDebug() << Q_FUNC_INFO << "minicon" << searchID << "was not found.";
    return nullptr;
}

Minicon* DataHandler::getMinicon(QString searchName){
    for(int i = 0; i < miniconList.size(); i++){
        if(miniconList[i].pickupToSpawn == searchName){
            return &miniconList[i];
        }
    }
    qDebug() << Q_FUNC_INFO << "minicon" << searchName << "was not found.";
    return nullptr;
}

void DataHandler::resetMinicons(){
    for(int i = 0; i < miniconList.size(); i++){
        miniconList[i].placed = false;
    }
}

void DataHandler::resetDatacons(){
    for(int i = 0; i < dataconList.size(); i++){
        dataconList[i].placed = false;
    }
}

void DataHandler::resetLevels(){
    for(int i = 0; i < levelList.size(); i++){
        levelList[i].dataconCount = 0;
        levelList[i].miniconCount = 0;
    }
}

Warpgate::Warpgate(dictItem copyItem){
    this->instanceIndex = copyItem.instanceIndex;
    this->position = copyItem.position;
    this->x_position = copyItem.position.x();
    this->y_position = copyItem.position.y();
    this->z_position = copyItem.position.z();
    this->attributes = copyItem.attributes;
    for(int i = 0; i < copyItem.attributes.size(); i++){
        if(copyItem.attributes[i]->name == "WarpGateNumber"){
            this->name = copyItem.attributes[i]->display();
        }
    }
}

Warpgate::Warpgate(){
    this->x_position = 0;
    this->y_position = 0;
    this->z_position = 0;
}

void DataHandler::loadFileReplacements(){
    QString modPath = QCoreApplication::applicationDirPath() + "/Replacements/";
    QDir modFolder(modPath);
    QDirIterator modIterator(modFolder.absolutePath());
    qDebug() << Q_FUNC_INFO << "next file info:" << modIterator.nextFileInfo().fileName() << "from path" << modFolder.absolutePath();
    bool headerFinished = false;
    TextProperty modProperty;
    QStringList propertyOptions = {"File Version", "Name", "Description", "Rarity", "File Count", "File Name", "Destination Path"};
    int modVersion = 0;
    int replacementCount = 0;

    while (modIterator.hasNext()){
        QFile currentModFile = modIterator.next();
        qDebug() << Q_FUNC_INFO << "Current file" << currentModFile.fileName();
        if (currentModFile.open(QIODevice::ReadOnly)){
            qDebug() << Q_FUNC_INFO << "Reading file";
            FileReplacement moddedFiles;
            FileData modBuffer;
            modBuffer.dataBytes = currentModFile.readAll();
            modBuffer.input = true;
            headerFinished = false;
            QString targetLevel;
            while(!headerFinished){
                modProperty = modBuffer.readProperty();
                qDebug() << Q_FUNC_INFO << "test property type:" << modProperty.name << "with value:" << modProperty.readValue;
                switch(propertyOptions.indexOf(modProperty.name)){
                case 0: //File Version
                    modVersion = modProperty.readValue.toInt();
                    break;
                case 1: //Name
                    moddedFiles.name = modProperty.readValue;
                    break;
                case 2: //Description
                    //for human use only, for now. tooltips later.
                    moddedFiles.description = modProperty.readValue;
                    break;
                case 3: //Rarity
                    //Only used for Randomizer
                    //the value x from 1/x that generates the chance of this file swap
                    //ex. a rarity of 4 has a 1/4 or 25% chance of swapping
                    moddedFiles.rarity = modProperty.readValue.toInt();
                    break;
                case 4: //File count
                    replacementCount = modProperty.readValue.toInt();
                    headerFinished = true;
                    break;
                default:
                    qDebug() << Q_FUNC_INFO << "Unknown property" << modProperty.name << "with value" << modProperty.readValue << "found at" << modBuffer.currentPosition;
                }
            }
            for(int i = 0; i < replacementCount*2; i++){ //verify this line with the SELF version
                modProperty = modBuffer.readProperty();
                switch(propertyOptions.indexOf(modProperty.name)){
                case 5: //File Name
                    moddedFiles.fileNames.push_back(modProperty.readValue);
                    break;
                case 6: //Destination path
                    moddedFiles.fileDestinations.push_back(modProperty.readValue);
                    break;
                default:
                    qDebug() << Q_FUNC_INFO << "Unknown property" << modProperty.name << "with value" << modProperty.readValue << "found at" << modBuffer.currentPosition;
                }
            }
            replacementList.push_back(moddedFiles);
        }
    }
}

void DataHandler::replaceFile(QString fileName, QString destinationPath){
    QString modFileDirectory = QDir::currentPath();
    bool didItWork = false;
    qDebug() << Q_FUNC_INFO << "current path:" << modFileDirectory;
    QString fileInputDirectory = modFileDirectory + "/ASSETS/" + fileName;
    QFile fileReplacement(fileInputDirectory);
    qDebug() << Q_FUNC_INFO << "checking if replacement image exists:" << fileReplacement.exists();

    QString fileOutputDirectory = parent->isoBuilder->copyOutputPath + destinationPath;

    qDebug() << Q_FUNC_INFO << "creating directory" << fileOutputDirectory;
    QDir createFile(fileOutputDirectory);
    if(!createFile.exists()){
        createFile.mkpath(".");
    }
    fileOutputDirectory += "/" + fileName;
    QFile original(fileOutputDirectory);
    qDebug() << Q_FUNC_INFO << "checking if original file exists:" << original.exists() << "path" << fileOutputDirectory;
    if(original.exists()){
        original.remove();
    }
    if(fileReplacement.exists()){
        didItWork = QFile::copy(fileInputDirectory, fileOutputDirectory);
        qDebug() << Q_FUNC_INFO << "did it work?" << didItWork;
    } else {
        parent->log("File " + fileName + " could not be replaced.");
    }
}

void DataHandler::replaceFile(FileReplacement fileToReplace){
    for(int i = 0; i < fileToReplace.fileNames.size(); i++){
        replaceFile(fileToReplace.fileNames[i], fileToReplace.fileDestinations[i]);
    }
}

void DataHandler::loadLevels(){
    for(int i = 0; i < 9; i++){
        Level nextLevel;
        nextLevel.miniconCount = 0;
        nextLevel.dataconCount = 0;
        switch(i){
        case 0: //Amazon 1
            nextLevel.levelName = "Amazon";
            nextLevel.outputName = "01_AMAZON_1";
            break;
        case 1: //Antarctica
            nextLevel.levelName = "Antarctica";
            nextLevel.outputName = "02_ANTARCTICA";
            break;
        case 2: //Amazon 2
            nextLevel.levelName = "Deep Amazon";
            nextLevel.outputName = "03_AMAZON_2";
            break;
        case 3: //Mid Atlantic Carrier
            nextLevel.levelName = "Mid-Atlantic with Carrier";
            nextLevel.outputName = "04_AIRCRAFTCARRIER";
            break;
        case 4: //Mid Atlantic Defeated
            nextLevel.levelName = "Mid-Atlantic Empty";
            nextLevel.outputName = "04_AIRCRAFTCARRIERDEFEATED";
            break;
        case 5: //Alaska
            nextLevel.levelName = "Alaska";
            nextLevel.outputName = "05_GREENLAND";
            break;
        case 6: //Starship
            nextLevel.levelName = "Starship";
            nextLevel.outputName = "06_SPACESHIP";
            break;
        case 7: //Pacific Island
            nextLevel.levelName = "Pacific Island";
            nextLevel.outputName = "07_EASTERISLAND";
            break;
        case 8: //Cybertron
            nextLevel.levelName = "Cybertron";
            nextLevel.outputName = "08_CYBERTRON";
            break;
        }
        for(int j = 0; j < parent->databaseList.size(); j++){
            if(parent->databaseList[j]->fileName.contains(nextLevel.outputName)){
                nextLevel.levelFile = parent->databaseList[j];
            }
        }
        levelList.push_back(nextLevel);
    }

    int totalRemoved = 0;
    std::vector<PickupLocation> tempLocations;
    for(int i = 0; i < levelList.size(); i++){
        std::vector<dictItem> filePickupsBase = levelList[i].levelFile->sendInstances("PickupPlaced");
        //std::vector<PickupLocation> filePickups = convertInstances<PickupLocation>(filePickupsBase);
        levelList[i].maxInstances = levelList[i].levelFile->maxInstances;
        levelList[i].removedInstances = filePickupsBase.size()-2; //to account for glide and highjump placements
        totalRemoved += levelList[i].removedInstances;
        for(int pickup = 0; pickup < filePickupsBase.size(); pickup++){
            tempLocations.push_back(filePickupsBase[pickup]);
            tempLocations[tempLocations.size()-1].level = i;
            pickupList.push_back(filePickupsBase[pickup]);
        }
    }

    int id = 0;

    foreach(PickupLocation currentLocation, tempLocations){
        currentLocation.uniqueID = id;
        //addedLocation.setAttribute("PickupToSpawn", "0");

        //to be placed inside the switch:
        currentLocation.locationName = "Unnamed Location";
        switch(currentLocation.level){
        case 0: //Amazon 1
            currentLocation.slipstreamDifficulty = 0;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 0:
                    currentLocation.locationName = "Claymore Cave";
                    break;
                case 18: //1
                    currentLocation.locationName = "Ruined Temple Courtyard";
                    break;
                case 2:
                    currentLocation.slipstreamDifficulty = 3;
                    currentLocation.locationName = "Spire";
                    break;
                case 17: //3
                    currentLocation.locationName = "Tutorial Minicon";
                    break;
                case 4:
                    currentLocation.locationName = "Neighboring Mountain";
                    break;
                case 5:
                    currentLocation.locationName = "Ravine Cliff Cave";
                    break;
                case 6:
                    currentLocation.locationName = "Pressurepoint's Corner";
                    break;
                case 7:
                    currentLocation.locationName = "Foot of the Mountain";
                    break;
                case 8:
                    currentLocation.locationName = "Mountain Ruins";
                    break;
                case 9:
                    currentLocation.locationName = "Before the Waterfall Bridge";
                    break;
                case 20: //10
                    currentLocation.locationName = "Hidden Cave";
                    break;
                case 11:
                    currentLocation.locationName = "Spidertank Triplets";
                    break;
                case 12:
                    currentLocation.locationName = "Ruined Temple Alcove";
                    break;
                case 13:
                    currentLocation.locationName = "Forest before the Bridge";
                    break;
                case 19: //14
                    currentLocation.locationName = "Light Unit Party";
                    break;
                case 15:
                    currentLocation.locationName = "Ruined Temple Ledge";
                    break;
                case 16:
                    currentLocation.locationName = "Forest before Firefight";
                    break;
                case 21:
                    currentLocation.locationName = "Riverside Ledge";
                    break;
                case 22:
                    currentLocation.locationName = "Waterfall Base";
                    break;
                case 23:
                    currentLocation.locationName = "Forest after the Bridge";
                    break;
                case 24:
                    currentLocation.locationName = "Surprise";
                    break;
                case 25:
                    currentLocation.locationName = "Forgettable Location";
                    break;
                case 26:
                    currentLocation.locationName = "Medium Unit Party";
                    break;
                case 27:
                    currentLocation.locationName = "Forest Across from the Basin 1";
                    break;
                case 28:
                    currentLocation.locationName = "Forest Across from the Basin 2";
                    break;
                case 42069:
                    currentLocation.locationName = "Beginner Weapon";
                    break;
                }
            break;
        case 1: //Antarctica
            currentLocation.slipstreamDifficulty = 0;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 49: //29
                    currentLocation.locationName = "Crevasse Field Overlook";
                    break;
                case 30:
                    currentLocation.locationName = "Rock of Power";
                    currentLocation.highjumpDifficulty = 1;
                    break;
                case 45: //31
                    currentLocation.locationName = "Research Center Hangar";
                    break;
                case 44: //32
                    currentLocation.locationName = "Crashed Icebreaker";
                    break;
                case 47: //33
                    currentLocation.locationName = "Crashed Plane";
                    break;
                case 34:
                    currentLocation.locationName = "Research Base Office 1";
                    break;
                case 46: //35
                    currentLocation.locationName = "Beachfront Property";
                    break;
                case 51: //36
                    currentLocation.locationName = "Research Base Office 2";
                    break;
                case 37:
                    currentLocation.locationName = "Midfield Beacon";
                    break;
                case 38:
                    currentLocation.locationName = "Hide-and-Seek Champion";
                    break;
                case 39:
                    currentLocation.locationName = "Research Base Containers 1";
                    break;
                case 40:
                    currentLocation.locationName = "Research Base Containers 2";
                    break;
                case 41:
                    currentLocation.locationName = "Research Base Office 3";
                    break;
                case 48: //42
                    currentLocation.locationName = "Research Base Office 4";
                    break;
                case 50: //43
                    currentLocation.locationName = "Lonely Island";
                    break;
                case 52:
                    currentLocation.locationName = "Across the Field";
                    break;
            }
            break;
        case 2: //Amazon 2
            currentLocation.slipstreamDifficulty = 0;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 53:
                    currentLocation.locationName = "Island Altar";
                    break;
                case 67: //54
                    currentLocation.locationName = "Jungle Village Warpgate";
                    break;
                case 55:
                    currentLocation.locationName = "Top of the Temple";
                    break;
                case 66: //56
                    currentLocation.locationName = "Village at the Foot of the Hill";
                    break;
                case 57:
                    currentLocation.locationName = "Spawn Ledge";
                    break;
                case 58:
                    currentLocation.locationName = "Waterfall Rock";
                    break;
                case 59:
                    currentLocation.locationName = "Jungle Ruins";
                    break;
                case 60:
                    currentLocation.slipstreamDifficulty = 4;
                    currentLocation.locationName = "Patrolling Dropship";
                    break;
                case 61:
                    currentLocation.locationName = "Jungle Village Outskirts";
                    break;
                case 62:
                    currentLocation.locationName = "Field before the Bridge";
                    break;
                case 69: //63
                    currentLocation.locationName = "Temple Pool";
                    break;
                case 64:
                    currentLocation.locationName = "Temple Climb";
                    break;
                case 65:
                    currentLocation.locationName = "Mid-Jungle Ditch";
                    break;
                case 68:
                    currentLocation.locationName = "Temple Dead End";
                    break;
                case 70:
                    currentLocation.locationName = "Temple Side-Path";
                    break;
                case 71:
                    currentLocation.locationName = "Forest before the Bridge";
                    break;
                case 72:
                    currentLocation.locationName = "Antechamber";
                    break;
                case 73:
                    currentLocation.locationName = "Antechamber Alcove";
                    currentLocation.highjumpDifficulty = 2;
                    break;
                case 74:
                    currentLocation.locationName = "Behind the Temple";
                    break;
            }
            break;
        case 3: //Mid atlantic 1
            currentLocation.slipstreamDifficulty = 0;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 75:
                    currentLocation.linkedLocationID = 79;
                    currentLocation.locationName = "Vanilla Progression";
                    break;
                case 76:
                    currentLocation.linkedLocationID = 80;
                    currentLocation.slipstreamDifficulty = 5;
                    //this is the only location that is completely impossible without slipstream
                    currentLocation.locationName = "Distant Island";
                    break;
                case 77:
                    currentLocation.linkedLocationID = 82;
                    currentLocation.locationName = "Pinnacle Rock";
                    break;
                case 78:
                    currentLocation.linkedLocationID = 81;
                    currentLocation.locationName = "Atoll";
                    break;
            }
            break;
        case 4: //Mid atlantic 2
            currentLocation.slipstreamDifficulty = 3;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 79:
                    currentLocation.linkedLocationID = 75;
                    currentLocation.locationName = "Vanilla Progression";
                    break;
                case 80:
                    currentLocation.linkedLocationID = 76;
                    currentLocation.slipstreamDifficulty = 5;
                    //this is the only location that is completely impossible without slipstream
                    currentLocation.locationName = "Distant Island";
                    break;
                case 81:
                    currentLocation.linkedLocationID = 78;
                    currentLocation.locationName = "Atoll";
                    break;
                case 82:
                    currentLocation.linkedLocationID = 77;
                    currentLocation.locationName = "Pinnacle Rock";
                    break;
            }
            break;
        case 5: //Alaska
            currentLocation.slipstreamDifficulty = 4;
            currentLocation.highjumpDifficulty = 3; //can be lower for early placements
            switch(currentLocation.uniqueID){
                case 83:
                    currentLocation.locationName = "Mountaintop";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 84:
                    currentLocation.locationName = "Small Forest";
                    break;
                case 98: //85
                    currentLocation.locationName = "Peninsula";
                    break;
                case 86:
                    currentLocation.locationName = "Cave's Far Ledge";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 87:
                    currentLocation.locationName = "Cave's Right Fork";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 88:
                    currentLocation.locationName = "Cave's Left Fork";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 89:
                    currentLocation.locationName = "Cave's Pool";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 90:
                    currentLocation.locationName = "Along the Canyon";
                    break;
                case 91:
                    currentLocation.locationName = "Ocean Overlook";
                    break;
                case 97: //92
                    currentLocation.locationName = "Dropship Island";
                    break;
                case 93:
                    currentLocation.locationName = "Hidden Hill";
                    break;
                case 94:
                    currentLocation.locationName = "Canyon Clearing Ledge 1";
                    break;
                case 95:
                    currentLocation.locationName = "Battlefield";
                    break;
                case 96:
                    currentLocation.locationName = "Canyon Clearing Ledge 2";
                    break;
                case 99:
                    currentLocation.locationName = "Escape";
                    currentLocation.slipstreamDifficulty = 5;
                    break;
                case 100:
                    currentLocation.locationName = "Hillside";
                    break;

            }
            break;
        case 6: //Starship
            //must have either Slipstream or Highjump before starship, even for cheaters
            currentLocation.slipstreamDifficulty = 5;
            currentLocation.highjumpDifficulty = 6;
            switch(currentLocation.uniqueID){
                case 101:
                    currentLocation.locationName = "Start of the Climb";
                    break;
                case 102:
                    currentLocation.locationName = "First Steps";
                    break;
                case 103:
                    currentLocation.locationName = "Main Elevator";
                    break;
                case 104:
                    currentLocation.locationName = "Sideways Elevator";
                    break;
                case 105:
                    currentLocation.locationName = "Lower Dropship";
                    break;
                case 106:
                    currentLocation.locationName = "Bait";
                    break;
                case 107:
                    currentLocation.locationName = "Free Space";
                    break;
                case 108:
                    currentLocation.locationName = "Across the Gap";
                    break;
                case 109:
                    currentLocation.locationName = "Early Gift";
                    break;
                case 110:
                    currentLocation.slipstreamDifficulty = 4;
                    //slipstream can technically be on the Bridge as long as highjump is acquired before then
                    currentLocation.linkedLocationID = 111;
                    currentLocation.locationName = "Pristine Bridge";
                    break;
                case 111:
                    currentLocation.linkedLocationID = 110;
                    currentLocation.locationName = "Crashed Bridge";
                    break;
                case 112:
                    currentLocation.locationName = "Top of the Crashed Ship";
                    break;
                case 113:
                    currentLocation.locationName = "Dropdown 1";
                    break;
                case 114:
                    currentLocation.locationName = "Risky Jump";
                    break;
                case 115:
                    currentLocation.locationName = "Rock 1";
                    break;
                case 116:
                    currentLocation.locationName = "Middle Dropship";
                    break;
                case 117:
                    currentLocation.locationName = "Dropdown 2";
                    break;
                case 118:
                    currentLocation.locationName = "Rock 2";
                    break;
                case 119:
                    currentLocation.locationName = "Rock 3";
                    break;
                case 120:
                    currentLocation.locationName = "Rock 4";
                    break;
            }
            break;
        case 7: //Pacific Island
            currentLocation.slipstreamDifficulty = 5;
            currentLocation.highjumpDifficulty = 6;
            switch(currentLocation.uniqueID){
                case 132: //121
                    currentLocation.locationName = "Forest Basin";
                    break;
                case 122:
                    currentLocation.locationName = "Small Silo";
                    break;
                case 123:
                    currentLocation.locationName = "Moai";
                    break;
                case 124:
                    currentLocation.locationName = "Back of the Volcano";
                    break;
                case 125:
                    currentLocation.locationName = "Volcano Path";
                    break;
                case 133: //126
                    currentLocation.locationName = "Hidden Ledge";
                    break;
                case 127:
                    currentLocation.locationName = "Lighthouse";
                    break;
                case 128:
                    currentLocation.locationName = "Waterfall Climb";
                    break;
                case 129:
                    currentLocation.locationName = "Stronghold Ledge";
                    break;
                case 136: //130
                    currentLocation.locationName = "Small Island";
                    break;
                case 131:
                    currentLocation.locationName = "River Basin Peak (I think?)";
                    break;
                case 134:
                    currentLocation.locationName = "Above the Waterfall";
                    break;
                case 135:
                    currentLocation.locationName = "Large Silo";
                    break;
                case 137:
                    currentLocation.locationName = "Bunker 1";
                    currentLocation.bunkerID = 526;
                    break;
                case 138:
                    currentLocation.locationName = "Bunker 2";
                    currentLocation.bunkerID = 525;
                    break;
                case 139:
                    currentLocation.locationName = "Bunker 3";
                    currentLocation.bunkerID = 522;
                    break;
                case 140:
                    currentLocation.locationName = "Bunker 4";
                    currentLocation.bunkerID = 521;
                    break;
                case 141:
                    currentLocation.locationName = "Bunker 5";
                    currentLocation.bunkerID = 527;
                    break;
                case 142:
                    currentLocation.locationName = "Bunker 6";
                    currentLocation.bunkerID = 523;
                    break;
                case 143:
                    currentLocation.locationName = "Bunker 7";
                    currentLocation.bunkerID = 520;
                    break;
                case 144:
                    currentLocation.locationName = "Bunker 8";
                    currentLocation.bunkerID = 519;
                    break;
                case 145:
                    currentLocation.locationName = "Bunker 9";
                    currentLocation.bunkerID = 524;
                    break;
            }
            break;
        default:
            currentLocation.slipstreamDifficulty = 6;
            currentLocation.highjumpDifficulty = 6;
            qDebug() << Q_FUNC_INFO << "no defined positions for level" << currentLocation.level;
        }
        loadedLocations.push_back(currentLocation);
        id++;

    }

    id = 0;
    bool positionIsDuplicate = false;
    std::vector<PickupLocation>::iterator currentLocation;
    for(currentLocation = loadedLocations.begin(); currentLocation != loadedLocations.end(); id++){
        positionIsDuplicate = duplicateLocation(*currentLocation);
        if (positionIsDuplicate){
            currentLocation = loadedLocations.erase(currentLocation);
        } else {
            currentLocation++;
        }
    }

    qDebug() << Q_FUNC_INFO << "Total loaded locations:" << loadedLocations.size();
    for(int i = 0; i < loadedLocations.size(); i++){
        QVector3D debugPosition = loadedLocations[i].searchAttributes<QVector3D>("Position");
        qDebug() << Q_FUNC_INFO << i << " " << loadedLocations[i].uniqueID << "  " << loadedLocations[i].linkedLocationID << "    "
                 << loadedLocations[i].level << "    " << loadedLocations[i].locationName << "    " << debugPosition.x()
                 << "   " << debugPosition.y() << "  " << debugPosition.z();
    }
}

void DataHandler::loadMinicons(){

    for(int i = 0; i < levelList.size(); i++){
        std::vector<dictItem> filePickupsBase = levelList[i].levelFile->sendInstances("PickupPlaced");
        for(int pickup = 0; pickup < filePickupsBase.size(); pickup++){
            //qDebug() << Q_FUNC_INFO << "pickup" << pickup << "has instance ID" << filePickupsBase[pickup].instanceIndex << "and name" << filePickupsBase[pickup].name;
            pickupList.push_back(filePickupsBase[pickup]);
        }
    }

    foreach(Pickup currentPickup, pickupList){
        bool miniconIsLoaded = false;
        bool dataconIsLoaded = false;

        qDebug() << Q_FUNC_INFO << "pickup properties for:" << currentPickup.pickupToSpawn << "enumID" << currentPickup.enumID << "dataID" << currentPickup.dataID;
        miniconIsLoaded = miniconLoaded(currentPickup.enumID);
        dataconIsLoaded = dataconLoaded(currentPickup.dataID);
        qDebug() << Q_FUNC_INFO << "already loaded? minicon:" << miniconIsLoaded << "datacon:" << dataconIsLoaded;
        if(!miniconIsLoaded && currentPickup.enumID != 3){
            //if it's a minicon we don't already have, add it to the minicon list
            if(weaponList.contains(currentPickup.enumID)){
                currentPickup.isWeapon = true;
            }
            miniconList.push_back(currentPickup);
        } else if (miniconIsLoaded && currentPickup.enumID != 3){
            //if it's a minicon we already have, don't add it again
            continue;
        } else if (!dataconIsLoaded && currentPickup.enumID == 3){
            //we now know it's a datacon (but still check to be sure). if it hasn't been loaded, add it to the datacon list
            dataconList.push_back(currentPickup);
        } else if (dataconIsLoaded && currentPickup.enumID == 3){
            //if it has been loaded, just skip it
            continue;
        } else {
            //this should never happen, logically, but better safe than confused.
            //put some debugs here, just in case
            continue;
        }
    }

    qDebug() << Q_FUNC_INFO << "Remaining pickups to process (should be 0):" << pickupList.size();

    qDebug() << Q_FUNC_INFO << "Total loaded minicons:" << miniconList.size();
    for(int i = 0; i < miniconList.size(); i++){
        qDebug() << Q_FUNC_INFO << i << " " << miniconList[i].enumID << "  " << miniconList[i].pickupToSpawn << "    "
                 << miniconList[i].dataID;
    }

    qDebug() << Q_FUNC_INFO << "Total loaded datacons:" << dataconList.size();
    for(int i = 0; i < dataconList.size(); i++){
        qDebug() << Q_FUNC_INFO << i << " " << dataconList[i].enumID << "  " << dataconList[i].pickupToSpawn << "    "
                 << dataconList[i].dataID;
    }


    //std::sort(miniconList.begin(), miniconList.end());
    //std::sort(dataconList.begin(), dataconList.end());
}

void DataHandler::loadAutobots(){
    std::shared_ptr<DatabaseFile> metagameFile;
    for(int i = 0; i < parent->databaseList.size(); i++){
        qDebug() << Q_FUNC_INFO << "checking file name" << parent->databaseList[i]->fileName;
        if(parent->databaseList[i]->fileName == "TFA-METAGAME"){
            metagameFile = parent->databaseList[i];
        }
    }

    autobotList = metagameFile->sendInstances("Autobot");
}

void DataHandler::loadMetagameMinicons(){
    std::shared_ptr<DatabaseFile> metagameFile;
    std::vector<dictItem> metagameMinicons;
    for(int i = 0; i < parent->databaseList.size(); i++){
        qDebug() << Q_FUNC_INFO << "checking file name" << parent->databaseList[i]->fileName;
        if(parent->databaseList[i]->fileName == "TFA-METAGAME"){
            metagameFile = parent->databaseList[i];
        }
    }
    QStringList miniconTypes = {"Minicon", "MiniconDamageBonus", "MiniconArmor", "MiniconEmergencyWarpgate", "MiniconRangeBonus", "MiniconRegeneration"};

    for(int type = 0; type < miniconTypes.size(); type++){
        metagameMinicons = metagameFile->sendInstances(miniconTypes[type]);
        for(int i = 0; i < metagameMinicons.size(); i++){
            QString currentName = metagameMinicons[i].searchAttributes<QString>("Name");
            Minicon *currentMinicon = getMinicon(currentName);
            if(currentMinicon != nullptr){
                //qDebug() << Q_FUNC_INFO << "successfully found minicon" << currentName << "with enumID" << currentMinicon->enumID;
                currentMinicon->name = metagameMinicons[i].name;
                currentMinicon->attributes = metagameMinicons[i].attributes;
            }
        }
    }

}

void DataHandler::loadCustomLocations(){
    //this is really making me want to handle these as database files
    //actually, doing them this way with the header is nice for adding the
    //description as a tooltip later for location packs
    QString modPath = QCoreApplication::applicationDirPath() + "/Locations/";
    QDir modFolder(modPath);
    QDirIterator modIterator(modFolder.absolutePath());
    qDebug() << Q_FUNC_INFO << "next file info:" << modIterator.nextFileInfo().fileName() << "from path" << modFolder.absolutePath();
    bool headerFinished = false;
    TextProperty modProperty;
    QStringList propertyOptions = {"File Version", "Name", "Author", "Description", "Location Count", "Level", "Location Name", "Coordinates", "Location ID", "Linked Locations", "Highjump Difficulty", "Slipstream Difficulty"};
    int modVersion = 0;

    while (modIterator.hasNext()){
        QFile currentModFile = modIterator.next();
        qDebug() << Q_FUNC_INFO << "Current file" << currentModFile.fileName();
        if (currentModFile.open(QIODevice::ReadOnly)){
            qDebug() << Q_FUNC_INFO << "Reading file";
            FileData modBuffer;
            modBuffer.dataBytes = currentModFile.readAll();
            modBuffer.input = true;
            CustomLocations currentLocations;
            headerFinished = false;
            int targetLevel = -1;
            int locationValue = 0;
            while(!headerFinished){
                modProperty = modBuffer.readProperty();
                qDebug() << Q_FUNC_INFO << "test property type:" << modProperty.name << "with value:" << modProperty.readValue;
                switch(propertyOptions.indexOf(modProperty.name)){
                case 0: //File Version
                    modVersion = modProperty.readValue.toInt();
                    break;
                case 1: //Name
                    currentLocations.name = modProperty.readValue;
                    break;
                case 2: //Author
                    currentLocations.author = modProperty.readValue;
                    break;
                case 3: //Description
                    currentLocations.description = modProperty.readValue;
                    //for human use only, for now. tooltips later.
                    break;
                case 4: //Location count
                    currentLocations.locationCount = modProperty.readValue.toInt();
                    headerFinished = true;
                    break;
                default:
                    qDebug() << Q_FUNC_INFO << "Unknown header property" << modProperty.name << "with value" << modProperty.readValue << "found at" << modBuffer.currentPosition;
                }
            }
            for(int i = 0; i <currentLocations.locationCount; i++){
                PickupLocation customLocation = PickupLocation();
                customLocation.level = targetLevel;
                //double-check that the below doesn't need to find the specific database for the target level
                customLocation.attributes = levelList[0].levelFile->generateAttributes("PickupPlaced");
                bool readingLocation = true;
                while(readingLocation){
                    modProperty = modBuffer.readProperty();
                    switch(propertyOptions.indexOf(modProperty.name)){
                        case 5: //Level
                            for(int i = 0; i < levelList.size(); i++){
                                qDebug() << Q_FUNC_INFO << "comparing level" << levelList[i].levelName << "to" << modProperty.readValue;
                                if(levelList[i].levelName == modProperty.readValue){
                                    qDebug() << Q_FUNC_INFO << "comparing level: match";
                                    customLocation.level = i;
                                }
                            }
                            if(customLocation.level == -1){
                                qDebug() << Q_FUNC_INFO << "Invalid level:" << modProperty.readValue;
                            }
                            break;
                        case 6: //Location Name
                            customLocation.name = modProperty.readValue;
                            qDebug() << Q_FUNC_INFO << "name read as" << modProperty.readValue;
                            break;
                        case 7: //Coordinates
                            {//combine the x y and z into a QVector3D.
                            QStringList locationSplit = modProperty.readValue.split(", ");
                            if(locationSplit.size() < 3){
                                qDebug() << Q_FUNC_INFO << "Invalid location:" << modProperty.readValue;
                                customLocation.setAttribute("Position", "0, 0, 0");
                            } else {
                                QString tempx = locationSplit[0];
                                float x = tempx.toFloat();
                                QString tempy = locationSplit[1];
                                float y = tempy.toFloat();
                                QString tempz = locationSplit[2];
                                float z = tempz.toFloat();
                                customLocation.setAttribute("Position", QString::number(x) + ", " + QString::number(y) + ", " + QString::number(z));
                            }
                            break;
                        }
                        case 8: //LocationID
                            customLocation.uniqueID = modProperty.readValue.toInt();
                            break;
                        case 9: //Linked Location
                            locationValue = modProperty.readValue.toInt();
                            if(locationValue != 0){
                                customLocation.linkedLocationID = locationValue;
                            }
                            break;
                        case 10: //Highjump difficulty
                            customLocation.highjumpDifficulty = modProperty.readValue.toInt();
                            break;
                        case 11: //Slipstream difficulty
                            customLocation.slipstreamDifficulty = modProperty.readValue.toInt();
                            readingLocation = false;
                            break;
                        default:
                            qDebug() << Q_FUNC_INFO << "Unknown property" << modProperty.name << "with value" << modProperty.readValue << "found at" << modBuffer.currentPosition;
                    }
                }
                qDebug() << Q_FUNC_INFO << "Adding location" << customLocation.name << "for level" << customLocation.level << "at coordinages" << customLocation.searchAttributes<QVector3D>("Position");
                currentLocations.locationList.push_back(customLocation);
            }
            customLocationList.push_back(currentLocations);
        }
        qDebug() << Q_FUNC_INFO << "file" << currentModFile.fileName();
    }
}



bool DataHandler::duplicateLocation(PickupLocation testLocation){
    QVector3D loadedPosition;
    QVector3D testPosition = testLocation.searchAttributes<QVector3D>("Position");
    int locationCount = 0;
    for(int i = 0; i < loadedLocations.size(); i++){
        loadedPosition = loadedLocations[i].searchAttributes<QVector3D>("Position");
        if((testPosition.x() < loadedPosition.x()+5 && testPosition.x() > loadedPosition.x() - 5)
                && (testPosition.y() < loadedPosition.y()+5 && testPosition.y() > loadedPosition.y() - 5)
                && (testPosition.z() < loadedPosition.z()+5 && testPosition.z() > loadedPosition.z() - 5)
                && (testLocation.level == loadedLocations[i].level)){
            locationCount++;
        }
        if(locationCount > 1){
           return true;
        }
    }
    return false;
}

bool DataHandler::miniconLoaded(int checkID){
    for(int i = 0; i < miniconList.size(); i++){
        if(checkID == miniconList[i].enumID){
            return true;
        }
    }
    return false;
}

bool DataHandler::dataconLoaded(int checkID){
    int pickupCount = 0;
    for(int i = 0; i < dataconList.size(); i++){
        if(checkID == dataconList[i].dataID){
            pickupCount++;
        }
        if(pickupCount > 0){
            return true;
        }
    }
    return false;
}

void PickupLocation::assignMinicon(int miniconID){
    qDebug() << Q_FUNC_INFO << "Assigning minicon" << miniconID << "to position" << uniqueID;
    setAttribute("PickupToSpawn", QString::number(miniconID));
    qDebug() << Q_FUNC_INFO << "checking placement" << searchAttributes<int>("PickupToSpawn") << "vs" << miniconID;
    qDebug() << Q_FUNC_INFO << "done assigning minicon";
}

void PickupLocation::assignDatacon(int dataID){
    qDebug() << Q_FUNC_INFO << "Assigning datacon" << dataID << "to position" << uniqueID;
    setAttribute("PickupToSpawn", QString::number(3));
    setAttribute("ProductionArt", QString::number(dataID));
    qDebug() << Q_FUNC_INFO << "done assigning datacon";
}

int PickupLocation::assignedMinicon(){
    int miniconID = 0;
    miniconID = searchAttributes<int>("PickupToSpawn");
    //qDebug() << Q_FUNC_INFO << "Position" << uniqueID << "currently has pickup ID" << miniconID;
    return miniconID;
}

PickupLocation::PickupLocation(dictItem fromItem){
    QVector3D location = fromItem.searchAttributes<QVector3D>("Position");
    setAttribute("Position", QString::number(location.x()) + ", " + QString::number(location.y()) + ", " + QString::number(location.z()));
    originalDatabaseInstance = fromItem.instanceIndex;
    attributes = fromItem.attributes;
    //assignMinicon(0);
    spoiled = false;
    this->name = fromItem.name;
    bunkerID = 0;
    instanceIndex = 0;
    linkedLocationID = 999;
    this->spawnEvent = fromItem.searchAttributes<QString>("SpawnEvent");
    setAttributeDefault("GenerationDifficulty");
    setAttributeDefault("ProductionArt");
}

PickupLocation::PickupLocation(){
    setAttribute("Position", "0, 0, 0");
    originalDatabaseInstance = 0;
    assignMinicon(0);
    bunkerID = 0;
    instanceIndex = 0;
    linkedLocationID = 999;
    spoiled = false;
}
