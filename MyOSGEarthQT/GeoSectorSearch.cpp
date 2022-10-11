#include <Windows.h>
#include "common.h"
#include "GeoSectorSearch.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

GeoSectorSearch::GeoSectorSearch(MapNode *mapNode, osg::Group *drawGroup)
    : DrawTool(mapNode, drawGroup)
{
    // clamp to the terrain skin as it pages in
    AltitudeSymbol* alt = _lineStyle.getOrCreate<AltitudeSymbol>();
    alt->clamping() = alt->CLAMP_TO_TERRAIN;
    alt->technique() = alt->TECHNIQUE_DRAPE;

    // offset to mitigate Z fighting
    RenderSymbol* render = _lineStyle.getOrCreate<RenderSymbol>();
    render->lighting() = false;
    render->depthOffset()->enabled() = true;
    render->depthOffset()->automatic() = true;

    // define a style for the line
    LineSymbol* ls = _lineStyle.getOrCreate<LineSymbol>();
    ls->stroke()->color() = Color::Yellow;
    ls->stroke()->width() = 3.0f;
    ls->stroke()->widthUnits() = Units::PIXELS;
    ls->tessellation() = 150;
}

void GeoSectorSearch::beginDraw(const osg::Vec3d &lla)
{
    _controlPoints.push_back(osg::Vec2(lla.x(), lla.y()));
    if (_controlPoints.empty() || _controlPoints.size() < 2)
        return;
    multiLine_.clear();

    multiLine_ = calculateParts(_controlPoints);

    if (!_featureNode.valid()) {
        Feature* feature = new Feature(new MultiGeometry, getMapNode()->getMapSRS(), _lineStyle);
        _featureNode = new FeatureNode(feature);
        //        _drawGroup->addChild(_featureNode);
        drawCommand(_featureNode);
    }

    MultiGeometry* multiGeom = dynamic_cast<MultiGeometry*>(_featureNode->getFeature()->getGeometry());
    multiGeom->getComponents().clear();
    if (multiGeom) {
        for (unsigned int i =0; i < multiLine_.size(); i++) {
            if (multiLine_[i].size() <= 2) {
                Geometry* seg = new LineString(2);
                seg->push_back(osg::Vec3(multiLine_[i][0].x(), multiLine_[i][0].y(), 0));
                seg->push_back(osg::Vec3(multiLine_[i][1].x(), multiLine_[i][1].y(), 0));
                multiGeom->add(seg);
            } else {
                Geometry* seg = new LineString(multiLine_[i].size());
                for (unsigned int j = 0; j < multiLine_[i].size(); j++) {
                    seg->push_back(osg::Vec3(multiLine_[i][j].x(), multiLine_[i][j].y(), 0));
                }
                multiGeom->add(seg);
            }
        }
    }
    _featureNode->init();

    if (_controlPoints.size() >= 2) {
        _controlPoints.clear();
        _featureNode = NULL;
    }
}

void GeoSectorSearch::moveDraw(const osg::Vec3d &lla)
{
    if (_controlPoints.empty() || _controlPoints.size() < 1)
        return;
    if (!_featureNode.valid()) {
        Feature* feature = new Feature(new MultiGeometry, getMapNode()->getMapSRS(), _lineStyle);
        _featureNode = new FeatureNode(feature);
        //        _drawGroup->addChild(_featureNode);
        drawCommand(_featureNode);
    }
    std::vector<osg::Vec2> ctrlPts = _controlPoints;
    ctrlPts.push_back(osg::Vec2(lla.x(), lla.y()));
    Math::MultiLineString multiLine = calculateParts(ctrlPts);

    MultiGeometry* multiGeom = dynamic_cast<MultiGeometry*>(_featureNode->getFeature()->getGeometry());
    multiGeom->getComponents().clear();
    if (multiGeom) {
        for (unsigned int i =0; i < multiLine.size(); i++) {
            if (multiLine[i].size() <= 2) {
                Geometry* seg = new LineString(2);
                seg->push_back(osg::Vec3(multiLine[i][0].x(), multiLine[i][0].y(), 0));
                seg->push_back(osg::Vec3(multiLine[i][1].x(), multiLine[i][1].y(), 0));
                multiGeom->add(seg);
            } else {
                Geometry* seg = new LineString(multiLine[i].size());
                for (unsigned int j = 0; j < multiLine[i].size(); j++) {
                    seg->push_back(osg::Vec3(multiLine[i][j].x(), multiLine[i][j].y(), 0));
                }
                multiGeom->add(seg);
            }
        }
    }
    _featureNode->init();
}

