#include <Windows.h>
#include "Common.h"

#include "PickEvent.h"
#include <osg/Depth>

//点击事件
PickEvent::PickEvent(QLabel* label, osgEarth::MapNode* mapNode, osg::Group* losGroup) :
	m_ActionEvent(EnumActionEvent::ActionNull),
	m_Label(label),
	m_mapNode(mapNode),
	m_losGroup(losGroup),
	m_bFirstClick(false),
	m_bLastPoint(false),
	m_ui_losHeight(2.0),//视高
	m_ui_numSpokes(200),
	m_ui_numSegment(200),
	m_mapName(QString::fromUtf8(mapNode->getMapSRS()->getName().c_str())),
	m_csysTitle(QString::fromLocal8Bit("坐标:"))
{
	m_spatRef = m_mapNode->getMapSRS();

	m_Group = new osg::Group();
	m_mapNode->addChild(m_Group);

	// 贴地的 圆 style
	m_circleStyle.getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() = osgEarth::Color(osgEarth::Color::White, 0.1f);
	m_circleStyle.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
	m_circleStyle.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_DRAPE;
	m_circleStyle.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->verticalOffset() = 0.1;//在贴地后几何体的顶点偏移
	m_circleStyle.getOrCreate<osgEarth::Symbology::RenderSymbol>()->order();//应用渲染命令

	// 贴地的 outline style
	m_circleOutLineStyle.getOrCreateSymbol<osgEarth::Symbology::LineSymbol>()->stroke()->color() = osgEarth::Color(osgEarth::Color::Yellow, 1.0f);
	m_circleOutLineStyle.getOrCreateSymbol<osgEarth::Symbology::LineSymbol>()->stroke()->width() = 2.0f;
	m_circleOutLineStyle.getOrCreate<osgEarth::Symbology::PolygonSymbol>()->fill()->color() = osgEarth::Symbology::Color(osgEarth::Color::White, 0.0); // 透明填充
	m_circleOutLineStyle.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->clamping() = osgEarth::Symbology::AltitudeSymbol::CLAMP_TO_TERRAIN;
	m_circleOutLineStyle.getOrCreate<osgEarth::Symbology::AltitudeSymbol>()->technique() = osgEarth::Symbology::AltitudeSymbol::TECHNIQUE_DRAPE;

	// 贴地的线:
	m_feature = new osgEarth::Features::Feature(new osgEarth::Symbology::LineString(), m_spatRef);
	m_feature->geoInterp() = osgEarth::Features::GEOINTERP_GREAT_CIRCLE;

	osgEarth::Symbology::AltitudeSymbol* _alt = m_feature->style()->getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
	_alt->clamping() = _alt->CLAMP_TO_TERRAIN;
	_alt->technique() = _alt->TECHNIQUE_DRAPE;

	osgEarth::Symbology::RenderSymbol* _render = m_feature->style()->getOrCreate<osgEarth::Symbology::RenderSymbol>();
	_render->depthOffset()->enabled() = true;
	_render->depthOffset()->automatic() = true;

	osgEarth::Symbology::LineSymbol* _ls = m_feature->style()->getOrCreate<osgEarth::Symbology::LineSymbol>();
	_ls->stroke()->color() = osgEarth::Color(osgEarth::Color::Yellow, 1.0f);
	_ls->stroke()->width() = 1.0f;
	_ls->tessellation() = 150;

	//特征转节点
	m_featureNode = new osgEarth::Annotation::FeatureNode(m_feature);
	m_Group->addChild(m_featureNode.get());

}

bool PickEvent::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
	//响应事件基础代码
	m_viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
	if (!m_viewer)
	{
		return false;
	}
	
	// 鼠标点击事件
	if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
	{
		if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
		{
			//得到x，y
			m_last_mouseX = ea.getX();
			m_last_mouseY = ea.getY();
		}
		else if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
		{
			if (m_last_mouseX == ea.getX() && m_last_mouseY == ea.getY())
			{
				FirstPoint = Screen2Geo(ea.getX(), ea.getY());
				pickLeft(FirstPoint);//左击
				LastPoint = FirstPoint;//就是将初始点赋值给之后的点，之后的点又作为初始点
			}
		}
	}
	
	// 鼠标滑动事件
	if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE)
	{
		auto MovePoint = Screen2Geo(ea.getX(), ea.getY());
		pickMove(MovePoint);
	}

	return false;
}

