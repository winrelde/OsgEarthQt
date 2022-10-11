#include <Windows.h>
#include "common.h"
#include "DrawPolygonTool.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

DrawPolygonTool::DrawPolygonTool(osgEarth::MapNode* mapNode, osg::Group* drawGroup)
	: DrawTool(mapNode, drawGroup)
	, _featureNode(nullptr)
	, _stippleFeatureNode(nullptr)
	, _polygonEdit(nullptr)
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

	_stippleLineStyle.getOrCreate<LineSymbol>()
		->stroke()
		->color()
		= Color::Red;
	_stippleLineStyle.getOrCreate<LineSymbol>()
		->stroke()
		->width()
		= 2.0;
	_stippleLineStyle.getOrCreate<LineSymbol>()
		->tessellation()
		= 20.0;
	_stippleLineStyle.getOrCreate<AltitudeSymbol>()
		->clamping()
		= AltitudeSymbol::CLAMP_TO_TERRAIN;
	_stippleLineStyle.getOrCreate<AltitudeSymbol>()
		->technique()
		= AltitudeSymbol::TECHNIQUE_DRAPE;
	_stippleLineStyle.getOrCreate<AltitudeSymbol>()
		->verticalOffset()
		= 0.1;
	_stippleLineStyle.getOrCreate<LineSymbol>()
		->stroke()
		->stipple()
		= 255;
}

void DrawPolygonTool::beginDraw(const osg::Vec3d& lla)
{
	_vecPoints.push_back(lla);
	if (_vecPoints.size() <= 2) {
		return;
	}

	if (_polygonEdit != nullptr) {
		_polygonEdit->removeChildren(0, _polygonEdit->getNumChildren());
		_polygonEdit = nullptr;
	}
	if (_featureNode == nullptr) {
		Feature* feature = new Feature(
			new osgEarth::Symbology::Polygon,
			getMapNode()->getMapSRS());

		_featureNode = new FeatureNode(feature, _polygonStyle);
	}

	Geometry* geom = _featureNode->getFeature()->getGeometry();
	geom->clear();
	_featureNode->setStyle(_polygonStyle);
	for (auto& n : _vecPoints) {
		geom->push_back(n);
	}

	_featureNode->init();
	if (_stippleFeatureNode != nullptr) {
		_stippleFeatureNode->getFeature()->getGeometry()->clear();
	}
	if (_polygonEdit == nullptr) {
		_polygonEdit = new FeatureEditor(_featureNode);
	}
	drawCommand(osg::NodeList{ _featureNode, _polygonEdit });
}

void DrawPolygonTool::moveDraw(const osg::Vec3d& lla)
{
	if (_vecPoints.size() < 2) {
		return;
	}
	if (_stippleFeatureNode == nullptr) {
		Feature* feature = new Feature(
			new LineString,
			getMapNode()->getMapSRS(), _stippleLineStyle);
		_stippleFeatureNode = new FeatureNode(feature, _stippleLineStyle);

		drawCommand(_stippleFeatureNode);
	}

	Geometry* geom = _stippleFeatureNode->getFeature()->getGeometry();
	geom->clear();
	_stippleFeatureNode->setStyle(_stippleLineStyle);
	geom->push_back(_vecPoints[0]);
	geom->push_back(lla);
	geom->push_back(_vecPoints[_vecPoints.size() - 1]);

	_stippleFeatureNode->init();
}

void DrawPolygonTool::endDraw(const osg::Vec3d& lla)
{
}

void DrawPolygonTool::resetDraw()
{
	_vecPoints.clear();
	if (_stippleFeatureNode != nullptr) {
		_stippleFeatureNode->getFeature()->getGeometry()->clear();
	}
	_featureNode = nullptr;
}
