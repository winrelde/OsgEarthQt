#ifndef GEOSECTORSEARCH_H
#define GEOSECTORSEARCH_H

#include "DrawTool.h"
#include "PlottingMath.h"

#include <osgEarthFeatures/Feature>
#include <osgEarthSymbology/Style>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthAnnotation/FeatureEditing>

class GeoSectorSearch : public DrawTool {
public:
    GeoSectorSearch(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

    virtual void beginDraw(const osg::Vec3d& lla);
    virtual void moveDraw(const osg::Vec3d& lla);
    virtual void endDraw(const osg::Vec3d& lla);
    virtual void resetDraw();

    Math::MultiLineString calculateParts(const std::vector<osg::Vec2>& controlPoints);

private:
    Math::MultiLineString multiLine_;
    osgEarth::Symbology::Style _lineStyle;
    osg::ref_ptr<osgEarth::Annotation::FeatureNode> _featureNode;
};

#endif
