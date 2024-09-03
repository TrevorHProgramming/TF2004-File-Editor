#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include "Headers/Databases/Database.h"

/*For handling data from database files in a more detailed manner*/

class Warpgate : public dictItem{
public:
    double x_position;
    double y_position;
    double z_position;
    bool isStartingGate;
    bool hasScript;

    Warpgate(dictItem copyItem);
    Warpgate();
    static std::vector<Warpgate*> createAmazonWarpgates();
};

class PickupLocation : public dictItem{
public:
    int uniqueID;
    int slipstreamDifficulty;
    int highjumpDifficulty;
    QString locationName;
    QString spawnEvent;
    int level;
    int originalDatabaseInstance;
    bool spoiled;
    int bunkerID;
    int instanceIndex;
    int linkedLocationID;

    PickupLocation* linkedLocation(); //if a minicon is placed at one of these, it's placed at both of them
    //only used for starship (vanilla Aftershock) and mid-atlantic, since it has separate database files

    PickupLocation();
    PickupLocation(dictItem fromItem);
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
    std::shared_ptr<DatabaseFile> levelFile;
    QString levelName;
    QString outputName;
    int miniconCount;
    int dataconCount;
    int maxInstances;
    int removedInstances;
};

class Pickup : public dictItem{
public:
    bool isMinicon;
    bool isWeapon;
    int spawnDifficulty;
    int enumID;
    int dataID;
    bool placed;
    QString pickupToSpawn;

    Pickup(dictItem copyItem);
    Pickup();


    bool operator < (const Pickup& compPickup) const
    {
        return (enumID < compPickup.enumID);
    }

    bool operator == (const Pickup& compPickup) const
    {
        if(dataID != 99){
            return (dataID == compPickup.dataID);
        } else {
            return (enumID == compPickup.enumID);
        }
    }
};

class Minicon : public Pickup{
public:
    bool isExplosive;
    bool isSlipstream;
    bool isHighjump;

    Minicon(Pickup copyItem);
    Minicon();
};

class ConverterOption{
public:
    QString name;
    QString author;
    QString description;
    bool enabled;
};

class RandomizerMod : public ConverterOption{
public:
    bool type; //0 for binary, 1 for text
    QString fileName;
};

class FileReplacement : public ConverterOption{
public:
    int rarity;
    QStringList fileNames;
    QStringList fileDestinations;
};

class CustomLocations : public ConverterOption{
public:
    int locationCount;
    std::vector<PickupLocation> locationList;
};

class DataHandler{
public:
    inline const static QList<int> weaponList {4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 18, 19, 20, 21, 22, 23, 24, 25, 27, 30, 36, 41, 46, 51};
    inline const static QList<int> bunkerList {137,140,141,142,143,144,145};

    ProgWindow* parent;

    //std::vector<dictItem> itemList;
    std::vector<Pickup> pickupList;
    std::vector<Minicon> miniconList;
    std::vector<Pickup> dataconList;
    std::vector<dictItem> autobotList;
    std::vector<PickupLocation> loadedLocations;
    std::vector<CustomLocations> customLocationList;
    std::vector<Level> levelList;

    std::vector<FileReplacement> replacementList;
    template <class instanceType>
    std::vector<instanceType> convertInstances(std::vector<dictItem> itemList){
        std::vector<instanceType> convertedList;
        for(int i = 0; i < itemList.size(); i++){
            instanceType nextInstance = instanceType(itemList[i]);
            convertedList.push_back(nextInstance);
        }
        return convertedList;
    };

    void loadFileReplacements();
    void loadMinicons();
    void loadLevels();
    void loadAutobots();
    void loadMetagameMinicons();
    bool miniconLoaded(int checkID);
    bool dataconLoaded(int checkID);
    void loadCustomLocations();
    //void addCustomLocation(int locationID, int level, QVector3D location);
    Minicon* getMinicon(int searchID);
    Minicon* getMinicon(QString searchName);
    bool duplicateLocation(PickupLocation testLocation);
    void replaceFile(QString fileName, QString destinationPath);
    void replaceFile(FileReplacement fileToReplace);

    void resetMinicons();
    void resetDatacons();
    void resetLevels();

    /*std::vector<Pickup> convertPickups(std::vector<dictItem> itemList);
    std::vector<Minicon> convertMinicons(std::vector<dictItem> itemList);
    std::vector<Warpgate> convertWarpgates(std::vector<dictItem> itemList);*/
};

#endif // DATAHANDLER_H
