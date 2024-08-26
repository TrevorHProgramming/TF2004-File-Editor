#include "Headers/Main/mainwindow.h"

/*
Randomizer class needs to have 3 lists of locations:
original, static list of positions  QMap<QVector3D, int> locationList; https://doc.qt.io/qt-6/qmap.html
    - this might technically just be generated with the constructor into the next list
working list of available locations
final randomized list of locations that will be written back to the files

calling randomizer rng generator "placemaster" for now

Currently errors for "no matching function" for erase and generate
generate was the wrong function for what I wanted - bounded is correct
erase needs an iterator instead of an int https://cplusplus.com/reference/vector/vector/erase/


check this out, make a test function with it:
https://doc.qt.io/qt-6/qbytearray.html#qUncompress-1
*/

Randomizer::Randomizer(ProgWindow *parentPass){
    //load minicons and locations here
    parent = parentPass;

    qDebug() << Q_FUNC_INFO << "loading database files";
    if(parent->loadDatabases() != 0){
        parent->log("Failed to find database files. Randomizer could not load.");
        return;
    }
    parent->clearWindow();

    QDir gameParent(parent->setW->getValue("Game extract path"));
    gameParent.cdUp();
    outputPath = gameParent.absolutePath() + "/Randomizer";
    //get containing directory
    //create Randomizer folder in that directory
    qDebug() << Q_FUNC_INFO << "path being checked:" << outputPath << "based on " << gameParent.absolutePath();
    QDir checkDir(outputPath);
    if(!checkDir.exists()){
        checkDir.mkpath(".");
    }

    loadLevels();
    loadMinicons();
    loadMods();
    loadCustomLocations();
    loadFileReplacements();

    for(int i = 0; i < replacementList.size(); i++){
        qDebug() << Q_FUNC_INFO << "replacement name:" << replacementList[i].name;
        for(int j = 0; j < replacementList[i].fileNames.size(); j++){
            qDebug() << Q_FUNC_INFO << "file" << j << "is named" << replacementList[i].fileNames[j] << "and is going to path" << replacementList[i].fileDestinations[j];
        }
    }

    seed = 0;
    randSettings.slipstreamDifficulty = 0;
    randSettings.highjumpDifficulty = 0;
    randSettings.overallDifficulty = 0;
    //generateDatacons = 0;
    settingsValue = "0000";

    QPushButton* buttonRandomize = new QPushButton("Randomize", parent->centralContainer);
    buttonRandomize->setGeometry(QRect(QPoint(50,220), QSize(150,30)));
    QAbstractButton::connect(buttonRandomize, &QPushButton::released, parent, [this] {randomize();});
    parent->currentModeWidgets.push_back(buttonRandomize);
    buttonRandomize->show();

    /*since the value of the settings and seed boxes will need to update when the settings/seed do,
    they need to be objects of the class so we can refer to them*/
    editSeed = new QLineEdit("", parent->centralContainer);
    editSeed->setGeometry(QRect(QPoint(200,220), QSize(150,30)));
    QLineEdit::connect(editSeed, &QLineEdit::textEdited, parent, [this](QString value) {setSeed(value);});
    parent->currentModeWidgets.push_back(editSeed);
    editSeed->show();

    editSettings = new QLineEdit("", parent->centralContainer);
    editSettings->setGeometry(QRect(QPoint(200,320), QSize(150,30)));
    parent->currentModeWidgets.push_back(editSettings);
    editSettings->show();

    QPushButton* buttonSetSettings = new QPushButton("Import Settings", parent->centralContainer);
    buttonSetSettings->setGeometry(QRect(QPoint(50,320), QSize(150,30)));
    QAbstractButton::connect(buttonSetSettings, &QPushButton::released, parent, [this] {manualSettings();});
    parent->currentModeWidgets.push_back(buttonSetSettings);
    buttonSetSettings->show();

    QPushButton* buttonGetSettings = new QPushButton("Export Settings", parent->centralContainer);
    buttonGetSettings->setGeometry(QRect(QPoint(50,350), QSize(150,30)));
    QAbstractButton::connect(buttonGetSettings, &QPushButton::released, parent, [this] {exportSettings();});
    parent->currentModeWidgets.push_back(buttonGetSettings);
    buttonGetSettings->show();

    QGroupBox *groupRandomizerOptions = new QGroupBox("Radomizer Options", parent->centralContainer);
    groupRandomizerOptions->setGeometry(QRect(QPoint(360,100), QSize(200,500)));
    groupRandomizerOptions->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0)");
    parent->currentModeWidgets.push_back(groupRandomizerOptions);

    QCheckBox *checkDatacon = new QCheckBox("Place Datacons", groupRandomizerOptions);
    checkDatacon->setGeometry(QRect(QPoint(20,20), QSize(200,30)));
    checkDatacon->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0)");
    QAbstractButton::connect(checkDatacon, &QCheckBox::stateChanged, parent, [checkDatacon, this]
        {randSettings.generateDatacons = checkDatacon->isChecked();
        qDebug() << Q_FUNC_INFO << "Datacon placement set to" << randSettings.generateDatacons;
        settingsValue.replace(3, 1, QString::number(randSettings.generateDatacons));});
    checkDatacon->show();
    checkDatacon->toggle();
    //parent->currentModeWidgets.push_back(checkDatacon);

    QSlider* sliderOverall  = new QSlider(Qt::Horizontal, groupRandomizerOptions);
    QLabel* labelOverall = new QLabel("General difficulty", groupRandomizerOptions);
    sliderOverall->setTickPosition(QSlider::TicksBelow);
    sliderOverall->setTickInterval(1);
    sliderOverall->setMaximum(5);
    labelOverall->setGeometry(QRect(QPoint(20,60), QSize(150,30)));
    sliderOverall->setGeometry(QRect(QPoint(20,90), QSize(150,30)));
    QSlider::connect(sliderOverall, &QAbstractSlider::valueChanged, parent, [this](int value) {setOverallDifficulty(value);});
    //parent->currentModeWidgets.push_back(sliderOverall);
    //labelOverall->setStyleSheet("QLabel { background-color : white;}");
    labelOverall->show();
    sliderOverall->show();

    QSlider* sliderSlipstream  = new QSlider(Qt::Horizontal, groupRandomizerOptions);
    QLabel* labelSlipstream = new QLabel("Slipstream difficulty", groupRandomizerOptions);
    sliderSlipstream->setTickPosition(QSlider::TicksBelow);
    sliderSlipstream->setTickInterval(1);
    sliderSlipstream->setMaximum(5);
    labelSlipstream->setGeometry(QRect(QPoint(20,120), QSize(150,30)));
    sliderSlipstream->setGeometry(QRect(QPoint(20,150), QSize(150,30)));
    QSlider::connect(sliderSlipstream, &QAbstractSlider::valueChanged, parent, [this](int value) {setSlipstreamDifficulty(value);});
    //parent->currentModeWidgets.push_back(sliderSlipstream);
    //labelSlipstream->setStyleSheet("QLabel { background-color : white;}");
    labelSlipstream->show();
    sliderSlipstream->show();

    QSlider* sliderHighjump  = new QSlider(Qt::Horizontal, groupRandomizerOptions);
    QLabel* labelHighjump = new QLabel("Highjump difficulty", groupRandomizerOptions);
    sliderHighjump->setTickPosition(QSlider::TicksBelow);
    sliderHighjump->setTickInterval(1);
    sliderHighjump->setMaximum(5);
    labelHighjump->setGeometry(QRect(QPoint(20,180), QSize(150,30)));
    sliderHighjump->setGeometry(QRect(QPoint(20,210), QSize(150,30)));
    QSlider::connect(sliderHighjump, &QAbstractSlider::valueChanged, parent, [this](int value) {setHighjumpDifficulty(value);});
    //parent->currentModeWidgets.push_back(sliderHighjump);
    //labelHighjump->setStyleSheet("QLabel { background-color : white;}");
    labelHighjump->show();
    sliderHighjump->show();

    randSettings.autoBuild = false;
    QCheckBox *checkAutoBuild = new QCheckBox("Automatically Build", groupRandomizerOptions);
    checkAutoBuild->setGeometry(QRect(QPoint(20,240), QSize(200,30)));
    QAbstractButton::connect(checkAutoBuild, &QCheckBox::stateChanged, parent, [checkAutoBuild, this] {randSettings.autoBuild = checkAutoBuild->isChecked();});
    checkAutoBuild->toggle();
    checkAutoBuild->show();
    //parent->currentModeWidgets.push_back(checkAutoBuild);

    QCheckBox *checkPowerBalance = new QCheckBox("Balanced Power Levels", groupRandomizerOptions);
    checkPowerBalance->setGeometry(QRect(QPoint(40,300), QSize(200,30)));
    QAbstractButton::connect(checkPowerBalance, &QCheckBox::stateChanged, parent, [checkPowerBalance, this] {randSettings.balancedPower = checkPowerBalance->isChecked();});
    checkPowerBalance->toggle();
    checkPowerBalance->hide();

    QCheckBox *checkPower = new QCheckBox("Randomize Power Levels", groupRandomizerOptions);
    checkPower->setGeometry(QRect(QPoint(20,270), QSize(200,30)));
    QAbstractButton::connect(checkPower, &QCheckBox::stateChanged, parent, [checkPower, checkPowerBalance, this]
        {randSettings.randomizePower = checkPower->isChecked();
        checkPowerBalance->setVisible(checkPower->isChecked());});
    checkPower->show();

    QCheckBox *checkTeamBalance = new QCheckBox("Balanced Team Colors", groupRandomizerOptions);
    checkTeamBalance->setGeometry(QRect(QPoint(40,360), QSize(200,30)));
    QAbstractButton::connect(checkTeamBalance, &QCheckBox::stateChanged, parent, [checkTeamBalance, this] {randSettings.balancedTeams = checkTeamBalance->isChecked();});
    checkTeamBalance->toggle();
    checkTeamBalance->hide();

    QCheckBox *checkTeam = new QCheckBox("Randomize Team Colors", groupRandomizerOptions);
    checkTeam->setGeometry(QRect(QPoint(20,330), QSize(200,30)));
    QAbstractButton::connect(checkTeam, &QCheckBox::stateChanged, parent, [checkTeam, checkTeamBalance, this]
        {randSettings.randomizePower = checkTeam->isChecked();
        checkTeamBalance->setVisible(checkTeam->isChecked());});
    checkTeam->show();

    groupRandomizerOptions->show();

    parent->centralContainer->setStyleSheet("QGroupBox{color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0);} "
                                            "QCheckBox{color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0);} "
                                            "QToolTip{color: rgb(0,0,0);}");

    QGroupBox *groupModOptions = new QGroupBox("Mod Options", parent->centralContainer);
    groupModOptions->setGeometry(QRect(QPoint(600,100), QSize(200,300)));
    /*groupModOptions->setStyleSheet("QGroupBox{color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0);} "
                                   "QCheckBox{color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0);} "
                                   "QToolTip{color: rgb(0,0,0);}");*/
    parent->currentModeWidgets.push_back(groupModOptions);

    for(int i = 0; i < modList.size(); i++){
        //this will need to be edited later for when we have more mods than will fit in the box to move to the next column. or scroll?

        QCheckBox *modCheck = new QCheckBox(modList[i].name, groupModOptions);
        modCheck->setGeometry(QRect(QPoint(20,20 + (40*i)), QSize(200,30)));
        //modCheck->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0);}");
        QAbstractButton::connect(modCheck, &QCheckBox::stateChanged, parent, [i, modCheck, this] {modList[i].enabled = modCheck->isChecked();});
        modCheck->setToolTip(modList[i].description);
        modCheck->show();
        parent->currentModeWidgets.push_back(modCheck);
    }

    QGroupBox *groupLocations = new QGroupBox("Custom Locations", parent->centralContainer);
    groupLocations->setGeometry(QRect(QPoint(800,100), QSize(200,300)));
    /*groupLocations->setStyleSheet("QGroupBox{color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0);} "
                                  "QCheckBox{color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0);} "
                                  "QToolTip{color: rgb(0,0,0);}");*/
    parent->currentModeWidgets.push_back(groupLocations);

    for(int i = 0; i < customLocationList.size(); i++){
        qDebug() << Q_FUNC_INFO << "location name for" << i << ":" << customLocationList[i].name;
        QCheckBox *locationCheck = new QCheckBox(customLocationList[i].name, groupLocations);
        locationCheck->setGeometry(QRect(QPoint(20,20 + (40*i)), QSize(200,30)));
        QAbstractButton::connect(locationCheck, &QCheckBox::stateChanged, parent, [i, locationCheck, this] {customLocationList[i].enabled = locationCheck->isChecked();});
        locationCheck->setToolTip(customLocationList[i].description);
        locationCheck->show();
        parent->currentModeWidgets.push_back(locationCheck);
    }


    groupModOptions->show();
    groupLocations->show();
    //checkBalancePatch->hide();
    //checkAlwaysGlider->hide();

    qDebug() << Q_FUNC_INFO << "testing randomization";

    //testAllPlacements();
    /*Uncommenting the above also requires commenting out the lines to spoil slipstream and highjump
    plus the removal of available locations in the place function*/

    //randomize();
}

