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
    this->attributes = copyItem.attributes;
    for(int i = 0; i < attributes.size(); i++){
        if(attributes[i]->name == "PickupToSpawn"){
            this->name = attributes[i]->display();
            this->enumID = attributes[i]->intValue();
            if(enumID > 3){
                this->isMinicon = true;
            } else {
                this->isMinicon = false;
            }
        }
        if(attributes[i]->name == "ProductionArt"){
            this->name = attributes[i]->display();
            this->dataID = attributes[i]->intValue();
        }
    }
    qDebug() << Q_FUNC_INFO << "adding pickup" << copyItem.instanceIndex << "name" << copyItem.name << "|" << name << "has enumID" << enumID << "is minicon?" << isMinicon;
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
    QVector3D location = copyItem.searchAttributes<QVector3D>("Position");
    this->setAttribute("Position", QString::number(location.x()) + ", " + QString::number(location.y()) + ", " + QString::number(location.z()));
    this->attributes = copyItem.attributes;
    for(int i = 0; i < attributes.size(); i++){
        if(attributes[i]->name == "PickupToSpawn"){
            this->name = attributes[i]->display();
            this->enumID = attributes[i]->intValue();
        }
    }
    placed = false;
}

Minicon::Minicon(){
    instanceIndex = 0;
    isWeapon = false;
    name = "Default Shockpunch";
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
}

