#include "mainwindow.h"

void GeometrySet::getVerticies(ProgWindow &ProgWindow){

    //NEED TO POPULATE FILEDATA FIRST
    //qDebug() << "this works";
    //return;

    QByteArrayMatcher findGeoSet("~GeometrySet");
    long startPos = findGeoSet.indexIn(ProgWindow.fileData, 0) +2;
    long endPos = findGeoSet.indexIn(ProgWindow.fileData, startPos + 12);
    int index[3];
    std::vector<QVector3D> tempVec;
    long currentPosition = startPos;

    geoSetVerticies.resize((endPos-startPos)/4);

    QString fileOut = QFileDialog::getSaveFileName(&ProgWindow, ProgWindow.tr("Select Output STL"), QDir::currentPath() + "/STL/", ProgWindow.tr("Model Files (*.stl)"));
    QFile stlOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    if (stlOut.open(QIODevice::ReadWrite)){
        QTextStream stream(&stlOut);
        stream << "solid Default" << Qt::endl;
        //qDebug() << Q_FUNC_INFO << "SceneNodes: " << this->nodeList.size();
//            for (int p = 0; p<int(allowedMeshes.size()); p++) {
//                int i = allowedMeshes[p];
        for (long i = 0; i < ((endPos-startPos)/4); i++){
            if (currentPosition + (i*4) > 173 and currentPosition + (i*4) < 490) {
                geoSetVerticies[i] = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition + (i*4), 4).toHex(), 2));
                index[0] = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition + (i*4), 4).toHex(), 2));
                index[1] = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition+4 + (i*4), 4).toHex(), 2));
                index[2] = ProgWindow.binChanger.hex_to_float(ProgWindow.binChanger.reverse_input(ProgWindow.fileData.mid(currentPosition+8 + (i*4), 4).toHex(), 2));
                qDebug() << Q_FUNC_INFO << "current position" <<  QString::number(currentPosition+(i*4), 16) << " Current float: " << geoSetVerticies[i];
                tempVec.push_back(QVector3D(index[0], index[1], index[2]));
                qDebug() << Q_FUNC_INFO << "created vector: " << QVector3D(index[0], index[1], index[2]);
                if (tempVec.size() > 2){
                    stream << "  facet normal 0 0 0" << Qt::endl;
                    stream << "    outer loop" << Qt::endl;
                    for (int n = i-44; n < i-41; ++n) {
                        stream << "      vertex ";
                        stream << QString::number(tempVec[n].x(), 'f', 3) << " " << QString::number(tempVec[n].y(), 'f', 3) << " " << QString::number(tempVec[n].z(), 'f', 3) << Qt::endl;
                    }
                    stream << "    endloop" << Qt::endl;
                    stream << "  endfacet" << Qt::endl;
                }
            }
        }
        stream << "endsolid Default" << Qt::endl;
    }
}

void GeometrySet::openMeshVBINFile(ProgWindow &ProgWindow){

    QString fileIn = QFileDialog::getOpenFileName(&ProgWindow, ProgWindow.tr("Select VBIN"), QDir::currentPath() + "/VBIN/", ProgWindow.tr("Model Files (*.vbin)"));
    filePath = fileIn;
    ProgWindow.fileData.clear();

    QFile inputFile(fileIn);
    inputFile.open(QIODevice::ReadOnly);
    ProgWindow.fileData = inputFile.readAll();

    qDebug() << Q_FUNC_INFO << "File data loaded.";
    getVerticies(ProgWindow);
    qDebug() << Q_FUNC_INFO << "File data read.";


}
