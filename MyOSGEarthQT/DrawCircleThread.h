#pragma once
#include "DrawLineCallback.h"//���û����߶εĻص�����

// ���������� OpenThread ʵ�ֵĻ�Ȧ��������ʱ���� �Ա���ʱ֮��
class DrawCircleThread : public OpenThreads::Thread
{
public:

	DrawCircleThread(osg::Vec3d start, double radius, double numSpokes, float losHeight, osg::Group* losGroup, osgEarth::MapNode* mapNode);
	~DrawCircleThread();
	inline osg::Group* get() { return m_group; }
	inline void setStart(osg::Vec3d start) { m_start = start; }
	inline void setLT(DrawLineCallback* plt) { m_pLT = plt; }
	void creatNode();
	virtual void run();
	void clear();

private:

	osg::ref_ptr<osgEarth::MapNode>		m_mapNode;
	osg::ref_ptr<osg::Group>			m_group;
	osg::ref_ptr<osg::Group>			m_losGroup;

	const osgEarth::SpatialReference*	m_spatRef;
	DrawLineCallback*					m_pLT;

	osg::Vec3d	m_start;
	osg::Vec4	m_goodColor;
	osg::Vec4	m_badColor;

	float		m_losHeight;
	double		m_numSpokes;
	double		m_radius;
};