void DataHandler::loadMinicons(){
    //go through the database files and get the minicon items. then split the minicons into minicons and positions
    //this is probably where all the static stuff is gonna be. Might move this to another file, or at least to the end of this one, just for that
    //ideally this will be handled with an external file - maybe a custom TDB?
    int totalRemoved = 0;
    /*for(int i = 0; i < parent->databaseList.size(); i++){
        qDebug() << Q_FUNC_INFO << "Checking database file:" << parent->databaseList[i]->fileName;
        std::vector<dictItem> filePickups = parent->databaseList[i]->sendInstances("PickupPlaced");
        if(filePickups.empty()){
            qDebug() << Q_FUNC_INFO << "File has no pickups. Skipping.";
            continue;
        }
        databaseLevels.push_back(parent->databaseList[i]);
        qDebug() << Q_FUNC_INFO << "File has pickups. Processing.";
    }*/
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

    qDebug() << Q_FUNC_INFO << "Total removed pickups (total available spaces):" << totalRemoved;

    //We'll need to go through the available locations and remove duplicate positions
    //or just not add them in the first place with the hard-coded positions we'll be using anyway

    /*addCustomLocation(42069, 0, QVector3D(887, -1741, 9)); //Beginner weapon
    addCustomLocation(200, 3, QVector3D(329, 2329, 121)); //Tidal Wave's Roof
    addCustomLocation(201, 4, QVector3D(1523, 1304, 2)); //Atoll Center*/

    //below is based on this answer:
    //https://stackoverflow.com/a/1604632
    //std::vector<Pickup>::iterator currentPickup;
    int id = 0;

    foreach(PickupLocation currentLocation, tempLocations){
        qDebug() << Q_FUNC_INFO << "level" << currentLocation.level << "pickup" << id << "named" << currentLocation.name << "is located at";
        currentLocation.uniqueID = id;
        //addedLocation.setAttribute("PickupToSpawn", "0");

        //to be placed inside the switch:
        currentLocation.name = "Unnamed Location";
        switch(currentLocation.level){
        case 0: //Amazon 1
            currentLocation.slipstreamDifficulty = 0;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 0:
                    currentLocation.name = "Claymore Cave";
                    break;
                case 18: //1
                    currentLocation.name = "Ruined Temple Courtyard";
                    break;
                case 2:
                    currentLocation.slipstreamDifficulty = 3;
                    currentLocation.name = "Spire";
                    break;
                case 17: //3
                    currentLocation.name = "Tutorial Minicon";
                    break;
                case 4:
                    currentLocation.name = "Neighboring Mountain";
                    break;
                case 5:
                    currentLocation.name = "Ravine Cliff Cave";
                    break;
                case 6:
                    currentLocation.name = "Pressurepoint's Corner";
                    break;
                case 7:
                    currentLocation.name = "Foot of the Mountain";
                    break;
                case 8:
                    currentLocation.name = "Mountain Ruins";
                    break;
                case 9:
                    currentLocation.name = "Before the Waterfall Bridge";
                    break;
                case 20: //10
                    currentLocation.name = "Hidden Cave";
                    break;
                case 11:
                    currentLocation.name = "Spidertank Triplets";
                    break;
                case 12:
                    currentLocation.name = "Ruined Temple Alcove";
                    break;
                case 13:
                    currentLocation.name = "Forest before the Bridge";
                    break;
                case 19: //14
                    currentLocation.name = "Light Unit Party";
                    break;
                case 15:
                    currentLocation.name = "Ruined Temple Ledge";
                    break;
                case 16:
                    currentLocation.name = "Forest before Firefight";
                    break;
                case 21:
                    currentLocation.name = "Riverside Ledge";
                    break;
                case 22:
                    currentLocation.name = "Waterfall Base";
                    break;
                case 23:
                    currentLocation.name = "Forest after the Bridge";
                    break;
                case 24:
                    currentLocation.name = "Surprise";
                    break;
                case 25:
                    currentLocation.name = "Forgettable Location";
                    break;
                case 26:
                    currentLocation.name = "Medium Unit Party";
                    break;
                case 27:
                    currentLocation.name = "Forest Across from the Basin 1";
                    break;
                case 28:
                    currentLocation.name = "Forest Across from the Basin 2";
                    break;
                case 42069:
                    currentLocation.name = "Beginner Weapon";
                    break;
                }
            break;
        case 1: //Antarctica
            currentLocation.slipstreamDifficulty = 0;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 49: //29
                    currentLocation.name = "Crevasse Field Overlook";
                    break;
                case 30:
                    currentLocation.name = "Rock of Power";
                    currentLocation.highjumpDifficulty = 1;
                    break;
                case 45: //31
                    currentLocation.name = "Research Center Hangar";
                    break;
                case 44: //32
                    currentLocation.name = "Crashed Icebreaker";
                    break;
                case 47: //33
                    currentLocation.name = "Crashed Plane";
                    break;
                case 34:
                    currentLocation.name = "Research Base Office 1";
                    break;
                case 46: //35
                    currentLocation.name = "Beachfront Property";
                    break;
                case 51: //36
                    currentLocation.name = "Research Base Office 2";
                    break;
                case 37:
                    currentLocation.name = "Midfield Beacon";
                    break;
                case 38:
                    currentLocation.name = "Hide-and-Seek Champion";
                    break;
                case 39:
                    currentLocation.name = "Research Base Containers 1";
                    break;
                case 40:
                    currentLocation.name = "Research Base Containers 2";
                    break;
                case 41:
                    currentLocation.name = "Research Base Office 3";
                    break;
                case 48: //42
                    currentLocation.name = "Research Base Office 4";
                    break;
                case 50: //43
                    currentLocation.name = "Lonely Island";
                    break;
                case 52:
                    currentLocation.name = "Across the Field";
                    break;
            }
            break;
        case 2: //Amazon 2
            currentLocation.slipstreamDifficulty = 0;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 53:
                    currentLocation.name = "Island Altar";
                    break;
                case 67: //54
                    currentLocation.name = "Jungle Village Warpgate";
                    break;
                case 55:
                    currentLocation.name = "Top of the Temple";
                    break;
                case 66: //56
                    currentLocation.name = "Village at the Foot of the Hill";
                    break;
                case 57:
                    currentLocation.name = "Spawn Ledge";
                    break;
                case 58:
                    currentLocation.name = "Waterfall Rock";
                    break;
                case 59:
                    currentLocation.name = "Jungle Ruins";
                    break;
                case 60:
                    currentLocation.slipstreamDifficulty = 4;
                    currentLocation.name = "Patrolling Dropship";
                    break;
                case 61:
                    currentLocation.name = "Jungle Village Outskirts";
                    break;
                case 62:
                    currentLocation.name = "Field before the Bridge";
                    break;
                case 69: //63
                    currentLocation.name = "Temple Pool";
                    break;
                case 64:
                    currentLocation.name = "Temple Climb";
                    break;
                case 65:
                    currentLocation.name = "Mid-Jungle Ditch";
                    break;
                case 68:
                    currentLocation.name = "Temple Dead End";
                    break;
                case 70:
                    currentLocation.name = "Temple Side-Path";
                    break;
                case 71:
                    currentLocation.name = "Forest before the Bridge";
                    break;
                case 72:
                    currentLocation.name = "Antechamber";
                    break;
                case 73:
                    currentLocation.name = "Antechamber Alcove";
                    currentLocation.highjumpDifficulty = 2;
                    break;
                case 74:
                    currentLocation.name = "Behind the Temple";
                    break;
            }
            break;
        case 3: //Mid atlantic 1
            currentLocation.slipstreamDifficulty = 0;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 75:
                    currentLocation.linkedLocationID = 79;
                    currentLocation.name = "Vanilla Progression";
                    break;
                case 76:
                    currentLocation.linkedLocationID = 80;
                    currentLocation.slipstreamDifficulty = 5;
                    //this is the only location that is completely impossible without slipstream
                    currentLocation.name = "Distant Island";
                    break;
                case 77:
                    currentLocation.linkedLocationID = 82;
                    currentLocation.name = "Pinnacle Rock";
                    break;
                case 78:
                    currentLocation.linkedLocationID = 81;
                    currentLocation.name = "Atoll";
                    break;
            }
            break;
        case 4: //Mid atlantic 2
            currentLocation.slipstreamDifficulty = 3;
            currentLocation.highjumpDifficulty = 0;
            switch(currentLocation.uniqueID){
                case 79:
                    currentLocation.linkedLocationID = 75;
                    currentLocation.name = "Vanilla Progression";
                    break;
                case 80:
                    currentLocation.linkedLocationID = 76;
                    currentLocation.slipstreamDifficulty = 5;
                    //this is the only location that is completely impossible without slipstream
                    currentLocation.name = "Distant Island";
                    break;
                case 81:
                    currentLocation.linkedLocationID = 78;
                    currentLocation.name = "Atoll";
                    break;
                case 82:
                    currentLocation.linkedLocationID = 77;
                    currentLocation.name = "Pinnacle Rock";
                    break;
            }
            break;
        case 5: //Alaska
            currentLocation.slipstreamDifficulty = 4;
            currentLocation.highjumpDifficulty = 3; //can be lower for early placements
            switch(currentLocation.uniqueID){
                case 83:
                    currentLocation.name = "Mountaintop";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 84:
                    currentLocation.name = "Small Forest";
                    break;
                case 98: //85
                    currentLocation.name = "Peninsula";
                    break;
                case 86:
                    currentLocation.name = "Cave's Far Ledge";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 87:
                    currentLocation.name = "Cave's Right Fork";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 88:
                    currentLocation.name = "Cave's Left Fork";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 89:
                    currentLocation.name = "Cave's Pool";
                    currentLocation.slipstreamDifficulty = 5;
                    currentLocation.highjumpDifficulty = 4;
                    break;
                case 90:
                    currentLocation.name = "Along the Canyon";
                    break;
                case 91:
                    currentLocation.name = "Ocean Overlook";
                    break;
                case 97: //92
                    currentLocation.name = "Dropship Island";
                    break;
                case 93:
                    currentLocation.name = "Hidden Hill";
                    break;
                case 94:
                    currentLocation.name = "Canyon Clearing Ledge 1";
                    break;
                case 95:
                    currentLocation.name = "Battlefield";
                    break;
                case 96:
                    currentLocation.name = "Canyon Clearing Ledge 2";
                    break;
                case 99:
                    currentLocation.name = "Escape";
                    currentLocation.slipstreamDifficulty = 5;
                    break;
                case 100:
                    currentLocation.name = "Hillside";
                    break;

            }
            break;
        case 6: //Starship
            //must have either Slipstream or Highjump before starship, even for cheaters
            currentLocation.slipstreamDifficulty = 5;
            currentLocation.highjumpDifficulty = 6;
            switch(currentLocation.uniqueID){
                case 101:
                    currentLocation.name = "Start of the Climb";
                    break;
                case 102:
                    currentLocation.name = "First Steps";
                    break;
                case 103:
                    currentLocation.name = "Main Elevator";
                    break;
                case 104:
                    currentLocation.name = "Sideways Elevator";
                    break;
                case 105:
                    currentLocation.name = "Lower Dropship";
                    break;
                case 106:
                    currentLocation.name = "Bait";
                    break;
                case 107:
                    currentLocation.name = "Free Space";
                    break;
                case 108:
                    currentLocation.name = "Across the Gap";
                    break;
                case 109:
                    currentLocation.name = "Early Gift";
                    break;
                case 110:
                    currentLocation.slipstreamDifficulty = 4;
                    //slipstream can technically be on the Bridge as long as highjump is acquired before then
                    currentLocation.linkedLocationID = 111;
                    currentLocation.name = "Pristine Bridge";
                    break;
                case 111:
                    currentLocation.linkedLocationID = 110;
                    currentLocation.name = "Crashed Bridge";
                    break;
                case 112:
                    currentLocation.name = "Top of the Crashed Ship";
                    break;
                case 113:
                    currentLocation.name = "Dropdown 1";
                    break;
                case 114:
                    currentLocation.name = "Risky Jump";
                    break;
                case 115:
                    currentLocation.name = "Rock 1";
                    break;
                case 116:
                    currentLocation.name = "Middle Dropship";
                    break;
                case 117:
                    currentLocation.name = "Dropdown 2";
                    break;
                case 118:
                    currentLocation.name = "Rock 2";
                    break;
                case 119:
                    currentLocation.name = "Rock 3";
                    break;
                case 120:
                    currentLocation.name = "Rock 4";
                    break;
            }
            break;
        case 7: //Pacific Island
            currentLocation.slipstreamDifficulty = 5;
            currentLocation.highjumpDifficulty = 6;
            switch(currentLocation.uniqueID){
                case 132: //121
                    currentLocation.name = "Forest Basin";
                    break;
                case 122:
                    currentLocation.name = "Small Silo";
                    break;
                case 123:
                    currentLocation.name = "Moai";
                    break;
                case 124:
                    currentLocation.name = "Back of the Volcano";
                    break;
                case 125:
                    currentLocation.name = "Volcano Path";
                    break;
                case 133: //126
                    currentLocation.name = "Hidden Ledge";
                    break;
                case 127:
                    currentLocation.name = "Lighthouse";
                    break;
                case 128:
                    currentLocation.name = "Waterfall Climb";
                    break;
                case 129:
                    currentLocation.name = "Stronghold Ledge";
                    break;
                case 136: //130
                    currentLocation.name = "Small Island";
                    break;
                case 131:
                    currentLocation.name = "River Basin Peak (I think?)";
                    break;
                case 134:
                    currentLocation.name = "Above the Waterfall";
                    break;
                case 135:
                    currentLocation.name = "Large Silo";
                    break;
                case 137:
                    currentLocation.name = "Bunker 1";
                    currentLocation.bunkerID = 526;
                    break;
                case 138:
                    currentLocation.name = "Bunker 2";
                    currentLocation.bunkerID = 525;
                    break;
                case 139:
                    currentLocation.name = "Bunker 3";
                    currentLocation.bunkerID = 522;
                    break;
                case 140:
                    currentLocation.name = "Bunker 4";
                    currentLocation.bunkerID = 521;
                    break;
                case 141:
                    currentLocation.name = "Bunker 5";
                    currentLocation.bunkerID = 527;
                    break;
                case 142:
                    currentLocation.name = "Bunker 6";
                    currentLocation.bunkerID = 523;
                    break;
                case 143:
                    currentLocation.name = "Bunker 7";
                    currentLocation.bunkerID = 520;
                    break;
                case 144:
                    currentLocation.name = "Bunker 8";
                    currentLocation.bunkerID = 519;
                    break;
                case 145:
                    currentLocation.name = "Bunker 9";
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


    foreach(Pickup currentPickup, pickupList){
        bool miniconIsLoaded = false;
        bool dataconIsLoaded = false;

        qDebug() << Q_FUNC_INFO << "pickup properties for:" << currentPickup.name << "enumID" << currentPickup.enumID << "dataID" << currentPickup.dataID;
        miniconIsLoaded = miniconLoaded(currentPickup.enumID);
        dataconIsLoaded = dataconLoaded(currentPickup.dataID);
        qDebug() << Q_FUNC_INFO << "already loaded? minicon:" << miniconIsLoaded << "datacon:" << dataconIsLoaded;
        if(!miniconIsLoaded && currentPickup.enumID != 3){
            //if it's a minicon we don't already have, add it to the minicon list and remove it from the general pickups
            if(weaponList.contains(currentPickup.enumID)){
                currentPickup.isWeapon = true;
            }
            miniconList.push_back(currentPickup);
            //currentPickup = pickupList.erase(currentPickup);
        } else if (miniconIsLoaded && currentPickup.enumID != 3){
            //if it's a minicon we already have, just remove it from the list
            //currentPickup = pickupList.erase(currentPickup);
            continue;
        } else if (!dataconIsLoaded && currentPickup.enumID == 3){
            //we now know it's a datacon (but still check to be sure). if it hasn't been loaded, add it to the datacon list
            dataconList.push_back(currentPickup);
            //currentPickup = pickupList.erase(currentPickup);
        } else if (dataconIsLoaded && currentPickup.enumID == 3){
            //if it has been loaded, just remove it from the pickup list
            //currentPickup = pickupList.erase(currentPickup);
            continue;
        } else {
            //this should never happen, logically, but better safe than confused.
            //put some debugs here, just in case
            //currentPickup++;
            continue;
        }
    }



    qDebug() << Q_FUNC_INFO << "Total loaded datacons:" << dataconList.size();
    qDebug() << Q_FUNC_INFO << "Remaining pickups to process (should be 0):" << pickupList.size();

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
                 << loadedLocations[i].level << "    " << loadedLocations[i].name << "    " << debugPosition.x()
                 << "   " << debugPosition.y() << "  " << debugPosition.z();
    }

    qDebug() << Q_FUNC_INFO << "Total loaded minicons:" << miniconList.size();
    for(int i = 0; i < miniconList.size(); i++){
        qDebug() << Q_FUNC_INFO << i << " " << miniconList[i].enumID << "  " << miniconList[i].name << "    "
                 << miniconList[i].dataID;
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

/*void DataHandler::addCustomLocation(int locationID, int level, QVector3D location){
    PickupLocation customLocation = PickupLocation();
    customLocation.uniqueID = locationID;
    customLocation.level = level;
    customLocation.attributes = levelList[0].levelFile->generateAttributes("PickupPlaced");
    customLocation.setPosition(location);
    switch(level){
        case 0: //Amazon 1
            customLocation.slipstreamDifficulty = 0;
            customLocation.highjumpDifficulty = 0;
            switch(customLocation.uniqueID){
                case 42069:
                    customLocation.name = "Beginner Weapon";
                    customLocation.slipstreamDifficulty = 6;
                    customLocation.highjumpDifficulty = 6;
                    break;
                }
            break;
        case 1: //Antarctica
            customLocation.slipstreamDifficulty = 0;
            customLocation.highjumpDifficulty = 0;
            switch(customLocation.uniqueID){

            }
            break;
        case 2: //Amazon 2
            customLocation.slipstreamDifficulty = 0;
            customLocation.highjumpDifficulty = 0;
            switch(customLocation.uniqueID){

            }
            break;
        case 3: //Mid atlantic 1
            customLocation.slipstreamDifficulty = 0;
            customLocation.highjumpDifficulty = 0;
            switch(customLocation.uniqueID){
                case 200:
                    customLocation.linkedLocationID = 201;
                    customLocation.name = "Tidal Wave's Roof";
                    break;
            }
            break;
        case 4: //Mid atlantic 2
            customLocation.slipstreamDifficulty = 3;
            customLocation.highjumpDifficulty = 0;
            switch(customLocation.uniqueID){
                case 201:
                    customLocation.linkedLocationID = 200;
                    customLocation.name = "Atoll Center";
                    break;
            }
            break;
        case 5: //Alaska
            customLocation.slipstreamDifficulty = 3;
            customLocation.highjumpDifficulty = 3; //can be lower for early placements
            switch(customLocation.uniqueID){


            }
            break;
        case 6: //Starship
            //must have either Slipstream or Highjump before starship, even for cheaters
            customLocation.slipstreamDifficulty = 5;
            customLocation.highjumpDifficulty = 6;
            switch(customLocation.uniqueID){

            }
            break;
        case 7: //Pacific Island
            customLocation.slipstreamDifficulty = 5;
            customLocation.highjumpDifficulty = 6;
            switch(customLocation.uniqueID){

            }
            break;
        default:
            customLocation.slipstreamDifficulty = 6;
            customLocation.highjumpDifficulty = 6;
            break;
    }

    loadedLocations.push_back(customLocation);
}*/

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
    qDebug() << Q_FUNC_INFO << "minicon list size:" << miniconList.size();
    for(int i = 0; i < miniconList.size(); i++){
        qDebug() << Q_FUNC_INFO << "comparing checkID" << checkID << "to minicon" << miniconList[i].name << "ID" << miniconList[i].enumID;
        if(checkID == miniconList[i].enumID){
            return true;
        }
    }
    return false;
}