void PickEvent::pickLeft(osg::Vec3d Point)
{
	switch (m_ActionEvent)
	{
		// 通视分析
		case EnumActionEvent::VisibilityAnalysis:
		{
			if (m_bFirstClick)
			{
				//auto 自动存储的临时变量
				auto _start = osgEarth::GeoPoint(m_spatRef->getGeographicSRS(), Point.x(), Point.y(), m_ui_losHeight, osgEarth::AltitudeMode::ALTMODE_RELATIVE);
				m_curLosNode = new osgEarth::Util::LinearLineOfSightNode(m_mapNode);//LinearLineOfSightNode 可用于显示点到点视线计算的节点
				//分割模式从一行的开始到第一次命中，画一条颜色好的线，然后从命中到结束，画一条颜色不好的线
				m_curLosNode->setDisplayMode(osgEarth::Util::LineOfSight::DisplayMode::MODE_SPLIT);
				m_curLosNode->setStart(_start);
				m_losGroup->addChild(m_curLosNode);

				osgEarth::Util::LinearLineOfSightEditor* relEditor = new osgEarth::Util::LinearLineOfSightEditor(m_curLosNode);
				m_losGroup->addChild(relEditor);

				m_bFirstClick = false;
			}
			else
			{
				m_bFirstClick = true;
			}
		}break;
		// 视域分析
		case EnumActionEvent::ViewshedAnalysis:
		{
			if (m_bFirstClick)
			{
				m_curCircleNode = new osgEarth::Annotation::CircleNode;
				m_curCircleNode->setStyle(m_circleStyle);
				m_curCircleNode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

				m_curCircleOutLine = new osgEarth::Annotation::CircleNode;
				m_curCircleOutLine->setStyle(m_circleOutLineStyle);
				m_curCircleOutLine->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

				auto _centerPoint = osgEarth::GeoPoint(m_spatRef->getGeographicSRS(), Point.x(), Point.y(), 0, osgEarth::AltitudeMode::ALTMODE_RELATIVE);

				m_curCircleNode->setPosition(_centerPoint);
				m_curCircleOutLine->setPosition(_centerPoint);
				m_losGroup->addChild(m_curCircleNode.get());
				m_losGroup->addChild(m_curCircleOutLine.get());

				m_feature->getGeometry()->clear();
				m_feature->getGeometry()->push_back(osg::Vec3d(Point.x(), Point.y(), 10));

				m_bFirstClick = false;
			}
			else
			{
				if (m_bLastPoint)
				{
					m_feature->getGeometry()->back() = osg::Vec3d(Point.x(), Point.y(), 10);
					m_bLastPoint = false;
				}
				else
				{
					m_feature->getGeometry()->push_back(osg::Vec3d(Point.x(), Point.y(), 10));
				}

				m_feature->getGeometry()->clear();
				m_featureNode->dirty();//重建特征节点

				// Analysis
				auto _start = osgEarth::GeoPoint(m_spatRef->getGeographicSRS(), LastPoint, osgEarth::AltitudeMode::ALTMODE_ABSOLUTE);
				auto _end = osgEarth::GeoPoint(m_spatRef->getGeographicSRS(), Point, osgEarth::AltitudeMode::ALTMODE_ABSOLUTE);
				//方位角
				auto _angle = osgEarth::GeoMath::bearing(osg::DegreesToRadians(_start.y()), osg::DegreesToRadians(_start.x()),
														osg::DegreesToRadians(_end.y()), osg::DegreesToRadians(_end.x()));

				m_pLT = new DrawLineCallback(LastPoint, 
											_angle, 
											osgEarth::GeoMath::distance(LastPoint, Point, m_spatRef), //距离作为半径
											(double)m_ui_numSpokes,
											(double)m_ui_numSegment, 
											m_ui_losHeight, m_mapNode);
				m_losGroup->addChild(m_pLT->get());//加入回调
				m_vLT.push_back(m_pLT);

				m_bFirstClick = true;
			}
		}break;
		// 雷达分析
		// 左键设置起点
		case EnumActionEvent::RadarAnalysis:
		{
				//auto _start = osgEarth::GeoPoint(m_spatRef->getGeographicSRS(), Point.x(), Point.y(), m_ui_losHeight, osgEarth::AltitudeMode::ALTMODE_RELATIVE);
			
			if (m_bFirstClick)
			{
				osgEarth::GeoTransform* xform = new osgEarth::GeoTransform();
				//xform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), 55.306, 32.041, 200));
				xform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), Point.x(), Point.y(), 200));
				osg::Geode* geode = new osg::Geode();
				//设置三维曲面采样步长
				double dStep = 5;
				//俯仰方向循环
				double dEle = -5;
				for (double dEle = -5; dEle < 90; dEle += dStep)
				{
					//三维曲面的半透明蒙皮，通过QUAD_STRIP图元合成
					osg::Geometry* polySkin = new osg::Geometry();
					//计算蒙皮所需要的采样点数量
					int numCoordsSkin = (360 / dStep + 1) * 2;
					//申请蒙皮采样点
					osg::Vec3* myCoordsSkin = new osg::Vec3[numCoordsSkin];
					osg::Vec4Array* colorsSkin = new osg::Vec4Array;
					//各采样点的法线方向，目前还没搞清楚这个法线方向有何作用
					osg::Vec3Array* normals = new osg::Vec3Array;
					normals->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
					//水平方向循环
					int index = 0;
					for (double dDir = 0; dDir <= 360; dDir += dStep)
					{
						double dEleR = osg::DegreesToRadians(dEle);
						double dDirR = osg::DegreesToRadians(dDir);
						double dEleR2 = osg::DegreesToRadians(dEle + dStep);
						//double dPercent1, dPercent2;
						//GetDistance为计算在指定方位角、俯仰角处雷达最大探测距离的核心函数，dPercent为与最大探测距离相对应的颜色色度
						//double dDis = 1000000.0;
						//double dDis2 = 800000.0;
						double dDis = GetDistance(dEleR, dDirR);
						//std::cout << dDis<<std::endl;
						double dDis2 = GetDistance(dEleR2, dDirR);
						//std::cout << dDis2 << std::endl;
						//计算蒙皮的采样点坐标，同时计算对应的显示颜色
						myCoordsSkin[index * 2] = osg::Vec3(dDis*sin(dDirR)*cos(dEleR), dDis*cos(dDirR)*cos(dEleR), dDis*sin(dEleR));
						//colorsSkin=GetColor(dDis);
						//colorsSkin->push_back(osg::Vec4(0.8f, 0.6f, 1.0f, 0.3f));//0.3为透明度，0为完全透明，1为完全不透明
						myCoordsSkin[index * 2 + 1] = osg::Vec3(dDis2*sin(dDirR)*cos(dEleR2), dDis2*cos(dDirR)*cos(dEleR2), dDis2*sin(dEleR2));
						//colorsSkin=GetColor(dDis2);
						colorsSkin->push_back(osg::Vec4(0.6f, 0.5f, 0.4f, 0.5f));
						if (index < 360 / dStep)
						{
							//三维曲面的经络骨骼，通过LINE_LOOP图元进行合成
							osg::Geometry* polyBone = new osg::Geometry();
							osg::Vec4Array* colorsBone = new osg::Vec4Array;
							//与蒙皮不同，由LINE_LOOP的特点决定，经络骨骼的图像无法一笔画，必须一个一个拼凑，因此顶点数量为4
							int numCoordsBone = 4;
							osg::Vec3* myCoordsBone = new osg::Vec3[numCoordsBone];
							double dDirR2 = osg::DegreesToRadians(dDir + dStep);
							//double dPercent3, dPercent4;
							//double dDis3 = 2000000.0;
							//double dDis4 = 1500000.0;
							double dDis3 = GetDistance(dEleR2, dDirR2);
							double dDis4 = GetDistance(dEleR, dDirR2);
							myCoordsBone[0] = osg::Vec3(dDis*sin(dDirR)*cos(dEleR), dDis*cos(dDirR)*cos(dEleR), dDis*sin(dEleR));
							//colorsBone = GetColor(dDis);
							colorsBone->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 0.5f));
							myCoordsBone[1] = osg::Vec3(dDis2*sin(dDirR)*cos(dEleR2), dDis2*cos(dDirR)*cos(dEleR2), dDis2*sin(dEleR2));
							//colorsBone = GetColor(dDis2);
							colorsBone->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 0.5f));
							myCoordsBone[2] = osg::Vec3(dDis3*sin(dDirR2)*cos(dEleR2), dDis3*cos(dDirR2)*cos(dEleR2), dDis3*sin(dEleR2));
							//colorsBone = GetColor(dDis3);
							colorsBone->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 0.5f));
							myCoordsBone[3] = osg::Vec3(dDis4*sin(dDirR2)*cos(dEleR), dDis4*cos(dDirR2)*cos(dEleR), dDis4*sin(dEleR));
							//colorsBone = GetColor(dDis4);
							colorsBone->push_back(osg::Vec4(1.0f, 1.0f, 0.8f, 0.5f));
							//设置曲线的图元类型为LINE_LOOP
							polyBone->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, numCoordsBone));
							osg::Vec3Array* verticesBone = new osg::Vec3Array(numCoordsBone, myCoordsBone);

							polyBone->setVertexArray(verticesBone);
							//设置曲线的颜色为逐点设置
							polyBone->setColorArray(colorsBone, osg::Array::BIND_PER_VERTEX);
							polyBone->setNormalArray(normals, osg::Array::BIND_OVERALL);

							geode->addDrawable(polyBone);
						}
						index++;
					}
					osg::Vec3Array* verticesSkin = new osg::Vec3Array(numCoordsSkin, myCoordsSkin);
					polySkin->setVertexArray(verticesSkin);
					//设置曲面的颜色为逐点设置
					polySkin->setColorArray(colorsSkin, osg::Array::BIND_PER_VERTEX);
					polySkin->setNormalArray(normals, osg::Array::BIND_OVERALL);
					//设置曲面的图元类型为QUAD_STRIP
					polySkin->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUAD_STRIP, 0, numCoordsSkin));
					//设置曲面半透明效果的一条关键语句，少了该句会导致半透明显示错误，暂未搞清楚该句的原理
					polySkin->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
					//开启曲面的半透明效果
					polySkin->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
					polySkin->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
					geode->addDrawable(polySkin);
				}

				xform->addChild(geode);
				//m_world->addChild(geode);
				m_losGroup->addChild(xform);

				m_bFirstClick = false;
			}
			else
			{
				m_bFirstClick = true;
			}
				
		}break;

		//水域分析

		//case EnumActionEvent::WaterAnalysis:
		//{
		//	if (m_bFirstClick)
		//	{
		//		m_vecPoints->push_back(Point);
		//		if (m_vecPoints->size() <= 2)
		//		{
		//			return;
		//		}
		//		drawWater(Point);
		//		m_Group = new osg::Group;
		//		m_Group->addChild(m_pWater);
		//		m_bFirstClick = false;
		//	}
		//	else
		//	{
		//		m_bFirstClick = true;
		//	}
		//}break;

		//火焰
		case EnumActionEvent::Fire:
		{
			if (m_bFirstClick)
			{

				osgEarth::GeoTransform* xform = new osgEarth::GeoTransform();
				
				xform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), Point.x(), Point.y(), 0));

				osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(Point, 10.0f);
				osgParticle::ExplosionDebrisEffect* explosionDebri = new osgParticle::ExplosionDebrisEffect(Point, 10.0f);
				osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect(Point, 10.0f);
				osgParticle::FireEffect* fire = new osgParticle::FireEffect(Point, 10.0f);

				osg::Vec3 wind(10.0f, 0.0f, 30.0f);
				//设置风向
				explosion->setWind(wind);
				explosionDebri->setWind(wind);
				smoke->setWind(wind);
				fire->setWind(wind);

				fire->setUseLocalParticleSystem(false);

				osg::ref_ptr<osg::Geode> geode = new osg::Geode;
				geode->addDrawable(fire->getParticleSystem());
				geode->addDrawable(explosion->getParticleSystem());
				geode->addDrawable(explosionDebri->getParticleSystem());
				geode->addDrawable(smoke->getParticleSystem());

				xform->addChild(explosion);
				xform->addChild(explosionDebri);
				xform->addChild(smoke);
				xform->addChild(fire);

				xform->addChild(geode);

				m_losGroup->addChild(xform);

				m_bFirstClick = false;
			}
			else
			{
				m_bFirstClick = true;
			}
		}break;

		//点标注
		case EnumActionEvent::AddPoint:
		{
			/*osg::Group* labelGroup = new osg::Group();
			m_mapNode->addChild(labelGroup);*/

			const osgEarth::SpatialReference* geoSRS = m_mapNode->getMapSRS()->getGeographicSRS();

			osgEarth::Symbology::Style pms;
			pms.getOrCreate<osgEarth::Symbology::IconSymbol>()->url()->setLiteral("E:/BaiduNetdiskDownload/OSG/Data/mark.png");
			pms.getOrCreate<osgEarth::Symbology::IconSymbol>()->declutter() = true;
			pms.getOrCreate<osgEarth::Symbology::IconSymbol>()->scale() = 0.05;

			// bunch of pins:
			m_losGroup->addChild(new osgEarth::Annotation::PlaceNode(osgEarth::GeoPoint(geoSRS, Point.x(), Point.y()),"" ,pms));

		}break;

		//线标注
		//case EnumActionEvent::AddLine:
		//{
		//	if (m_bFirstClick)
		//	{
		//		m_feature->getGeometry()->clear();
		//		m_feature->getGeometry()->push_back(osg::Vec3d(Point.x(), Point.y(), 10));

		//		m_bFirstClick = false;
		//	}
		//	else
		//	{
		//		if (m_bLastPoint)
		//		{
		//			m_feature->getGeometry()->back() = osg::Vec3d(Point.x(), Point.y(), 10);
		//			m_bLastPoint = false;
		//		}
		//		else
		//		{
		//			m_feature->getGeometry()->push_back(osg::Vec3d(Point.x(), Point.y(), 10));
		//		}

		//		m_bFirstClick = true;
		//	}
		//}break;

		//case EnumActionEvent::AddPolygon:
		//{
		//	if (m_bFirstClick)
		//	{
		//		

		//		m_bFirstClick = false;
		//	}
		//	else
		//	{
		//		m_bFirstClick = true;
		//	}
		//}break;

		//case EnumActionEvent::AddCircle:
		//{
		//	if (m_bFirstClick)
		//	{
		//		

		//		m_bFirstClick = false;
		//	}
		//	else
		//	{
		//		m_bFirstClick = true;
		//	}
		//}break;

		default:break;
	}
}

