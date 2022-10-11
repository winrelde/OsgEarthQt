#include <Windows.h>
#include "ScaleBar.h"

#include <osg/GraphicsContext>
#include <osgEarth/GeoMath>
#include <osgEarth/Terrain>

double ScaleBar::normalizeScaleMeters(double meters)
{
	if (meters <= 3) {
		meters = 1;
	}
	else if (meters <= 7.5) {
		meters = 5;
	}
	else if (meters <= 15) {
		meters = 10;
	}
	else if (meters <= 35) {
		meters = 20;
	}
	else if (meters <= 75) {
		meters = 50;
	}
	else if (meters <= 150) {
		meters = 100;
	}
	else if (meters <= 350) {
		meters = 200;
	}
	else if (meters <= 750) {
		meters = 500;
	}
	else if (meters <= 1500) {
		meters = 1000;
	}
	else if (meters <= 3500) {
		meters = 2000;
	}
	else if (meters <= 7500) {
		meters = 5000;
	}
	else if (meters <= 15000) {
		meters = 10000;
	}
	else if (meters <= 35000) {
		meters = 20000;
	}
	else if (meters <= 55000) {
		meters = 50000;
	}
	else if (meters <= 150000) {
		meters = 100000;
	}
	else if (meters <= 350000) {
		meters = 200000;
	}
	else if (meters <= 750000) {
		meters = 500000;
	}
	else if (meters <= 1500000) {
		meters = 1000000;
	}
	else {
		meters = 2000000;
	}
	return meters;
}

double ScaleBar::normalizeScaleFeet(double feet)
{
	double feetPerMile = 5280.0;
	if (feet <= 7.5) {
		feet = 5;
	}
	else if (feet <= 15) {
		feet = 10;
	}
	else if (feet <= 35) {
		feet = 20;
	}
	else if (feet <= 75) {
		feet = 50;
	}
	else if (feet <= 150) {
		feet = 100;
	}
	else if (feet <= 350) {
		feet = 200;
	}
	else if (feet <= 750) {
		feet = 500;
	}
	else if (feet <= 1500) {
		feet = 1000;
	}
	else if (feet <= 3640) {
		feet = 2000;
	}
	else if (feet <= 1.5 * feetPerMile) {
		feet = 1 * feetPerMile;
	}
	else if (feet <= 3.5 * feetPerMile) {
		feet = 2 * feetPerMile;
	}
	else if (feet <= 7.5 * feetPerMile) {
		feet = 5 * feetPerMile;
	}
	else if (feet <= 15 * feetPerMile) {
		feet = 10 * feetPerMile;
	}
	else if (feet <= 35 * feetPerMile) {
		feet = 20 * feetPerMile;
	}
	else if (feet <= 75 * feetPerMile) {
		feet = 50 * feetPerMile;
	}
	else if (feet <= 150 * feetPerMile) {
		feet = 100 * feetPerMile;
	}
	else if (feet <= 350 * feetPerMile) {
		feet = 200 * feetPerMile;
	}
	else if (feet <= 750 * feetPerMile) {
		feet = 500 * feetPerMile;
	}
	else if (feet <= 1500 * feetPerMile) {
		feet = 1000 * feetPerMile;
	}
	else {
		feet = 2000 * feetPerMile;
	}
	return feet;
}

double ScaleBar::normalizeScaleNauticalMiles(double nmi)
{
	//double feetPerMile = 6076.12;
	if (nmi <= 0.0015) {
		nmi = 0.001;
	}
	else if (nmi <= 0.0035) {
		nmi = 0.002;
	}
	else if (nmi <= 0.0075) {
		nmi = 0.005;
	}
	else if (nmi <= 0.015) {
		nmi = 0.01;
	}
	else if (nmi <= 0.035) {
		nmi = 0.02;
	}
	else if (nmi <= 0.075) {
		nmi = 0.05;
	}
	else if (nmi <= 0.15) {
		nmi = 0.1;
	}
	else if (nmi <= 0.35) {
		nmi = 0.2;
	}
	else if (nmi <= 0.75) {
		nmi = 0.5;
	}
	else if (nmi <= 1.5) {
		nmi = 1;
	}
	else if (nmi <= 3.5) {
		nmi = 2;
	}
	else if (nmi <= 7.5) {
		nmi = 5;
	}
	else if (nmi <= 15) {
		nmi = 10;
	}
	else if (nmi <= 35) {
		nmi = 20;
	}
	else if (nmi <= 75) {
		nmi = 50;
	}
	else if (nmi <= 150) {
		nmi = 100;
	}
	else if (nmi <= 350) {
		nmi = 200;
	}
	else if (nmi <= 750) {
		nmi = 500;
	}
	else if (nmi <= 1500) {
		nmi = 1000;
	}
	else {
		nmi = 2000;
	}
	return nmi;
}

