
#include <Windows.h>
#include "Common.h"

#include "DrawLineCallback.h"
#include <osgEarth/GLUtils>

DrawLineCallback::DrawLineCallback(osg::Vec3d start, double angle, double radius, double numSpokes, double numSegment, float losHeight, osgEarth::MapNode* mapNode) :
	m_angle(angle),
	m_radius(radius),
	m_numSpokes(numSpokes),//辐条数
	m_losHeight(losHeight),//视高
	m_mapNode(mapNode),
	m_spatRef(mapNode->getMapSRS()),
	m_numSegment(numSegment),//分割数
	m_goodColor(0.0f, 1.0f, 0.0f, 1.0f),//green
	m_badColor(1.0f, 0.0f, 0.0f, 1.0f)//red
{
	// 基于分段数初始小段距离
	m_tempDis = m_radius / (double)m_numSegment;
	m_NodeCount = 0;//已有的节点数

	//m_pLs = new osgEarth::Symbology::LineString[m_numSpokes]; // 这样写在 GCC 下无法编译过
	for (int i = 0; i < m_numSpokes; i++)
	{
		m_vLs.push_back(new osgEarth::Symbology::LineString());//生成若干辐条
	}

	m_lineStyle.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->width() = 1.0;
	m_lineStyle.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
	m_lineStyle.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_DRAPE;
	m_lineStyle.getOrCreate<osgEarth::Symbology::RenderSymbol>()->depthOffset()->automatic() = true;

	// 构造视线
	m_LosNode = new osgEarth::Util::LinearLineOfSightNode(m_mapNode);//获取此LineOfSightNode对其进行操作的MapNode
	m_LosNode->setTerrainOnly(true);//只与地形影响
	setStart(start);

	// 初始化视线 bool 二维数组
	//m_bLosArry = nullptr; 这样写在 GCC 下无法编译过
	//m_bLosArry = new bool*[m_numSpokes];
	//for (int i = 0; i < m_numSpokes; i++)
	//{
	//	m_bLosArry[i] = new bool[m_numSegment];
	//}

	m_bLosArry = (bool **)malloc(sizeof(bool*) * m_numSpokes);    //分配指针数组
	m_bLosArry[0] = (bool *)malloc(sizeof(bool) * m_numSpokes * m_numSegment);//一次性分配所有空间
	for (int i = 1; i < m_numSpokes; i++)
	{
		m_bLosArry[i] = m_bLosArry[i - 1] + m_numSegment;

	}//采用如上内存分配方法，意味着将 m_bLosArry 的值初始化为 m_numSpokes*m_numSegment 的二维数组首地址,且这块内存连续

	// 计算每次旋转的弧度
	m_delta = osg::PI * 2.0 / m_numSpokes;

	// FeatureNode 添加的总group
	m_group = new osg::Group();
	osgEarth::GLUtils::setLighting(m_group->getOrCreateStateSet(), osg::StateAttribute::OFF);
	m_group->setUpdateCallback(this);//设置自己的更新回调
}

DrawLineCallback::~DrawLineCallback()
{
	if (m_group)
	{
		//释放组节点
		m_group.release();
		m_group = nullptr;
	}

	if (m_LosNode)
	{
		//释放视线节点
		m_LosNode.release();
		m_LosNode = nullptr;
	}

	for (std::vector<osgEarth::Symbology::LineString*>::iterator it = m_vLs.begin(); it != m_vLs.end(); it++)
	{
		//释放视线
		if (nullptr != *it)
		{
			delete *it;
			*it = nullptr;
		}
	}
	m_vLs.clear();

	if (m_bLosArry)
	{
		free(m_bLosArry[0]);
		free(m_bLosArry);
		m_bLosArry = nullptr;
	}
}

void DrawLineCallback::setStart(osg::Vec3d start)
{
	m_start = start;
	m_lat = osg::DegreesToRadians(m_start.y()); //得到x，y，后面还是用的初始方式表示x，y，为什么还要用这个呢？是为了转为弧度。
	m_lon = osg::DegreesToRadians(m_start.x());

	auto _start = osgEarth::GeoPoint(m_spatRef->getGeographicSRS(), m_start.x(), m_start.y(), m_losHeight, osgEarth::AltitudeMode::ALTMODE_RELATIVE);
	m_LosNode->setStart(_start);
}

