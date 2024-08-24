#include "Headers/Main/mainwindow.h"

void Randomizer::loadLevels(){
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
        levelList.push_back(nextLevel);
    }
}

void Randomizer::loadMinicons(){
    //go through the database files and get the minicon items. then split the minicons into minicons and positions
    //this is probably where all the static stuff is gonna be. Might move this to another file, or at least to the end of this one, just for that
    //ideally this will be handled with an external file - maybe a custom TDB?
    int totalRemoved = 0;
    for(int i = 0; i < parent->databaseList.size(); i++){
        std::vector<Pickup> filePickups = parent->databaseList[i]->sendPickups();
        levelList[i].maxInstances = parent->databaseList[i]->maxInstances;
        levelList[i].removedInstances = filePickups.size()-2; //to account for glide and highjump placements
        totalRemoved += levelList[i].removedInstances;
        for(int pickup = 0; pickup < filePickups.size(); pickup++){
            pickupList.push_back(filePickups[pickup]);
            pickupList[pickupList.size()-1].level = i;
        }
    }

    qDebug() << Q_FUNC_INFO << "Total removed pickups (total available spaces):" << totalRemoved;

    //We'll need to go through the available locations and remove duplicate positions
    //or just not add them in the first place with the hard-coded positions we'll be using anyway

    addCustomLocation(42069, 0, QVector3D(887, -1741, 9)); //Beginner weapon
    addCustomLocation(200, 3, QVector3D(329, 2329, 121)); //Tidal Wave's Roof
    addCustomLocation(201, 4, QVector3D(1523, 1304, 2)); //Atoll Center

    //below is based on this answer:
    //https://stackoverflow.com/a/1604632
    std::vector<Pickup>::iterator currentPickup;
    int id = 0;
    //note that id increases in the for statement, despite not being used in the iterator. This is probably a big no-no but I think it's cool so it stays (for now)
    for(currentPickup = pickupList.begin(); currentPickup != pickupList.end(); id++){
        qDebug() << Q_FUNC_INFO << "level" << currentPickup->level << "pickup" << id << "named" << currentPickup->name << "is located at" << currentPickup->position() << "with vanilla spawn" << currentPickup->enumID;
        PickupLocation addedLocation(*currentPickup);
        addedLocation.level = currentPickup->level;
        addedLocation.uniqueID = id;

        //to be placed inside the switch:
        addedLocation.name = "Unnamed Location";
        switch(currentPickup->level){
        case 0: //Amazon 1
            addedLocation.slipstreamDifficulty = 0;
            addedLocation.highjumpDifficulty = 0;
            switch(addedLocation.uniqueID){
                case 0:
                    addedLocation.name = "Claymore Cave";
                    break;
                case 18: //1
                    addedLocation.name = "Ruined Temple Courtyard";
                    break;
                case 2:
                    addedLocation.slipstreamDifficulty = 3;
                    addedLocation.name = "Spire";
                    break;
                case 17: //3
                    addedLocation.name = "Tutorial Minicon";
                    break;
                case 4:
                    addedLocation.name = "Neighboring Mountain";
                    break;
                case 5:
                    addedLocation.name = "Ravine Cliff Cave";
                    break;
                case 6:
                    addedLocation.name = "Pressurepoint's Corner";
                    break;
                case 7:
                    addedLocation.name = "Foot of the Mountain";
                    break;
                case 8:
                    addedLocation.name = "Mountain Ruins";
                    break;
                case 9:
                    addedLocation.name = "Before the Waterfall Bridge";
                    break;
                case 20: //10
                    addedLocation.name = "Hidden Cave";
                    break;
                case 11:
                    addedLocation.name = "Spidertank Triplets";
                    break;
                case 12:
                    addedLocation.name = "Ruined Temple Alcove";
                    break;
                case 13:
                    addedLocation.name = "Forest before the Bridge";
                    break;
                case 19: //14
                    addedLocation.name = "Light Unit Party";
                    break;
                case 15:
                    addedLocation.name = "Ruined Temple Ledge";
                    break;
                case 16:
                    addedLocation.name = "Forest before Firefight";
                    break;
                case 21:
                    addedLocation.name = "Riverside Ledge";
                    break;
                case 22:
                    addedLocation.name = "Waterfall Base";
                    break;
                case 23:
                    addedLocation.name = "Forest after the Bridge";
                    break;
                case 24:
                    addedLocation.name = "Surprise";
                    break;
                case 25:
                    addedLocation.name = "Forgettable Location";
                    break;
                case 26:
                    addedLocation.name = "Medium Unit Party";
                    break;
                case 27:
                    addedLocation.name = "Forest Across from the Basin 1";
                    break;
                case 28:
                    addedLocation.name = "Forest Across from the Basin 2";
                    break;
                case 42069:
                    addedLocation.name = "Beginner Weapon";
                    break;
                }
            break;
        case 1: //Antarctica
            addedLocation.slipstreamDifficulty = 0;
            addedLocation.highjumpDifficulty = 0;
            switch(addedLocation.uniqueID){
                case 49: //29
                    addedLocation.name = "Crevasse Field Overlook";
                    break;
                case 30:
                    addedLocation.name = "Rock of Power";
                    addedLocation.highjumpDifficulty = 1;
                    break;
                case 45: //31
                    addedLocation.name = "Research Center Hangar";
                    break;
                case 44: //32
                    addedLocation.name = "Crashed Icebreaker";
                    break;
                case 47: //33
                    addedLocation.name = "Crashed Plane";
                    break;
                case 34:
                    addedLocation.name = "Research Base Office 1";
                    break;
                case 46: //35
                    addedLocation.name = "Beachfront Property";
                    break;
                case 51: //36
                    addedLocation.name = "Research Base Office 2";
                    break;
                case 37:
                    addedLocation.name = "Midfield Beacon";
                    break;
                case 38:
                    addedLocation.name = "Hide-and-Seek Champion";
                    break;
                case 39:
                    addedLocation.name = "Research Base Containers 1";
                    break;
                case 40:
                    addedLocation.name = "Research Base Containers 2";
                    break;
                case 41:
                    addedLocation.name = "Research Base Office 3";
                    break;
                case 48: //42
                    addedLocation.name = "Research Base Office 4";
                    break;
                case 50: //43
                    addedLocation.name = "Lonely Island";
                    break;
                case 52:
                    addedLocation.name = "Across the Field";
                    break;
            }
            break;
        case 2: //Amazon 2
            addedLocation.slipstreamDifficulty = 0;
            addedLocation.highjumpDifficulty = 0;
            switch(addedLocation.uniqueID){
                case 53:
                    addedLocation.name = "Island Altar";
                    break;
                case 67: //54
                    addedLocation.name = "Jungle Village Warpgate";
                    break;
                case 55:
                    addedLocation.name = "Top of the Temple";
                    break;
                case 66: //56
                    addedLocation.name = "Village at the Foot of the Hill";
                    break;
                case 57:
                    addedLocation.name = "Spawn Ledge";
                    break;
                case 58:
                    addedLocation.name = "Waterfall Rock";
                    break;
                case 59:
                    addedLocation.name = "Jungle Ruins";
                    break;
                case 60:
                    addedLocation.slipstreamDifficulty = 4;
                    addedLocation.name = "Patrolling Dropship";
                    break;
                case 61:
                    addedLocation.name = "Jungle Village Outskirts";
                    break;
                case 62:
                    addedLocation.name = "Field before the Bridge";
                    break;
                case 69: //63
                    addedLocation.name = "Temple Pool";
                    break;
                case 64:
                    addedLocation.name = "Temple Climb";
                    break;
                case 65:
                    addedLocation.name = "Mid-Jungle Ditch";
                    break;
                case 68:
                    addedLocation.name = "Temple Dead End";
                    break;
                case 70:
                    addedLocation.name = "Temple Side-Path";
                    break;
                case 71:
                    addedLocation.name = "Forest before the Bridge";
                    break;
                case 72:
                    addedLocation.name = "Antechamber";
                    break;
                case 73:
                    addedLocation.name = "Antechamber Alcove";
                    addedLocation.highjumpDifficulty = 2;
                    break;
                case 74:
                    addedLocation.name = "Behind the Temple";
                    break;
            }
            break;
        case 3: //Mid atlantic 1
            addedLocation.slipstreamDifficulty = 0;
            addedLocation.highjumpDifficulty = 0;
            switch(addedLocation.uniqueID){
                case 75:
                    addedLocation.linkedLocationID = 79;
                    addedLocation.name = "Vanilla Progression";
                    break;
                case 76:
                    addedLocation.linkedLocationID = 80;
                    addedLocation.slipstreamDifficulty = 5;
                    //this is the only location that is completely impossible without slipstream
                    addedLocation.name = "Distant Island";
                    break;
                case 77:
                    addedLocation.linkedLocationID = 82;
                    addedLocation.name = "Pinnacle Rock";
                    break;
                case 78:
                    addedLocation.linkedLocationID = 81;
                    addedLocation.name = "Atoll";
                    break;
            }
            break;
        case 4: //Mid atlantic 2
            addedLocation.slipstreamDifficulty = 3;
            addedLocation.highjumpDifficulty = 0;
            switch(addedLocation.uniqueID){
                case 79:
                    addedLocation.linkedLocationID = 75;
                    addedLocation.name = "Vanilla Progression";
                    break;
                case 80:
                    addedLocation.linkedLocationID = 76;
                    addedLocation.slipstreamDifficulty = 5;
                    //this is the only location that is completely impossible without slipstream
                    addedLocation.name = "Distant Island";
                    break;
                case 81:
                    addedLocation.linkedLocationID = 78;
                    addedLocation.name = "Atoll";
                    break;
                case 82:
                    addedLocation.linkedLocationID = 77;
                    addedLocation.name = "Pinnacle Rock";
                    break;
            }
            break;
        case 5: //Alaska
            addedLocation.slipstreamDifficulty = 4;
            addedLocation.highjumpDifficulty = 3; //can be lower for early placements
            switch(addedLocation.uniqueID){
                case 83:
                    addedLocation.name = "Mountaintop";
                    addedLocation.slipstreamDifficulty = 5;
                    addedLocation.highjumpDifficulty = 4;
                    break;
                case 84:
                    addedLocation.name = "Small Forest";
                    break;
                case 98: //85
                    addedLocation.name = "Peninsula";
                    break;
                case 86:
                    addedLocation.name = "Cave's Far Ledge";
                    addedLocation.slipstreamDifficulty = 5;
                    addedLocation.highjumpDifficulty = 4;
                    break;
                case 87:
                    addedLocation.name = "Cave's Right Fork";
                    addedLocation.slipstreamDifficulty = 5;
                    addedLocation.highjumpDifficulty = 4;
                    break;
                case 88:
                    addedLocation.name = "Cave's Left Fork";
                    addedLocation.slipstreamDifficulty = 5;
                    addedLocation.highjumpDifficulty = 4;
                    break;
                case 89:
                    addedLocation.name = "Cave's Pool";
                    addedLocation.slipstreamDifficulty = 5;
                    addedLocation.highjumpDifficulty = 4;
                    break;
                case 90:
                    addedLocation.name = "Along the Canyon";
                    break;
                case 91:
                    addedLocation.name = "Ocean Overlook";
                    break;
                case 97: //92
                    addedLocation.name = "Dropship Island";
                    break;
                case 93:
                    addedLocation.name = "Hidden Hill";
                    break;
                case 94:
                    addedLocation.name = "Canyon Clearing Ledge 1";
                    break;
                case 95:
                    addedLocation.name = "Battlefield";
                    break;
                case 96:
                    addedLocation.name = "Canyon Clearing Ledge 2";
                    break;
                case 99:
                    addedLocation.name = "Escape";
                    addedLocation.slipstreamDifficulty = 5;
                    break;
                case 100:
                    addedLocation.name = "Hillside";
                    break;

            }
            break;
        case 6: //Starship
            //must have either Slipstream or Highjump before starship, even for cheaters
            addedLocation.slipstreamDifficulty = 5;
            addedLocation.highjumpDifficulty = 6;
            switch(addedLocation.uniqueID){
                case 101:
                    addedLocation.name = "Start of the Climb";
                    break;
                case 102:
                    addedLocation.name = "First Steps";
                    break;
                case 103:
                    addedLocation.name = "Main Elevator";
                    break;
                case 104:
                    addedLocation.name = "Sideways Elevator";
                    break;
                case 105:
                    addedLocation.name = "Lower Dropship";
                    break;
                case 106:
                    addedLocation.name = "Bait";
                    break;
                case 107:
                    addedLocation.name = "Free Space";
                    break;
                case 108:
                    addedLocation.name = "Across the Gap";
                    break;
                case 109:
                    addedLocation.name = "Early Gift";
                    break;
                case 110:
                    addedLocation.slipstreamDifficulty = 4;
                    //slipstream can technically be on the Bridge as long as highjump is acquired before then
                    addedLocation.linkedLocationID = 111;
                    addedLocation.name = "Pristine Bridge";
                    break;
                case 111:
                    addedLocation.linkedLocationID = 110;
                    addedLocation.name = "Crashed Bridge";
                    break;
                case 112:
                    addedLocation.name = "Top of the Crashed Ship";
                    break;
                case 113:
                    addedLocation.name = "Dropdown 1";
                    break;
                case 114:
                    addedLocation.name = "Risky Jump";
                    break;
                case 115:
                    addedLocation.name = "Rock 1";
                    break;
                case 116:
                    addedLocation.name = "Middle Dropship";
                    break;
                case 117:
                    addedLocation.name = "Dropdown 2";
                    break;
                case 118:
                    addedLocation.name = "Rock 2";
                    break;
                case 119:
                    addedLocation.name = "Rock 3";
                    break;
                case 120:
                    addedLocation.name = "Rock 4";
                    break;
            }
            break;
        case 7: //Pacific Island
            addedLocation.slipstreamDifficulty = 5;
            addedLocation.highjumpDifficulty = 6;
            switch(addedLocation.uniqueID){
                case 132: //121
                    addedLocation.name = "Forest Basin";
                    break;
                case 122:
                    addedLocation.name = "Small Silo";
                    break;
                case 123:
                    addedLocation.name = "Moai";
                    break;
                case 124:
                    addedLocation.name = "Back of the Volcano";
                    break;
                case 125:
                    addedLocation.name = "Volcano Path";
                    break;
                case 133: //126
                    addedLocation.name = "Hidden Ledge";
                    break;
                case 127:
                    addedLocation.name = "Lighthouse";
                    break;
                case 128:
                    addedLocation.name = "Waterfall Climb";
                    break;
                case 129:
                    addedLocation.name = "Stronghold Ledge";
                    break;
                case 136: //130
                    addedLocation.name = "Small Island";
                    break;
                case 131:
                    addedLocation.name = "River Basin Peak (I think?)";
                    break;
                case 134:
                    addedLocation.name = "Above the Waterfall";
                    break;
                case 135:
                    addedLocation.name = "Large Silo";
                    break;
                case 137:
                    addedLocation.name = "Bunker 1";
                    addedLocation.bunkerID = 526;
                    break;
                case 138:
                    addedLocation.name = "Bunker 2";
                    addedLocation.bunkerID = 525;
                    break;
                case 139:
                    addedLocation.name = "Bunker 3";
                    addedLocation.bunkerID = 522;
                    break;
                case 140:
                    addedLocation.name = "Bunker 4";
                    addedLocation.bunkerID = 521;
                    break;
                case 141:
                    addedLocation.name = "Bunker 5";
                    addedLocation.bunkerID = 527;
                    break;
                case 142:
                    addedLocation.name = "Bunker 6";
                    addedLocation.bunkerID = 523;
                    break;
                case 143:
                    addedLocation.name = "Bunker 7";
                    addedLocation.bunkerID = 520;
                    break;
                case 144:
                    addedLocation.name = "Bunker 8";
                    addedLocation.bunkerID = 519;
                    break;
                case 145:
                    addedLocation.name = "Bunker 9";
                    addedLocation.bunkerID = 524;
                    break;
            }
            break;
        default:
            addedLocation.slipstreamDifficulty = 6;
            addedLocation.highjumpDifficulty = 6;
            qDebug() << Q_FUNC_INFO << "no defined positions for level" << currentPickup->level;
        }
        loadedLocations.push_back(addedLocation);

        bool miniconIsLoaded = false;
        bool dataconIsLoaded = false;

        qDebug() << Q_FUNC_INFO << "pickup properties for:" << currentPickup->name << "enumID" << currentPickup->enumID << "dataID" << currentPickup->dataID;
        miniconIsLoaded = miniconLoaded(currentPickup->enumID);
        dataconIsLoaded = dataconLoaded(currentPickup->dataID);
        qDebug() << Q_FUNC_INFO << "already loaded? minicon:" << miniconIsLoaded << "datacon:" << dataconIsLoaded;
        if(!miniconIsLoaded && currentPickup->enumID != 3){
            //if it's a minicon we don't already have, add it to the minicon list and remove it from the general pickups
            if(weaponList.contains(currentPickup->enumID)){
                currentPickup->isWeapon = true;
            }
            miniconList.push_back(*currentPickup);
            currentPickup = pickupList.erase(currentPickup);
        } else if (miniconIsLoaded && currentPickup->enumID != 3){
            //if it's a minicon we already have, just remove it from the list
            currentPickup = pickupList.erase(currentPickup);
        } else if (!dataconIsLoaded && currentPickup->enumID == 3){
            //we now know it's a datacon (but still check to be sure). if it hasn't been loaded, add it to the datacon list
            dataconList.push_back(*currentPickup);
            currentPickup = pickupList.erase(currentPickup);
        } else if (dataconIsLoaded && currentPickup->enumID == 3){
            //if it has been loaded, just remove it from the pickup list
            currentPickup = pickupList.erase(currentPickup);
        } else {
            //this should never happen, logically, but better safe than confused.
            //put some debugs here, just in case
            currentPickup++;
        }
    }
    qDebug() << Q_FUNC_INFO << "Total loaded minicons:" << miniconList.size();
    if(miniconList.size() > 40){
        qDebug() << Q_FUNC_INFO << "Too many minicons loaded. Outputting list now.";
        for(int i = 0; i < miniconList.size(); i++){
            qDebug() << Q_FUNC_INFO << "Loaded minicon" << i << "has name" << miniconList[i].name << "and ID" << miniconList[i].enumID;
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
        qDebug() << Q_FUNC_INFO << i << " " << loadedLocations[i].uniqueID << "  " << loadedLocations[i].linkedLocationID << "    "
                 << loadedLocations[i].level << "    " << loadedLocations[i].name << "    " << loadedLocations[i].position().x()
                 << "   " << loadedLocations[i].position().y() << "  " << loadedLocations[i].position().z();
    }
}

void Randomizer::loadCustomLocations(){
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
                customLocation.attributes = parent->databaseList[0]->generateAttributes("PickupPlaced");
                bool readingLocation = true;
                while(readingLocation){
                    modProperty = modBuffer.readProperty();
                    switch(propertyOptions.indexOf(modProperty.name)){
                        case 5: //Level
                            for(int i = 0; i < levelList.size(); i++){
                                if(levelList[i].levelName == modProperty.readValue){
                                    targetLevel = i;
                                }
                            }
                            if(targetLevel == -1){
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
                                customLocation.setPosition(QVector3D(0,0,0));
                            } else {
                                QString tempx = locationSplit[0];
                                float x = tempx.toFloat();
                                QString tempy = locationSplit[1];
                                float y = tempy.toFloat();
                                QString tempz = locationSplit[2];
                                float z = tempz.toFloat();
                                customLocation.setPosition(QVector3D(x,y,z));
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
                qDebug() << Q_FUNC_INFO << "Adding location" << customLocation.name << "for level" << customLocation.level << "at coordinages" << customLocation.position();
                currentLocations.locationList.push_back(customLocation);
            }
            customLocationList.push_back(currentLocations);
        }
        qDebug() << Q_FUNC_INFO << "file" << currentModFile.fileName();
    }
}

void Randomizer::addCustomLocation(int locationID, int level, QVector3D location){
    PickupLocation customLocation = PickupLocation();
    customLocation.uniqueID = locationID;
    customLocation.level = level;
    customLocation.attributes = parent->databaseList[0]->generateAttributes("PickupPlaced");
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
}

bool Randomizer::duplicateLocation(PickupLocation testLocation){
    QVector3D loadedPosition;
    QVector3D testPosition = testLocation.position();
    int locationCount = 0;
    for(int i = 0; i < loadedLocations.size(); i++){
        loadedPosition = loadedLocations[i].position();
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

bool Randomizer::miniconLoaded(int checkID){
    qDebug() << Q_FUNC_INFO << "minicon list size:" << miniconList.size();
    for(int i = 0; i < miniconList.size(); i++){
        qDebug() << Q_FUNC_INFO << "comparing checkID" << checkID << "to minicon" << miniconList[i].name << "ID" << miniconList[i].enumID;
        if(checkID == miniconList[i].enumID){
            return true;
        }
    }
    return false;
}

bool Randomizer::dataconLoaded(int checkID){
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
    isData = true;
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
    qDebug() << Q_FUNC_INFO << "Position" << uniqueID << "currently has pickup ID" << miniconID;
    return miniconID;
    /*for(int i = 0; i < attributes.size(); i++){
        if(attributes[i]->name == "PickupToSpawn"){
            miniconID = attributes[i]->intValue();
            return miniconID;
        }
    }
    return miniconID;*/
}


PickupLocation::PickupLocation(Pickup fromItem){
    setPosition(fromItem.position());
    originalDatabaseInstance = fromItem.instanceIndex;
    assignMinicon(0);
    spoiled = false;
    isData = false;
    bunkerID = 0;
    instanceIndex = 0;
    linkedLocationID = 999;
    attributes = fromItem.attributes;
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
    setPosition(QVector3D());
    originalDatabaseInstance = 0;
    assignMinicon(0);
    bunkerID = 0;
    instanceIndex = 0;
    linkedLocationID = 999;
    spoiled = false;
    isData = false;
}
