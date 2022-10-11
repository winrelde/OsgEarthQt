#pragma once

#ifndef SCALEBAR_H
#define SCALEBAR_H 1

#include <osgEarthUtil/Controls>
#include <osgEarth/MapNode>
#include <osgEarth/Map>

enum ScaleBarUnits {
	UNITS_METERS,
	UNITS_INTL_FEET,
	UNITS_US_SURVEY_FEET,
	UNITS_NAUTICAL_MILES
};

// ScaleBar
class ScaleBar : public osg::Referenced {
public:
	static double normalizeScaleMeters(double meters);
	static double normalizeScaleFeet(double feet);
	static double normalizeScaleNauticalMiles(double nmi);

	ScaleBar(osgEarth::MapNode* mapNode, osgViewer::View* view);

	void setVisible(bool visible);
	double computeScale();

	osg::ref_ptr<osgEarth::Util::Controls::LabelControl> _scaleLabel;
	osg::ref_ptr<osgEarth::Util::Controls::Frame> _scaleBar;
	osg::ref_ptr<osgEarth::MapNode> _mapNode;
	osg::ref_ptr<osgEarth::Map> _map;
	osg::ref_ptr<osgViewer::View> _view;
	int _windowWidth, _windowHeight;
	double _mapScale;
	ScaleBarUnits _scaleBarUnits;
};

// ScaleBarHandler
struct ScaleBarHandler : public osgGA::GUIEventHandler {
	ScaleBarHandler(ScaleBar* scaleBar) : scaleBar_(scaleBar) { }
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
	osg::ref_ptr<ScaleBar> scaleBar_;
};

#endif

