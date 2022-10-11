#pragma once

#include "DrawLineCallback.h"

#include <osgGA/GUIEventHandler>
#include <osgEarth/MapNode>
#include <osgEarth/GLUtils>

#include <osgViewer/Viewer>

#include <osgEarthUtil/LinearLineOfSight>
#include <osgEarthUtil/RadialLineOfSight>
#include <osgEarthAnnotation/CircleNode>
#include <osgEarthUtil/TerrainProfile>
#include <osgEarthAnnotation/PlaceNode>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

#include <QtWidgets/QLabel>

//枚举事件类型
enum EnumActionEvent
{
	ActionNull = 0,
	VisibilityAnalysis,//通视
	ViewshedAnalysis,//视域
	RadarAnalysis,//雷达
	WaterAnalysis,//水域
	TerrainProfileAnalysis,//剖面
	FeaturesQuery,//要素查询
	Fire,//火焰
	Distance,//距离测量
	Area,//面积测量
	slopeAnalysis,//坡度分析
	AspectAnalysis,//坡向分析
	AddPoint,//点标注
	AddLine,//线标注
	AddPolygon,//多边形标注
	AddCircle//圆标注

};

class PickEvent : public osgGA::GUIEventHandler
{
public:

	PickEvent(QLabel* label, osgEarth::MapNode* MapNode, osg::Group* losGroup);
	~PickEvent() {};
	virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) override;
	void setActionEvent(const EnumActionEvent &ae);
	void setLosHeight(float height);
	void setViewshedPara(int numSpokes, int numSegment);
	void RemoveAnalysis();
	void drawWater(osg::Vec3d pos);

protected:

	virtual void pickLeft(osg::Vec3d Point);
	virtual void pickMove(osg::Vec3d Point);

	osg::Vec3d Screen2Geo(float x, float y);//视图转为地理
private:

	DrawLineCallback*							m_pLT;

	std::vector<DrawLineCallback*>				m_vLT;

	osg::ref_ptr<osgEarth::MapNode>				m_mapNode;
	osg::ref_ptr<osgViewer::Viewer>				m_viewer;
	osg::ref_ptr<osg::Group>					m_losGroup;
	osg::ref_ptr<osg::Group>					m_Group;
	const osgEarth::SpatialReference*			m_spatRef;

	osg::ref_ptr<osgEarth::Util::LinearLineOfSightNode>			m_curLosNode;

	osgEarth::Symbology::Style									m_circleStyle;
	osgEarth::Symbology::Style									m_circleOutLineStyle;
	osg::ref_ptr<osgEarth::Annotation::CircleNode>				m_curCircleNode;
	osg::ref_ptr<osgEarth::Annotation::CircleNode>				m_curCircleOutLine;
	
	
	osg::ref_ptr<osgEarth::Annotation::FeatureNode>				m_featureNode;
	osg::ref_ptr<osgEarth::Features::Feature>					m_feature;

	osg::Vec3d	FirstPoint;
	osg::Vec3d	LastPoint;

	bool	m_bFirstClick;
	bool	m_bLastPoint;
	float	m_ui_losHeight;
	int		m_ui_numSpokes;
	int		m_ui_numSegment;
	float	m_last_mouseX , m_last_mouseY;

	QLabel*			m_Label;
	QString			m_mapName;
	QString			m_csysTitle;
	EnumActionEvent m_ActionEvent;

	osg::ref_ptr<osg::Vec3dArray> m_vecPoints;
	osg::Geode* m_pWater;
	char* m_waterFrag;
	char* m_waterVert;
	//osg::Vec3d pos;

};
double GetDistance(double ele, double dir);

osg::Texture2D* creatText2D(const QString& strFile);


