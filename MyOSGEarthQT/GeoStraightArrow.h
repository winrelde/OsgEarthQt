#pragma once
#ifndef GEOSTRAIGHTARROW_H
#define GEOSTRAIGHTARROW_H
#include "DrawTool.h"
#include <osgEarthAnnotation/FeatureEditing>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthSymbology/Style>

class GeoStraightArrow : public DrawTool {
public:
    GeoStraightArrow(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

    virtual void beginDraw(const osg::Vec3d& lla);
    virtual void moveDraw(const osg::Vec3d& lla);
    virtual void endDraw(const osg::Vec3d& lla);
    virtual void resetDraw();

private:
    /**
     * 计算两个控制点时直箭头的所有绘制点
     * 两个控制点的直箭头绘制点只需要7个就可以构成
     */
    std::vector<osg::Vec2> calculateTwoPoints(const std::vector<osg::Vec2>& ctrlPts);
    /**
     * 计算三个或三个以上的控制点时的所有绘制点
     * 由于中间的控制点之间会进行差值，产生曲线效果，所以所需绘制点会很多
     * 这里使用的思想是将所有用户控制点连接起来形成一条折线段，
     * 每一条线段向左右两边扩充两条平行线，这样就形成了一个折线形式的箭头，
     * 然后在拐角进行曲线化处理（二次贝塞尔曲线差值），就形成了效果比较好的箭头
     */
    std::vector<osg::Vec2> calculateMorePoints(const std::vector<osg::Vec2>& ctrlPts);

    /**
     * 重写了父类的方法
     * 用于通过控制点计算箭头的所有绘制点
     */
    void calculateParts();

private:
    float _ratio;
    osgEarth::Symbology::Style _polygonStyle;
    osg::ref_ptr<osgEarth::Annotation::FeatureNode>_featureNode;
    osg::ref_ptr<osgEarth::Annotation::FeatureEditor> _polygonEdit;
};

#endif