void Randomizer::testAllPlacements(){
    for(int i = 0; i < availableLocations.size(); i++){
        place(27, availableLocations[i].uniqueID);
    }
    editDatabases();
    writeSpoilers();
}

void Randomizer::setSeed(QString value){
    //need to have a check if the seed is a valid number
    seed = value.toUInt();
}

void Randomizer::exportSettings(){
    parent->log("Current settings:");
    parent->log(settingsValue);
}

void Randomizer::manualSettings(){
    //check and set each setting if value is valid
    //if value is invalid, display error and reset value
    //if all values are valid, set settingsValue to new value
    QString checkSettings = editSettings->displayText();
    bool realInt = false;
    int checkSettingsInt = checkSettings.toInt(&realInt);
    if(!realInt){
        parent->log("Invalid settings value entered.");
        return;
    }
    /*currently 4 settings:
    generate datacons
    overall difficulty
    slipstream difficulty
    highjump difficulty
    */
    int dataconCheck = 0;
    int overallCheck = 0;
    int slipstreamCheck = 0;
    int highjumpCheck = 0;
    for(int i = 0; i < 4; i++){
        switch(i){
        case 0: //datacons
            dataconCheck = checkSettingsInt%10;
            checkSettingsInt /= 10;
            break;
        case 1: //overall difficulty
            overallCheck = checkSettingsInt%10;
            checkSettingsInt /= 10;
            break;
        case 2: //slipstream difficulty
            slipstreamCheck = checkSettingsInt%10;
            checkSettingsInt /= 10;
            break;
        case 3: //highjump difficulty
            highjumpCheck = checkSettingsInt%10;
            checkSettingsInt /= 10;
            break;
        default:
            parent->log("Invalid settings value entered.");
            return;
        }

    }
    if(dataconCheck > 1 || overallCheck > 4 || slipstreamCheck > 4 || highjumpCheck > 4){
        parent->log("Invalid settings value entered.");
        return;
    }

    randSettings.generateDatacons = dataconCheck;
    randSettings.overallDifficulty = overallCheck;
    randSettings.slipstreamDifficulty = slipstreamCheck;
    randSettings.highjumpDifficulty = highjumpCheck;

    settingsValue = checkSettings;

    parent->log("Settings imported. Note that sliders will not change visibly.");
}


