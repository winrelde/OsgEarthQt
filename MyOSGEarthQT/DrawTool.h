#pragma once

#ifndef DRAWTOOL_H
#define DRAWTOOL_H 1

#include <osgEarth/MapNode>
#include <osgGA/GUIEventHandler>
#include <osgViewer/View>
#include <osgEarthSymbology/Style>
#include <osgEarthAnnotation/PlaceNode>

class DrawTool : public osgGA::GUIEventHandler {
public:

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	// 绘制Gorup
	osg::Group* getDrawGroup() { return _drawGroup; }
	osgEarth::MapNode* getMapNode() { return _mapNode; }

	void setMapNode(osgEarth::MapNode* mapNode) { _mapNode = mapNode; }

	virtual void beginDraw(const osg::Vec3d& lla) = 0;
	virtual void moveDraw(const osg::Vec3d& lla) = 0;
	virtual void endDraw(const osg::Vec3d& lla) = 0;
	virtual void resetDraw() = 0;

	void drawCommand(osg::Node* node);
	void drawCommand(const osg::NodeList& nodes);

public:
	// 获取点所在地理坐标
	bool getLocationAt(osgViewer::View* view, double x, double y, double& lon, double& lat, double& alt);

protected:
	DrawTool(osgEarth::MapNode* mapNode, osg::Group* drawGroup);

	bool _active;
	bool _dbClick;
	osgViewer::View* _view;
	osgEarth::MapNode* _mapNode;
	osg::Node::NodeMask _intersectionMask;
	osg::Group* _drawGroup;
	float _mouseDownX, _mouseDownY;
	osg::ref_ptr<osg::Group> _tmpGroup; // 临时绘制节点
	osgEarth::Symbology::Style _pnStyle;
	std::vector<osg::Vec2> _controlPoints;
	std::vector<osg::Vec2> _drawParts;
	osg::ref_ptr<osgEarth::Annotation::PlaceNode> _coordPn;
};

#endif
