#pragma once
#ifndef DRAWPOLYGON_H
#define DRAWPOLYGON_H 1

#include <osgEarthAnnotation/FeatureEditing>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthSymbology/Style>
#include "DrawTool.h"

class DrawPolygonTool : public DrawTool {
public:
	DrawPolygonTool(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

	virtual void beginDraw(const osg::Vec3d& lla);
	virtual void moveDraw(const osg::Vec3d& lla);
	virtual void endDraw(const osg::Vec3d& lla);
	virtual void resetDraw();

private:
	std::vector<osg::Vec3d> _vecPoints;
	osgEarth::Symbology::Style _polygonStyle;
	osgEarth::Annotation::FeatureNode* _featureNode;
	osgEarth::Annotation::FeatureEditor* _polygonEdit;
	osgEarth::Symbology::Style _stippleLineStyle;
	osgEarth::Annotation::FeatureNode* _stippleFeatureNode;
};

#endif
