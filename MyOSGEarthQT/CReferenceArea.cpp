#include <Windows.h>
#include "Common.h"

#include "CReferenceArea.h"

#include <osg/Depth>
#include <osgEarth/GLUtils>

//构造函数
CReferenceArea::CReferenceArea(osgEarth::MapNode* mapNode):
	m_mapNode(mapNode),//初始化
	m_spatRef(mapNode->getMapSRS()),
	m_numSpokes(0.0),
	m_start_lat(0.0),
	m_start_lon(0.0),
	m_start_delta(0.0),
	m_radius(0.0)
{
	//鼠标点击的特征
	m_CircleFeature = new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(), m_spatRef);
	m_CircleFeature->geoInterp() = osgEarth::Features::GEOINTERP_GREAT_CIRCLE;//地理插值方法为圆弧

	//包含位置特性的特征
	osgEarth::Symbology::AltitudeSymbol* _alt_Circle = m_CircleFeature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
	_alt_Circle->clamping() = _alt_Circle->CLAMP_TO_TERRAIN;//夹紧地形
	_alt_Circle->technique() = _alt_Circle->TECHNIQUE_DRAPE;//使用投影纹理在绘制时夹紧几何体

	//包含渲染特性的特征
	osgEarth::Symbology::RenderSymbol* _render_Circle = m_CircleFeature->style()->getOrCreate<osgEarth::Symbology::RenderSymbol>();
	_render_Circle->depthOffset()->enabled() = true;//允许深度偏移
	_render_Circle->depthOffset()->automatic() = true;//自动计算最小范围

	//描述怎么绘制线性几何体的特征
	osgEarth::Symbology::LineSymbol* _ls_Circle = m_CircleFeature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
	_ls_Circle->stroke()->color() = osgEarth::Color(osgEarth::Color::Yellow, 1.0f);//线的颜色，默认为黄色
	_ls_Circle->stroke()->width() = 1.0f;

	m_CircleNode = new osgEarth::Annotation::FeatureNode(m_CircleFeature.get());//从m_CircleFeature中复制特征节点
	osgEarth::GLUtils::setLighting(m_CircleNode->getOrCreateStateSet(), osg::StateAttribute::OFF);//光照设置
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

		//在给定起点，方位角和距离的情况下计算终点
		//起点经纬度，方位角，距离，终点的经度（以弧度为单位）
		osgEarth::GeoMath::destination(m_start_lat, m_start_lon, _angle, m_radius, _clat, _clon, m_spatRef->getEllipsoid()->getRadiusEquator());
		m_CircleFeature->getGeometry()->push_back(osg::Vec3d(osg::RadiansToDegrees(_clon), osg::RadiansToDegrees(_clat), 1.0));
	}
	m_CircleFeature->getGeometry()->push_back(m_CircleFeature->getGeometry()->front());
	m_CircleNode->dirty();//
}
void CReferenceArea::setStart(osg::Vec3d start)
{
	//得到开始点的x，y坐标
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
	//清除特征
	m_CircleFeature->getGeometry()->clear();
	m_CircleNode->dirty();
}
