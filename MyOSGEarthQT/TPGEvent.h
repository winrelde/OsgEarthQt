#pragma once
#include "TerrainProfileGraph.h"

class TerrainProfileEventHandler : public osgGA::GUIEventHandler
{
public:
	TerrainProfileEventHandler(osgEarth::MapNode* mapNode, osg::Group* root, osgEarth::Util::TerrainProfileCalculator* profileCalculator);

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
	
	//计算
	void compute();
	
	void Remove();


private:
	osgEarth::MapNode* _mapNode;
	osg::Group* _root;
	osgEarth::Util::TerrainProfileCalculator* _profileCalculator;
	osg::ref_ptr< FeatureNode > _featureNode;
	bool _startValid;
	osg::Vec3d _start;
	osg::Vec3d _end;
};

osg::Camera* createHud(double width, double height);