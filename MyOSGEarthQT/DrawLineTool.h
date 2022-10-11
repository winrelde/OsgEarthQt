#pragma once
#ifndef DRAWLINETOOL_H
#define DRAWLINETOOL_H 1

#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthFeatures/Feature>
#include <osgEarthSymbology/Style>
#include "DrawTool.h"

class DrawLineTool : public DrawTool {
public:
	DrawLineTool(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

	virtual void beginDraw(const osg::Vec3d& lla);
	virtual void moveDraw(const osg::Vec3d& lla);
	virtual void endDraw(const osg::Vec3d& lla);
	virtual void resetDraw();

private:
	osgEarth::Symbology::Style _lineStyle;
	osg::ref_ptr<osgEarth::Features::Feature> _feature;
	osg::ref_ptr<osgEarth::Annotation::FeatureNode> _featureNode;
	osgEarth::Symbology::Style _stippleLineStyle;
	osg::ref_ptr<osgEarth::Features::Feature> _stippleFeature;
	osg::ref_ptr<osgEarth::Annotation::FeatureNode> _stippleFeatureNode;
	std::vector<osg::Vec3d> _vecPoint;
};

#endif
