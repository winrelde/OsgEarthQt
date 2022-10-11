#pragma once
#include <osgViewer/Viewer>
#include <osgEarthSymbology/Style>
#include <osgEarthAnnotation/AnnotationUtils>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthFeatures/FeatureIndex>
#include <osgEarthAnnotation/LabelNode>
#include <iostream>
#include <algorithm> 
#include <vector> 

using namespace osgEarth::Annotation;
using namespace std;

class DistanceCaculator : public osgGA::GUIEventHandler
{
public:
	DistanceCaculator(const SpatialReference* srs, osg::Group* annoGroup, osgEarth::MapNode* mapnode);
	~DistanceCaculator();

protected:
	virtual void slotPicked(osg::Vec3d pos);
	virtual void slotMoving(osg::Vec3d pos);
	virtual void slotRightHandle();
private:
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
	osg::Vec3d getPos(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Vec3d& pos);
	string doubleToString(const double &dbNum)
	{
		char *chCode;
		chCode = new char[20];
		sprintf(chCode, "Distance:%.2lf m", dbNum);
		string strCode(chCode);
		delete[] chCode;
		return strCode;
	}
	double GetDis(osg::Vec3d front, osg::Vec3d current, const osgEarth::SpatialReference* srs);
	double GetLineDis(osg::Vec3Array* vec, const osgEarth::SpatialReference* srs);
	//double getdis(osg::Vec3* point1, osg::Vec3* point2);
	osg::Vec3d IntersectPoint(osg::Vec3 XPosition, osg::Vec3 YPosition, osg::ref_ptr<osg::Node> Node);
	double getdis(osg::Vec3 from, osg::Vec3 to);
private:
	Style _lineStyle;
	Feature* _pFeature;
	FeatureNode* _pFeatureNode;

	Style _stippleLineStyle;
	Feature* _pStippleFeature;
	FeatureNode* _pStippleFeatureNode;

	Style _labelStyle;
	std::vector<osg::Vec3d> _vecPoint;
	std::vector<LabelNode*> _vecLabel;
	const SpatialReference* _srs;
	osg::Group* _annoGroup;

	osgEarth::MapNode* _mapnode;

};

