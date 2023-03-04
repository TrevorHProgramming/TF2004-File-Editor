#ifndef ANTIOCH2_H
#define ANTIOCH2_H

#include <QVector>
#include <QQuaternion>

class VBIN;

class AnimationBase{
public:
    VBIN *file;
    QString name;
    long sectionLength;
    long sectionEnd;
    QString animationType; //Linear, cubic, scalar, etc

    int unknownValue1;
    int unknownValue2;
    int unknownValue3;
    int unknownValue4;
    int keyframeCount;
    int unknownValue5;

    float maxTime;

    std::vector<float> keyframeTimes;

    std::vector<QVector3D> vectorList;
    std::vector<QQuaternion> rotationList;
    std::vector<QVector3D> velocityList;
    std::vector<QQuaternion> velocityList2;
    float scale;
    bool hasScalar;

    static AnimationBase *makeAnimation(QString animationSignature);
    float findSplineAtTime(float time);
    virtual void readLinear();
    virtual void readCubic();
    virtual void readCubicC2();


    void readScalar();
};

class AnimationTranslation : public AnimationBase {
public:
    void readLinear();
    void readCubic();
    void readCubicC2();
};

class AnimationOrientation : public AnimationBase {
    void readLinear();
    void readCubic();
    void readCubicC2();
};

class AnimationScale : public AnimationBase {
};

class AnimationStream{
public:
    VBIN* file;
    long fileLocation;
    QString name;
    long sectionLength;
    long sectionEnd;

    int unknownValue1;
    int unknownValue2;
    int unknownValue3;
    int unknownValue4;
    float unknownFloat1;
    int channelCount;

    std::vector<AnimationBase*> channelArray;

    void readAnimationStream();
};

class AnimationSourceSet{
  public:
    VBIN* file;
    long fileLocation;
    QString name;
    long sectionLength;
    long sectionEnd;

    int unknownValue1;
    int animationCount;

    std::vector<AnimationStream*> streamArray;

    void readAnimationSet();
};

#endif // ANTIOCH2_H