//鼠标移动
void PickEvent::pickMove(osg::Vec3d Point)
{
	// 更新右下角的坐标
	auto tempStr = QString(m_csysTitle + " Lon: " + QString::number(Point.x(), 'f', 6) + " | Lat: " + QString::number(Point.y(), 'f', 6) + " | Hei: " + QString::number(Point.z(), 'f', 6) + " |  " + m_mapName);
	m_Label->setText(tempStr);

	switch (m_ActionEvent)
	{
		// 通视分析
		case EnumActionEvent::VisibilityAnalysis:
		{
			if (!m_bFirstClick)
			{
				osgEarth::GeoPoint _end(m_spatRef->getGeographicSRS(), Point.x(), Point.y(), 0.0, osgEarth::AltitudeMode::ALTMODE_RELATIVE);
				m_curLosNode->setEnd(_end);
			}
		}break;
		// 视域分析
		case EnumActionEvent::ViewshedAnalysis:
		{
			if (!m_bFirstClick)
			{
				auto _dis = osgEarth::GeoMath::distance(FirstPoint, Point, m_spatRef);
				m_curCircleNode->setRadius(_dis);
				m_curCircleOutLine->setRadius(_dis);

				if (!m_bLastPoint)
				{
					m_feature->getGeometry()->push_back(osg::Vec3d(Point.x(), Point.y(), 0));
					m_bLastPoint = true;
				}
				else
				{
					m_feature->getGeometry()->back() = osg::Vec3d(Point.x(), Point.y(), 0);
				}

				m_featureNode->dirty();
			}

		}break;

		////线标注
		//case EnumActionEvent::AddLine:
		//{
		//	if (m_bFirstClick)
		//	{

		//		if (!m_bLastPoint)
		//		{
		//			m_feature->getGeometry()->push_back(osg::Vec3d(Point.x(), Point.y(), 0));
		//			m_bLastPoint = true;
		//		}
		//		else
		//		{
		//			m_feature->getGeometry()->back() = osg::Vec3d(Point.x(), Point.y(), 0);
		//		}

		//		m_featureNode->dirty();
		//	}
		//}break;
		
	/*	case EnumActionEvent::WaterAnalysis:
		{
			if (!m_bFirstClick)
			{
				if (m_vecPoints->size() < 2)
				{
					return;
				}

				drawWater(Point);
			}

		}break;*/
		
		default:break;
	}
}

