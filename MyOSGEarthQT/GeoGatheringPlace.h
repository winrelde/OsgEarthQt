#ifndef GEOGATHERINGPLACE_H
#define GEOGATHERINGPLACE_H

#include "DrawTool.h"
#include <osgEarthAnnotation/FeatureEditing>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthSymbology/Style>

class GeoGatheringPlace : public DrawTool {
public:

    GeoGatheringPlace(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

    virtual void beginDraw(const osg::Vec3d& lla);
    virtual void moveDraw(const osg::Vec3d& lla);
    virtual void endDraw(const osg::Vec3d& lla);
    virtual void resetDraw();

    /**
    * Method: calculateParts
    * 重写了父类的方法
    * 用于通过控制点计算聚集地符号的所有点
    */
    std::vector<osg::Vec2> calculateParts(const std::vector<osg::Vec2>& controlPoints);

private:
    osgEarth::Symbology::Style _polygonStyle;
    osg::ref_ptr<osgEarth::Annotation::FeatureNode>_featureNode;
    osg::ref_ptr<osgEarth::Annotation::FeatureEditor> _polygonEdit;
};

#endif
