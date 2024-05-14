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
    groupRandomizerOptions->setGeometry(QRect(QPoint(360,100), QSize(200,300)));
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
    checkAutoBuild->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0)");
    QAbstractButton::connect(checkAutoBuild, &QCheckBox::stateChanged, parent, [checkAutoBuild, this] {randSettings.autoBuild = checkAutoBuild->isChecked();});
    checkAutoBuild->toggle();
    checkAutoBuild->show();
    //parent->currentModeWidgets.push_back(checkAutoBuild);

    groupRandomizerOptions->show();

    QGroupBox *groupModOptions = new QGroupBox("Mod Options", parent->centralContainer);
    groupModOptions->setGeometry(QRect(QPoint(600,100), QSize(200,300)));
    groupModOptions->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0)");
    parent->currentModeWidgets.push_back(groupModOptions);

    QCheckBox *checkAlwaysHighjump = new QCheckBox("Permanent Highjump", groupModOptions);
    checkAlwaysHighjump->setGeometry(QRect(QPoint(20,20), QSize(200,30)));
    checkAlwaysHighjump->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0)");
    QAbstractButton::connect(checkAlwaysHighjump, &QCheckBox::stateChanged, parent, [checkAlwaysHighjump, this] {modSettings.alwaysHighjump = checkAlwaysHighjump->isChecked();});
    checkAlwaysHighjump->show();
    //parent->currentModeWidgets.push_back(checkAlwaysHighjump);

    QCheckBox *checkAlwaysGlider = new QCheckBox("Permanent Glider", groupModOptions);
    checkAlwaysGlider->setGeometry(QRect(QPoint(20,60), QSize(200,30)));
    checkAlwaysGlider->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0)");
    QAbstractButton::connect(checkAlwaysGlider, &QCheckBox::stateChanged, parent, [checkAlwaysGlider, this] {modSettings.alwaysGlider = checkAlwaysGlider->isChecked();});
    //checkAlwaysGlider->show();
    //parent->currentModeWidgets.push_back(checkAlwaysGlider);

    QCheckBox *checkBalancePatch = new QCheckBox("Balance Patch", groupModOptions);
    checkBalancePatch->setGeometry(QRect(QPoint(20,100), QSize(200,30)));
    checkBalancePatch->setStyleSheet("color: rgb(255, 255, 255); background-color: rgba(255, 255, 255, 0)");
    QAbstractButton::connect(checkBalancePatch, &QCheckBox::stateChanged, parent, [checkBalancePatch, this] {modSettings.ptBalancePatch = checkBalancePatch->isChecked();});
    //checkBalancePatch->show();
    //parent->currentModeWidgets.push_back(checkBalancePatch);

    //groupModOptions->show();

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

