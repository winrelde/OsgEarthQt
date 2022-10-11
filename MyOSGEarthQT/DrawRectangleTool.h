#pragma once
#ifndef DRAW_RECTANGLE_TOOL_H
#define DRAW_RECTANGLE_TOOL_H 1

#include <osgEarthAnnotation/RectangleNode>
#include "DrawTool.h"

// 矩形绘制工具
class DrawRectangleTool : public DrawTool {
public:
	DrawRectangleTool(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

	virtual void beginDraw(const osg::Vec3d& v);
	virtual void moveDraw(const osg::Vec3d& v);
	virtual void endDraw(const osg::Vec3d& v);
	virtual void resetDraw();

	// 判断点在矩形所在角位置
	osgEarth::Annotation::RectangleNode::Corner getPointCorner(const osgEarth::GeoPoint& corner);

private:
	std::vector<osg::Vec3d> _vecPoints;
	osgEarth::Symbology::Style _rectangleStyle;
	osgEarth::Annotation::RectangleNode* _rectangleNode;
};

#endif