void Randomizer::setHighjumpDifficulty(int value){
    randSettings.highjumpDifficulty = value;
    qDebug() << Q_FUNC_INFO << "Highjump difficulty set to:" << randSettings.highjumpDifficulty;
    settingsValue.replace(0, 1, QString::number(randSettings.highjumpDifficulty));
}

void Randomizer::setSlipstreamDifficulty(int value){
    randSettings.slipstreamDifficulty = value;
    qDebug() << Q_FUNC_INFO << "Slipstream difficulty set to:" << randSettings.slipstreamDifficulty;
    settingsValue.replace(1, 1, QString::number(randSettings.slipstreamDifficulty));
}

void Randomizer::setOverallDifficulty(int value){
    randSettings.overallDifficulty = value;
    qDebug() << Q_FUNC_INFO << "Overall difficulty set to:" << randSettings.overallDifficulty;
    settingsValue.replace(2, 1, QString::number(randSettings.overallDifficulty));
}

/*void Randomizer::setDataconOutput(bool placeDatacons){
    randSettings.generateDatacons = placeDatacons;
    qDebug() << Q_FUNC_INFO << "Datacon placement set to" << randSettings.generateDatacons;
    settingsValue.replace(3, 1, QString::number(randSettings.generateDatacons));
}*/

void Randomizer::reset(){
    placedLocations.clear();
    availableLocations.clear();

    for(int i = 0; i < levelList.size(); i++){
        levelList[i].dataconCount = 0;
        levelList[i].miniconCount = 0;
    }

    for(int i = 0; i < miniconList.size(); i++){
        miniconList[i].placed = false;
    }
    for(int i = 0; i < dataconList.size(); i++){
        dataconList[i].placed = false;
    }

}

void Randomizer::randomize(){
    if(placedLocations.size() > 0){
        reset();
    }
    if(seed == 0){
        parent->log("No seed set - generating random seed.");
        seed = QRandomGenerator::global()->generate();
        editSeed->setText(QString::number(seed));
    }
    qDebug() << Q_FUNC_INFO << "difficulties - slipstream:" << randSettings.slipstreamDifficulty << "highjump" << randSettings.highjumpDifficulty << "overall" << randSettings.overallDifficulty;
    //sort minicon list by enumID
    std::sort(miniconList.begin(), miniconList.end());
    availableLocations = loadedLocations;

    qDebug() << Q_FUNC_INFO << "Starting randomization. Available locations:" << availableLocations.size();
    placemaster.seed(seed);
    placeSlipstream();
    placeHighjump();
    placeRangefinder();
    placeShepherd();
    if(randSettings.overallDifficulty == 0){
        placeStarterWeapon();
    }
    placeAll();
    parent->log("Minicon placements randomized.");

    std::sort(placedLocations.begin(), placedLocations.end());
    qDebug() << Q_FUNC_INFO << "attempting to modify database files";
    editDatabases();
    writeSpoilers();

    fileReplacements();
    applyModifications();

    if(randSettings.autoBuild){
        parent->isoBuilder->packRandomizer();
    }
    parent->log("Database and spoiler files written. Randomization complete.");
    availableLocations.clear();
}

