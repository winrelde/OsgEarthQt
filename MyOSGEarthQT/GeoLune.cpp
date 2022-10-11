#include <Windows.h>
#include "common.h"
#include "GeoLune.h"
#include "PlottingMath.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

GeoLune::GeoLune(MapNode *mapNode, osg::Group *drawGroup)
    : DrawTool(mapNode, drawGroup)
    , _sides(360.0)
{
    _polygonStyle.getOrCreate<PolygonSymbol>()
        ->fill()
        ->color()
        = Color(Color::Yellow, 0.25);

    LineSymbol* ls = _polygonStyle.getOrCreate<LineSymbol>();
    ls->stroke()->color() = Color::White;
    ls->stroke()->width() = 2.0f;
    ls->stroke()->widthUnits() = osgEarth::Units::PIXELS;

    _polygonStyle.getOrCreate<AltitudeSymbol>()
        ->clamping()
        = AltitudeSymbol::CLAMP_TO_TERRAIN;
    _polygonStyle.getOrCreate<AltitudeSymbol>()
        ->technique()
        = AltitudeSymbol::TECHNIQUE_DRAPE;
    _polygonStyle.getOrCreate<AltitudeSymbol>()
        ->verticalOffset()
        = 0.1;
}

void GeoLune::beginDraw(const osg::Vec3d &lla)
{
    _controlPoints.push_back(osg::Vec2(lla.x(), lla.y()));

    if (_controlPoints.empty() || _controlPoints.size() < 3)
        return;

    _drawParts.clear();
    _drawParts = calculateParts(_controlPoints);

    if (!_featureNode.valid()) {
         Feature* feature = new Feature(new osgEarth::Polygon, getMapNode()->getMapSRS(), _polygonStyle);
        _featureNode = new FeatureNode(feature);
        drawCommand(_featureNode);
    }

    Geometry* geom = _featureNode->getFeature()->getGeometry();
    geom->clear();
    for (auto& n : _drawParts) {
        geom->push_back(osg::Vec3(n.x(), n.y(), 0));
    }
    _featureNode->init();

    _controlPoints.clear();
    _featureNode = NULL;

}

void GeoLune::moveDraw(const osg::Vec3d &lla)
{
    if (_controlPoints.empty() || _controlPoints.size() < 2)
        return;
    if (!_featureNode.valid()) {
         Feature* feature = new Feature(new osgEarth::Polygon, getMapNode()->getMapSRS(), _polygonStyle);
        _featureNode = new FeatureNode(feature);
        drawCommand(_featureNode);
    }
    if (_featureNode.valid()) {
        std::vector<osg::Vec2> ctrlPts = _controlPoints;
        std::vector<osg::Vec2> drawPts;
        ctrlPts.push_back(osg::Vec2(lla.x(), lla.y()));
        drawPts = calculateParts(ctrlPts);
        Geometry* geom = _featureNode->getFeature()->getGeometry();
        geom->clear();
        for (auto& n : drawPts) {
            geom->push_back(osg::Vec3(n.x(), n.y(), 0));
        }
        _featureNode->init();
    }
}

void GeoLune::endDraw(const osg::Vec3d &lla)
{

}

void GeoLune::resetDraw()
{
    _controlPoints.clear();
    _featureNode = NULL;
}

std::vector<osg::Vec2> GeoLune::calculateParts(const std::vector<osg::Vec2> &ctrlPts)
{
    std::vector<osg::Vec2> controlPoints = ctrlPts;

    //两个点时绘制半圆

    if (controlPoints.size() == 2) {
        osg::Vec2 pointA = controlPoints[0];
        osg::Vec2 pointB = controlPoints[1];
        osg::Vec2 centerP = Math::calculateMidpoint(pointA, centerP);
        float radius = Math::calculateDistance(pointA,pointB) / 2;
        float angleS = Math::calculateAngle(pointA, centerP);
        return Math::calculateArc(centerP,radius,angleS,angleS+osg::PI,-1);
    }
    //至少需要三个控制点

    if (ctrlPts.size() > 2) {
        osg::Vec2 pointA = controlPoints[0];
        osg::Vec2 pointB = controlPoints[1];
        osg::Vec2 pointC = controlPoints[2];
        std::vector<osg::Vec2> points;
        //以第一个点A、第二个点B为圆弧的端点，C为圆弧上的一点
        //计算A点和B点的中点

        osg::Vec2 midPointAB = Math::calculateMidpoint(pointA, pointB);
        //计算B点和C点的中点

        osg::Vec2 midPointBC = Math::calculateMidpoint(pointB, pointC);
        //计算向量AB

        osg::Vec2 vectorAB(pointB.x() - pointA.x(), pointB.y() - pointA.y());
        //计算向量BC

        osg::Vec2 vectorBC(pointC.x() - pointB.x(), pointC.y() - pointB.y());
        //判断三点是否共线，若共线，返回三点（直线）

        if (fabs(vectorAB.x()*vectorBC.y()-vectorBC.x()*vectorAB.y()) < 0.00001) {
            points.push_back(pointA);
            points.push_back(pointC);
            points.push_back(pointB);
            return points;
        }
        //计算过AB中点且与向量AB垂直的向量（AB的中垂线向量）

        osg::Vec2 vector_center_midPointAB = Math::calculateVector(vectorAB)[1];
        //计算过BC中点且与向量BC垂直的向量（BC的中垂线向量）

        osg::Vec2 vector_center_midPointBC = Math::calculateVector(vectorBC)[1];
        //计算圆弧的圆心
        osg::Vec2 centerPoint = Math::calculateIntersection(vector_center_midPointAB, vector_center_midPointBC, midPointAB, midPointBC);
        //计算圆弧的半径

        float radius = Math::calculateDistance(centerPoint, pointA);
        //分别计算三点所在的直径线与X轴的夹角

        float angleA = Math::calculateAngle(pointA, centerPoint);
        float angleB = Math::calculateAngle(pointB, centerPoint);
        float angleC = Math::calculateAngle(pointC, centerPoint);

        /*圆弧绘制思路为：
          angleA、angleB中最小的角对应的点为起点，最大的角对应的点为终点，若angleC不同时小于或不同时大于angleA与angleB，
          则从起点开始逆时针（direction=1）绘制点，直至终点；否则，从起点开始顺时针（direction=-1）绘制点，直至终点。
        */

        float direction = 1;
        float startAngle = angleA;
        float endAngle = angleB;
        osg::Vec2 startP, endP;
        if (angleA > angleB) {
            startAngle = angleB;
            endAngle = angleA;
            startP = pointB;
            endP = pointA;
        } else {
            startP = pointA;
            endP = pointB;
        }
        float length = endAngle-startAngle;
        if ((angleC<angleB &&angleC <angleA)||(angleC>angleB &&angleC >angleA))
        {
           direction = -1;
           length = startAngle+(2*osg::PI-endAngle);
        }

        //计算圆弧上点，默认每隔1°绘制2个点

        float step = osg::PI/_sides/2.0;
        float stepDir = step*direction;
        points.push_back(startP);
        for (float radians =startAngle,i = 0; i <length-step;i+=step) {
            radians+=stepDir;
            radians=radians<0?(radians+2*osg::PI):radians;
            radians=radians> 2*osg::PI?(radians-2*osg::PI):radians;
            osg::Vec2 circlePoint(cosf(radians) * radius + centerPoint.x(), sinf(radians) * radius + centerPoint.y());
            points.push_back(circlePoint);
        }
        points.push_back(endP);
        return points;
    }
}
