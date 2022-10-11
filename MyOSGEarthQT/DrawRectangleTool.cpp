#include <Windows.h>
#include "DrawRectangleTool.h"
#include <osgEarth/GeoMath>
#include <osgEarthAnnotation/AnnotationEditing>

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Annotation;

DrawRectangleTool::DrawRectangleTool(MapNode* mapNode, osg::Group* drawGroup)
	: DrawTool(mapNode, drawGroup)
	, _rectangleNode(nullptr)
{
	_rectangleStyle.getOrCreate<PolygonSymbol>()
		->fill()
		->color()
		= Color(Color::Yellow, 0.25);

	LineSymbol* ls = _rectangleStyle.getOrCreate<LineSymbol>();
	ls->stroke()->color() = Color::White;
	ls->stroke()->width() = 2.0f;
	ls->stroke()->widthUnits() = Units::PIXELS;

	//    RenderSymbol* render = _rectangleStyle.getOrCreate<RenderSymbol>();
	//    render->depthOffset()->enabled() = true;
	//    render->depthOffset()->automatic() = true;

	AltitudeSymbol* alt = _rectangleStyle.getOrCreate<AltitudeSymbol>();
	alt->clamping() = alt->CLAMP_TO_TERRAIN;
	alt->technique() = alt->TECHNIQUE_DRAPE;
}

void DrawRectangleTool::beginDraw(const osg::Vec3d& lla)
{
	if (_rectangleNode) {
		GeoPoint end = GeoPoint(getMapNode()->getMapSRS(), lla);
		_rectangleNode->setCorner(getPointCorner(end), end);
		return;
	}
	_vecPoints.push_back(lla);
	if (_vecPoints.size() <= 1) {
		return;
	}

	if (!_rectangleNode) {
		GeoPoint start = GeoPoint(getMapNode()->getMapSRS(), _vecPoints[0]);
		GeoPoint end = GeoPoint(getMapNode()->getMapSRS(), _vecPoints[1]);
		_rectangleNode = new RectangleNode(start,
			Distance(0, Units::KILOMETERS),
			Distance(0, Units::KILOMETERS),
			_rectangleStyle);
		_rectangleNode->setCorner(getPointCorner(end), end);
		drawCommand(osg::NodeList{ _rectangleNode, new RectangleNodeEditor(_rectangleNode) });
	}
}

void DrawRectangleTool::moveDraw(const osg::Vec3d& v)
{
}

void DrawRectangleTool::endDraw(const osg::Vec3d& v)
{
}

void DrawRectangleTool::resetDraw()
{
	_vecPoints.clear();
	_rectangleNode = nullptr;
}

RectangleNode::Corner DrawRectangleTool::getPointCorner(const GeoPoint& corner)
{
	GeoPoint center = _rectangleNode->getPosition();

	//Figure out the new width and height
	double earthRadius = center.getSRS()->getEllipsoid()->getRadiusEquator();

	double lat = osg::DegreesToRadians(center.y());
	double lon = osg::DegreesToRadians(center.x());
	double halfWidthMeters = _rectangleNode->getWidth().as(Units::METERS) / 2.0;
	double halfHeightMeters = _rectangleNode->getHeight().as(Units::METERS) / 2.0;

	double eastLon, eastLat;
	double westLon, westLat;
	double northLon, northLat;
	double southLon, southLat;

	//Get the current corners
	GeoMath::destination(lat, lon, osg::DegreesToRadians(90.0), halfWidthMeters, eastLat, eastLon, earthRadius);
	GeoMath::destination(lat, lon, osg::DegreesToRadians(-90.0), halfWidthMeters, westLat, westLon, earthRadius);
	GeoMath::destination(lat, lon, osg::DegreesToRadians(0.0), halfHeightMeters, northLat, northLon, earthRadius);
	GeoMath::destination(lat, lon, osg::DegreesToRadians(180.0), halfHeightMeters, southLat, southLon, earthRadius);

	if (osg::DegreesToRadians(corner.x()) < eastLon && osg::DegreesToRadians(corner.y()) > southLat) {
		return RectangleNode::CORNER_UPPER_LEFT;
	}
	else if (osg::DegreesToRadians(corner.x()) > westLon && osg::DegreesToRadians(corner.y()) > southLat) {
		return RectangleNode::CORNER_UPPER_RIGHT;
	}
	else if (osg::DegreesToRadians(corner.x()) < eastLon && osg::DegreesToRadians(corner.y()) < northLat) {
		return RectangleNode::CORNER_LOWER_LEFT;
	}
	else if (osg::DegreesToRadians(corner.x()) > westLon && osg::DegreesToRadians(corner.y()) < northLat) {
		return RectangleNode::CORNER_LOWER_RIGHT;
	}
}
