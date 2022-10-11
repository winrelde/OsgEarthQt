#pragma once
#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/FeatureNode>
#include <osgEarthUtil/LinearLineOfSight>

// DrawLineThread 后面 由于重写成了回调形式，更名为 DrawLineCallback
class DrawLineCallback : public osg::Callback
{
public:

	DrawLineCallback(osg::Vec3d start, double angle, double radius, double numSpokes, double numSegment, float losHeight, osgEarth::MapNode* mapNode);
	virtual ~DrawLineCallback();
	inline osg::Group* get() { return m_group; }
	inline void setStart(osg::Vec3d start);
	virtual bool run(osg::Object* object, osg::Object* data);
	void clear();

private:

	osg::ref_ptr<osgEarth::MapNode>								m_mapNode;
	const osgEarth::SpatialReference*							m_spatRef;
	osg::ref_ptr<osg::Group>									m_group;
	//osgEarth::Symbology::LineString*							m_pLs;
	std::vector<osgEarth::Symbology::LineString*>				m_vLs; 
	osg::ref_ptr<osgEarth::Util::LinearLineOfSightNode>			m_LosNode; 
	osgEarth::Annotation::Style									m_lineStyle;

	osg::Vec3d	m_start;
	osg::Vec4	m_goodColor; 
	osg::Vec4	m_badColor;

	bool			**m_bLosArry;
	int				m_NodeCount;
	double			m_delta;
	double			m_lon;
	double			m_lat;
	double			m_angle;
	float			m_losHeight; 
	double			m_numSpokes; 
	double			m_radius;
	double			m_tempDis;
	unsigned int	m_numSegment;
};
