 #include <Windows.h>
#include "Common.h"

#include "DrawCircleThread.h"
#include <osgEarth/GLUtils>

DrawCircleThread::DrawCircleThread(osg::Vec3d start, double radius, double numSpokes, float losHeight, osg::Group* losGroup, osgEarth::MapNode* mapNode):
	m_start(start),
	m_radius(radius),
	m_numSpokes(numSpokes),
	m_losHeight(losHeight),
	m_losGroup(losGroup),
	m_mapNode(mapNode),
	m_spatRef(mapNode->getMapSRS()),
	m_goodColor(0.0f, 1.0f, 0.0f, 1.0f),
	m_badColor(1.0f, 0.0f, 0.0f, 1.0f)
{
	m_group = new osg::Group();
	creatNode();
}

DrawCircleThread::~DrawCircleThread()
{
	if (m_group)
	{
		m_group = nullptr;
	}
}

void DrawCircleThread::creatNode()
{
	for (int i = 0; i < (int)m_numSpokes; i++)
	{
		osg::ref_ptr<osgEarth::Features::Feature> _feature = new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(), m_spatRef);
		_feature->geoInterp() = osgEarth::Features::GEOINTERP_GREAT_CIRCLE;

		osgEarth::Symbology::AltitudeSymbol* alt = _feature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
		alt->clamping() = alt->CLAMP_TO_TERRAIN;
		alt->technique() = alt->TECHNIQUE_GPU;

		osgEarth::Symbology::RenderSymbol* render = _feature->style()->getOrCreate<osgEarth::Symbology::RenderSymbol>();
		render->depthOffset()->enabled() = true;
		render->depthOffset()->automatic() = true;

		osgEarth::Symbology::LineSymbol* ls = _feature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
		ls->stroke()->color() = osgEarth::Color(osgEarth::Color::Yellow, 0.2f);
		ls->stroke()->width() = 2.0f;

		osg::ref_ptr<osgEarth::Annotation::FeatureNode> _featureNode = new osgEarth::Annotation::FeatureNode(_feature.get());
		osgEarth::GLUtils::setLighting(_featureNode->getOrCreateStateSet(), osg::StateAttribute::OFF);
		m_group->addChild(_featureNode.get());
	}
}

void DrawCircleThread::run()
{
	unsigned int _num = m_group->getNumChildren();
	osgEarth::Annotation::FeatureNode* _fNode;

	double _earthRadius = m_spatRef->getEllipsoid()->getRadiusEquator();
	double _lat = osg::DegreesToRadians(m_start.y());
	double _lon = osg::DegreesToRadians(m_start.x());
	double _tempDis = m_radius / (double)_num;

	double _delta = osg::PI * 2.0 / _num;

	
	for (unsigned int i = 0; i < _num; i++)
	{
		double _clat, _clon;
		_fNode = dynamic_cast<osgEarth::Annotation::FeatureNode*>(m_group->getChild(i));
		for (unsigned int j = 0; j <= m_numSpokes; j++)
		{
			double angle = _delta * (double)j;
			osgEarth::GeoMath::destination(_lat, _lon, angle, _tempDis * (i+1), _clat, _clon, _earthRadius);
			_fNode->getFeature()->getGeometry()->push_back(osg::Vec3d(osg::RadiansToDegrees(_clon), osg::RadiansToDegrees(_clat), 0.0));
		}
		_fNode->dirty();
	}
}

void DrawCircleThread::clear()
{
	osgEarth::Annotation::FeatureNode* _fNode;
	for (unsigned int i = 0; i < m_group->getNumChildren(); i++)
	{
		_fNode = dynamic_cast<osgEarth::Annotation::FeatureNode*>(m_group->getChild(i));
		_fNode->getFeature()->getGeometry()->clear();
		_fNode->dirty();
	}

	m_group->removeChild(0, m_group->getNumChildren());
}