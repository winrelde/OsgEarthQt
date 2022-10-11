#include <Windows.h>
#include "common.h"
#include "GeoParallelSearch.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;


GeoParallelSearch::GeoParallelSearch(MapNode *mapNode, osg::Group *drawGroup)
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

void GeoParallelSearch::beginDraw(const osg::Vec3d &lla)
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
}

void GeoParallelSearch::moveDraw(const osg::Vec3d &lla)
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

void GeoParallelSearch::endDraw(const osg::Vec3d &lla)
{

}

void GeoParallelSearch::resetDraw()
{
    _controlPoints.clear();
    _featureNode = NULL;
}


Math::MultiLineString GeoParallelSearch::calculateParts(const std::vector<osg::Vec2> &controlPoints)
{
    Math::MultiLineString multiLine;
    //两个控制点时，绘制直线
//    if (controlPoints.size() > 1) {
//        multiLine.push_back(controlPoints);
//    }

    if (controlPoints.size() > 2) {
        osg::Vec2 firstP = controlPoints[0];
        osg::Vec2 secondP = controlPoints[1];
        //第一、二个点的向量为基准向量

        osg::Vec2 vectorBase = firstP-secondP;
        //基准向量的法向量

        osg::Vec2 vectorNormal = Math::calculateVector(vectorBase)[0];
        //从第三个点开始，当i为奇数，则第i-1、i个点的向量垂直于基准向量，当i为偶数，则第i-1、i个点的向量平行于垂直基准向量

        bool isParalel = false;
        std::vector<osg::Vec2> points;
        points.push_back(firstP);

        for (int i = 1; i < controlPoints.size(); i++) {
            //判断是否平行

            isParalel = (i%2 != 0);
            osg::Vec2 pointI = controlPoints[i];
            //平行

            if (isParalel) {
                osg::Vec2 previousP = points[i-1];
                osg::Vec2 point = Math::calculateIntersection(vectorNormal,vectorBase,pointI,previousP);
                points.push_back(point);
                Math::MultiLineString arrowLines = Math::calculateArrowLines(previousP,point,15);
                multiLine.push_back(arrowLines[0]);
                multiLine.push_back(arrowLines[1]);
            } else { //垂直

                osg::Vec2 previousP = points[i-1];
                osg::Vec2 point = Math::calculateIntersection(vectorBase, vectorNormal, pointI, previousP);
                points.push_back(point);
                Math::MultiLineString arrowLines = Math::calculateArrowLines(previousP,point,15);
                multiLine.push_back(arrowLines[0]);
                multiLine.push_back(arrowLines[1]);
            }
        }
        multiLine.insert(multiLine.begin(), points);
    }
    return multiLine;
}
