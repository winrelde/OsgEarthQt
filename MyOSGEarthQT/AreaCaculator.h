#pragma once
#include <osgViewer/Viewer>
#include <osgEarthSymbology/Style>
#include <osgEarthAnnotation/AnnotationUtils>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthFeatures/FeatureIndex>
#include <osgEarthAnnotation/LabelNode>
#include <osg/TriangleFunctor>
#include <osgUtil/Tessellator>
#include <osgEarthFeatures/GeometryUtils>

using namespace osgEarth::Annotation;
using namespace std;

class AreaCaculator :
	public osgGA::GUIEventHandler
{
public:
	AreaCaculator(const SpatialReference* srs, osg::Group* annoGroup, osgEarth::MapNode* mapnode);
	~AreaCaculator();
	void enable() { _isEnable = true; }
	void disable() { _isEnable = false; }
	bool isEnabled() { return _isEnable; }

protected:
	virtual void slotPicked(osg::Vec3d pos);
	virtual void slotMoving(osg::Vec3d pos);
	virtual void slotRightHandle();
private:
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
	osg::Vec3d getPos(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Vec3d& pos);
	double caculateArea(const Geometry *geo);
	string toString(const long long &dbNum)
	{
		char *chCode;
		chCode = new char[50];
		sprintf(chCode, "Area:%lld m^2", dbNum);
		string strCode(chCode);
		delete[] chCode;
		return strCode;
	}
	double calcSurfaceArea(const std::vector<osg::Vec3d>& points, osgEarth::MapNode* mapnode);
	osg::Vec3d latlnghigh2xyz(osgEarth::MapNode* mapnode, const osg::Vec3d& llh);
	double caltrianglearea(const osg::Vec3d& a, const osg::Vec3d& b, const osg::Vec3d& c);
	void trianglation(const std::vector<osg::Vec3d>& points, std::vector<std::vector<osg::Vec3d>>& triangles, double minarea);
	double calcArea(const std::vector<osg::Vec3d>& points, osgEarth::MapNode* mapnode);

private:
	Feature* _pFeature;
	FeatureNode* _pFeatureNode;
	std::vector<osg::Vec3d> _vecPoint;
	LabelNode* _label;
	const SpatialReference* _srs;
	osg::Group* _annoGroup;
	osgEarth::MapNode* _mapnode;
	bool _isEnable;
};

