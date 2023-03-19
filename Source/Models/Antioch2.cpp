#include "Headers/Main/mainwindow.h"

AnimationBase *makeAnimation(QString animationSignature){
    if (animationSignature == "~anAnimationTranslation") {
        return new AnimationTranslation;
    } else if (animationSignature == "~anAnimationOrientation") {
        return new AnimationOrientation;
    } /*else if (animationSignature == "~anAnimationScale") {
        return new AnimationScale;
    } else {
        return nullptr;
    }*/
    return nullptr;
}

void AnimationSourceSet::readAnimationSet(){
    version = file->fileData->readInt();
    sourceCount = file->fileData->readInt();
    SectionHeader signature;


    file->fileData->signature(&signature);
    if(signature.type != "~StreamArray"){
        qDebug() << Q_FUNC_INFO << "Expected Stream Array at" << file->fileData->currentPosition;
        return;
    }

    int arrayLength = file->fileData->readInt();

    for (int animation = 0; animation < sourceCount; animation++) {
        AnimationStream *animationStream = new AnimationStream;
        animationStream->file = file;
        animationStream->readAnimationStream();
        streamArray.push_back(animationStream);
    }
}

void AnimationStream::readAnimationStream(){
    SectionHeader signature;
    file->fileData->signature(&signature);
    if(signature.type != "~anAnimationStream") {
        qDebug() << Q_FUNC_INFO << "Expected Animation Stream at" << file->fileData->currentPosition;
        return;
    }

    int nameLength = file->fileData->readInt();
    file->fileData->hexValue(&name, nameLength);
    //name = file->fileData->readHex(nameLength);
    sectionLength = file->fileData->readInt();

    version1 = file->fileData->readInt();
    minimumLOD = file->fileData->readInt();
    version2 = file->fileData->readInt();
    animationPlaybackType = file->fileData->readInt();
    defaultPlaybackSpeed = file->fileData->readFloat();
    channelCount = file->fileData->readInt();

    file->fileData->signature(&signature);
    if(signature.type != "~channelArray") {
        qDebug() << Q_FUNC_INFO << "Expected Channel Array at" << file->fileData->currentPosition;
        return;
    }

    int channelDataLength = file->fileData->readInt();

    for(int channelIndex = 0; channelIndex < channelCount; channelIndex++){
        SectionHeader channel;
        file->fileData->signature(&channel);
        int ending = channel.sectionLength + channel.sectionLocation;

        AnimationBase *animationChannel = makeAnimation(channel.type);
        if (animationChannel == nullptr) {
            qDebug() << Q_FUNC_INFO << "Unsupported channel type" << channel.type << ". Skipping for now.";
            file->parent->fileData.currentPosition = ending;
            continue;
        }
        qDebug() << Q_FUNC_INFO << "reading an animation channel" << channel.name << "of type" << channel.type << "at" << file->fileData->currentPosition;
        animationChannel->file = file;
        animationChannel->animationType = channel.type;

        animationChannel->name = channel.name;
        animationChannel->sectionLength = channel.sectionLength;
        animationChannel->version3 = file->fileData->readInt();
        animationChannel->keyframeCount = file->fileData->readInt();
        animationChannel->subVersion = file->fileData->readInt();
        if(channel.type == "~anAnimationOrientation"){
            animationChannel->has4DSpline = file->fileData->readBool();
            animationChannel->hasPackedSpline = file->fileData->readBool();
        }

        SectionHeader motion;
        file->fileData->signature(&motion);
        if(motion.type == "Scalar"){
            animationChannel->readScalar();
            file->fileData->signature(&motion);
        }

        if (motion.type == "~Linear") {
            animationChannel->readLinear();
        } else if (motion.type == "~Cubic") {
            animationChannel->readCubic();
        } else if (motion.type == "~CubicC2") {
            animationChannel->readCubicC2();
        } else {
            qDebug() << Q_FUNC_INFO << "Unsupported motion type" << motion.type << ". Skipping for now.";
            file->fileData->currentPosition = ending;
        }
        animationChannel->maxTime = 0;
        for(int i = 0; i < animationChannel->keyframeTimes.size(); i++){
            if(animationChannel->keyframeTimes[i] > animationChannel->maxTime){
                animationChannel->maxTime = animationChannel->keyframeTimes[i];
            }
        }
        channelArray.push_back(animationChannel);
    }
}

void AnimationBase::readScalar(){
    qDebug() << Q_FUNC_INFO << "reading an animation scalar modifier";
    hasScalar = true;
    sectionLength = file->fileData->readInt();
    version3 = file->fileData->readInt();
    keyframeCount = file->fileData->readInt();
    subVersion = file->fileData->readInt();
    float x_pos = file->fileData->readFloat();
    float y_pos = file->fileData->readFloat();
    float z_pos = file->fileData->readFloat();
    vectorList.push_back(QVector3D(x_pos, y_pos, z_pos));
}

void AnimationTranslation::readLinear(){
    qDebug() << Q_FUNC_INFO << "reading a translation animation linear motion";
    sectionLength = file->fileData->readInt();
    version3 = file->fileData->readInt();
    keyframeCount = file->fileData->readInt();
    subVersion = file->fileData->readInt();
    qDebug() << Q_FUNC_INFO << "reading" << keyframeCount << "values";
    for (int frameData = 0; frameData < keyframeCount; frameData++) {
        keyframeTimes.push_back(file->parent->fileData.readFloat());
        float x_pos = file->fileData->readFloat();
        float y_pos = file->fileData->readFloat();
        float z_pos = file->fileData->readFloat();
        vectorList.push_back(QVector3D(x_pos, y_pos, z_pos));
    }
}

