#ifndef RANDOMIZER_H
#define RANDOMIZER_H

#include <QRandomGenerator>
#include "Headers/Databases/Database.h"

class ProgWindow;

class PickupLocation : public Pickup{
public:
    int uniqueID;
    int slipstreamDifficulty;
    int highjumpDifficulty;
    QString name;
    QString spawnEvent;
    int level;
    int originalDatabaseInstance;
    bool isData;
    bool spoiled;
    int bunkerID;
    int instanceIndex;
    int linkedLocationID;

    PickupLocation* linkedLocation(); //if a minicon is placed at one of these, it's placed at both of them
    //only used for starship (vanilla Aftershock) and mid-atlantic, since it has separate database files

    PickupLocation();
    PickupLocation(Pickup fromItem);
    int assignedMinicon();
    void assignMinicon(int miniconID);
    void assignDatacon(int dataID);
    void generateAttributes(); //for custom locations
    //implementing the < operator so we can simply sort the location vector
    //using std::sort instead of making a whole function for it
    bool operator < (const PickupLocation& compLocation) const
    {
        return (level < compLocation.level);
    }
};

class Level{
public:
    QString levelName;
    QString outputName;
    int miniconCount;
    int dataconCount;
    int maxInstances;
    int removedInstances;
};

class RandomizerOption{
public:
    QString name;
    QString author;
    QString description;
    bool enabled;
};

class RandomizerMod : public RandomizerOption{
public:
    bool type; //0 for binary, 1 for text
    QString fileName;
};

class FileReplacement : public RandomizerOption{
public:
    int rarity;
    QStringList fileNames;
    QStringList fileDestinations;
};

class CustomLocations : public RandomizerOption{
public:
    int locationCount;
    std::vector<PickupLocation> locationList;
};

class Randomizer{
public:
    Randomizer(ProgWindow *parentPass);
    inline const static QList<int> weaponList {4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 18, 19, 20, 21, 22, 23, 24, 25, 27, 30, 36, 41, 46, 51};
    inline const static QList<int> bunkerList {137,140,141,142,143,144,145};

    ProgWindow* parent;
    std::vector<Level> levelList;
    std::vector<RandomizerMod> modList;
    std::vector<FileReplacement> replacementList;
    QString outputPath;
    QRandomGenerator placemaster;
    QLineEdit* editSeed;
    QLineEdit* editSettings;
    QString settingsValue;
    quint32 seed;
    bool enableCustomLocations;
    struct randomizerSettings {
        bool generateDatacons = false;
        bool autoBuild = false;
        int overallDifficulty = 0;
        int slipstreamDifficulty = 0;
        int highjumpDifficulty = 0;
        bool randomizeTeams = false;
        bool balancedTeams = false;
        bool randomizePower = false;
        bool balancedPower = false;
    } randSettings;

    //https://stackoverflow.com/questions/1604588/how-do-you-remove-elements-from-a-stdvector-while-iterating
    //use the above for turning pickuplist into the minicon and datacon lists

    //there are duplicate minicons on the miniconlist because of minicons with different placements in different difficulties.
    //see if we can clear those out.

    //also set a < operator for minicon based on their enumID
    //will probably want to subtract 3 when reading and add 3 when writing these
    std::vector<Pickup> pickupList;
    std::vector<Minicon> miniconList;
    std::vector<Pickup> dataconList;
    std::vector<PickupLocation> loadedLocations;
    std::vector<PickupLocation> availableLocations;
    std::vector<PickupLocation> placedLocations;
    std::vector<CustomLocations> customLocationList;

    void reset();

    void loadMinicons();
    void loadLevels();
    bool miniconLoaded(int checkID);
    bool dataconLoaded(int checkID);
    void loadCustomLocations();
    void addCustomLocation(int locationID, int level, QVector3D location);
    Minicon* getMinicon(int searchID);

    void testAllPlacements();
    void randomizeTeamColors();
    void randomizePowers();

    void randomize();
    void loadFileReplacements();
    void fileReplacements();
    void placeAll();
    void place(int miniconToPlace, int placementID);
    void setOverallDifficulty(int difficulty);
    void setSlipstreamDifficulty(int difficulty);
    void setHighjumpDifficulty(int difficulty);
    void placeSlipstreamRequirement(int miniconID, int placementID);
    void placeRangefinder();
    void placeShepherd();
    void placeSlipstream();
    void placeHighjump();
    void placeStarterWeapon();
    void manualSettings();
    void exportSettings();
    void fixBunkerLinks(int level);
    void setSeed(QString value);
    bool duplicateLocation(PickupLocation testLocation);
    void replaceFile(QString fileName, QString destinationPath);
    void applyModifications();
    void loadMods();

    int writeSpoilers();
    void spoilMinicon(PickupLocation placement, QTextStream& stream);
    void spoilMinicon(int miniconID, QTextStream& stream);

    int editDatabases();

};

#endif // RANDOMIZER_H