void Randomizer::loadFileReplacements(){
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



void Randomizer::fileReplacements(){
    for(int i = 0; i < replacementList.size(); i++){
        int replaceChance = placemaster.generate();
        if(replaceChance % replacementList[i].rarity == 0){
            for(int j = 0; j < replacementList[i].fileNames.size(); j++){
                replaceFile(replacementList[i].fileNames[j], replacementList[i].fileDestinations[j]);
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "Replacing title screen.";
    replaceFile("ARMADALOGO.ITF", "/TFA/USERINTERFACE/TEXTURES");
    return;
}

void Randomizer::loadMods(){
    QString modPath = QCoreApplication::applicationDirPath() + "/Mods/";
    QDir modFolder(modPath);
    QDirIterator modIterator(modFolder.absolutePath());
    qDebug() << Q_FUNC_INFO << "next file info:" << modIterator.nextFileInfo().fileName() << "from path" << modFolder.absolutePath();
    bool headerFinished = false;
    TextProperty modProperty;
    QStringList propertyOptions = {"Version", "Mod Name", "Mod Description", "Mod type", "Mod sections", "Starting address", "Section lines"};
    int modVersion = 0;

    while (modIterator.hasNext()){
        QFile currentModFile = modIterator.next();
        RandomizerMod currentModData;
        qDebug() << Q_FUNC_INFO << "Current file" << QFileInfo(currentModFile).fileName();
        currentModData.fileName = QFileInfo(currentModFile).fileName();
        if(QFileInfo(currentModFile).suffix() == "txt"){
            currentModData.type = 1;
        } else {
            currentModData.type = 0;
            qDebug() << Q_FUNC_INFO << "Skipping binary mod for now - not currently supported by SELF.";
            continue;
        }
        if (currentModFile.open(QIODevice::ReadOnly)){
            qDebug() << Q_FUNC_INFO << "Reading file";
            FileData modBuffer;
            modBuffer.dataBytes = currentModFile.readAll();
            modBuffer.input = true;
            headerFinished = false;
            while(!headerFinished){
                modProperty = modBuffer.readProperty();
                qDebug() << Q_FUNC_INFO << "test property type:" << modProperty.name << "with value:" << modProperty.readValue;
                switch(propertyOptions.indexOf(modProperty.name)){
                case 0: //Version
                    modVersion = modProperty.readValue.toInt();
                    if(modVersion < 2){
                        headerFinished = true;
                    }
                    break;
                case 1: //Mod Name
                    currentModData.name = modProperty.readValue;
                    break;
                case 2: //Mod Description
                    currentModData.description = modProperty.readValue;
                    headerFinished = true;
                    break;
                default:
                    qDebug() << Q_FUNC_INFO << "Unknown property" << modProperty.name << "with value" << modProperty.readValue << "found at" << modBuffer.currentPosition;
                }
            }
            qDebug() << Q_FUNC_INFO << "mod name:" << currentModData.name << "mod description:" << currentModData.description;
            modList.push_back(currentModData);
        }
    }
}

void Randomizer::applyModifications(){
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationDirPath();
    QString mipsEditorPath = QCoreApplication::applicationDirPath() + "/SELF.exe";
    qDebug() << Q_FUNC_INFO << mipsEditorPath;

    QStringList args;
    QString gamePath = parent->setW->getValue("Game extract path") + "/SLUS_206.68";
    QString modPath = QCoreApplication::applicationDirPath() + "/Mods/";
    if(gamePath == ""){
        qDebug() << Q_FUNC_INFO << "Process cancelled.";
        return;
    }
    parent->log("Modifying ELF from: " + gamePath);
    args.append(gamePath);
    args.append(modPath);

    QString modFiles = "";
    for(int i = 0; i < modList.size(); i++){
        if(modList[i].enabled){
            if(modFiles != ""){
                modFiles += "|";
            }
            modFiles += modList[i].fileName;
        }
    }
    qDebug() << Q_FUNC_INFO << "mod file list:" << modFiles;
    args.append(modFiles);

    QString randomizerPath = outputPath + "/SLUS_206.68";
    parent->log("Putting modified ELF in " + randomizerPath);
    args.append(randomizerPath);

    if(modFiles != ""){
        QProcess *mipsEditor = new QProcess();
        QObject::connect(mipsEditor, &QProcess::readyReadStandardOutput, [mipsEditor]() {
            QString debugOutput = mipsEditor->readAllStandardOutput();
            qDebug() << debugOutput;
            int outputCode = 0;
            if(debugOutput.left(4) == "Code"){
                debugOutput = debugOutput.right(debugOutput.length() - 5);
                qDebug() << "trimmed debug output:" << debugOutput;
                outputCode = debugOutput.left(4).toInt();
            } else {
                outputCode = 0;
            }
            switch(outputCode){
            case 0:
                //nothing to worry about
                break;
            default:
                //either nothing to worry about or something we haven't handled yet. output it.
                qDebug() << "Unhandled output code:" << debugOutput;
                break;
            }
        });
        parent->log("Modding ELF.");
        mipsEditor->start(mipsEditorPath, args);
    }

    qDebug() << Q_FUNC_INFO << "program arguments:" << args;

}

void Randomizer::placeSlipstream(){
    qDebug() << Q_FUNC_INFO << "Placing slipstream. Available locations" << availableLocations.size();
    std::vector<PickupLocation> slipstreamLocations;
    for(int i = 0; i < availableLocations.size(); i++){
        if(availableLocations[i].slipstreamDifficulty <= randSettings.slipstreamDifficulty){
            slipstreamLocations.push_back(availableLocations[i]);
        }
    }
    qDebug() << Q_FUNC_INFO << "Available slipstream locations:" << slipstreamLocations.size();
    int locationNumber = placemaster.bounded(slipstreamLocations.size());
    qDebug() << Q_FUNC_INFO << "Placing slipstream at" << slipstreamLocations[locationNumber].uniqueID;
    place(28, slipstreamLocations[locationNumber].uniqueID);
    if(randSettings.slipstreamDifficulty > 3){
        placeSlipstreamRequirement(44, slipstreamLocations[locationNumber].uniqueID);
        placeSlipstreamRequirement(14, slipstreamLocations[locationNumber].uniqueID);
        placeSlipstreamRequirement(49, slipstreamLocations[locationNumber].uniqueID);
    }
}

void Randomizer::placeHighjump(){
    qDebug() << Q_FUNC_INFO << "Placing highjump. Available locations" << availableLocations.size();
    std::vector<PickupLocation> highjumpLocations;
    for(int i = 0; i < availableLocations.size(); i++){
        if(availableLocations[i].highjumpDifficulty <= randSettings.highjumpDifficulty){
            highjumpLocations.push_back(availableLocations[i]);
        }
    }
    int locationNumber = placemaster.bounded(highjumpLocations.size());
    qDebug() << Q_FUNC_INFO << "Placing highjump at" << highjumpLocations[locationNumber].uniqueID;
    place(50, highjumpLocations[locationNumber].uniqueID);
}

void Randomizer::placeRangefinder(){
    qDebug() << Q_FUNC_INFO << "Placing rangefinder. Available locations" << availableLocations.size();
    std::vector<PickupLocation> rangefinderLocations;
    for(int i = 0; i < availableLocations.size(); i++){
        if((availableLocations[i].level <= 3 && randSettings.overallDifficulty < 4)
                || randSettings.overallDifficulty > 3){
            rangefinderLocations.push_back(availableLocations[i]);
        }
    }
    int locationNumber = placemaster.bounded(rangefinderLocations.size());
    qDebug() << Q_FUNC_INFO << "Placing Rangefinder at" << rangefinderLocations[locationNumber].uniqueID;
    place(43, rangefinderLocations[locationNumber].uniqueID);
}

void Randomizer::placeStarterWeapon(){
    std::vector<Minicon> starterMinicons;
    qDebug() << Q_FUNC_INFO << "Finding placement for starter weapon. minicons:" << miniconList.size();
    for(int i = 0; i < miniconList.size(); i++){
        qDebug() << Q_FUNC_INFO << "checking minicon" << i << "is weapon?" << miniconList[i].isWeapon;
        if(miniconList[i].isWeapon){
            qDebug() << Q_FUNC_INFO << "adding minicon:" << miniconList[i].enumID << miniconList[i].name;
            starterMinicons.push_back(miniconList[i]);
        }
    }
    int miniconNumber = placemaster.bounded(starterMinicons.size());
    qDebug() << Q_FUNC_INFO << "Placing starter weapon" << starterMinicons[miniconNumber].name;
    place(starterMinicons[miniconNumber].enumID, 42069); //locationID tbd
}

void Randomizer::placeShepherd(){
    qDebug() << Q_FUNC_INFO << "Finding placement for shepherd. available locations:" << availableLocations.size();
    std::vector<PickupLocation> shepherdLocations;
    for(int i = 0; i < availableLocations.size(); i++){
        if((bunkerList.contains(availableLocations[i].uniqueID) && randSettings.overallDifficulty > 3)
                || (availableLocations[i].level == 7 && randSettings.overallDifficulty == 3)
                || (randSettings.overallDifficulty == 2)
                || (availableLocations[i].level < 5 && randSettings.overallDifficulty < 2)){
            shepherdLocations.push_back(availableLocations[i]);
        }
    }
    int locationNumber = placemaster.bounded(shepherdLocations.size());
    qDebug() << Q_FUNC_INFO << "Placing shepherd at" << shepherdLocations[locationNumber].uniqueID;
    place(42, shepherdLocations[locationNumber].uniqueID);
}

void Randomizer::placeSlipstreamRequirement(int miniconID, int placementID){
    //for use in higher difficulty slipstream settings
    //ex. hailstorm + comeback for stasis lock ragdoll launches
    std::vector<PickupLocation> prereqLocations;
    int placementLevel = 0;
    //looking through placedlocations since slipstream should already be placed
    for(int i = 0; i < placedLocations.size(); i++){
        if(placedLocations[i].uniqueID == placementID){
            placementLevel  = placedLocations[i].level;
            //qDebug() << Q_FUNC_INFO << "prereq slipstream level found as" << placementLevel;
        }
    }
    //qDebug() << Q_FUNC_INFO << "determining prerequisite locations for" << miniconID << "and placement ID" << placementID;
    for(int i = 0; i < availableLocations.size(); i++){
        if((availableLocations[i].slipstreamDifficulty <= 3)
                && (availableLocations[i].level <= placementLevel)
                && (availableLocations[i].uniqueID != 42069)){ //we don't want one of the requirements spawning right at the start - the player asked for pain.
            //qDebug() << Q_FUNC_INFO << "prereq location" << availableLocations[i].uniqueID << "name" << availableLocations[i].name << "added to possible list.";
            prereqLocations.push_back(availableLocations[i]);
        }
    }
    //qDebug() << Q_FUNC_INFO << "prereq locations:" << prereqLocations.size();
    int locationNumber = placemaster.bounded(prereqLocations.size());
    place(miniconID, prereqLocations[locationNumber].uniqueID);
}

void Randomizer::place(int miniconToPlace, int placementID){
    //assign the minicon to the location
    //copy the location to the placed list
    //then remove that location and minicon from the working lists
    qDebug() << Q_FUNC_INFO << "Placing minicon" << miniconToPlace << "at" << placementID;
    Minicon* chosenMinicon = nullptr;
    for(int i = 0; i < miniconList.size(); i++){
        if(miniconList[i].enumID == miniconToPlace){
            //miniconList[i].placed = true;
            chosenMinicon = &miniconList[i];
        }
    }
    if(chosenMinicon == nullptr){
        parent->log("Minicon "  + QString::number(miniconToPlace) + " was not found. | " + QString(Q_FUNC_INFO));
        return;
    }
    for(int i = 0; i < availableLocations.size(); i++){
        if(availableLocations[i].uniqueID != placementID){
            continue;
        }
        qDebug() << Q_FUNC_INFO << "found location ID" << placementID << "with linked location" << availableLocations[i].linkedLocationID;
        qDebug() << Q_FUNC_INFO << "current placed pickups" << placedLocations.size();
        int linkedLocation = availableLocations[i].linkedLocationID;
        availableLocations[i].assignMinicon(miniconToPlace);
        placedLocations.push_back(availableLocations[i]);
        levelList[availableLocations[i].level].miniconCount++;
        availableLocations.erase(availableLocations.begin() + i);
        if(linkedLocation != 999){
            qDebug() << Q_FUNC_INFO << "searching for linked location" << linkedLocation;
            bool foundLocation = false;
            for(int j = 0; j < availableLocations.size(); j++){
                //qDebug() << Q_FUNC_INFO << "checking linked location ID" << availableLocations[j].uniqueID << "vs" << linkedLocation;
                if(availableLocations[j].uniqueID != linkedLocation){
                    continue;
                }
                qDebug() << Q_FUNC_INFO << "found linked location ID" << linkedLocation;
                foundLocation = true;
                availableLocations[j].assignMinicon(miniconToPlace);
                placedLocations.push_back(availableLocations[j]);
                levelList[availableLocations[j].level].miniconCount++;
                availableLocations.erase(availableLocations.begin() + j);
            }
            if(!foundLocation){
                qDebug() << Q_FUNC_INFO << "did not find location" << linkedLocation;
            }
        }
        qDebug() << Q_FUNC_INFO << "current placed pickups" << placedLocations.size();
        chosenMinicon->placed = true;
        return;
    }
    parent->log("Location "  + QString::number(placementID) + " was not found. | " + QString(Q_FUNC_INFO));
}

void Randomizer::placeAll(){
    //use std::shuffle with the randomizer's rng
    qDebug() << Q_FUNC_INFO << "Placing all remaining minicons. Currently" << miniconList.size() << "pickups to go through and place";
    //std::shuffle(std::begin(availableLocations), std::end(availableLocations), placemaster);
    //with this, we have three lists: the minicon list, the location list, and the list of placed locations
    //since we'll have more locations than minicons, we can shuffle the location list
    //and then assign minicons to it 1-to-1, copying those to the placed locations list.
    //then the placed locations combos would be written back to the database files
    int locationNumber = 0;
    int availableLevel = 0;
    for(int i = 0; i < miniconList.size(); i++){
        qDebug() << Q_FUNC_INFO << "has minicon" << i << miniconList[i].name << "already been placed?" << miniconList[i].placed;
        if(!miniconList[i].placed){
            qDebug() << Q_FUNC_INFO << "Placing minicon.";
            locationNumber = placemaster.bounded(availableLocations.size());
            qDebug() << Q_FUNC_INFO << "level" << levelList[availableLocations[locationNumber].level].levelName << "has" << levelList[availableLocations[locationNumber].level].miniconCount << "placements already";
            //The below doesn't account for special placement counts (slipstream, highjump, etc) so some levels end up stacked anyway.
            availableLevel = availableLocations[locationNumber].level;
            while(levelList[availableLevel].miniconCount > 6 /*|| (levelList[availableLevel].removedInstances < levelList[availableLevel].miniconCount)*/){
                qDebug() << Q_FUNC_INFO << "Level" << levelList[availableLevel].levelName << "has too many placements. Rerolling.";
                qDebug() << Q_FUNC_INFO << "Max placements:" << levelList[availableLevel].removedInstances << "current placements:" << levelList[availableLevel].miniconCount;
                locationNumber = placemaster.bounded(availableLocations.size());
                availableLevel = availableLocations[locationNumber].level;
            }
            //qDebug() << Q_FUNC_INFO << "Placing minicon" << miniconList[i].enumID << "at" << availableLocations[locationNumber].uniqueID << "or" << availableLocations[locationNumber].level << availableLocations[locationNumber].position();
//            availableLocations[locationNumber].assignMinicon(miniconList[i].enumID);
//            placedLocations.push_back(availableLocations[locationNumber]);
//            levelList[availableLevel].miniconCount++;
//            miniconList[i].placed = true;
//            availableLocations.erase(availableLocations.begin() + locationNumber);
            place(miniconList[i].enumID, availableLocations[locationNumber].uniqueID);
        } else {
            qDebug() << Q_FUNC_INFO << "Skipping placement.";
        }
    }

    if(randSettings.generateDatacons){
        qDebug() << Q_FUNC_INFO << "Placing datacons.";
        std::sort(dataconList.begin(), dataconList.end());
        for(int i = 0; i < dataconList.size(); i++){
            qDebug() << Q_FUNC_INFO << "has datacon" << i << dataconList[i].name << "already been placed?" << dataconList[i].placed;
            if(!dataconList[i].placed){
                qDebug() << Q_FUNC_INFO << "Placing datacon.";
                qDebug() << Q_FUNC_INFO << "current placed pickups" << placedLocations.size();
                locationNumber = placemaster.bounded(availableLocations.size());
                availableLevel = availableLocations[locationNumber].level;
                while(levelList[availableLevel].dataconCount > 12){
                    qDebug() << Q_FUNC_INFO << "Level" << levelList[availableLevel].levelName << "has too many datacons. Rerolling.";
                    qDebug() << Q_FUNC_INFO << "Max placements:" << levelList[availableLevel].removedInstances << "current datacons:" << levelList[availableLevel].dataconCount << "available locations:" << availableLocations.size();
                    locationNumber = placemaster.bounded(availableLocations.size());
                    availableLevel = availableLocations[locationNumber].level;
                }
                //fix below to work with linked locations
                int linkedLocation = availableLocations[locationNumber].linkedLocationID;
                availableLocations[locationNumber].assignDatacon(dataconList[i].dataID);
                placedLocations.push_back(availableLocations[locationNumber]);
                levelList[availableLevel].dataconCount++;
                availableLocations.erase(availableLocations.begin() + locationNumber);
                if(linkedLocation != 999){
                    bool foundLocation = false;
                    qDebug() << Q_FUNC_INFO << "searching for linked location" << linkedLocation;
                    for(int j = 0; j < availableLocations.size(); j++){
                        if(availableLocations[j].uniqueID != linkedLocation){
                            continue;
                        }
                        qDebug() << Q_FUNC_INFO << "found linked location ID" << linkedLocation;
                        foundLocation = true;
                        availableLocations[j].assignDatacon(dataconList[i].dataID);
                        placedLocations.push_back(availableLocations[j]);
                        levelList[availableLocations[j].level].dataconCount++;
                        availableLocations.erase(availableLocations.begin() + j);
                    }
                    if(!foundLocation){
                        qDebug() << Q_FUNC_INFO << "did not find location" << linkedLocation;
                    }
                }
                qDebug() << Q_FUNC_INFO << "current placed pickups" << placedLocations.size();
                dataconList[i].placed = true;
            } else {
                qDebug() << Q_FUNC_INFO << "Skipping placement.";
            }
        }
    }
}

Minicon* Randomizer::getMinicon(int searchID){
    for(int i = 0; i < miniconList.size(); i++){
        if(miniconList[i].enumID == searchID){
            return &miniconList[i];
        }
    }
    qDebug() << Q_FUNC_INFO << "minicon" << searchID << "was not found.";
    return nullptr;
}

void Randomizer::spoilMinicon(PickupLocation placement, QTextStream& stream){
    //qDebug() << Q_FUNC_INFO << "attempting to place" << placement.uniqueID << "with assigned minicon"
    //         << placement.assignedMinicon << "out of" << miniconList.size() << "minicons";
    QVector3D placedPosition = placement.position();
    stream << getMinicon(placement.assignedMinicon())->name << " is located at " << placement.name << " id " << placement.uniqueID << ", or x" <<
                                                             placedPosition.x() << " y" << placedPosition.y() << " z" << placedPosition.z() << Qt::endl;
    placement.spoiled = true;
}

void Randomizer::spoilMinicon(int miniconID, QTextStream& stream){
    PickupLocation placement;
    placement.uniqueID = 0;
    qDebug() << Q_FUNC_INFO << "placedlocations count:" << placedLocations.size();
    for(int i = 0; i < placedLocations.size(); i++){
        if(placedLocations[i].assignedMinicon() == miniconID){
            placement = placedLocations[i];
            //placedLocations[i].spoiled = true;
        }
    }
    if(placement.uniqueID == 0){
        parent->log("Could not find minicon " + QString::number(miniconID) + " at any placement." );
    }
    QVector3D placedPosition = placement.position();
    qDebug() << Q_FUNC_INFO << "spoiling minicon" << miniconID << "at location" << placement.uniqueID;
    QString miniconName = getMinicon(placement.assignedMinicon())->name;
    stream << miniconName << " is located at " << levelList[placement.level].levelName << "'s " << placement.name << " id " << placement.uniqueID << ", or x" <<
                                                             placedPosition.x() << " y" << placedPosition.y() << " z" << placedPosition.z() << Qt::endl;
}

int Randomizer::writeSpoilers(){
    //use game directory, make new directory in same directory as game
    //this will actually be read earlier in the process, just doing this here for now
    QString outputFile = outputPath + "/Spoilers.txt";
    QFile spoilerOut(outputFile);
    QFile file(outputFile);
    qDebug() << Q_FUNC_INFO << "checking if randomizer directory exists:" << file.exists();
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();
    qDebug() << Q_FUNC_INFO << "checking if randomizer directory exists again:" << file.exists();


    int currentLevel = -1;
    qDebug() << Q_FUNC_INFO << "attempting to write spoiler file";
    if (spoilerOut.open(QIODevice::ReadWrite)){
        QTextStream fileStream(&spoilerOut);

        fileStream << "!SPOILERS FOR SEED " << QString::number(seed) << " BELOW!" << Qt::endl;
        qDebug() << Q_FUNC_INFO << "Attempting to write spoilers";
        qDebug() << Q_FUNC_INFO << "Placed locations:" << placedLocations.size();
        //write slipstream location
        spoilMinicon(28, fileStream);

        //if slipstream requirements are active, write those
        if(randSettings.slipstreamDifficulty > 4){ //actual difficulty tbd
            for(int i = 0; i < placedLocations.size(); i++){
                if(placedLocations[i].assignedMinicon() == 1){
                    spoilMinicon(placedLocations[i], fileStream);
                }
            }
            for(int i = 0; i < placedLocations.size(); i++){
                if(placedLocations[i].assignedMinicon() == 1){
                    spoilMinicon(placedLocations[i], fileStream);
                }
            }
        }

        //write highjump location
        spoilMinicon(50, fileStream);
        //write all others, in order of level
        //qDebug() << Q_FUNC_INFO << "Writing all locations. Total placements:" << placedLocations.size();
        for(int i = 0; i < placedLocations.size(); i++){
            if(currentLevel != placedLocations[i].level){
                currentLevel++;
                fileStream << Qt::endl;
                fileStream << Qt::endl;
                fileStream << "Level " << QString::number(currentLevel+1) << ": " << levelList[currentLevel].levelName << Qt::endl;
                fileStream << "Contains " << QString::number(levelList[currentLevel].dataconCount) << " Datacons";
                fileStream << Qt::endl;
            }
            //qDebug() << Q_FUNC_INFO << "writing location" << placedLocations[i].uniqueID << "with assigned minicon" << placedLocations[i].assignedMinicon << "to location" << placedLocations[i].position;
            if(!placedLocations[i].spoiled){
                if(placedLocations[i].isData){
                    //not gonna worry about spoiling datacons yet. Maybe just a count of datacons?
                } else {
                    spoilMinicon(placedLocations[i], fileStream);
                }
            }
        }

        //include location name and coords
    } else {
        qDebug() << Q_FUNC_INFO << "failed to open file for writing.";
        return 1;
    }
    return 0;
}

/*Maybe the database files can just have Warpgate, Minicon, and Datacon lists
then this system can use that instead of trying to add that system to the base file

then these files would have a home in the database section instead of being loose

Placement Objects could become its own header, containing the definitions for warpgates, minis, and datas
This also makes assigning specific properties to each type easier, like mini and data spawn difficulty

*/

int Randomizer::editDatabases(){
    /*
    specific concerns with clearing pickupplaced:
        Check deep amazon to see if the dropship not being able to find its minicon causes problems
        check bunkers to make sure that script isn't sad without a datacon
    */
    for(int i = 0; i<parent->databaseList.size(); i++){
        qDebug() << Q_FUNC_INFO << "removing all pickups from" << parent->databaseList[i]->fileName;
        parent->databaseList[i]->removeAll("PickupPlaced");
    }
    QString modFileDirectory = QDir::currentPath();
    QString slipstreamInPath = modFileDirectory + "/ASSETS/INTROGLIDING.CS";
    QFile slipstreamFix(slipstreamInPath);
    bool didItWork = false;

    QString levelPath;
    QString slipstreamOutPath;
    Minicon itemToAdd;
    int level = 0;
    for(int i = 0; i < placedLocations.size(); i++){
        qDebug() << Q_FUNC_INFO << "checking level for location" << i << "uniqueID" << placedLocations[i].uniqueID << "level" << placedLocations[i].level;
        if(placedLocations[i].level != level){
            qDebug() << Q_FUNC_INFO << "output path for this level will be" << QString(outputPath + "/TFA/LEVELS/EPISODES/" + levelList[level].outputName + "/CREATURE.TDB");
            levelPath = outputPath + "/TFA/LEVELS/EPISODES/" + levelList[level].outputName;
            //get containing directory
            //create Randomizer folder in that directory
            qDebug() << Q_FUNC_INFO << "path being checked:" << levelPath;
            QDir checkDir(levelPath);
            if(!checkDir.exists()){
                checkDir.mkpath(".");
            }
            //since the placedLocations list should be sorted by level
            //this just saves on a couple hundred loops
            parent->databaseList[level]->outputPath = levelPath + "/CREATURE.TDB";
            parent->databaseList[level]->save("TDB");

            //I am purposefully leaving the slipstream fix out of pacific island
            //if you get that far without it, you don't get the cutscene or the option to equip. You clearly don't need it anyway.
            slipstreamOutPath = levelPath + "/CINEMA";
            qDebug() << Q_FUNC_INFO << "Slipstream cinema output directory:" << slipstreamOutPath;
            checkDir.setPath(slipstreamOutPath);
            qDebug() << Q_FUNC_INFO << "Slipstream output directory exists?" << checkDir.exists();
            if(!checkDir.exists()){
                checkDir.mkpath(".");
            }
            slipstreamOutPath += "/INTROGLIDING.CS";
            if(slipstreamFix.exists()){
                didItWork = QFile::copy(slipstreamInPath, slipstreamOutPath);
                qDebug() << Q_FUNC_INFO << "did the slipstream copy work?" << didItWork;
            } else {
                parent->log("Could not place Slipstream-fix cinema file.");
            }

            level++;
        }
        //itemToAdd = *getMinicon(placedLocations[i].assignedMinicon());
        placedLocations[i].instanceIndex = parent->databaseList[level]->addInstance(placedLocations[i]);
        qDebug() << Q_FUNC_INFO << "placed location instance index is" << placedLocations[i].instanceIndex;
    }
    qDebug() << Q_FUNC_INFO << "Correcting Pacific island bunker links";
    fixBunkerLinks(level);
    qDebug() << Q_FUNC_INFO << "output path for this level will be" << QString(outputPath + "/TFA/LEVELS/EPISODES/" + levelList[level].outputName + "/CREATURE.TDB");
    levelPath = outputPath + "/TFA/LEVELS/EPISODES/" + levelList[level].outputName;
    //get containing directory
    //create Randomizer folder in that directory
    qDebug() << Q_FUNC_INFO << "path being checked:" << levelPath;
    QDir checkDir(levelPath);
    if(!checkDir.exists()){
        checkDir.mkpath(".");
    }
    //since the placedLocations list should be sorted by level
    //this just saves on a couple hundred loops
    parent->databaseList[level]->outputPath = levelPath + "/CREATURE.TDB";
    parent->databaseList[level]->save("TDB");
    level++;

    return 0;
}

void Randomizer::fixBunkerLinks(int level){
    std::shared_ptr<DatabaseFile> pacificFile = parent->databaseList[level];
    std::vector<int> usedBunkers;
    qDebug() << Q_FUNC_INFO << "bunker file level name:" << pacificFile->fullFileName();
    for(int i = 0; i < placedLocations.size(); i++){
        if(placedLocations[i].bunkerID == 0 || !placedLocations[i].isData){
            continue;
        }
        qDebug() << Q_FUNC_INFO << "bunker ID for pickup" << placedLocations[i].uniqueID << "is" << placedLocations[i].bunkerID;
        for(int j = 0; j < pacificFile->instances.size(); j++){
            if(pacificFile->instances[j].instanceIndex != placedLocations[i].bunkerID){
                continue;
            }
            qDebug() << Q_FUNC_INFO << "Matching bunker index ID found at" << pacificFile->instances[j].instanceIndex;
            usedBunkers.push_back(pacificFile->instances[j].instanceIndex);
            for(int attribute = 0; attribute < pacificFile->instances[j].attributes.size(); attribute++){
                pacificFile->instances[j].setAttribute("Reward_PickupLink", QString::number(placedLocations[i].instanceIndex));
                /*if(pacificFile->instances[j].attributes[attribute]->name == "Reward_PickupLink"){
                    qDebug() << Q_FUNC_INFO << "original link" << pacificFile->instances[j].attributes[attribute]->intValue() << "replacing with" << placedLocations[i].instanceIndex;
                    pacificFile->instances[j].attributes[attribute]->setValue(QString::number(placedLocations[i].instanceIndex));
                }*/
            }
        }
    }

    if (usedBunkers.empty()){
        qDebug() << Q_FUNC_INFO << "no datacons assigned to bunkers.";
        return;
    }
    for(int j = 0; j < pacificFile->instances.size(); j++){
        if(std::find(usedBunkers.begin(), usedBunkers.end(), pacificFile->instances[j].instanceIndex) != usedBunkers.end()){
            continue;
        }
        //qDebug() << Q_FUNC_INFO << "Unused bunker index ID found at" << pacificFile->instances[j].instanceIndex;
        for(int attribute = 0; attribute < pacificFile->instances[j].attributes.size(); attribute++){
            pacificFile->instances[j].setAttributeDefault("Reward_PickupLink");
            /*if(pacificFile->instances[j].attributes[attribute]->name == "Reward_PickupLink"){
                qDebug() << Q_FUNC_INFO << "Setting bunker reward to default";
                pacificFile->instances[j].attributes[attribute]->isDefault = true;
            }*/
        }
    }
}

void Randomizer::replaceFile(QString fileName, QString destinationPath){
    QString modFileDirectory = QDir::currentPath();
    bool didItWork = false;
    qDebug() << Q_FUNC_INFO << "current path:" << modFileDirectory;
    QString fileInputDirectory = modFileDirectory + "/ASSETS/" + fileName;
    QFile fileReplacement(fileInputDirectory);
    qDebug() << Q_FUNC_INFO << "checking if replacement image exists:" << fileReplacement.exists();

    QString fileOutputDirectory = outputPath + destinationPath;

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

void Randomizer::randomizeTeamColors(){
    int teamIndex = 0;
    int shuffleValue = 0;
    std::vector<int> teamList;
    if(randSettings.balancedTeams){
        for(int i = 0; i <miniconList.size(); i++){
            teamList.push_back(miniconList[i].searchAttributes<int>("Team"));
        }
    }
    for(int i = 0; i <miniconList.size(); i++){
        teamIndex = miniconList[i].searchAttributes<int>("Team");
        if(randSettings.balancedTeams){
            /*Randomize the teams in a balanced way - there will be as many of each team in the final result as the base game*/
            shuffleValue = placemaster.bounded(teamList.size());
            miniconList[i].setAttribute("Team", QString::number(teamList[shuffleValue]));
        } else {
            /*Otherwise, assign a random team value with no regard to team count.*/
            shuffleValue = placemaster.generate();
            miniconList[i].setAttribute("Team", QString::number(shuffleValue%5));
        }
    }
}

void Randomizer::randomizePowers(){
    int powerLevel = 0;
    int shuffleValue = 0;
    for(int i = 0; i <miniconList.size(); i++){
        shuffleValue = placemaster.generate();
        powerLevel = miniconList[i].searchAttributes<int>("PowerCost");
        if(randSettings.balancedPower){
            /*Randomize the powers in a balanced way all powers will stay close to their original*/
            switch(powerLevel){
            case 10: //power can go up, down, or stay the same
                if(shuffleValue % 3 == 0){
                    miniconList[i].setAttribute("PowerCost", QString::number(powerLevel - 10));
                } else if(shuffleValue % 3 == 1) {
                    miniconList[i].setAttribute("PowerCost", QString::number(powerLevel + 10));
                }
                break;
            case 20: //power can go up, down, or stay the same
                if(shuffleValue % 3 == 0){
                    miniconList[i].setAttribute("PowerCost", QString::number(powerLevel - 10));
                } else if(shuffleValue % 3 == 1) {
                    miniconList[i].setAttribute("PowerCost", QString::number(powerLevel + 10));
                }
                break;
            case 30: //power can go up, down, or stay the same
                if(shuffleValue % 3 == 0){
                    miniconList[i].setAttribute("PowerCost", QString::number(powerLevel - 10));
                } else if(shuffleValue % 3 == 1) {
                    miniconList[i].setAttribute("PowerCost", QString::number(powerLevel + 10));
                }
                break;
            case 40: //power can only go down or stay the same
                if(shuffleValue % 2 == 0){
                    miniconList[i].setAttribute("PowerCost", QString::number(powerLevel - 10));
                }
                break;
            default:
                break;
            }
        } else {
            /*Otherwise, assign a random team value with no regard to team count.*/
            miniconList[i].setAttribute("PowerCost", QString::number((shuffleValue%5)*10));
        }
    }
}