void Randomizer::fileReplacements(){
    int replaceCyclonus = placemaster.generate();
    if(replaceCyclonus % 4 == 0){
        //parent->log("Replacing Cyclonus theme. Please make sure this works.");
        replaceFile("CYCLO1.MS", "/SOUND/MUSIC");
        replaceFile("CYCLO2.MS", "/SOUND/MUSIC");
    }

    int replaceTidal = placemaster.generate();
    if(replaceTidal % 4 == 0){
        //parent->log("Replacing Tidal Wave theme. Please make sure this works.");
        replaceFile("TIDAL1.MS", "/SOUND/MUSIC");
        replaceFile("TIDAL2.MS", "/SOUND/MUSIC");
    }

    int enableRagdoll = placemaster.generate();
    if(enableRagdoll % 4 == 0){
        //parent->log("Replacing bunker ragdoll cutscene. Please make sure this works.");
        replaceFile("BUNKEREXPLODE.CS", "/TFA/LEVELS/EPISODES/07_EASTERISLAND/CINEMA");
    }

    int enablePacificRain = placemaster.generate();
    if(enablePacificRain % 4 == 0){
        //parent->log("Replacing Pacific Island weather. PLEASE make sure this works.");
        replaceFile("ENVIRONMENT.TDB", "/TFA/LEVELS/EPISODES/07_EASTERISLAND");
        replaceFile("ISLANDSKY4.ITF", "/TFA/LEVELS/EPISODES/07_EASTERISLAND/GEOMETRY");
    }

    int enableMegaMock = placemaster.generate();
    if(enableMegaMock % 2 == 0){
        //parent->log("Replacing Megatron mock in intro cutscene. Please make sure this works.");
        replaceFile("MEGATRONINTRO.CS", "/TFA/LEVELS/EPISODES/07_EASTERISLAND/CINEMA");
        enableMegaMock = placemaster.generate();
        if(enableMegaMock % 2 == 0){
            //parent->log("Using alternate Megatron mock in intro cutscene. Please make sure this works.");
            replaceFile("mg_mock.ms", "/TFA/LEVELS/EPISODES/07_EASTERISLAND/CINEMA");
        }
    }

    int starshipCrash = placemaster.generate();
    if(starshipCrash % 2 == 0){
        //parent->log("Replacing Starship crash dialog. Please make sure this works.");
        replaceFile("RA_S03_2.MS", "/SOUND/ENGLISH/AUDIOCUE/SPACESHP");
        replaceFile("OP_S03_2.MS", "/SOUND/ENGLISH/AUDIOCUE/SPACESHP");
    }

    int redAlertUnicron = placemaster.generate();
    if(redAlertUnicron % 3 == 0){
        //parent->log("Replacing Unicron Warning dialog. Please make sure this works.");
        replaceFile("OP_MQ1.MS", "/SOUND/ENGLISH/AUDIOCUE/MISC");
        replaceFile("OP_TC2.MS", "/SOUND/ENGLISH/AUDIOCUE/MISC");
    }

    int alphaGuerilla = placemaster.generate();
    if(alphaGuerilla % 3 == 0){
        //parent->log("Replacing guerilla texture. Please make sure this works.");
        replaceFile("THIGHS.ITF", "/TFA/ANIMATION");
        replaceFile("SHOULDER.ITF", "/TFA/ANIMATION");
        replaceFile("SHIN.ITF", "/TFA/ANIMATION");
        replaceFile("PELVIS.ITF", "/TFA/ANIMATION");
        replaceFile("HEAD.ITF", "/TFA/ANIMATION");
        replaceFile("HAND.ITF", "/TFA/ANIMATION");
        replaceFile("GUT.ITF", "/TFA/ANIMATION");
        replaceFile("FOREARM.ITF", "/TFA/ANIMATION");
        replaceFile("CHEST.ITF", "/TFA/ANIMATION");
        replaceFile("BICEP.ITF", "/TFA/ANIMATION");
    }

    qDebug() << Q_FUNC_INFO << "Replacing title screen.";
    replaceFile("ARMADALOGO.ITF", "/TFA/USERINTERFACE/TEXTURES");
}

void Randomizer::applyModifications(){
    qDebug() << Q_FUNC_INFO << QCoreApplication::applicationDirPath();
    QString mipsEditorPath = QCoreApplication::applicationDirPath() + "/SELF.exe";
    qDebug() << Q_FUNC_INFO << mipsEditorPath;

    QStringList args;
    QString gamePath = parent->setW->getValue("Game extract path") + "/SLUS_206.68";
    if(gamePath == ""){
        qDebug() << Q_FUNC_INFO << "Process cancelled.";
        return;
    }
    parent->log("Modifying ELF from: " + gamePath);
    args.append(gamePath);

    QString modFiles = "";
    if(modSettings.alwaysHighjump){
        modFiles += QCoreApplication::applicationDirPath() + "/Mods/HighjumpAlways.bin";
    }
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
    if(randSettings.slipstreamDifficulty > 4){ //actual difficulty tbd
        placeSlipstreamRequirement(44, slipstreamLocations[locationNumber].uniqueID);
        placeSlipstreamRequirement(14, slipstreamLocations[locationNumber].uniqueID);
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
        }
    }
    for(int i = 0; i < availableLocations.size(); i++){
        //actual difficulty tbd
        if((availableLocations[i].slipstreamDifficulty <= 4)
                && (availableLocations[i].level <= placementLevel)
                && (availableLocations[i].uniqueID != 42069)){ //we don't want one of the requirements spawning right at the start - the player asked for pain.
            prereqLocations.push_back(availableLocations[i]);
        }
    }
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
                if(pacificFile->instances[j].attributes[attribute]->name == "Reward_PickupLink"){
                    qDebug() << Q_FUNC_INFO << "original link" << pacificFile->instances[j].attributes[attribute]->intValue() << "replacing with" << placedLocations[i].instanceIndex;
                    pacificFile->instances[j].attributes[attribute]->setValue(QString::number(placedLocations[i].instanceIndex));
                }
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
            if(pacificFile->instances[j].attributes[attribute]->name == "Reward_PickupLink"){
                qDebug() << Q_FUNC_INFO << "Setting bunker reward to default";
                pacificFile->instances[j].attributes[attribute]->isDefault = true;
            }
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

