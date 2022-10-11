#ifndef GEODOUBLEARROW_H
#define GEODOUBLEARROW_H

#include "DrawTool.h"
#include <osgEarthAnnotation/FeatureEditing>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthSymbology/Style>

class GeoDoubleArrow : public DrawTool {
public:
    GeoDoubleArrow(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

    virtual void beginDraw(const osg::Vec3d& lla);
    virtual void moveDraw(const osg::Vec3d& lla);
    virtual void endDraw(const osg::Vec3d& lla);
    virtual void resetDraw();

    std::vector<osg::Vec2> calculateParts(const std::vector<osg::Vec2>& ctrlPts);

private:
    osgEarth::Symbology::Style _polygonStyle;
    osg::ref_ptr<osgEarth::Annotation::FeatureNode>_featureNode;
};

#endif