ScaleBar::ScaleBar(osgEarth::MapNode* mapNode, osgViewer::View* view)
	: _mapNode(mapNode)
	, _view(view)
	, _windowWidth(500)
	, _windowHeight(500)
	, _scaleBarUnits(UNITS_METERS)
{
	_map = mapNode->getMap();

	_scaleLabel = new osgEarth::Util::Controls::LabelControl("- km", 12.0f);
	_scaleLabel->setForeColor(osg::Vec4f(0, 0, 0, 1));
	_scaleBar = new osgEarth::Util::Controls::Frame();
	_scaleBar->setVertFill(true);
	_scaleBar->setForeColor(osg::Vec4f(0, 0, 0, 0.8));
	_scaleBar->setBackColor(osg::Vec4f(1, 1, 1, 0.5));
	_scaleBar->setBorderColor(osg::Vec4f(0, 0, 0, 1));
	_scaleBar->setBorderWidth(1.0);
}

void ScaleBar::setVisible(bool visible)
{
	if (_scaleLabel.valid()) {
		_scaleLabel->setVisible(visible);
	}
	if (_scaleBar.valid()) {
		_scaleBar->setVisible(visible);
	}
}

double ScaleBar::computeScale()
{
	if (!_scaleLabel.valid() || !_scaleLabel->visible()) {
		return -1.0;
	}
	if (!_mapNode.valid() || _mapNode->getTerrain() == NULL) {
		//        ERROR("No map");
		return -1.0;
	}
	if (!_view.valid()) {
		//        ERROR("No viewer");
		return -1.0;
	}

	osg::ref_ptr<osg::GraphicsContext> gc = _view->getCamera()->getGraphicsContext();
	if (gc) {
		auto t = gc->getTraits();
		_windowWidth = t->width;
		_windowHeight = t->height;
	}

	double x, y;
	double pixelWidth = _windowWidth * 0.1 * 2.0;
	if (pixelWidth < 10)
		pixelWidth = 10;
	if (pixelWidth > 150)
		pixelWidth = 150;
	x = (double)(_windowWidth - 1) / 2.0 - pixelWidth / 2.0;
	y = (double)(_windowHeight - 1) / 2.0;

	osg::Vec3d world1, world2;
	if (!_mapNode->getTerrain()->getWorldCoordsUnderMouse(_view->asView(), x, y, world1)) {
		// off map
		//        TRACE("Off map coords: %g %g", x, y);
		_scaleLabel->setText("");
		_scaleBar->setWidth(0);
		return -1.0;
	}
	x += pixelWidth;
	if (!_mapNode->getTerrain()->getWorldCoordsUnderMouse(_view->asView(), x, y, world2)) {
		// off map
		//        TRACE("Off map coords: %g %g", x, y);
		_scaleLabel->setText("");
		_scaleBar->setWidth(0);
		return -1.0;
	}

#if 0
	TRACE("w1: %g %g %g w2: %g %g %g",
		world1.x(), world1.y(), world1.z(),
		world2.x(), world2.y(), world2.z());
#endif

	double meters;
	double radius = 6378137.0;
	if (_mapNode->getMapSRS() && _mapNode->getMapSRS()->getEllipsoid()) {
		radius = _mapNode->getMapSRS()->getEllipsoid()->getRadiusEquator();
	}
	if (!_map->isGeocentric() && _mapNode->getMapSRS() && _mapNode->getMapSRS()->isGeographic()) {
		//        TRACE("Map is geographic");
		// World cords are already lat/long
		// Compute great circle distance
		meters = osgEarth::GeoMath::distance(world1, world2, _mapNode->getMapSRS());
	}
	else if (_mapNode->getMapSRS()) {
		// Get map coords in lat/long
		osgEarth::GeoPoint mapPoint1, mapPoint2;
		mapPoint1.fromWorld(_mapNode->getMapSRS(), world1);
		mapPoint1.makeGeographic();
		mapPoint2.fromWorld(_mapNode->getMapSRS(), world2);
		mapPoint2.makeGeographic();
		// Compute great circle distance
		meters = osgEarth::GeoMath::distance(osg::DegreesToRadians(mapPoint1.y()),
			osg::DegreesToRadians(mapPoint1.x()),
			osg::DegreesToRadians(mapPoint2.y()),
			osg::DegreesToRadians(mapPoint2.x()),
			radius);
	}
	else {
		// Assume geocentric?
		//        ERROR("No map SRS");
		_scaleLabel->setText("");
		_scaleBar->setWidth(0);
		return -1.0;
	}

	double scale = meters / pixelWidth;
	// 1mi = 5280 feet
	//double scaleMiles = scale / 1609.344; // International mile = 1609.344m
	//double scaleNauticalMiles = scale / 1852.0; // nautical mile = 1852m
	//double scaleUSSurveyMiles = scale / 1609.347218694; // US survey mile = 5280 US survey feet
	//double scaleUSSurveyFeet = scale * 3937.0/1200.0; // US survey foot = 1200/3937 m
#if 0
	TRACE("m: %g px: %g m/px: %g", meters, pixelWidth, scale);
#endif
	_mapScale = scale;
	switch (_scaleBarUnits) {
	case UNITS_NAUTICAL_MILES: {
		double nmi = meters / 1852.0;
		scale = nmi / pixelWidth;
		nmi = normalizeScaleNauticalMiles(nmi);
		pixelWidth = nmi / scale;
		if (_scaleLabel.valid()) {
			_scaleLabel->setText(osgEarth::Stringify()
				<< nmi
				<< " nmi");
		}
	} break;
	case UNITS_US_SURVEY_FEET: {
		double feet = meters * 3937.0 / 1200.0;
		scale = feet / pixelWidth;
		feet = normalizeScaleFeet(feet);
		pixelWidth = feet / scale;
		if (_scaleLabel.valid()) {
			if (feet >= 5280) {
				_scaleLabel->setText(osgEarth::Stringify()
					<< feet / 5280.0
					<< " miUS");
			}
			else {
				_scaleLabel->setText(osgEarth::Stringify()
					<< feet
					<< " ftUS");
			}
		}
	} break;
	case UNITS_INTL_FEET: {
		double feet = 5280.0 * meters / 1609.344;
		scale = feet / pixelWidth;
		feet = normalizeScaleFeet(feet);
		pixelWidth = feet / scale;
		if (_scaleLabel.valid()) {
			if (feet >= 5280) {
				_scaleLabel->setText(osgEarth::Stringify()
					<< feet / 5280.0
					<< " mi");
			}
			else {
				_scaleLabel->setText(osgEarth::Stringify()
					<< feet
					<< " ft");
			}
		}
	} break;
	case UNITS_METERS:
	default: {
		meters = normalizeScaleMeters(meters);
		pixelWidth = meters / scale;
		if (_scaleLabel.valid()) {
			if (meters >= 1000) {
				_scaleLabel->setText(osgEarth::Stringify()
					<< meters / 1000.0
					<< " km");
			}
			else {
				_scaleLabel->setText(osgEarth::Stringify()
					<< meters
					<< " m");
			}
		}
	} break;
	}
	if (_scaleBar.valid()) {
		_scaleBar->setWidth(pixelWidth);
	}
	return scale;
}

bool ScaleBarHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
	if (view) {
		if (ea.getEventType() == ea.SCROLL || ea.getEventType() == ea.KEYDOWN) {
			scaleBar_->computeScale();
			return false;
		}
	}
	return false;
}
