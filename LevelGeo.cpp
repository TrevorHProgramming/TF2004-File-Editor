#include "mainwindow.h"

void GeometrySet::getVerticies(){

    //NEED TO POPULATE FILEDATA FIRST
    //qDebug() << "this works";
    //return;

    QByteArrayMatcher findGeoSet("~GeometrySet");
    long startPos = findGeoSet.indexIn(parent->fileData.dataBytes, 0) +2;
    long endPos = findGeoSet.indexIn(parent->fileData.dataBytes, startPos + 12);
    int index[3];
    std::vector<QVector3D> tempVec;
    long currentPosition = startPos;

    geoSetVerticies.resize((endPos-startPos)/4);

    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output STL"), QDir::currentPath() + "/STL/", parent->tr("Model Files (*.stl)"));
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
                geoSetVerticies[i] = parent->fileData.readFloat();
                index[0] = parent->fileData.readFloat();
                index[1] = parent->fileData.readFloat();
                index[2] = parent->fileData.readFloat();
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

void GeometrySet::openMeshVBINFile(){

    QString fileIn = QFileDialog::getOpenFileName(parent, parent->tr("Select VBIN"), QDir::currentPath() + "/VBIN/", parent->tr("Model Files (*.vbin)"));
    filePath = fileIn;
    parent->fileData.readFile(fileIn);
//    parent->fileData.dataBytes.clear();

//    QFile inputFile(fileIn);
//    inputFile.open(QIODevice::ReadOnly);
//    parent->fileData.dataBytes = inputFile.readAll();

    qDebug() << Q_FUNC_INFO << "File data loaded.";
    getVerticies();
    qDebug() << Q_FUNC_INFO << "File data read.";


}
