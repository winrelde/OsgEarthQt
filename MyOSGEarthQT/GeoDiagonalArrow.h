#pragma once
#ifndef GEODIAGONALARROW_H
#define GEODIAGONALARROW_H
#include "DrawTool.h"
#include <osgEarthAnnotation/FeatureEditing>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthSymbology/Style>


class GeoDiagonalArrow : public DrawTool
{
public:
    GeoDiagonalArrow(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

    virtual void beginDraw(const osg::Vec3d& lla);
    virtual void moveDraw(const osg::Vec3d& lla);
    virtual void endDraw(const osg::Vec3d& lla);
    virtual void resetDraw();

    /**
     * @brief 只有两个控制点时
     * @param ctrlPts
     * @return
     */
    std::vector<osg::Vec2> calculateTwoPoints(const std::vector<osg::Vec2>& ctrlPts);
    /**
     * @brief 有三个或三个以上的控制点时
     * @param ctrlPts
     * @return
     */
   // std::vector<osg::Vec2> calculateMorePoints(const std::vector<osg::Vec2>& ctrlPts);


private:
    osgEarth::Symbology::Style _polygonStyle;
    osg::ref_ptr<osgEarth::Annotation::FeatureNode>_featureNode;
    float _ratio;
};

#endif