bool DataHandler::dataconLoaded(int checkID){
    //qDebug() << Q_FUNC_INFO << "minicon list size:" << miniconList.size();
    int pickupCount = 0;
    for(int i = 0; i < pickupList.size(); i++){
        //qDebug() << Q_FUNC_INFO << "comparing checkID" << checkID << "to minicon" << miniconList[i].name << "ID" << miniconList[i].enumID;
        if(checkID == pickupList[i].dataID){
            pickupCount++;
        }
        if(pickupCount > 1){
            return true;
        }
    }
    return false;
}

void PickupLocation::assignMinicon(int miniconID){
    qDebug() << Q_FUNC_INFO << "Assigning minicon" << miniconID << "to position" << uniqueID;
    setAttribute("PickupToSpawn", QString::number(miniconID));
    qDebug() << Q_FUNC_INFO << "checking placement" << searchAttributes<int>("PickupToSpawn") << "vs" << miniconID;
    /*for(int i = 0; i < attributes.size(); i++){
        if(attributes[i]->name == "PickupToSpawn"){
            attributes[i]->setValue(QString::number(miniconID));
            //qDebug() << Q_FUNC_INFO << "pickuptospawn current minicon:" << attributes[i]->display();
        }
    }*/
    qDebug() << Q_FUNC_INFO << "done assigning minicon";
}

