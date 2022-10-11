#pragma once
#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/FeatureNode>

// 鼠标事件中，圆形选框 【已废弃】
class CReferenceArea
{
public:

	CReferenceArea(osgEarth::MapNode* MapNode);
	virtual ~CReferenceArea() {}

	inline osg::Group* get() { return m_CircleNode.get(); }
	inline void setNumSpokes(double numSpokes) { m_numSpokes = numSpokes; m_start_delta = osg::PI * 2.0 / m_numSpokes; }
	inline osgEarth::Symbology::Geometry* getGeometry() { return m_CircleFeature->getGeometry(); }
	void setStart(osg::Vec3d point);
	void setRadius(double radius);
	void init();
	void clear();

private:

	osg::ref_ptr<osgEarth::Annotation::FeatureNode>		m_CircleNode;
	osg::ref_ptr<osgEarth::Features::Feature>			m_CircleFeature;

	osg::ref_ptr<osgEarth::MapNode>						m_mapNode;
	const osgEarth::SpatialReference*					m_spatRef;

	osg::Vec3d	m_start;
	osg::Vec3d	m_end;
	double		m_radius;
	double		m_numSpokes;

	double		m_start_lat;
	double		m_start_lon;
	double		m_start_delta;
};