void PickEvent::setActionEvent(const EnumActionEvent &ae)
{
	m_ActionEvent = ae;
	m_bFirstClick = true;
}

void PickEvent::RemoveAnalysis()
{
	// 通视分析结果清理
	m_losGroup->removeChildren(0, m_losGroup->getNumChildren());
	m_feature->getGeometry()->clear();
	m_featureNode->dirty();
	//m_pFA->clear();

	m_vLT.clear();
//	m_vLT.swap(std::vector<DrawLineCallback*>());

}

//屏幕坐标转地理坐标
osg::Vec3d PickEvent::Screen2Geo(float x, float y)
{
	osg::Vec3d world;
	osgEarth::GeoPoint geoPoint;
	if (m_mapNode->getTerrain()->getWorldCoordsUnderMouse(m_viewer, x, y, world))//得到世界坐标
	{
		geoPoint.fromWorld(m_spatRef, world);//从世界坐标到地理坐标
	}
	return geoPoint.vec3d();
}

//设置视高
void PickEvent::setLosHeight(float height)
{
	m_ui_losHeight = height;
}

void PickEvent::setViewshedPara(int numSpokes, int numSegment)
{
	m_ui_numSpokes = numSpokes;//圈采样次数
	m_ui_numSegment = numSegment;//线采样次数
}