void AnimationTranslation::readCubic(){
    if(hasScalar){
        qDebug() << Q_FUNC_INFO << "reading a translation animation cubic motion";
        sectionLength = file->fileData->readInt();
        version3 = file->fileData->readInt();
        keyframeCount = file->fileData->readInt();
        subVersion = file->fileData->readInt();
        qDebug() << Q_FUNC_INFO << "reading" << keyframeCount << "values at" << file->fileData->currentPosition;
        for (int frameData = 0; frameData < keyframeCount; frameData++) {
            keyframeTimes.push_back(file->parent->fileData.readFloat());
            float x_pos = file->parent->fileData.readFloat();
            float y_pos = file->parent->fileData.readFloat();
            float z_pos = file->parent->fileData.readFloat();
            vectorList.push_back(QVector3D(x_pos, y_pos, z_pos));
        }
    } else {
        keyframeTimes.push_back(file->parent->fileData.readFloat());
        for(int i = 0; i < 3; i++){
            float x_pos = file->parent->fileData.readFloat();
            float y_pos = file->parent->fileData.readFloat();
            float z_pos = file->parent->fileData.readFloat();
            vectorList.push_back(QVector3D(x_pos, y_pos, z_pos));
        }
    }
}

void AnimationOrientation::readLinear(){
    qDebug() << Q_FUNC_INFO << "reading an orientation animation linear motion";
    sectionLength = file->fileData->readInt();
    version3 = file->fileData->readInt();
    keyframeCount = file->fileData->readInt();
    subVersion = file->fileData->readInt();
    qDebug() << Q_FUNC_INFO << "reading" << keyframeCount << "values at" << file->fileData->currentPosition;
    for (int frameData = 0; frameData < keyframeCount; frameData++) {
        keyframeTimes.push_back(file->parent->fileData.readFloat());
        if (hasPackedSpline) {
            rotationList.push_back(file->fileData->readMiniQuaternion());
        } else {
            rotationList.push_back(file->fileData->readQuaternion());
        }
    }
}

void AnimationOrientation::readCubic(){
    //might need to add check for miniquat
    if(hasScalar){
        qDebug() << Q_FUNC_INFO << "reading an orientation animation cubic motion";
        sectionLength = file->parent->fileData.readInt();
        version3 = file->parent->fileData.readInt();
        keyframeCount = file->parent->fileData.readInt();
        subVersion = file->parent->fileData.readInt();
        qDebug() << Q_FUNC_INFO << "reading" << keyframeCount << "values";
        for (int frameData = 0; frameData < keyframeCount; frameData++) {
            keyframeTimes.push_back(file->parent->fileData.readFloat());
            rotationList.push_back(file->fileData->readMiniQuaternion());
        }
    } else {
        keyframeTimes.push_back(file->parent->fileData.readFloat());
        for(int i = 0; i < 3; i++){
            rotationList.push_back(file->fileData->readMiniQuaternion());
        }
    }
}

void AnimationOrientation::readCubicC2(){
    //might need to add check for miniquat
    qDebug() << Q_FUNC_INFO << "reading an orientation animation cubic2 motion";
    sectionLength = file->parent->fileData.readInt();
    version3 = file->parent->fileData.readInt();
    keyframeCount = file->parent->fileData.readInt();
    subVersion = file->parent->fileData.readInt();
    qDebug() << Q_FUNC_INFO << "reading" << keyframeCount << "values";
    for (int frameData = 0; frameData < keyframeCount; frameData++) {
        keyframeTimes.push_back(file->parent->fileData.readFloat());
        rotationList.push_back(file->fileData->readMiniQuaternion());
        velocityList2.push_back(file->fileData->readMiniQuaternion());
        qDebug() << Q_FUNC_INFO << "keyframe" << keyframeTimes[frameData] << "has rotation" << rotationList[frameData] << "and velocity" << velocityList2[frameData];
    }
}

void AnimationTranslation::readCubicC2(){
    qDebug() << Q_FUNC_INFO << "reading an translation animation cubic2 motion";
    sectionLength = file->parent->fileData.readInt();
    version3 = file->parent->fileData.readInt();
    keyframeCount = file->parent->fileData.readInt();
    subVersion = file->parent->fileData.readInt();
    qDebug() << Q_FUNC_INFO << "reading" << keyframeCount << "values";
    for (int frameData = 0; frameData < keyframeCount; frameData++) {
        keyframeTimes.push_back(file->parent->fileData.readFloat());
        vectorList.push_back(file->fileData->read3DVector());
        velocityList.push_back(file->fileData->read3DVector());
        qDebug() << Q_FUNC_INFO << "keyframe" << keyframeTimes[frameData] << "has offset" << vectorList[frameData];
    }
}

float AnimationBase::findSplineAtTime(float time){
    for (int i = 0; i < keyframeTimes.size()-1; i++) {
        if(time > keyframeTimes[i] && time < keyframeTimes[i+1]){
            return (time - keyframeTimes[i]) / (keyframeTimes[i+1] - keyframeTimes[i]);
        }
    }
    return 0;
}

void AnimationBase::readLinear(){
    file->parent->messageError("THIS SHOULD NOT RUN");
}

void AnimationBase::readCubic(){
    file->parent->messageError("THIS SHOULD NOT RUN");
}

void AnimationBase::readCubicC2(){
    file->parent->messageError("THIS SHOULD NOT RUN");
}
