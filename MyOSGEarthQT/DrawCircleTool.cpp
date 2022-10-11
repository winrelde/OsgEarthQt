#include <Windows.h>
#include "DrawCircleTool.h"
#include <osgEarth/GeoMath>
#include <osgEarth/Units>

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Annotation;

DrawCircleTool::DrawCircleTool(MapNode* mapNode, osg::Group* drawGroup)
	: DrawTool(mapNode, drawGroup)
{
	//getDrawGroup()->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	_circleStyle.getOrCreate<PolygonSymbol>()->fill()->color()
		= Color(Color::Yellow, 0.25);

	LineSymbol* ls = _circleStyle.getOrCreate<LineSymbol>();
	ls->stroke()->color() = Color::White;
	ls->stroke()->width() = 1.0f;
	ls->stroke()->widthUnits() = Units::PIXELS;

	//    RenderSymbol* render = _circleStyle.getOrCreate<RenderSymbol>();
	//    render->depthOffset()->enabled() = true;
	//    render->depthOffset()->automatic() = true;

	AltitudeSymbol* alt = _circleStyle.getOrCreate<AltitudeSymbol>();
	alt->verticalOffset() = 0.1;
	alt->clamping() = alt->CLAMP_TO_TERRAIN;
	alt->technique() = alt->TECHNIQUE_DRAPE;
}

void DrawCircleTool::beginDraw(const osg::Vec3d& lla)
{
	if (_centerPoint == osg::Vec3d() && lla == osg::Vec3d()) {
		return;
	}
	else if (_centerPoint == osg::Vec3d()) {
		_centerPoint = lla;
	}

	if (_circleNode == 0) {
		_circleNode = new CircleNode();
		_circleNode->set(
			GeoPoint(getMapNode()->getMapSRS(), _centerPoint, ALTMODE_RELATIVE),
			Distance(300, Units::KILOMETERS),
			_circleStyle,
			Angle(0, Units::DEGREES),
			Angle(0, Units::DEGREES),
			true);

		_circleEdit = new CircleNodeEditor(_circleNode);
		drawCommand(osg::NodeList{ _circleNode, _circleEdit });
	}

	double radius = GeoMath::distance(_centerPoint, lla, getMapNode()->getMapSRS());

	osg::Vec3d midVec((lla.x() + _centerPoint.x()) / 2, (lla.y() + _centerPoint.y()) / 2, (lla.z() + _centerPoint.z()) / 2);
	osgEarth::GeoPoint midPoint(getMapNode()->getMapSRS(), midVec);

#ifdef SHOW_CIRCLE_RADIUS
	if (!_radiusNode.valid()) {
		_radiusNode = new PlaceNode(getMapNode(), midPoint, NULL, osgEarth::Stringify() << "r=" << radius);
		drawCommand(_radiusNode);
	}
	_radiusNode->setPosition(midPoint);
	_radiusNode->setText(osgEarth::Stringify() << "r=" << osgEarth::Distance(radius).as(Units::KILOMETERS) << "km");
#endif

	_circleNode->setRadius(radius);
	if (_centerPoint != lla) {
		_circleNode = NULL;
		_centerPoint = osg::Vec3d();
	}

}

void DrawCircleTool::moveDraw(const osg::Vec3d& lla)
{
	if (_circleNode != 0 && _centerPoint != osg::Vec3d()) {
		osg::Vec3d midVec((lla.x() + _centerPoint.x()) / 2, (lla.y() + _centerPoint.y()) / 2, (lla.z() + _centerPoint.z()) / 2);
		osgEarth::GeoPoint midPoint(getMapNode()->getMapSRS(), midVec);
		double radius = GeoMath::distance(_centerPoint, lla, getMapNode()->getMapSRS());
		_circleNode->setRadius(radius);
#ifdef SHOW_CIRCLE_RADIUS
		_radiusNode->setPosition(midPoint);
		_radiusNode->setText(osgEarth::Stringify() << "r=" << osgEarth::Distance(radius).as(Units::KILOMETERS) << "km");
#endif
	}
}

void DrawCircleTool::endDraw(const osg::Vec3d& lla)
{
}

void DrawCircleTool::resetDraw()
{
	_centerPoint = osg::Vec3d();
	_circleNode = NULL;
#ifdef SHOW_CIRCLE_RADIUS
	_radiusNode = NULL;
#endif
}
