#include <Windows.h>
#include "Common.h"

#include "CReferenceArea.h"

#include <osg/Depth>
#include <osgEarth/GLUtils>

//���캯��
CReferenceArea::CReferenceArea(osgEarth::MapNode* mapNode):
	m_mapNode(mapNode),//��ʼ��
	m_spatRef(mapNode->getMapSRS()),
	m_numSpokes(0.0),
	m_start_lat(0.0),
	m_start_lon(0.0),
	m_start_delta(0.0),
	m_radius(0.0)
{
	//�����������
	m_CircleFeature = new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(), m_spatRef);
	m_CircleFeature->geoInterp() = osgEarth::Features::GEOINTERP_GREAT_CIRCLE;//�����ֵ����ΪԲ��

	//����λ�����Ե�����
	osgEarth::Symbology::AltitudeSymbol* _alt_Circle = m_CircleFeature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
	_alt_Circle->clamping() = _alt_Circle->CLAMP_TO_TERRAIN;//�н�����
	_alt_Circle->technique() = _alt_Circle->TECHNIQUE_DRAPE;//ʹ��ͶӰ�����ڻ���ʱ�н�������

	//������Ⱦ���Ե�����
	osgEarth::Symbology::RenderSymbol* _render_Circle = m_CircleFeature->style()->getOrCreate<osgEarth::Symbology::RenderSymbol>();
	_render_Circle->depthOffset()->enabled() = true;//�������ƫ��
	_render_Circle->depthOffset()->automatic() = true;//�Զ�������С��Χ

	//������ô�������Լ����������
	osgEarth::Symbology::LineSymbol* _ls_Circle = m_CircleFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
	_ls_Circle->stroke()->color() = osgEarth::Color(osgEarth::Color::Yellow, 1.0f);//�ߵ���ɫ��Ĭ��Ϊ��ɫ
	_ls_Circle->stroke()->width() = 1.0f;

	m_CircleNode = new osgEarth::Annotation::FeatureNode(m_CircleFeature.get());//��m_CircleFeature�и��������ڵ�
	osgEarth::GLUtils::setLighting(m_CircleNode->getOrCreateStateSet(), osg::StateAttribute::OFF);//��������
}

void CReferenceArea::init()
{
	if (m_numSpokes == 0.0)
	{
		// automatically calculate
		const osgEarth::Distance _radius = m_radius;
		double segLen = _radius.as(osgEarth::Units::METERS) / 8.0;
		double circumference = 2 * osg::PI* _radius.as(osgEarth::Units::METERS);
		m_numSpokes = (unsigned)::ceil(circumference / segLen);
		m_start_delta = osg::PI * 2.0 / m_numSpokes;
	}

	double _angle = 0.0;
	double _clat = 0.0, _clon = 0.0;

	clear();
	for (int i = (unsigned int)m_numSpokes - 1; i >= 0; --i)
	{
		_angle = m_start_delta * (double)i;

		//�ڸ�����㣬��λ�Ǻ;��������¼����յ�
		//��㾭γ�ȣ���λ�ǣ����룬�յ�ľ��ȣ��Ի���Ϊ��λ��
		osgEarth::GeoMath::destination(m_start_lat, m_start_lon, _angle, m_radius, _clat, _clon, m_spatRef->getEllipsoid()->getRadiusEquator());
		m_CircleFeature->getGeometry()->push_back(osg::Vec3d(osg::RadiansToDegrees(_clon), osg::RadiansToDegrees(_clat), 1.0));
	}
	m_CircleFeature->getGeometry()->push_back(m_CircleFeature->getGeometry()->front());
	m_CircleNode->dirty();//
}
void CReferenceArea::setStart(osg::Vec3d start)
{
	//�õ���ʼ���x��y����
	m_start = start;
	m_start_lat = osg::DegreesToRadians(m_start.y());
	m_start_lon = osg::DegreesToRadians(m_start.x());
	clear();
}

void CReferenceArea::setRadius(double radius)
{
	m_radius = radius;
	init();
}

void CReferenceArea::clear()
{
	//�������
	m_CircleFeature->getGeometry()->clear();
	m_CircleNode->dirty();
}