//重写run()函数
bool DrawLineCallback::run(osg::Object* object, osg::Object* data)
{

	osg::Group* _group = dynamic_cast<osg::Group*>(object);

	double _clat = 0.0, _clon = 0.0;
	double _angle = -m_delta * (double)m_NodeCount + m_angle;//旋转的角度
	for (unsigned int i = 0; i < m_numSegment; i++)
	{
		//可以说是计算终点
		osgEarth::GeoMath::destination(m_lat, m_lon, _angle, m_tempDis * i, _clat, _clon);

		//得到终点
		osgEarth::GeoPoint _end(m_spatRef->getGeographicSRS(), osg::RadiansToDegrees(_clon), osg::RadiansToDegrees(_clat), 1.0, osgEarth::AltitudeMode::ALTMODE_RELATIVE);

		m_vLs[m_NodeCount]->push_back(_end.vec3d());//加入到m_vLs

		m_LosNode->setEnd(_end);
		m_bLosArry[m_NodeCount][i] = m_LosNode->getHasLOS();
	}

	//m_bLosArry存放视线是否通视
	bool _curLos = m_bLosArry[m_NodeCount][0];
	osg::Vec3d _lastPoint(m_start.x(), m_start.y(), m_start.z());

	for (unsigned int i = 0; i < m_numSegment; i++)
	{
		if (_curLos != m_bLosArry[m_NodeCount][i])
		{
			osg::ref_ptr<osgEarth::Symbology::LineString> _ls = new osgEarth::Symbology::LineString();
			osg::ref_ptr<osgEarth::Features::Feature> _feature = new osgEarth::Features::Feature(_ls, m_spatRef);
			osg::ref_ptr<osgEarth::Annotation::FeatureNode> _featureNode = new osgEarth::Annotation::FeatureNode(_feature.get());
			//_curLos为true就是绿色
			m_lineStyle.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = _curLos ? osgEarth::Symbology::Color::Green : osgEarth::Symbology::Color::Red;
			_featureNode->setStyle(m_lineStyle);
			_feature->getGeometry()->push_back(_lastPoint);
			_feature->getGeometry()->push_back(m_vLs[m_NodeCount]->at(i));
			m_group->addChild(_featureNode.get());

			_lastPoint = m_vLs[m_NodeCount]->at(i);
			_curLos = m_bLosArry[m_NodeCount][i];
		}
	}

	osg::ref_ptr<osgEarth::Symbology::LineString> _ls = new osgEarth::Symbology::LineString();
	osg::ref_ptr<osgEarth::Features::Feature> _feature = new osgEarth::Features::Feature(_ls, m_spatRef);
	osg::ref_ptr<osgEarth::Annotation::FeatureNode> _featureNode = new osgEarth::Annotation::FeatureNode(_feature.get());
	m_lineStyle.getOrCreate<osgEarth::Symbology::LineSymbol>()->stroke()->color() = _curLos ? osgEarth::Symbology::Color::Green : osgEarth::Symbology::Color::Red;
	_featureNode->setStyle(m_lineStyle);
	_feature->getGeometry()->push_back(_lastPoint);
	_feature->getGeometry()->push_back(m_vLs[m_NodeCount]->at(m_numSegment - 1));
	m_group->addChild(_featureNode.get());

	m_NodeCount++;
	if (m_NodeCount == m_numSpokes)
	{
		_group->removeUpdateCallback(this);//结束
		return false;
	}
	return traverse(object, data);
}

void DrawLineCallback::clear()
{
	//从组节点中清除辐条
	osgEarth::Annotation::FeatureNode* _fNode;
	for (unsigned int i = 0; i < m_group->getNumChildren(); i++)
	{
		_fNode = dynamic_cast<osgEarth::Annotation::FeatureNode*>(m_group->getChild(i));
		_fNode->getFeature()->getGeometry()->clear();
		_fNode->dirty();
	}

	m_group->removeChild(0, m_group->getNumChildren());
}