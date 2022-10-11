#ifndef GEOLUNE_H
#define GEOLUNE_H 1

#include "DrawTool.h"

#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthSymbology/Style>

class GeoLune : public DrawTool {
public:
    GeoLune(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

    virtual void beginDraw(const osg::Vec3d& lla);
    virtual void moveDraw(const osg::Vec3d& lla);
    virtual void endDraw(const osg::Vec3d& lla);
    virtual void resetDraw();

    std::vector<osg::Vec2> calculateParts(const std::vector<osg::Vec2>& ctrlPts);

private:
    osgEarth::Symbology::Style _polygonStyle;
    osg::ref_ptr<osgEarth::Annotation::FeatureNode>_featureNode;
    /**
    * APIProperty: sides
    * {Integer} 弓形上圆弧的点密度。默认为720，即每隔1°绘制两个点。
    */
    int _sides;
};

#endif