double GetDistance(double ele, double dir)
{
	//double r, p, k, g, ranta, segema, n, b, fn, omin, t,fy;
	//k = 1 * 10 ^ (-23);
	//p = 10 ^ 6;
	//g = 40;
	double r = 22071.1231;
	double f = sqrt(sqrt(abs(sin(ele))))*cos(ele)*cos(ele);
	double result = r * f;
	//percent = result;

	return result;
};

osg::Texture2D* creatText2D(const QString& strFile)
{
	if (strFile.isEmpty())
	{
		return NULL;
	}
	osg::ref_ptr<osg::Texture2D> pText2D = new osg::Texture2D;
	pText2D->setImage(osgDB::readImageFile(strFile.toStdString()));
	pText2D->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
	pText2D->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
	pText2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	pText2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture::LINEAR);
	return pText2D.release();
}

void PickEvent::drawWater(osg::Vec3d pos /*= osg::Vec3d()*/)
{
	const char* m_waterVert = {
		"uniform float osg_FrameTime;\n"
		"varying vec4 projCoords;\n"
		"varying vec3 lightDir, eyeDir;\n"
		"varying vec2 flowCoords, rippleCoords;\n"

		"void main()\n"
		"{\n"
		"   vec3 T = vec3(0.0, 1.0, 0.0);\n"
		"   vec3 N = vec3(0.0, 0.0, 1.0);\n"
		"   vec3 B = vec3(1.0, 0.0, 0.0);\n"
		"   T = normalize(gl_NormalMatrix * T);\n"
		"   B = normalize(gl_NormalMatrix * B);\n"
		"   N = normalize(gl_NormalMatrix * N);\n"

		"   mat3 TBNmat;\n"
		"   TBNmat[0][0] = T[0]; TBNmat[1][0] = T[1]; TBNmat[2][0] = T[2];\n"
		"   TBNmat[0][1] = B[0]; TBNmat[1][1] = B[1]; TBNmat[2][1] = B[2];\n"
		"   TBNmat[0][2] = N[0]; TBNmat[1][2] = N[1]; TBNmat[2][2] = N[2];\n"

		"   vec3 vertexInEye = vec3(gl_ModelViewMatrix * gl_Vertex);\n"
		"   lightDir =  gl_LightSource[0].position.xyz - vertexInEye;\n"
		"   lightDir = normalize(TBNmat * lightDir);\n"
		"   eyeDir = normalize(TBNmat * (-vertexInEye));\n"

		"   vec2 t1 = vec2(osg_FrameTime*0.02, osg_FrameTime*0.02);\n"
		"   vec2 t2 = vec2(osg_FrameTime*0.05, osg_FrameTime*0.05);\n"
		"   flowCoords = gl_MultiTexCoord0.xy + t1/10.0;\n" //* 5.0 + t1
		"   rippleCoords = gl_MultiTexCoord0.xy + t1;\n"   //

		"   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
		"   gl_Position = ftransform();\n"
		"   projCoords = gl_Position;\n"
		"}\n"
	};

	const char* m_waterFrag = {
	   "uniform sampler2D defaultTex;\n"
	   "uniform sampler2D reflection;\n"
	   "uniform sampler2D refraction;\n"
	   "uniform sampler2D normalTex;\n"
	   "varying vec4 projCoords;\n"
	   "varying vec3 lightDir, eyeDir;\n"
	   "varying vec2 flowCoords, rippleCoords;\n"

	   "void main()\n"
	   "{\n"
	   "   vec2 rippleEffect = 0.02 * texture2D(refraction, rippleCoords * 0.01).xy;\n"
	   "   vec4 N = texture2D(normalTex, flowCoords + rippleEffect);\n"
	   "   N = N * 2.0 - vec4(1.0);\n"
	   "   N.a = 1.0; N = normalize(N);\n"

	   "   vec3 refVec = normalize(reflect(-lightDir, vec3(N) * 0.6));\n"
	   "   float refAngle = clamp(dot(eyeDir, refVec), 0.0, 1.0);\n"
	   "   vec4 specular = vec4(pow(refAngle, 40.0));\n"

	   "   vec2 dist = texture2D(refraction, flowCoords + rippleEffect).xy;\n"
	   "   dist = (dist * 2.0 - vec2(1.0)) * 0.1;\n"
	   "   vec2 uv = projCoords.xy / projCoords.w;\n"
	   "   uv = clamp((uv + 1.0) * 0.5 + dist, 0.0, 1.0);\n"

	   "   vec4 base = texture2D(defaultTex, uv);\n"
	   "   vec4 refl = texture2D(reflection, uv);\n"
	   "   gl_FragColor = mix(base, refl + specular, 0.6);\n"
	   "}\n"
	};

	osg::ref_ptr<osg::Vec3dArray> vecPoints = new osg::Vec3dArray;
	vecPoints = m_vecPoints;
	if (pos == osg::Vec3d() && m_vecPoints->size() <= 2)
	{
		return;
	}
	if (pos != osg::Vec3d())
	{
		vecPoints->push_back(pos);
	}
	osg::ref_ptr<osg::Geometry> pWater = new osg::Geometry;
	pWater->setVertexArray(vecPoints);
	pWater->addPrimitiveSet(
		new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, vecPoints->size()));

	//纹理
	osg::ref_ptr<osg::Texture2D> pText2D = new osg::Texture2D;
	pText2D->setTextureSize(1024, 1024);
	pText2D->setInternalFormat(GL_RGBA);

	pWater->getOrCreateStateSet()->setTextureAttributeAndModes(0, pText2D);
	pWater->getOrCreateStateSet()->setTextureAttributeAndModes(1,
		creatText2D("splat/water2.jpg"));
	//pWater->getOrCreateStateSet()->setTextureAttributeAndModes(2,
	//	creatText2D("E:/GreatMap/project/version/Data/Bmp/water_DUDV.jpg"));
	//pWater->getOrCreateStateSet()->setTextureAttributeAndModes(3,
	//	creatText2D("E:/GreatMap/project/version/Data/Bmp/water_NM.jpg"));

	// shader
	osg::ref_ptr<osg::Shader> pVeter = new osg::Shader(osg::Shader::VERTEX, m_waterVert);
	osg::ref_ptr<osg::Shader> pFrag = new osg::Shader(osg::Shader::FRAGMENT, m_waterFrag);
	osg::ref_ptr<osg::Program> pProgram = new osg::Program;
	pProgram->addShader(pVeter);
	pProgram->addShader(pFrag);
	pWater->getOrCreateStateSet()->addUniform(new osg::Uniform("reflection", 0));
	pWater->getOrCreateStateSet()->addUniform(new osg::Uniform("defaultTex", 1));
	/*pWater->getOrCreateStateSet()->addUniform(new osg::Uniform("refraction", 2));
	pWater->getOrCreateStateSet()->addUniform(new osg::Uniform("normalTex", 3));*/
	pWater->getOrCreateStateSet()->setAttributeAndModes(pProgram);
/*
	if (m_pWater == NULL)
	{
		m_pWater = new osg::Geode;
		m_Group->addChild(m_pWater);
	}*/
	m_pWater->addDrawable(pWater);
	m_pWater->setStateSet(pWater->getOrCreateStateSet());
}