void PickupLocation::assignDatacon(int dataID){
    qDebug() << Q_FUNC_INFO << "Assigning datacon" << dataID << "to position" << uniqueID;
    setAttribute("PickupToSpawn", QString::number(3));
    setAttribute("ProductionArt", QString::number(dataID));
    /*for(int i = 0; i < attributes.size(); i++){
        if(attributes[i]->name == "PickupToSpawn"){
            attributes[i]->setValue(QString::number(3));
        }
        if(attributes[i]->name == "ProductionArt"){
            attributes[i]->setValue(QString::number(dataID));
        }
    }*/
    qDebug() << Q_FUNC_INFO << "done assigning datacon";
}

int PickupLocation::assignedMinicon(){
    int miniconID = 0;
    miniconID = searchAttributes<int>("PickupToSpawn");
    //qDebug() << Q_FUNC_INFO << "Position" << uniqueID << "currently has pickup ID" << miniconID;
    return miniconID;
    /*for(int i = 0; i < attributes.size(); i++){
        if(attributes[i]->name == "PickupToSpawn"){
            miniconID = attributes[i]->intValue();
            return miniconID;
        }
    }
    return miniconID;*/
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
    /*for(int i = 0; i < attributes.size(); i++){
        if(fromItem.attributes[i]->name == "SpawnEvent"){
            this->spawnEvent = fromItem.attributes[i]->stringValue();
        }
        if(attributes[i]->name == "GenerationDifficulty"){
            //this should work without issues?
            attributes[i]->isDefault = true;
        }
        if(attributes[i]->name == "ProductionArt"){
            //this should work without issues?
            attributes[i]->isDefault = true;
        }
    }*/
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