void GeoSectorSearch::endDraw(const osg::Vec3d &lla)
{

}

void GeoSectorSearch::resetDraw()
{
    _controlPoints.clear();
    _featureNode = NULL;
}

Math::MultiLineString GeoSectorSearch::calculateParts(const std::vector<osg::Vec2> &controlPoints)
{
    //两个控制点时，绘制直线

    Math::MultiLineString multiLine;
    //第一个点为起点，也是中心点

    osg::Vec2 centerPoint = controlPoints[0];
    float offsetX = 2.0*centerPoint.x();
    float offsetY = 2.0*centerPoint.y();
    //第二个点确定半径和起始方向，且为第一个扇形(Fisrst)的点

    osg::Vec2 point_FB = controlPoints[controlPoints.size()-1];
    float radius = Math::calculateDistance(centerPoint, point_FB);
    osg::Vec2 vector_S = centerPoint - point_FB;
    //起始方向向右120°为第二个方向，确定第一个扇形的点

    std::vector<osg::Vec2> vectors = Math::calculateVector(vector_S, 4*osg::PI/3, radius);
    osg::Vec2 vector_FR = vectors[0];
    osg::Vec2 point_FC(vector_FR.x()+centerPoint.x(), vector_FR.y()+centerPoint.y());

    //第二个(second)扇形

    osg::Vec2 point_SB(-point_FC.x()+offsetX, -point_FC.y()+offsetY);
    osg::Vec2 vector_SL = vectors[1];
    osg::Vec2 point_SC(vector_SL.x()+centerPoint.x(), vector_SL.y()+centerPoint.y());

    //第三个(Third)扇形

    osg::Vec2 point_TB(-point_SC.x()+offsetX, -point_SC.y()+offsetY);
    osg::Vec2 point_TC(-point_FB.x()+offsetX, -point_FB.y()+offsetY);

    //连接点成扇形搜寻符号

    std::vector<osg::Vec2> points{centerPoint,point_FB,point_FC,point_SB,point_SC,point_TB,point_TC,centerPoint};
    multiLine.push_back(points);

    //计算各边的箭头

    Math::MultiLineString arrows_FA = Math::calculateArrowLines(centerPoint, point_FB);
    Math::MultiLineString arrows_FB= Math::calculateArrowLines(point_FB, point_FC);
    Math::MultiLineString arrows_FC= Math::calculateArrowLines(point_FC, point_SB);
    Math::MultiLineString arrows_SB= Math::calculateArrowLines(point_SB, point_SC);
    Math::MultiLineString arrows_SC= Math::calculateArrowLines(point_SC, point_TB);
    Math::MultiLineString arrows_TB= Math::calculateArrowLines(point_TB, point_TC);
    Math::MultiLineString arrows_TC= Math::calculateArrowLines(point_TC, centerPoint);
    //arrows_FA
    multiLine.push_back(arrows_FA[0]);
    multiLine.push_back(arrows_FA[1]);
    //arrows_FB
    multiLine.push_back(arrows_FB[0]);
    multiLine.push_back(arrows_FB[1]);
    //arrows_FC
    multiLine.push_back(arrows_FC[0]);
    multiLine.push_back(arrows_FC[1]);
    //arrows_SB
    multiLine.push_back(arrows_SB[0]);
    multiLine.push_back(arrows_SB[1]);
    //arrows_SC
    multiLine.push_back(arrows_SC[0]);
    multiLine.push_back(arrows_SC[1]);
    //arrows_TB
    multiLine.push_back(arrows_TB[0]);
    multiLine.push_back(arrows_TB[1]);
    //arrows_TC
    multiLine.push_back(arrows_TC[0]);
    multiLine.push_back(arrows_TC[1]);
    return multiLine;

}
