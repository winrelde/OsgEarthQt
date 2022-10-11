#include <Windows.h>
#include "common.h"
#include "GeoGatheringPlace.h"
#include "PlottingMath.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

GeoGatheringPlace::GeoGatheringPlace(osgEarth::MapNode *mapNode, osg::Group *drawGroup)
    : DrawTool(mapNode, drawGroup)
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

void GeoGatheringPlace::beginDraw(const osg::Vec3d &lla)
{
    _controlPoints.push_back(osg::Vec2(lla.x(), lla.y()));
    if (_controlPoints.empty() || _controlPoints.size() < 2)
        return;
    if (_controlPoints.size() == 2 && _controlPoints[0]==_controlPoints[1])
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

void GeoGatheringPlace::moveDraw(const osg::Vec3d &lla)
{
    if (_controlPoints.empty())
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

void GeoGatheringPlace::endDraw(const osg::Vec3d &lla)
{
}

void GeoGatheringPlace::resetDraw()
{
    _controlPoints.clear();
    _featureNode = NULL;
}

std::vector<osg::Vec2> GeoGatheringPlace::calculateParts(const std::vector<osg::Vec2> &controlPoints)
{
    //取第一个点作为第一控制点

    osg::Vec2 originP = controlPoints[0];
    //取最后一个作为第二控制点

    osg::Vec2 lastP = controlPoints[controlPoints.size()-1];
    std::vector<osg::Vec2> points;
    // 向量originP_lastP

    osg::Vec2 vectorOL(lastP.x()-originP.x(), lastP.y()-originP.y());
    // 向量originP_lastP的模

    float dOL = sqrtf(vectorOL.x() * vectorOL.x()+vectorOL.y() * vectorOL.y());

    //计算第一个插值控制点
    //向量originP_P1以originP为起点，与向量originP_lastP的夹角设为30，模为√3/12*dOL，

    std::vector<osg::Vec2> v_O_P1_lr = Math::calculateVector(vectorOL, osg::PI/3.0, sqrtf(3.0)/12.0*dOL);
    //取左边的向量作为向量originP_P1

    osg::Vec2 originP_P1 = v_O_P1_lr[0];
    osg::Vec2 p1(originP_P1.x()+originP.x(), originP_P1.y()+originP.y());

    //计算第二个插值控制点，取第一控制点和第二控制点的中点为第二个插值控制点

    osg::Vec2 p2((originP.x()+lastP.x())/2.0, (originP.y()+lastP.y())/2.0);

    //计算第三个插值控制点
    //向量originP_P3以lastP为起点，与向量originP_lastP的夹角设为150°，模为√3/12*dOL，

    std::vector<osg::Vec2> v_L_P3_lr = Math::calculateVector(vectorOL, osg::PI*2.0/3.0, sqrtf(3.0)/12.0*dOL);
    //取左边的向量作为向量originP_P1

    osg::Vec2 lastP_P3 = v_L_P3_lr[0];
    osg::Vec2 p3(lastP_P3.x()+lastP.x(), lastP_P3.y()+lastP.y());

    //计算第四个插值控制点
    //向量originP_P4以向量originP_lastP中点为起点，与向量originP_lastP的夹角设为90°，模为1/2*dOL，

    std::vector<osg::Vec2> v_O_P5_lr = Math::calculateVector(vectorOL, osg::PI_2, 1.0/2.0*dOL);
    //取左边的向量作为向量originP_P1

    osg::Vec2 v_O_P5 = v_O_P5_lr[1];
    osg::Vec2 p5(v_O_P5.x()+p2.x(), v_O_P5.y()+p2.y());

    osg::Vec2 P0 = originP;
    osg::Vec2 P4 = lastP;
    points.push_back(P0);
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(P4);
    points.push_back(p5);

    std::vector<osg::Vec2> cardinalPoints = Math::createCloseCardinal(points);
    return Math::createBezier3(cardinalPoints, 100);
}

