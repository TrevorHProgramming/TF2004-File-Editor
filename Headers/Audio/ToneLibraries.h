#ifndef TONELIBRARIES_H
#define TONELIBRARIES_H

//https://sites.google.com/site/musicgapi/technical-documents/wav-file-format

#include <QString>
#include "Headers/Main/BinChanger.h"

class ProgWindow;

class VACFile : public TFFile {
  public:
    long fileSize;
    int compressionCode;
    int channels;
    int bitsPerSample;
    QString name;
    long offset;
    int rootNote;
    bool loop;

    std::vector<int> noteList;
    std::vector<std::vector<int>> freqList;

    void tempRead();
    void tempWrite();
};

class ToneLibrary{
  public:
    int version;
    int tones;
    QString dataFile;
    std::vector<VACFile> toneList;
};

#endif // TONELIBRARIES_H
