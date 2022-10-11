#include <Windows.h>
#include "DrawLineTool.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

DrawLineTool::DrawLineTool(osgEarth::MapNode* mapNode, osg::Group* drawGroup)
	: DrawTool(mapNode, drawGroup)
	, _feature(0)
	, _featureNode(0)
	, _stippleFeature(0)
	, _stippleFeatureNode(0)

{
	_vecPoint.clear();

	_stippleLineStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Red;
	_stippleLineStyle.getOrCreate<LineSymbol>()->stroke()->width() = 2.0;
	_stippleLineStyle.getOrCreate<LineSymbol>()->tessellation() = 20.0;
	_stippleLineStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_TO_TERRAIN;
	_stippleLineStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_DRAPE;
	_stippleLineStyle.getOrCreate<AltitudeSymbol>()->verticalOffset() = 0.1;
	_stippleLineStyle.getOrCreate<LineSymbol>()->stroke()->stipple() = 255;

	// Define the path feature:
	_feature = new Feature(new LineString(), getMapNode()->getMapSRS());
	_feature->geoInterp() = GEOINTERP_GREAT_CIRCLE;

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

void DrawLineTool::beginDraw(const osg::Vec3d& lla)
{
	_vecPoint.push_back(lla);
	if (_vecPoint.size() <= 1) {
		return;
	}
	if (_featureNode == 0) {
		_feature = new Feature(
			new LineString,
			getMapNode()->getMapSRS(), _lineStyle);
		_featureNode = new FeatureNode(_feature, _lineStyle);

		drawCommand(_featureNode);
	}
	_feature->getGeometry()->clear();
	_featureNode->setStyle(_lineStyle);
	for (auto& n : _vecPoint) {
		_feature->getGeometry()->push_back(n);
	}

	_featureNode->init();

	if (_stippleFeatureNode != 0) {
		_stippleFeature->getGeometry()->clear();
	}
}

void DrawLineTool::moveDraw(const osg::Vec3d& lla)
{
	if (_vecPoint.size() <= 0) {
		return;
	}
	if (_stippleFeatureNode == 0) {
		_stippleFeature = new Feature(
			new LineString,
			getMapNode()->getMapSRS(), _lineStyle);
		_stippleFeatureNode = new FeatureNode(_stippleFeature, _lineStyle);

		drawCommand(_stippleFeatureNode);
	}

	_stippleFeature->getGeometry()->clear();
	_stippleFeature->getGeometry()->push_back(_vecPoint[_vecPoint.size() - 1]);
	_stippleFeature->getGeometry()->push_back(lla);

	_stippleFeatureNode->init();
}

void DrawLineTool::endDraw(const osg::Vec3d& lla)
{
}

void DrawLineTool::resetDraw()
{
	_vecPoint.clear();
	if (_stippleFeatureNode != 0) {
		_stippleFeature->getGeometry()->clear();
	}
	_featureNode = 0;
}
