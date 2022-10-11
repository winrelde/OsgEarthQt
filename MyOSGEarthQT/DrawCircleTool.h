#pragma once
#ifndef DRAWCIRCLE_TOOL_H
#define DRAWCIRCLE_TOOL_H 1

#include <osgEarthAnnotation/AnnotationEditing>
#include <osgEarthAnnotation/CircleNode>
#include <osgEarthAnnotation/PlaceNode>
#include <osgEarthSymbology/Style>
#include "DrawTool.h"

class DrawCircleTool : public DrawTool {
public:
	DrawCircleTool(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

	virtual void beginDraw(const osg::Vec3d& lla);
	virtual void moveDraw(const osg::Vec3d& lla);
	virtual void endDraw(const osg::Vec3d& lla);
	virtual void resetDraw();

private:
	osg::Vec3d _centerPoint;
	osgEarth::Symbology::Style _circleStyle;
	osg::ref_ptr<osgEarth::Annotation::CircleNode> _circleNode;
	osg::ref_ptr<osgEarth::Annotation::CircleNodeEditor> _circleEdit;
	osg::ref_ptr<osgEarth::Annotation::PlaceNode> _radiusNode;
};
#endif
