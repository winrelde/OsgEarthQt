#include <Windows.h>
#include "Common.h"

#include "MyOSGEarthQT.h"
#include <osg/Depth>
#include <osg/LineWidth>
#include <osgViewer/ViewerEventHandlers>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <osgUtil/SmoothingVisitor>
using namespace std;
/*---------------------------------------------------------------主函数--------------------------------------------------*/
MyOSGEarthQT::MyOSGEarthQT(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowState(Qt::WindowActive);

	// 初始化设置视高子窗口
	sub_setLosHeight = new SetLosHeight(this);
	sub_viewshedPara = new ViewshedPara(this);
	sub_information = new kfz(this);

	// 设置状态栏
	auto _lab = new QLabel(this);
	_lab->setFrameStyle(QFrame::NoFrame);
	_lab->setText("");
	_lab->setMinimumSize(_lab->sizeHint());
	_lab->setFont(QFont("宋体", 12, QFont::Normal));
	ui.statusBar->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
	ui.statusBar->addPermanentWidget(_lab);
	QPalette pal;
	pal.setColor(QPalette::Background, Qt::white);
	ui.statusBar->setAutoFillBackground(true);   
	ui.statusBar->setPalette(pal);

	// 世界 Group
	m_world = new osg::Group();

    m_mapNode = osgEarth::MapNode::findMapNode(osgDB::readNodeFile("E:/BaiduNetdiskDownload/OSG/Data/clean.earth"));
	m_world->addChild(m_mapNode.get()); 
	
	// 近地面自动裁剪
	ui.openGLWidget->getCamera()->addCullCallback(new osgEarth::Util::AutoClipPlaneCullCallback(m_mapNode.get()));

	// 星空
	skyNode = osgEarth::Util::SkyNode::create();//提供天空、灯光和其他环境效果的类的接口
	//osg::ref_ptr<osgEarth::Util::Ephemeris> ephemeris = new osgEarth::Util::Ephemeris;//星历表给出了自然发生的天体的位置；在这种情况下，太阳和月亮；还包括一些相关的实用功能
	//skyNode->setEphemeris(ephemeris);//确定太阳和月亮的位置
	skyNode->setName("skyNode");
	//skyNode->setDateTime(osgEarth::DateTime());//现在
	skyNode->setDateTime(osgEarth::DateTime(2021, 4, 21, 22));
	skyNode->attach(ui.openGLWidget->getViewer(), 0);
	skyNode->setLighting(false);
	skyNode->addChild(m_mapNode);
	m_world->addChild(skyNode);


	// 绘制线的组
	m_losGroup = new osg::Group();
	m_losGroup->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	m_losGroup->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::Function::ALWAYS));
	m_mapNode->addChild(m_losGroup);

	ui.openGLWidget->getViewer()->setSceneData(m_world.get());

	//连接到QT
	connect(ui.VisibilityAnalysis, &QAction::triggered, this, &MyOSGEarthQT::on_VisibilityAnalysis);
	connect(ui.ViewshedAnalysis, &QAction::triggered, this, &MyOSGEarthQT::on_ViewshedAnalysis);
	connect(ui.RadarAnalysis, &QAction::triggered, this, &MyOSGEarthQT::on_RadarAnalysis);
	connect(ui.ClearAnalysis, &QAction::triggered, this, &MyOSGEarthQT::on_ClearAnalysis);
	connect(ui.WaterAnalysis, &QAction::triggered, this, &MyOSGEarthQT::on_WaterAnalysis);
	connect(ui.TerrainProfileAnalysis, &QAction::triggered, this, &MyOSGEarthQT::on_TerrainProfileAnalysis);
    connect(ui.SetLosHeight, &QAction::triggered, this, &MyOSGEarthQT::on_SetLosHeight);
	connect(ui.ViewshedPara, &QAction::triggered, this, &MyOSGEarthQT::on_SetViewshedPara);
	connect(ui.information, &QAction::triggered, this, &MyOSGEarthQT::on_kfz);
	connect(ui.ProvincialBoundary, &QAction::triggered, this, &MyOSGEarthQT::on_shengjiexian);
	connect(ui.Fuzhou, &QAction::triggered, this, &MyOSGEarthQT::on_Fuzhou);
	connect(ui.Boston, &QAction::triggered, this, &MyOSGEarthQT::on_Boston);
	connect(ui.Airport, &QAction::triggered, this, &MyOSGEarthQT::on_Airport);
	connect(ui.DongBaoShan, &QAction::triggered, this, &MyOSGEarthQT::on_DongBaoShan);
	connect(ui.SciencePark, &QAction::triggered, this, &MyOSGEarthQT::on_SciencePark);
	connect(ui.FeaturesQuery, &QAction::triggered, this, &MyOSGEarthQT::on_FeaturesQuery);
	connect(ui.AddModel, &QAction::triggered, this, &MyOSGEarthQT::on_AddModel);
	connect(ui.FogEffect, &QAction::triggered, this, &MyOSGEarthQT::on_FogEffect);
	connect(ui.Rain, &QAction::triggered, this, &MyOSGEarthQT::on_Rain);
	connect(ui.Snow, &QAction::triggered, this, &MyOSGEarthQT::on_Snow);
	connect(ui.Cloud, &QAction::triggered, this, &MyOSGEarthQT::on_Cloud);
	connect(ui.Fire, &QAction::triggered, this, &MyOSGEarthQT::on_Fire);
	connect(ui.AddCityModel, &QAction::triggered, this, &MyOSGEarthQT::on_AddCityModel);
	connect(ui.PathRoaming, &QAction::triggered, this, &MyOSGEarthQT::on_PathRoaming);
	connect(ui.Distance, &QAction::triggered, this, &MyOSGEarthQT::on_Distance);
	connect(ui.Area, &QAction::triggered, this, &MyOSGEarthQT::on_Area);
	connect(ui.Angle, &QAction::triggered, this, &MyOSGEarthQT::on_Angle);
	connect(ui.slopeAnalysis, &QAction::triggered, this, &MyOSGEarthQT::on_slopeAnalysis);
	//connect(ui.AspectAnalysis, &QAction::triggered, this, &MyOSGEarthQT::on_AspectAnalysis);
	connect(ui.AddTiltphotographymodel, &QAction::triggered, this, &MyOSGEarthQT::on_AddTiltphotographymodel);
	connect(ui.Addgraticule, &QAction::triggered, this, &MyOSGEarthQT::on_Addgraticule);
	connect(ui.Addsatellite, &QAction::triggered, this, &MyOSGEarthQT::on_Addsatellite);
	connect(ui.AddCompass, &QAction::triggered, this, &MyOSGEarthQT::on_AddCompass);
	connect(ui.AddScaleBar, &QAction::triggered, this, &MyOSGEarthQT::on_AddScaleBar);
	connect(ui.AddMiniMap, &QAction::triggered, this, &MyOSGEarthQT::on_AddMiniMap);
	connect(ui.AddKml, &QAction::triggered, this, &MyOSGEarthQT::on_AddKml);
	connect(ui.AddVector, &QAction::triggered, this, &MyOSGEarthQT::on_AddVector);
	connect(ui.AddPoint, &QAction::triggered, this, &MyOSGEarthQT::on_AddPoint);
	connect(ui.AddLine, &QAction::triggered, this, &MyOSGEarthQT::on_AddLine);
	connect(ui.AddPolygon, &QAction::triggered, this, &MyOSGEarthQT::on_AddPolygon);
	connect(ui.AddCircle, &QAction::triggered, this, &MyOSGEarthQT::on_AddCircle);
	connect(ui.AddRectangle, &QAction::triggered, this, &MyOSGEarthQT::on_AddRectangle);
	connect(ui.StraightArrow, &QAction::triggered, this, &MyOSGEarthQT::on_StraightArrow);
	connect(ui.DiagonalArrow, &QAction::triggered, this, &MyOSGEarthQT::on_DiagonalArrow);
	connect(ui.DoubleArrow, &QAction::triggered, this, &MyOSGEarthQT::on_DoubleArrow);
	connect(ui.Lune, &QAction::triggered, this, &MyOSGEarthQT::on_GeoLune);
	connect(ui.GatheringPlace, &QAction::triggered, this, &MyOSGEarthQT::on_GatheringPlace);
	connect(ui.ParallelSearch, &QAction::triggered, this, &MyOSGEarthQT::on_ParallelSearch);
	connect(ui.SectorSearch, &QAction::triggered, this, &MyOSGEarthQT::on_SectorSearch);
	connect(ui.AddEarth, &QAction::triggered, this, &MyOSGEarthQT::on_AddEarth);
	connect(ui.AddElevation, &QAction::triggered, this, &MyOSGEarthQT::on_AddElevation);
	connect(ui.Tianwa, &QAction::triggered, this, &MyOSGEarthQT::on_Tianwa);
	connect(ui.Dynamictexture, &QAction::triggered, this, &MyOSGEarthQT::on_Dynamictexture);
	connect(ui.TrailLine, &QAction::triggered, this, &MyOSGEarthQT::on_TrailLine);
	connect(ui.ChangeTime, &QAction::triggered, this, &MyOSGEarthQT::on_ChangeTime);

	m_PickEvent = new PickEvent(_lab, m_mapNode, m_losGroup);
	ui.openGLWidget->getViewer()->addEventHandler(m_PickEvent);
	//ui.openGLWidget->getViewer()->addEventHandler(new osgViewer::StatsHandler);

	osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
	em->setViewpoint(osgEarth::Viewpoint(NULL, 110, 24, 7500000, 0, -90, 12000000), 2);
	em->setHomeViewpoint(osgEarth::Viewpoint(NULL, 110, 24, 7060660, 0, -90, 12000000), 2);
}
/*---------------------------------------------------------------可视分析--------------------------------------------------*/
void MyOSGEarthQT::on_VisibilityAnalysis(bool checked)
{
	if (checked)
	{
		ui.openGLWidget->getViewer()->getEventHandlers();
		m_PickEvent->setActionEvent(EnumActionEvent::VisibilityAnalysis);
	}
	else
	{
		m_PickEvent->setActionEvent(EnumActionEvent::ActionNull);
		ui.VisibilityAnalysis->setChecked(false);//不用传入对应信号
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
}
/*---------------------------------------------------------------视域分析--------------------------------------------------*/
void MyOSGEarthQT::on_ViewshedAnalysis(bool checked)
{
	if (checked)
	{
		m_PickEvent->setActionEvent(EnumActionEvent::ViewshedAnalysis);
	}
	else
	{
		m_PickEvent->setActionEvent(EnumActionEvent::ActionNull);
		ui.ViewshedAnalysis->setChecked(false);
		return;
	}
	ui.VisibilityAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
}
/*---------------------------------------------------------------雷达包络--------------------------------------------------*/
void MyOSGEarthQT::on_RadarAnalysis(bool checked)
{
	if (checked)
	{
		m_PickEvent->setActionEvent(EnumActionEvent::RadarAnalysis);
	}
	else
	{
		m_PickEvent->setActionEvent(EnumActionEvent::ActionNull);
		ui.RadarAnalysis->setChecked(false);
		return;
	}
	ui.VisibilityAnalysis->setChecked(false);
	ui.ViewshedAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
}
/*---------------------------------------------------------------清除分析--------------------------------------------------*/
void MyOSGEarthQT::on_ClearAnalysis()
{
	ui.VisibilityAnalysis->setChecked(false);
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	m_PickEvent->setActionEvent(EnumActionEvent::ActionNull);

	m_PickEvent->RemoveAnalysis();//结果清理
	m_world->removeChild(m_hud);

}
/*------------------------------------------------------飞行路径（不改变姿态参数）--------------------------------------------*/
osg::AnimationPath* CreateflyPath(osg::Vec4Array *ctrl)
{
	osg::Matrix TurnMatrix;
	double Hz_Angle;
	double Vt_Anglle;
	double time = 0;

	double lon, lat, height;

	//旋转4元素,旋转角度
	osg::Quat _rotation;

	//世界坐标系内的两点
	osg::Vec3d positionCur;
	osg::Vec3d positionNext;

	osg::ref_ptr<osg::AnimationPath>animationPath = new osg::AnimationPath;
	//设置是否循环飞行
	animationPath->setLoopMode(osg::AnimationPath::LOOP);

	//循环读取预设点      迭代器
	for (osg::Vec4Array::iterator iter = ctrl->begin(); iter != ctrl->end(); iter++)
	{
		//iter2为iter的下一个点
		osg::Vec4Array::iterator iter2 = iter;
		iter2++;

		//如果只有两个点，跳出循环
		if (iter2 == ctrl->end())
			break;

		double x, y, z;

		osg::ref_ptr<osg::CoordinateSystemNode> csn1 = new osg::CoordinateSystemNode;
		csn1->setEllipsoidModel(new osg::EllipsoidModel);
		//把经纬度转换为绝对世界坐标系   csn->获取椭球体模型    latitude纬度  longtitude经度
		csn1->getEllipsoidModel()->convertLatLongHeightToXYZ(osg::DegreesToRadians(iter->y()), osg::DegreesToRadians(iter->x()), iter->z(), x, y, z);
		positionCur = osg::Vec3(x, y, z);
		csn1->getEllipsoidModel()->convertXYZToLatLongHeight(x, y, z, lat, lon, height);
		csn1->getEllipsoidModel()->convertLatLongHeightToXYZ(osg::DegreesToRadians(iter2->y()), osg::DegreesToRadians(iter2->x()), iter2->z(), x, y, z);
		positionNext = osg::Vec3(x, y, z);
		//求出水平夹角
		//如果经度相同
		if (iter->x() == iter2->x())
		{
			//如果两点经度相同，水平夹角为90度
			Hz_Angle = osg::PI_2;
		}
		else
		{
			//atan（维度/经度）
			Hz_Angle = atan((iter2->y() - iter->y()) / (iter2->x() - iter->x()));
			if (iter2->x() > iter->x())
			{
				Hz_Angle += osg::PI;
			}
		}

		//求垂直夹角
		if (iter->z() == iter2->z())
		{
			Vt_Anglle = 0;
		}
		else
		{
			//如果如果两点在同一经维度，只有高度不同时  sqrt[(x2-x2)**2 + (y2-y1)**2 + (z2-z1)**2 - (z2-z1)**2]=0
			if (0 == sqrt(((positionNext.x() - positionCur.x())* (positionNext.x() - positionCur.x()) + (positionNext.y() - positionCur.y())*(positionNext.y() - positionCur.y()) + (positionNext.z() - positionCur.z())*(positionNext.z() - positionCur.z())) - pow((positionNext.z() - positionCur.z()), 2)))
			{
				Vt_Anglle = osg::PI_2;
			}
			//如果经纬度不同
			else
			{
				//                 垂直高度/底边
				//垂直角度   atan{(z2-z1) / (sqrt[(x2-x2)**2 + (y2-y1)**2 + (z2-z1)**2 - (z2-z1)**2])}
				Vt_Anglle = atan((iter2->z() - iter->z()) / (sqrt(((positionNext.x() - positionCur.x())* (positionNext.x() - positionCur.x()) + (positionNext.y() - positionCur.y())*(positionNext.y() - positionCur.y()) + (positionNext.z() - positionCur.z())*(positionNext.z() - positionCur.z())) - pow((positionNext.z() - positionCur.z()), 2))));
			}
			if (Vt_Anglle >= osg::PI_2)
			{
				Vt_Anglle = osg::PI_2;
			}
			if (Vt_Anglle <= -osg::PI_2)
			{
				Vt_Anglle = -osg::PI_2;
			}
		}
		//1弧度=57.3°
		//std::cout << "shuiping=" << osg::RadiansToDegrees(shuipingAngle)<< "   chuizhi=" << osg::RadiansToDegrees(chuizhiAnglle)<< std::endl;
		//前面求出了第一个点处飞机的朝向，接下来需要求朝向所对应的变换矩阵(TurnMatrix)，求出此变换矩阵后，和飞机矩阵相乘，就能形成飞机转向的动作
		//求出飞机世界坐标系在经纬高度坐标系下的值
		csn1->getEllipsoidModel()->computeLocalToWorldTransformFromLatLongHeight(osg::DegreesToRadians(iter->y()), osg::DegreesToRadians(iter->x()), iter->z(), TurnMatrix);
		//飞机转向角          绕x轴 不用转                               绕y轴旋转                            绕z轴旋转
		//_rotation.makeRotate(0, osg::Vec3(1.0, 0.0, 0.0), Vt_Anglle, osg::Vec3(0.0, 1.0, 0.0), Hz_Angle, osg::Vec3(0.0, 0.0, 1.0));
		_rotation.makeRotate(0, osg::Vec3(1.0, 0.0, 0.0), 0, osg::Vec3(0.0, 1.0, 0.0), 0, osg::Vec3(0.0, 0.0, 1.0));
		//把旋转矩阵乘到飞机的变换矩阵上
		TurnMatrix.preMultRotate(_rotation);

		//生成插入点，ControlPoint包含模型位置和朝向     生成第一个点的AnimationPath
		animationPath->insert(time, osg::AnimationPath::ControlPoint(positionCur, TurnMatrix.getRotate()));

		//求下一个点的时间
		//      斜边长度=sqrt[(x2-x2)**2 + (y2-y1)**2 + (z2-z1)**2]
		double dist = sqrt(((positionNext.x() - positionCur.x())* (positionNext.x() - positionCur.x()) + (positionNext.y() - positionCur.y())*(positionNext.y() - positionCur.y()) + (positionNext.z() - positionCur.z())*(positionNext.z() - positionCur.z())));
		//std::cout << "距离=" << dist << std::endl;
		//如果速度等于0，飞机到达该点时间无限大
		if (iter2->w() == 0)
		{
			time += 100000000;
		}
		//如果速度不等于0，飞机到达该点时间等于 距离/速度
		else
		{
			time += (dist / iter2->w());
		}
	}
	//生成最后一个点的AnimationPath
	animationPath->insert(time, osg::AnimationPath::ControlPoint(positionNext, TurnMatrix.getRotate()));
	//std::cout << "时间=" << time << std::endl;
	return animationPath.release();
}
/*---------------------------------------------------------------水域分析--------------------------------------------------*/
osg::Geode* createScreenQuad(float width, float height, float scale)
{
	osg::Geometry* geom = osg::createTexturedQuadGeometry(
		osg::Vec3(), osg::Vec3(width, 0.0f, 0.0f), osg::Vec3(0.0f, height, 0.0f),
		0.0f, 0.0f, width*scale, height*scale);
	osg::ref_ptr<osg::Geode> quad = new osg::Geode;
	quad->addDrawable(geom);

	int values = osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED;
	quad->getOrCreateStateSet()->setAttribute(
		new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL), values);
	quad->getOrCreateStateSet()->setMode(GL_LIGHTING, values);
	return quad.release();
}
osg::Camera* createRTTCamera(osg::Camera::BufferComponent buffer, osg::Texture* tex, bool isAbsolute)
{
	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setClearColor(osg::Vec4());
	camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	camera->setRenderOrder(osg::Camera::PRE_RENDER);
	if (tex)
	{
		tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		camera->setViewport(0, 0, tex->getTextureWidth(), tex->getTextureHeight());
		camera->attach(buffer, tex);
	}

	if (isAbsolute)
	{
		camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0));
		camera->setViewMatrix(osg::Matrix::identity());
		camera->addChild(createScreenQuad(1.0f, 1.0f, 1.0f));
	}
	return camera.release();
}
osg::Texture2D* createTexture(const std::string& filename)
{
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(osgDB::readImageFile(filename));
	texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
	texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	return texture.release();
}
void MyOSGEarthQT::on_WaterAnalysis(bool checked)
{
	if (checked)
	{
		static const char* waterVert = {
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
			"   flowCoords = gl_MultiTexCoord0.xy * 5.0 + t1;\n"
			"   rippleCoords = gl_MultiTexCoord0.xy * 10.0 + t2;\n"

			"   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
			"   gl_Position = ftransform();\n"
			"   projCoords = gl_Position;\n"
			"}\n"
		};
		static const char* waterFrag = {
			"uniform sampler2D defaultTex;\n"
			"uniform sampler2D reflection;\n"
			"uniform sampler2D refraction;\n"
			"uniform sampler2D normalTex;\n"
			"varying vec4 projCoords;\n"
			"varying vec3 lightDir, eyeDir;\n"
			"varying vec2 flowCoords, rippleCoords;\n"

			"void main()\n"
			"{\n"
			"   vec2 rippleEffect = 0.02 * texture2D(refraction, rippleCoords * 0.1).xy;\n"
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

		osg::ref_ptr<osg::Node> scene = new osg::Node;
		scene = osgDB::readNodeFile("cessna.osg.5,5,5.scale");
		osgEarth::Registry::shaderGenerator().run(scene);
		scene->setNodeMask(0);

		float z = -20.0f;
		osg::ref_ptr<osg::MatrixTransform> reverse = new osg::MatrixTransform;
		reverse->preMult(osg::Matrix::translate(0.0f, 0.0f, -z) *
			osg::Matrix::scale(100.0f, 100.0f, -100.0f) *
			osg::Matrix::translate(0.0f, 0.0f, z));
		reverse->addChild(scene.get());
		 
		//waterxform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), 119.255745, 25.993915, 100));
		waterxform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), 119, 26, 700));

		osg::ref_ptr<osg::ClipPlane> clipPlane = new osg::ClipPlane;
		clipPlane->setClipPlane(0.0, 0.0, -1.0, z);
		clipPlane->setClipPlaneNum(0);

		osg::ref_ptr<osg::ClipNode> clipNode = new osg::ClipNode;
		clipNode->addClipPlane(clipPlane.get());
		clipNode->addChild(reverse.get());

		// The RTT camera
		osg::ref_ptr<osg::Texture2D> tex2D = new osg::Texture2D;
		tex2D->setTextureSize(1024, 1024);
		tex2D->setInternalFormat(GL_RGBA);

		osg::ref_ptr<osg::Camera> rttCamera = createRTTCamera(osg::Camera::COLOR_BUFFER, tex2D.get(), true);
		rttCamera->addChild(clipNode.get());

		// The water plane
		const osg::Vec3& center = scene->getBound().center();

		float planeSize = 20.0f * scene->getBound().radius();
		//float planeSize = 20.0f;
		osg::Vec3 planeCorner(center.x() - 0.5f*planeSize, center.y() - 0.5f*planeSize, z);
		osg::ref_ptr<osg::Geometry> quad = osg::createTexturedQuadGeometry(
			planeCorner, osg::Vec3(planeSize, 0.0f, 0.0f), osg::Vec3(0.0f, planeSize, 0.0f));

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		geode->addDrawable(quad.get());

		osg::StateSet* ss = geode->getOrCreateStateSet();
		ss->setTextureAttributeAndModes(0, tex2D.get());
		ss->setTextureAttributeAndModes(1, createTexture("E:\\sxearth2.10.2\\data\\images\\water\\water.jpg"));
		ss->setTextureAttributeAndModes(2, createTexture("E:\\sxearth2.10.2\\data\\images\\water\\water_DUDV.jpg"));
		ss->setTextureAttributeAndModes(3, createTexture("E:\\sxearth2.10.2\\data\\images\\water\\water_NM.jpg"));

		osg::ref_ptr<osg::Program> program = new osg::Program;
		program->addShader(new osg::Shader(osg::Shader::VERTEX, waterVert));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, waterFrag));
		geode->getOrCreateStateSet()->setAttributeAndModes(program.get());
		geode->getOrCreateStateSet()->addUniform(new osg::Uniform("reflection", 0));
		geode->getOrCreateStateSet()->addUniform(new osg::Uniform("defaultTex", 1));
		geode->getOrCreateStateSet()->addUniform(new osg::Uniform("refraction", 2));
		geode->getOrCreateStateSet()->addUniform(new osg::Uniform("normalTex", 3));

		waterxform->addChild(rttCamera.get());
		waterxform->addChild(geode.get());
		waterxform->addChild(scene.get());

		osg::ref_ptr<osg::Vec4Array> vatemp = new osg::Vec4Array;
		vatemp->push_back(osg::Vec4(119, 26, 500, 3));
		vatemp->push_back(osg::Vec4(119, 26, 600, 3));
		vatemp->push_back(osg::Vec4(119, 26, 700, 3));
		vatemp->push_back(osg::Vec4(119, 26, 900, 3));

		osg::AnimationPath *ap = new osg::AnimationPath;
		ap = CreateflyPath(vatemp);

		waterxform->setUpdateCallback(new osg::AnimationPathCallback(ap, 0.0, 1.0));

		m_mapNode->addChild(waterxform);

		ui.openGLWidget->getCamera()->setSmallFeatureCullingPixelSize(-1);

		osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		em->setViewpoint(osgEarth::Viewpoint(NULL, 119, 26, 2060.66, -2.5, 90, 20000), 2);
	}
	else
	{
		m_mapNode->removeChild(waterxform);
		return;
	}
}
/*---------------------------------------------------------------剖面分析--------------------------------------------------*/
void MyOSGEarthQT::on_TerrainProfileAnalysis(bool checked)
{
	if (checked)
	{
		tp_hud = createHud(800, 400);
		m_world->addChild(tp_hud);

		//核心函数TerrainProfileCalculator,初始线段
		calculator = new TerrainProfileCalculator(m_mapNode,
			GeoPoint(m_mapNode->getMapSRS(), -124.0, 40.0),
			GeoPoint(m_mapNode->getMapSRS(), -75.1, 39.2)
		);

		profileNode = new TerrainProfileGraph(calculator.get(), 400, 200);
		tp_hud->addChild(profileNode);

		tp = new TerrainProfileEventHandler(m_mapNode, m_mapNode, calculator.get());
		ui.openGLWidget->getViewer()->addEventHandler(tp);
	}
	else
	{
		tp->Remove();
		ui.openGLWidget->getViewer()->removeEventHandler(tp);
		m_world->removeChild(tp_hud);
		return;
	}
	ui.VisibilityAnalysis->setChecked(false);
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
}
/*---------------------------------------------------------------设置视高--------------------------------------------------*/
void MyOSGEarthQT::on_SetLosHeight()
{
	sub_setLosHeight->show();
	sub_setLosHeight->move(this->pos());//移动控件到指定位置
}
void MyOSGEarthQT::sendLosHeight(float height)
{
	m_PickEvent->setLosHeight(height);
}
/*---------------------------------------------------------------视域设置--------------------------------------------------*/
void MyOSGEarthQT::on_SetViewshedPara()
{
	sub_viewshedPara->show();
	sub_viewshedPara->move(this->pos());
}
void MyOSGEarthQT::sendViewshedPara(int numSpokes, int numSegment)
{
	m_PickEvent->setViewshedPara(numSpokes, numSegment);
}
/*--------------------------------------------------------显示开发者信息----------------------------------------------------*/
void MyOSGEarthQT::on_kfz()
{
	sub_information->show();
	sub_information->move(this->pos());
}
/*---------------------------------------------------------------添加省界线--------------------------------------------------*/
void MyOSGEarthQT::on_shengjiexian(bool checked)
{
	if (checked)
	{
		osgEarth::Map* map = new osgEarth::Map();
		osgEarth::Drivers::OGRFeatureOptions featureData;
		map = m_mapNode->getMap();
		// Configures the feature driver to load the vectors from a shapefile:
		featureData.url() = "E:\\BaiduNetdiskDownload\\OSG\\250m\\gadm36_CHN_shp\\gadm36_CHN_1.shp";

		// Make a feature source layer and add it to the Map:
		osgEarth::Drivers::FeatureSourceLayerOptions ogrLayer;
		ogrLayer.name() = "vector-data";
		ogrLayer.featureSource() = featureData;
		osgEarth::Drivers::FeatureSourceLayer* featurelayer = new osgEarth::Drivers::FeatureSourceLayer(ogrLayer);
		map->addLayer(featurelayer);

		osgEarth::Symbology::Style style;

		osgEarth::Symbology::LineSymbol* ls = style.getOrCreateSymbol<osgEarth::Symbology::LineSymbol>();
		ls->stroke()->color() = osgEarth::Symbology::Color::Yellow;
		ls->stroke()->width() = 2.0f;
		ls->tessellationSize()->set(100, osgEarth::Symbology::Units::KILOMETERS);

		osgEarth::Symbology::AltitudeSymbol* alt = style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
		alt->clamping() = alt->CLAMP_TO_TERRAIN;
		alt->technique() = alt->TECHNIQUE_GPU;

		ls->tessellationSize()->set(100, osgEarth::Symbology::Units::KILOMETERS);

		//osgEarth::Symbology::PolygonSymbol *polygonSymbol = style.getOrCreateSymbol<osgEarth::Symbology::PolygonSymbol>();
		//polygonSymbol->fill()->color() = osgEarth::Symbology::Color(152.0f / 255, 251.0f / 255, 152.0f / 255, 0.8f); //238 230 133
		//polygonSymbol->outline() = true;

		osgEarth::Symbology::RenderSymbol* render = style.getOrCreate<osgEarth::Symbology::RenderSymbol>();
		render->depthOffset()->enabled() = true;

		osgEarth::Features::FeatureModelLayerOptions fml;
		fml.name() = "My Features";
		fml.featureSourceLayer() = "vector-data";
		fml.styles() = new osgEarth::Features::StyleSheet();
		fml.styles()->addStyle(style);
		fml.enableLighting() = false;
		osgEarth::Features::FeatureModelLayer* fmllayer = new osgEarth::Features::FeatureModelLayer(fml);
		map->addLayer(fmllayer);

		osgEarth::Symbology::Style labelStyle;

		osgEarth::Symbology::TextSymbol* text = labelStyle.getOrCreateSymbol<osgEarth::Symbology::TextSymbol>();
		std::string name = "[NL_NAME_1]";		//如果需要显示汉字，则需要转换成UTF-8编码
		text->font() = "C:/Windows/Fonts/simhei.ttf";
		text->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;
		text->content() = osgEarth::Symbology::StringExpression(name);
		text->size() = 10.0f;
		text->alignment() = osgEarth::Symbology::TextSymbol::ALIGN_CENTER_CENTER;
		text->fill()->color() = osgEarth::Symbology::Color::White;
		text->halo()->color() = osgEarth::Symbology::Color::DarkGray;

		// and configure a model layer:
		osgEarth::Features::FeatureModelLayerOptions fmlopt;
		fmlopt.name() = "Labels";
		fmlopt.featureSourceLayer() = "vector-data";
		fmlopt.styles() = new osgEarth::Features::StyleSheet();
		fmlopt.styles()->addStyle(labelStyle);
		osgEarth::Features::FeatureModelLayer* fmllayer2 = new osgEarth::Features::FeatureModelLayer(fmlopt);
		map->addLayer(fmllayer2);

		osgEarth::GLUtils::setGlobalDefaults(ui.openGLWidget->getViewer()->getCamera()->getOrCreateStateSet());
	}
	else
	{
		osg::ref_ptr<osgEarth::Features::FeatureModelLayer> textlayer = dynamic_cast<osgEarth::Features::FeatureModelLayer*>(m_mapNode->getMap()->getLayerByName("Labels"));
		m_mapNode->getMap()->removeLayer(textlayer.get());

		osg::ref_ptr<osgEarth::Features::FeatureModelLayer> featureslayer = dynamic_cast<osgEarth::Features::FeatureModelLayer*>(m_mapNode->getMap()->getLayerByName("My Features"));
		m_mapNode->getMap()->removeLayer(featureslayer.get());

		return;
	}
}
/*---------------------------------------------------------------中英文字符转换--------------------------------------------------*/
void unicodeToUTF8(const std::wstring &src, std::string& result)
{
	int n = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, 0, 0, 0, 0);
	result.resize(n);
	::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, (char*)result.c_str(), result.length(), 0, 0);
}
void gb2312ToUnicode(const std::string& src, std::wstring& result)
{
	int n = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0);
	result.resize(n);
	::MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, (LPWSTR)result.c_str(), result.length());
}
void gb2312ToUtf8(const std::string& src, std::string& result)
{
	std::wstring strWideChar;
	gb2312ToUnicode(src, strWideChar);
	unicodeToUTF8(strWideChar, result);
}
/*---------------------------------------------------------------福州大学山区--------------------------------------------------*/
void MyOSGEarthQT::on_Fuzhou()
{
	// 设置视点
	osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
	em->setViewpoint(osgEarth::Viewpoint(NULL, 119, 26, 2060.66, -2.5, -10, 20000), 2);
}
/*---------------------------------------------------------------波士顿--------------------------------------------------*/
void MyOSGEarthQT::on_Boston()
{
	osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
	em->setViewpoint(osgEarth::Viewpoint(NULL, -71.076262, 42.34425, 2060.66, -2.5, -10, 3450), 2);
}
/*---------------------------------------------------------------福州长乐机场--------------------------------------------------*/
void MyOSGEarthQT::on_Airport()
{
	// 设置视点
	osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
	em->setViewpoint(osgEarth::Viewpoint(NULL, 119.675751, 25.864162, 2060.66, -2.5, -10, 3450), 2);

	osg::Group* labelGroup = new osg::Group();
	m_mapNode->addChild(labelGroup);

	const SpatialReference* geoSRS = m_mapNode->getMapSRS()->getGeographicSRS();
	std::string name = "福州长乐机场";
	std::string _strWideName;
	gb2312ToUtf8(name, _strWideName);

	Style pm;
	pm.getOrCreate<osgEarth::Symbology::TextSymbol>()->font() = "C:/Windows/Fonts/simhei.ttf";//指定中文字体路径
	pm.getOrCreate<osgEarth::Symbology::TextSymbol>()->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;
	pm.getOrCreate<IconSymbol>()->url()->setLiteral("E:/BaiduNetdiskDownload/OSG/Data/placemark32.png");
	pm.getOrCreate<IconSymbol>()->declutter() = true;
	pm.getOrCreate<TextSymbol>()->halo() = Color("#5f5f5f");

	// bunch of pins:
	labelGroup->addChild(new osgEarth::Annotation::PlaceNode(GeoPoint(geoSRS, 119.6717, 25.934486), _strWideName, pm));
}
/*---------------------------------------------------------------福大科技园--------------------------------------------------*/
void MyOSGEarthQT::on_SciencePark()
{
	// 设置视点
	osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
	em->setViewpoint(osgEarth::Viewpoint(NULL, 119.19771, 26.05517, 500, -2.5, -10, 3450), 2);

	osg::Group* labelGroup = new osg::Group();
	m_mapNode->addChild(labelGroup);

	const SpatialReference* geoSRS = m_mapNode->getMapSRS()->getGeographicSRS();
	std::string name = "福州大学";
	std::string _strWideName;
	gb2312ToUtf8(name, _strWideName);

	Style pm;
	pm.getOrCreate<osgEarth::Symbology::TextSymbol>()->font() = "C:/Windows/Fonts/simhei.ttf";//指定中文字体路径
	pm.getOrCreate<osgEarth::Symbology::TextSymbol>()->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;
	pm.getOrCreate<IconSymbol>()->url()->setLiteral("E:/BaiduNetdiskDownload/OSG/Data/placemark32.png");
	pm.getOrCreate<IconSymbol>()->declutter() = true;
	pm.getOrCreate<TextSymbol>()->halo() = Color("#5f5f5f");

	// bunch of pins:
	labelGroup->addChild(new osgEarth::Annotation::PlaceNode(GeoPoint(geoSRS, 119.196704, 26.066218), _strWideName, pm));
}
/*--------------------------------------------------------------东宝山--------------------------------------------------*/
void MyOSGEarthQT::on_DongBaoShan()
{
	// 设置视点
	osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
	em->setViewpoint(osgEarth::Viewpoint(NULL, 117.064, 25.107, 500, -2.5, -10, 3450), 2);
}
/*---------------------------------------------------------------要素查询--------------------------------------------------*/
class ReadoutCallback : public RTTPicker::Callback
{
public:
	ReadoutCallback(ControlCanvas* container) : _lastFID(~0)
	{
		_grid = new Grid();
		_grid->setBackColor(osg::Vec4(1, 1, 1, 0.2f));
		container->addControl(_grid);
	}
	void onHit(ObjectID id)
	{
		FeatureIndex* index = Registry::objectIndex()->get<FeatureIndex>(id).get();
		Feature* feature = index ? index->getFeature(id) : 0L;
		if (feature && feature->getFID() != _lastFID)
		{
			_grid->clearControls();
			unsigned r = 0;

			_grid->setControl(0, r, new LabelControl("FID", Color::Red));
			_grid->setControl(1, r, new LabelControl(Stringify() << feature->getFID(), Color::Black));
			++r;

			const AttributeTable& attrs = feature->getAttrs();
			for (AttributeTable::const_iterator i = attrs.begin(); i != attrs.end(); ++i, ++r)
			{
				_grid->setControl(0, r, new LabelControl(i->first, 14.0f, Color::Yellow));
				_grid->setControl(1, r, new LabelControl(i->second.getString(), 14.0f, Color::Black));
			}
			if (!_grid->visible())
				_grid->setVisible(true);

			_lastFID = feature->getFID();
		}
	}
	void onMiss()
	{
		_grid->setVisible(false);
		_lastFID = 0u;
	}
	bool accept(const osgGA::GUIEventAdapter& ea, const osgGA::GUIActionAdapter& aa)
	{
		return ea.getEventType() == ea.RELEASE; // click
	}
	Grid*     _grid;
	FeatureID _lastFID;
	ControlCanvas* container;
};
void MyOSGEarthQT::on_FeaturesQuery(bool checked)
{
	if (checked)
	{
		viewer = ui.openGLWidget->getViewer();
		feaquerycanvas = new ControlCanvas;
		m_mapNode->addChild(feaquerycanvas);
		picker = new RTTPicker();
		if (m_mapNode)
		{
			vbox = new VBox();
			vbox->setVertAlign(Control::ALIGN_TOP);
			vbox->setHorizAlign(Control::ALIGN_RIGHT);
			vbox->addControl(new LabelControl("Feature Query Demo", Color::Yellow));
			vbox->addControl(new LabelControl("Click on a feature to see its attributes."));
			feaquerycanvas->addControl(vbox);

			viewer->stopThreading();

			viewer->addEventHandler(picker);
			picker->addChild(m_mapNode);
			picker->setDefaultCallback(new ReadoutCallback(feaquerycanvas));

			viewer->startThreading();
		}
	}
	else
	{
		viewer->stopThreading();
		picker->removeChild(m_mapNode);
		ui.openGLWidget->getViewer()->removeEventHandler(picker);
		m_mapNode->removeChild(feaquerycanvas);
		viewer->startThreading();
		return;
	}
}
/*---------------------------------------------------------------热气球航拍--------------------------------------------------*/
class MaskCallBack :public osg::NodeCallback
{
public:
	MaskCallBack(osg::Node* node) :m_mask(0), m_node(node), m_tick(0) {}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		if (m_tick < 50)
		{
			m_tick++;
			return;
		}
		else
		{
			m_tick = 0;
		}
		if (m_node)
			m_node->setNodeMask(m_mask);
		if (node)
			node->setNodeMask(m_mask = ~m_mask);
	}
protected:
	~MaskCallBack() {}
	unsigned int m_mask;
	osg::ref_ptr<osg::Node> m_node;

	unsigned int m_tick;
};
class AdjustSphereSegmentCallback : public osgGA::GUIEventHandler
{
public:
	void scaleSphereSegment(osgSim::SphereSegment* ss, float scale)
	{
		if (!ss) return;
		float azMin, azMax, elevMin, elevMax;
		ss->getArea(azMin, azMax, elevMin, elevMax);

		ss->setArea(azMin, azMax, elevMin*scale, elevMax*scale);
	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/, osg::Object* object, osg::NodeVisitor* /*nv*/)
	{
		if (ea.getHandled()) return false;

		switch (ea.getEventType())
		{
		case(osgGA::GUIEventAdapter::KEYDOWN):
		{
			if (ea.getKey() == ea.KEY_B)
			{
				scaleSphereSegment(dynamic_cast<osgSim::SphereSegment*>(object), 1.1f);
				return true;
			}
			else if (ea.getKey() == ea.KEY_C)
			{
				scaleSphereSegment(dynamic_cast<osgSim::SphereSegment*>(object), 1.0f / 1.1f);
			}
			break;
		}
		default:
			break;
		}
		return false;
	}
};
void MyOSGEarthQT::on_AddModel(bool checked)
{
	if(checked)
	{
		osg::Matrix TurnMatrixinit;

		//旋转4元素,旋转角度
		osg::Quat _rotation;

		osg::ref_ptr< osg::Node >  plane = osgDB::readRefNodeFile("E:\\BaiduNetdiskDownload\\OSG\\Data\\Balloon\\1.osg");

		//osgEarth::Registry::shaderGenerator().run(plane);

		osg::ref_ptr<osg::MatrixTransform> mtfly = new osg::MatrixTransform;
		//TurnMatrixinit.preMult(osg::Matrix::rotate(osg::inDegrees(-60.0), osg::Vec3(0, 1, 0)));
		mtfly->setMatrix(TurnMatrixinit);
		mtfly->addChild(plane);

		osgSim::SphereSegment * ss1 = new osgSim::SphereSegment(osg::Vec3d(0.0, 0.0, 1.0),
			71000.0f, // radius
			osg::DegreesToRadians(0.0f),
			osg::DegreesToRadians(360.0f),
			osg::DegreesToRadians(-45.0f),
			osg::DegreesToRadians(-45.0f),
			4);

		ss1->setAllColors(osg::Vec4(0.5f, 1.0f, 1.0f, 0.5f));
		ss1->setSideColor(osg::Vec4(0.5f, 1.0f, 1.0f, 0.5f));
		ss1->addEventCallback(new AdjustSphereSegmentCallback);

		osgSim::SphereSegment * ss2 = new osgSim::SphereSegment(osg::Vec3d(0.0, 0.0, 1.0),
			0.807f, // radius
			osg::DegreesToRadians(0.0f),
			osg::DegreesToRadians(360.0f),
			osg::DegreesToRadians(-45.0f),
			osg::DegreesToRadians(-45.0f),
			4);

		ss2->setAllColors(osg::Vec4(0.5f, 1.0f, 1.0f, 0.5f));
		ss2->setSideColor(osg::Vec4(0.5f, 1.0f, 1.0f, 0.5f));
		ss1->setUpdateCallback(new MaskCallBack(ss2));
		ss2->setUpdateCallback(new MaskCallBack(ss1));

		mtfly->addChild(ss1);
		mtfly->addChild(ss2);

		//ss->setNodeMask(0);

		planeform = new osgEarth::GeoTransform();
		//xform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), 55.306, 32.041, 200));
		planeform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), 119, 26, 900));
		planeform->addChild(mtfly);

		osg::ref_ptr<osg::Vec4Array> vatemp = new osg::Vec4Array;
		vatemp->push_back(osg::Vec4(119, 26, 900, 30));
		vatemp->push_back(osg::Vec4(119, 26, 3600, 50));
		vatemp->push_back(osg::Vec4(119, 26, 31000, 100));
		vatemp->push_back(osg::Vec4(119.3, 26.5, 31000, 200));

		osg::AnimationPath *ap = new osg::AnimationPath;
		ap = CreateflyPath(vatemp);

		planeform->setUpdateCallback(new osg::AnimationPathCallback(ap, 0.0, 1.0));

		osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());

		Viewpoint vp = em->getViewpoint();
		vp.setNode(planeform);
		vp.range() = 5000.0;
		vp.pitch() = 180.0;
		em->setViewpoint(vp, 2.0);

		m_mapNode->addChild(planeform);

		//ballooncanvas = ControlCanvas::get(ui.openGLWidget->getViewer());
		ballooncanvas = new ControlCanvas();
		m_mapNode->addChild(ballooncanvas);

		Grid* grid = new Grid();
		grid->setBackColor(0, 0, 0, 0.5);
		grid->setMargin(10);
		grid->setPadding(10);
		grid->setChildSpacing(10);
		grid->setChildVertAlign(Control::ALIGN_CENTER);
		grid->setAbsorbEvents(true);
		grid->setVertAlign(Control::ALIGN_TOP);

		ballooncanvas->addControl(grid);

		//Add a label to display the distance
		// Add a text label:
		std::string name1 = "经度:";
		std::string _strWideName1;
		gb2312ToUtf8(name1, _strWideName1);
		osgText::Font* fonts = osgText::readFontFile("C:/Windows/Fonts/simhei.ttf");
		LabelControl* tlabel1 = new LabelControl(_strWideName1);
		tlabel1->setEncoding(osgText::String::ENCODING_UTF8);//设置本地编码
		tlabel1->setFont(fonts);

		std::string name2 = "纬度:";
		std::string _strWideName2;
		gb2312ToUtf8(name2, _strWideName2);
		LabelControl* tlabel2 = new LabelControl(_strWideName2);
		tlabel2->setEncoding(osgText::String::ENCODING_UTF8);//设置本地编码
		tlabel2->setFont(fonts);

		std::string name3 = "高度:";
		std::string _strWideName3;
		gb2312ToUtf8(name3, _strWideName3);
		LabelControl* tlabel3 = new LabelControl(_strWideName3);
		tlabel3->setEncoding(osgText::String::ENCODING_UTF8);//设置本地编码
		tlabel3->setFont(fonts);

		std::string name4 = "面积:";
		std::string _strWideName4;
		gb2312ToUtf8(name4, _strWideName4);
		LabelControl* tlabel4 = new LabelControl(_strWideName4);
		tlabel4->setEncoding(osgText::String::ENCODING_UTF8);//设置本地编码
		tlabel4->setFont(fonts);

		grid->setControl(0, 0, tlabel1);
		grid->setControl(0, 1, tlabel2);
		grid->setControl(0, 2, tlabel3);
		grid->setControl(0, 3, tlabel4);

		LabelControl* lonlabel = new LabelControl();
		lonlabel->setFont(osgEarth::Registry::instance()->getDefaultFont());
		lonlabel->setFontSize(18.0f);
		lonlabel->setHorizAlign(Control::ALIGN_LEFT);
		grid->setControl(1, 0, lonlabel);

		LabelControl* latlabel = new LabelControl();
		latlabel->setFont(osgEarth::Registry::instance()->getDefaultFont());
		latlabel->setFontSize(18.0f);
		latlabel->setHorizAlign(Control::ALIGN_LEFT);
		grid->setControl(1, 1, latlabel);

		LabelControl* altlabel = new LabelControl();
		altlabel->setFont(osgEarth::Registry::instance()->getDefaultFont());
		altlabel->setFontSize(18.0f);
		altlabel->setHorizAlign(Control::ALIGN_LEFT);
		grid->setControl(1, 2, altlabel);

		LabelControl* arealabel = new LabelControl();
		arealabel->setFont(osgEarth::Registry::instance()->getDefaultFont());
		arealabel->setFontSize(18.0f);
		arealabel->setHorizAlign(Control::ALIGN_LEFT);
		grid->setControl(1, 3, arealabel);

		std::string name5 = "卫星参数:WorldView-1\n\n演示焦距:0.807m\n\n水平视场角:97.91°\n\n垂直视场角:80.56°";
		std::string _strWideName5;
		gb2312ToUtf8(name5, _strWideName5);
		LabelControl* namelabel = new LabelControl(_strWideName5);
		namelabel->setMargin(10);
		namelabel->setPadding(10);
		namelabel->setEncoding(osgText::String::ENCODING_UTF8);//设置本地编码
		namelabel->setFont(fonts);
		namelabel->setFontSize(18.0f);
		namelabel->setSize(200, 400);
		//namelabel->setText("WorldView-1");
		namelabel->setHorizAlign(Control::ALIGN_RIGHT);
		namelabel->setVertAlign(Control::ALIGN_TOP);
		namelabel->setBackColor(0, 0, 0, 0.5);
		ballooncanvas->addControl(namelabel);

		labelEvent = new LabelEvent(lonlabel, latlabel, altlabel, arealabel);

		ui.openGLWidget->getViewer()->addEventHandler(labelEvent);
	}
	else 
	{
		m_mapNode->removeChild(ballooncanvas);
		m_mapNode->removeChild(planeform);
		ui.openGLWidget->getViewer()->removeEventHandler(labelEvent);
	}
}
/*---------------------------------------------------------------添加倾斜摄影模型--------------------------------------------------*/
void MyOSGEarthQT::on_AddTiltphotographymodel(bool checked)
{
	if (checked)
	{
		std::string filePath = "E:\\BaiduNetdiskDownload\\OSG\\Data\\osgb\\Data\\Model\\Model.osgb";
		osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filePath);
		
		tpxform->addChild(node);
		tpxform->setTerrain(m_mapNode->getTerrain());
		osgEarth::GeoPoint point(m_mapNode->getMap()->getSRS(), 119.19771, 26.05517, -15);                 //使用绝对高，正高
		//osgEarth::GeoPoint point(map->getSRS(), 119.19771, 26.05517, 50);
		tpxform->setPosition(point);

		m_mapNode->addChild(tpxform);
		osgEarth::Registry::shaderGenerator().run(node);

		osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		em->setViewpoint(osgEarth::Viewpoint(NULL, 119.19771, 26.05517, 500, -2.5, -10, 3450), 2);
	}
	else
	{
		m_mapNode->removeChild(tpxform);
		return;
	}
}
/*---------------------------------------------------------------全球雾效--------------------------------------------------*/
osg::StateAttribute* createHaze()
{
	char s_hazeVertShader[] =
		"#version " GLSL_VERSION_STR "\n"
		"out vec3 v_pos; \n"
		"void setup_haze(inout vec4 VertexVIEW) \n"
		"{ \n"
		"    v_pos = vec3(VertexVIEW); \n"
		"} \n";
	char s_hazeFragShader[] =
		"#version " GLSL_VERSION_STR "\n"
		"in vec3 v_pos; \n"
		"void apply_haze(inout vec4 color) \n"
		"{ \n"
		"    float dist = clamp( length(v_pos)/3e4, 0.0, 0.75 ); \n"
		"    color = mix(color, vec4(1.0, 1.0, 1.0, 1.0), dist); \n"
		"} \n";

	osgEarth::VirtualProgram* vp = new osgEarth::VirtualProgram();
	vp->setFunction("setup_haze", s_hazeVertShader, osgEarth::ShaderComp::LOCATION_VERTEX_VIEW);
	vp->setFunction("apply_haze", s_hazeFragShader, osgEarth::ShaderComp::LOCATION_FRAGMENT_LIGHTING);
	vp->setShaderLogging(true, "shaders.txt");
	return vp;
}
void MyOSGEarthQT::on_FogEffect(bool checked)
{
	if (checked)
	{
		g = new osg::Group();
		g->addChild(m_mapNode);
		g->getOrCreateStateSet()->setAttributeAndModes(createHaze(), osg::StateAttribute::ON);
		m_world->addChild(g);
	}
	else
	{
		m_world->removeChild(g);
		return;
	}
}
/*---------------------------------------------------------------点火源--------------------------------------------------*/
void MyOSGEarthQT::on_Fire(bool checked)
{
	if (checked)
	{
		m_PickEvent->setActionEvent(EnumActionEvent::Fire);
	}
	else
	{
		m_PickEvent->setActionEvent(EnumActionEvent::ActionNull);
		ui.VisibilityAnalysis->setChecked(false);//不用传入对应信号
		return;
	}
	ui.VisibilityAnalysis->setChecked(false);
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
}
/*---------------------------------------------------------------雨效--------------------------------------------------*/
void MyOSGEarthQT::on_Rain(bool checked)
{
	osg::Vec3 position(119, 26, 0);
	if (checked)
	{
		rainxform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), position.x(), position.y(), 0));
		rainpre->rain(1.0);
		rainpre->setParticleSize(0.05);
		rainpre->setParticleColor(osg::Vec4(1, 1, 1, 1));

		rainxform->addChild(rainpre);
		m_mapNode->addChild(rainxform);
	}
	else
	{
		m_mapNode->removeChild(rainxform);
		return;
	}
}
/*---------------------------------------------------------------雪效--------------------------------------------------*/
void MyOSGEarthQT::on_Snow(bool checked)
{
	osg::Vec3 position(119, 26, 0);
	if (checked)
	{
		snowxform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), position.x(), position.y(), 0));
		pre->snow(1.0);
		pre->setParticleSize(0.05);
		pre->setParticleColor(osg::Vec4(1, 1, 1, 1));

		snowxform->addChild(pre);
		m_mapNode->addChild(snowxform);
	}
	else
	{
		m_mapNode->removeChild(snowxform);
		return;
	}
}
/*---------------------------------------------------------------小云层--------------------------------------------------*/
osg::Image* makeGlow(int width, int height, float expose, float sizeDisc)
{
	osg::ref_ptr<osg::Image> image = new osg::Image;
	image->allocateImage(width, height, 1, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE);

	unsigned char* data = image->data();
	for (int y = 0; y < height; ++y)
	{
		float dy = (y + 0.5f) / height - 0.5f;
		for (int x = 0; x < width; ++x)
		{
			float dx = (x + 0.5f) / width - 0.5f;
			float dist = sqrtf(dx*dx + dy * dy);
			float intensity = 2 - osg::minimum(2.0f, powf(2.0f, osg::maximum(dist - sizeDisc, 0.0f) * expose));
			float noise = 1.0f;

			unsigned char color = (unsigned char)(noise * intensity * 255.0f + 0.5f);
			*(data++) = color;
			*(data++) = color;
		}
	}
	return image.release();
}
void readCloudCells(CloudBlock::CloudCells& cells, const std::string& file)
{
	std::ifstream is(file.c_str());
	if (!is) return;

	double x, y, z;
	unsigned int density, brightness;
	while (!is.eof())
	{
		is >> x >> y >> z >> density >> brightness;

		CloudBlock::CloudCell cell;
		cell._pos.set(x, y, z);
		cell._density = (float)density;
		cell._brightness = (float)brightness;
		cells.push_back(cell);
	}
}
void MyOSGEarthQT::on_Cloud(bool checked)
{
	osg::Vec3 position(119.19771, 26.05517, 50);
	if (checked)
	{
		cloudxform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), position.x(), position.y(), 200));

		
		CloudBlock::CloudCells cells;
		readCloudCells(cells, "E:/BaiduNetdiskDownload/OSG/qt/osgRecipes-master/cookbook/data/data.txt");

		osg::ref_ptr<CloudBlock> clouds = new CloudBlock;
		clouds->setCloudCells(cells);

		osg::StateSet* ss = clouds->getOrCreateStateSet();
		ss->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
		ss->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
		ss->setTextureAttributeAndModes(0, new osg::Texture2D(makeGlow(512, 512, 2.0f, 0.0f)));

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		geode->addDrawable(clouds.get());
		osgEarth::Registry::shaderGenerator().run(geode);
		//cloudxform->setMatrix(osg::Matrixd::scale(1000, 1000, 1000));

		ui.openGLWidget->getViewer()->setLightingMode(osg::View::SKY_LIGHT);
		cloudxform->addChild(geode);
		
		m_mapNode->addChild(cloudxform);

		osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		em->setViewpoint(osgEarth::Viewpoint(NULL, 119.19771, 26.05517, 500, -2.5, -10, 3450), 2);
	}
	else
	{
		m_mapNode->removeChild(cloudxform);
		return;
	}
}
/*---------------------------------------------------------------添加城市---------------------------------------------------*/
void addBuildings(Map* map)
{
	// create a feature source to load the building footprint shapefile.
	osgEarth::Drivers::OGRFeatureOptions buildingData;
	buildingData.name() = "buildings";
	buildingData.url() = "E:\\BaiduNetdiskDownload\\OSG\\Data\\boston_buildings_utm19.shp";
	buildingData.buildSpatialIndex() = true;

	// a style for the building data:
	Style buildingStyle;
	buildingStyle.setName("buildings");

	// Extrude the shapes into 3D buildings.
	ExtrusionSymbol* extrusion = buildingStyle.getOrCreate<ExtrusionSymbol>();
	extrusion->heightExpression() = NumericExpression("3.5 * max( [story_ht_], 1 )");
	extrusion->flatten() = true;
	extrusion->wallStyleName() = "building-wall";
	extrusion->roofStyleName() = "building-roof";

	PolygonSymbol* poly = buildingStyle.getOrCreate<PolygonSymbol>();
	//poly->fill()->color() = Color(Color::Blue,0.3f);
	poly->fill()->color() = Color(Color::White);

	// Clamp the buildings to the terrain.
	AltitudeSymbol* alt = buildingStyle.getOrCreate<AltitudeSymbol>();
	alt->clamping() = alt->CLAMP_TO_TERRAIN;
	alt->binding() = alt->BINDING_VERTEX;
	
	// a style for the wall textures:
	Style wallStyle;
	wallStyle.setName("building-wall");
	SkinSymbol* wallSkin = wallStyle.getOrCreate<SkinSymbol>();
	wallSkin->library() = "us_resources";
	wallSkin->addTag("building");
	wallSkin->randomSeed() = 1;

	// a style for the rooftop textures:
	Style roofStyle;
	roofStyle.setName("building-roof");
	SkinSymbol* roofSkin = roofStyle.getOrCreate<SkinSymbol>();
	roofSkin->library() = "us_resources";
	roofSkin->addTag("rooftop");
	roofSkin->randomSeed() = 1;
	roofSkin->isTiled() = true;

	// assemble a stylesheet and add our styles to it:
	StyleSheet* styleSheet = new StyleSheet();
	styleSheet->addStyle(buildingStyle);
	styleSheet->addStyle(wallStyle);
	styleSheet->addStyle(roofStyle);

	// load a resource library that contains the building textures.
	ResourceLibrary* reslib = new ResourceLibrary("us_resources", "E:\\BaiduNetdiskDownload\\OSG\\Data\\resources\\textures_us\\catalog.xml");
	styleSheet->addResourceLibrary(reslib);

	// set up a paging layout for incremental loading. The tile size factor and
	// the visibility range combine to determine the tile size, such that
	// tile radius = max range / tile size factor.
	FeatureDisplayLayout layout;
	layout.tileSize() = 500;
	layout.addLevel(FeatureLevel(0.0f, 20000.0f, "buildings"));

	FeatureModelLayer* layer = new FeatureModelLayer();
	layer->setName("Buildings");
	layer->options().featureSource() = buildingData;
	layer->options().styles() = styleSheet;
	layer->options().layout() = layout;

	map->addLayer(layer);
}
void addStreets(Map* map)
{
	// create a feature source to load the street shapefile.
	osgEarth::Drivers::OGRFeatureOptions feature_opt;
	feature_opt.name() = "streets";
	feature_opt.url() = "E:\\BaiduNetdiskDownload\\OSG\\Data\\boston-scl-utm19n-meters.shp";
	feature_opt.buildSpatialIndex() = true;

	// a resampling filter will ensure that the length of each segment falls
	// within the specified range. That can be helpful to avoid cropping 
	// very long lines segments.
	ResampleFilterOptions resample;
	resample.minLength() = 0.0f;
	resample.maxLength() = 25.0f;
	feature_opt.filters().push_back(resample);

	// a style:
	Style style;
	style.setName("streets");

	// Render the data as translucent yellow lines that are 7.5m wide.
	LineSymbol* line = style.getOrCreate<LineSymbol>();
	line->stroke()->color() = Color(Color::Yellow, 0.5f);
	line->stroke()->width() = 5.5f;
	line->stroke()->widthUnits() = Units::METERS;

	// Clamp the lines to the terrain.
	AltitudeSymbol* alt = style.getOrCreate<AltitudeSymbol>();
	alt->clamping() = alt->CLAMP_TO_TERRAIN;

	// Apply a depth offset to avoid z-fighting. The "min bias" is the minimum
	// apparent offset (towards the camera) of the geometry from its actual position.
	// The value here was chosen empirically by tweaking the "oe_doff_min_bias" uniform.
	RenderSymbol* render = style.getOrCreate<RenderSymbol>();
	render->depthOffset()->minBias() = 6.6f;

	// Set up a paging layout. The tile size factor and the visibility range combine
	// to determine the tile size, such that tile radius = max range / tile size factor.
	FeatureDisplayLayout layout;
	layout.tileSize() = 500;
	layout.maxRange() = 5000.0f;

	// create a model layer that will render the buildings according to our style sheet.
	FeatureModelLayerOptions streets;
	streets.name() = "streets";
	streets.featureSource() = feature_opt;
	streets.layout() = layout;
	streets.styles() = new StyleSheet();
	streets.styles()->addStyle(style);

	map->addLayer(new FeatureModelLayer(streets));
}
void addParks(Map* map)
{
	// create a feature source to load the shapefile.
	osgEarth::Drivers::OGRFeatureOptions parksData;
	parksData.name() = "parks";
	parksData.url() = "E:\\BaiduNetdiskDownload\\OSG\\Data\\boston-parks.shp";
	parksData.buildSpatialIndex() = true;

	// a style:
	Style style;
	style.setName("parks");

	// Render the data using point-model substitution, which replaces each point
	// in the feature geometry with an instance of a 3D model. Since the input
	// data are polygons, the PLACEMENT_RANDOM directive below will scatter
	// points within the polygon boundary at the specified density.
	ModelSymbol* model = style.getOrCreate<ModelSymbol>();
	model->url()->setLiteral("E:\\BaiduNetdiskDownload\\OSG\\Data\\loopix\\tree4.osgb");
	model->placement() = model->PLACEMENT_RANDOM;
	model->density() = 3000.0f; // instances per sqkm
	model->scale() = 0.3;

	// Clamp to the terrain:
	AltitudeSymbol* alt = style.getOrCreate<AltitudeSymbol>();
	alt->clamping() = alt->CLAMP_TO_TERRAIN;

	// Since the tree model contains alpha components, we will discard any data
	// that's sufficiently transparent; this will prevent depth-sorting anomalies
	// common when rendering lots of semi-transparent objects.
	RenderSymbol* render = style.getOrCreate<RenderSymbol>();
	render->transparent() = true;
	render->minAlpha() = 0.15f;

	// Set up a paging layout. The tile size factor and the visibility range combine
	// to determine the tile size, such that tile radius = max range / tile size factor.
	FeatureDisplayLayout layout;
	layout.tileSize() = 650;
	layout.addLevel(FeatureLevel(0.0f, 2000.0f, "parks"));

	// create a model layer that will render the buildings according to our style sheet.
	FeatureModelLayerOptions parks;
	parks.name() = "parks";
	parks.featureSource() = parksData;
	parks.layout() = layout;
	parks.styles() = new StyleSheet();
	parks.styles()->addStyle(style);

	Layer* parksLayer = new FeatureModelLayer(parks);
	map->addLayer(parksLayer);

	if (parksLayer->getStatus().isError())
	{
		OE_WARN << parksLayer->getStatus().message() << std::endl;
	}
}
void MyOSGEarthQT::on_AddCityModel(bool checked)
{
	if (checked)
	{
		Map* map = new Map();

		map = m_mapNode->getMap();

		addBuildings(map);
		addStreets(map);
		addParks(map);

		osgEarth::Util::ShadowCaster* _shadowCaster = new osgEarth::Util::ShadowCaster;
		_shadowCaster->getShadowCastingGroup()->addChild(m_mapNode->getLayerNodeGroup());
		_shadowCaster->setLight(ui.openGLWidget->getViewer()->getLight());
		_shadowCaster->addChild(m_mapNode);
		m_world->addChild(_shadowCaster);

		osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		em->setViewpoint(osgEarth::Viewpoint(NULL, -71.076262, 42.34425, 2060.66, -2.5, -10, 3450), 2);
	}
	else
	{
		osg::ref_ptr<osgEarth::Features::FeatureModelLayer> Buildings = dynamic_cast<osgEarth::Features::FeatureModelLayer*>(m_mapNode->getMap()->getLayerByName("Buildings"));
		m_mapNode->getMap()->removeLayer(Buildings.get());

		osg::ref_ptr<osgEarth::Features::FeatureModelLayer> streets = dynamic_cast<osgEarth::Features::FeatureModelLayer*>(m_mapNode->getMap()->getLayerByName("streets"));
		m_mapNode->getMap()->removeLayer(streets.get());

		osg::ref_ptr<osgEarth::Features::FeatureModelLayer> parks = dynamic_cast<osgEarth::Features::FeatureModelLayer*>(m_mapNode->getMap()->getLayerByName("parks"));
		m_mapNode->getMap()->removeLayer(parks.get());
		
		return;
	}
}
/*-------------------------------------------------------漫游路径（改变姿态参数）--------------------------------------------*/
osg::AnimationPath* CreateflyPath2(osg::Vec4Array *ctrl)
{
	osg::Matrix TurnMatrix;
	double Hz_Angle;
	double Vt_Anglle;
	double time = 0;

	double lon, lat, height;

	//旋转4元素,旋转角度
	osg::Quat _rotation;

	//世界坐标系内的两点
	osg::Vec3d positionCur;
	osg::Vec3d positionNext;

	osg::ref_ptr<osg::AnimationPath>animationPath = new osg::AnimationPath;
	//设置是否循环飞行
	animationPath->setLoopMode(osg::AnimationPath::LOOP);

	//循环读取预设点      迭代器
	for (osg::Vec4Array::iterator iter = ctrl->begin(); iter != ctrl->end(); iter++)
	{
		//iter2为iter的下一个点
		osg::Vec4Array::iterator iter2 = iter;
		iter2++;

		//如果只有两个点，跳出循环
		if (iter2 == ctrl->end())
			break;

		double x, y, z;

		osg::ref_ptr<osg::CoordinateSystemNode> csn1 = new osg::CoordinateSystemNode;
		csn1->setEllipsoidModel(new osg::EllipsoidModel);
		//把经纬度转换为绝对世界坐标系   csn->获取椭球体模型    latitude纬度  longtitude经度
		csn1->getEllipsoidModel()->convertLatLongHeightToXYZ(osg::DegreesToRadians(iter->y()), osg::DegreesToRadians(iter->x()), iter->z(), x, y, z);
		positionCur = osg::Vec3(x, y, z);
		csn1->getEllipsoidModel()->convertXYZToLatLongHeight(x, y, z, lat, lon, height);
		csn1->getEllipsoidModel()->convertLatLongHeightToXYZ(osg::DegreesToRadians(iter2->y()), osg::DegreesToRadians(iter2->x()), iter2->z(), x, y, z);
		positionNext = osg::Vec3(x, y, z);
		//求出水平夹角
		//如果经度相同
		if (iter->x() == iter2->x())
		{
			//如果两点经度相同，水平夹角为90度
			Hz_Angle = osg::PI_2;
		}
		else
		{
			//atan（纬度/经度）
			Hz_Angle = atan((iter2->y() - iter->y()) / (iter2->x() - iter->x()));
			if (iter2->x() > iter->x())
			{
				Hz_Angle += osg::PI;
			}
		}

		//求垂直夹角
		if (iter->z() == iter2->z())
		{
			Vt_Anglle = 0;
		}
		else
		{
			//如果如果两点在同一经维度，只有高度不同时  sqrt[(x2-x2)**2 + (y2-y1)**2 + (z2-z1)**2 - (z2-z1)**2]=0
			if (0 == sqrt(((positionNext.x() - positionCur.x())* (positionNext.x() - positionCur.x()) + (positionNext.y() - positionCur.y())*(positionNext.y() - positionCur.y()) + (positionNext.z() - positionCur.z())*(positionNext.z() - positionCur.z())) - pow((positionNext.z() - positionCur.z()), 2)))
			{
				Vt_Anglle = osg::PI_2;
			}
			//如果经纬度不同
			else
			{
				//                 垂直高度/底边
				//垂直角度   atan{(z2-z1) / (sqrt[(x2-x2)**2 + (y2-y1)**2 + (z2-z1)**2 - (z2-z1)**2])}
				Vt_Anglle = atan((iter2->z() - iter->z()) / (sqrt(((positionNext.x() - positionCur.x())* (positionNext.x() - positionCur.x()) + (positionNext.y() - positionCur.y())*(positionNext.y() - positionCur.y()) + (positionNext.z() - positionCur.z())*(positionNext.z() - positionCur.z())) - pow((positionNext.z() - positionCur.z()), 2))));
			}
			if (Vt_Anglle >= osg::PI_2)
			{
				Vt_Anglle = osg::PI_2;
			}
			if (Vt_Anglle <= -osg::PI_2)
			{
				Vt_Anglle = -osg::PI_2;
			}
		}
		//1弧度=57.3°
		//std::cout << "shuiping=" << osg::RadiansToDegrees(shuipingAngle)<< "   chuizhi=" << osg::RadiansToDegrees(chuizhiAnglle)<< std::endl;
		//前面求出了第一个点处飞机的朝向，接下来需要求朝向所对应的变换矩阵(TurnMatrix)，求出此变换矩阵后，和飞机矩阵相乘，就能形成飞机转向的动作
		//求出飞机世界坐标系在经纬高度坐标系下的值
		csn1->getEllipsoidModel()->computeLocalToWorldTransformFromLatLongHeight(osg::DegreesToRadians(iter->y()), osg::DegreesToRadians(iter->x()), iter->z(), TurnMatrix);
		//飞机转向角          绕x轴 不用转                               绕y轴旋转                            绕z轴旋转
		_rotation.makeRotate(0, osg::Vec3(1.0, 0.0, 0.0), Vt_Anglle, osg::Vec3(0.0, 1.0, 0.0), Hz_Angle, osg::Vec3(0.0, 0.0, 1.0));
		//把旋转矩阵乘到飞机的变换矩阵上
		TurnMatrix.preMultRotate(_rotation);

		//生成插入点，ControlPoint包含模型位置和朝向     生成第一个点的AnimationPath
		animationPath->insert(time, osg::AnimationPath::ControlPoint(positionCur, TurnMatrix.getRotate()));

		//求下一个点的时间
		//      斜边长度=sqrt[(x2-x2)**2 + (y2-y1)**2 + (z2-z1)**2]
		double dist = sqrt(((positionNext.x() - positionCur.x())* (positionNext.x() - positionCur.x()) + (positionNext.y() - positionCur.y())*(positionNext.y() - positionCur.y()) + (positionNext.z() - positionCur.z())*(positionNext.z() - positionCur.z())));
		//std::cout << "距离=" << dist << std::endl;
		//如果速度等于0，飞机到达该点时间无限大
		if (iter2->w() == 0)
		{
			time += 100000000;
		}
		//如果速度不等于0，飞机到达该点时间等于 距离/速度
		else
		{
			time += (dist / iter2->w());
		}
	}
	//生成最后一个点的AnimationPath
	animationPath->insert(time, osg::AnimationPath::ControlPoint(positionNext, TurnMatrix.getRotate()));
	//std::cout << "时间=" << time << std::endl;
	return animationPath.release();
}
/*---------------------------------------------------------------彩带尾迹回调--------------------------------------------------*/
class Trailercallback : public osg::NodeCallback
{
public:
	Trailercallback(osg::Geometry* ribbon, int size)
	{
		_ribbon = ribbon;
		this->size = size;
	}
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		osg::MatrixTransform* trans = dynamic_cast<osg::MatrixTransform*>(node);
		if (trans && _ribbon.valid())
		{
			osg::Matrix matrix = trans->getMatrix();
			osg::Vec3Array* vectex = dynamic_cast<osg::Vec3Array*>(_ribbon->getVertexArray());

			for (unsigned int i = 0; i < size - 3; i += 2)
			{
				(*vectex)[i] = (*vectex)[i + 2];
				(*vectex)[i + 1] = (*vectex)[i + 3];
			}
			(*vectex)[size - 2] = osg::Vec3(0.0f, -10, 0.0f)*matrix;
			(*vectex)[size - 1] = osg::Vec3(0.0f, 10, 0.0f)*matrix;

			vectex->dirty();
			_ribbon->dirtyBound();
		}
	}
private:
	osg::observer_ptr<osg::Geometry> _ribbon;
	int size;
};
/*---------------------------------------------------------------路径漫游--------------------------------------------------*/
void MyOSGEarthQT::on_PathRoaming(bool checked)
{
	//labelEvent = 0;
	if (checked)
	{
		osg::Matrix TurnMatrixinit;

		//旋转4元素,旋转角度
		osg::Quat _rotation;

		osg::ref_ptr< osg::Node >  airport = osgDB::readRefNodeFile("E:\\BaiduNetdiskDownload\\OSG\\Data\\Airport.ive");
		osgEarth::Registry::shaderGenerator().run(airport);

		osg::ref_ptr<osg::MatrixTransform> mtairport = new osg::MatrixTransform;
		TurnMatrixinit.preMult(osg::Matrix::rotate(osg::inDegrees(-30.0), osg::Vec3(0, 0, 1)));
		mtairport->setMatrix(TurnMatrixinit);
		mtairport->addChild(airport);

		airportform = new osgEarth::GeoTransform();
		//airportform->setTerrain(m_mapNode->getTerrain());
		airportform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), 119.646946, 25.906602, 10));
		airportform->addChild(mtairport);

		m_mapNode->addChild(airportform);

		//osg::ref_ptr< osg::Node >  plane = osgDB::readRefNodeFile("E:\\BaiduNetdiskDownload\\OSG\\Data\\B737.ive");
		//osg::ref_ptr< osg::Node >  plane = osgDB::readRefNodeFile("E:\\BaiduNetdiskDownload\\OSG\\Data\\WZ.ive");
		osg::ref_ptr< osg::Node >  plane = osgDB::readRefNodeFile("E:\\BaiduNetdiskDownload\\OSG\\Data\\J10.ive");
		//osg::ref_ptr< osg::Node >  plane = osgDB::readRefNodeFile("C:\\Users\\12775\\Desktop\\Balloon\\Air_Balloon.3ds");
		//osg::ref_ptr< osg::Node >  plane = osgDB::readRefNodeFile("C:\\Users\\12775\\Desktop\\mongolfire\\airballoon.obj");
		//osg::ref_ptr< osg::Node >  plane = osgDB::readRefNodeFile("E:\\BaiduNetdiskDownload\\OSG\\Data\\spaceship.osgt");
		//osg::ref_ptr< osg::Node >  plane = osgDB::readRefNodeFile("C:\\Users\\12775\\Desktop\\Balloon\\1.osg");
		osgEarth::Registry::shaderGenerator().run(plane);

		osg::ref_ptr<osg::MatrixTransform> mtfly = new osg::MatrixTransform;
		TurnMatrixinit.preMult(osg::Matrix::rotate(osg::inDegrees(120.0), osg::Vec3(0, 0, 1)));
		mtfly->setMatrix(TurnMatrixinit);
		mtfly->addChild(plane);

		planeform = new osgEarth::GeoTransform();
		//xform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), 55.306, 32.041, 200));
		//planeform->setPosition(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), 119.664707, 25.937581, 50));
		planeform->addChild(mtfly);

		///*-------------------火焰尾迹---------------------*/
		/*osg::ref_ptr<osgParticle::FireEffect> fire = new osgParticle::FireEffect(osg::Vec3(0, 0, 0));
		fire->setUseLocalParticleSystem(false);
		mtfly->addChild(fire);

		osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		geode->addDrawable(fire->getParticleSystem());
		m_mapNode->addChild(geode);*/

		/*-------------------彩带尾迹---------------------*/
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		
		osg::ref_ptr<osg::Vec3Array> vectex = new osg::Vec3Array(512);
		osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(512);

		for (unsigned int i = 0; i < 512; i+=2)
		{
			(*vectex)[i] = osg::Vec3(0, 0, 0);
			(*vectex)[i+1] = osg::Vec3(0, 0, 0);

			float alpha = sinf(osg::PI*(float)i / (float)512);
			(*colors)[i] = osg::Vec4(osg::Vec3(1.0, 0.0, 1.0), alpha);
			(*colors)[i+1] = osg::Vec4(osg::Vec3(1.0, 0.0, 1.0), alpha);
		}
		geom->setDataVariance(osg::Object::DYNAMIC);
		geom->setUseDisplayList(false);
		geom->setUseVertexBufferObjects(true);

		geom->setVertexArray(vectex);

		geom->setColorArray(colors);
		geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
		geom->addPrimitiveSet(new osg::DrawArrays(GL_QUAD_STRIP, 0, 512));

		ribbongeode = new osg::Geode;
		ribbongeode->addDrawable(geom);
		geom->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		geom->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		mtfly->addUpdateCallback(new Trailercallback(geom, 512));

		mtfly->addChild(ribbongeode);
		m_mapNode->addChild(ribbongeode);

		osg::ref_ptr<osg::Vec4Array> vatemp = new osg::Vec4Array;
		vatemp->push_back(osg::Vec4(119.664707, 25.937581, 15, 30));
		vatemp->push_back(osg::Vec4(119.646946, 25.906602, 50, 50));
		vatemp->push_back(osg::Vec4(119.588453, 25.794919, 100, 100));
		vatemp->push_back(osg::Vec4(119.535971, 25.755275, 500, 200));
		vatemp->push_back(osg::Vec4(116.0, 59.0, 60.0, 11.0));

		//要加读取csv路径文件进行漫游，也很简单，读取每行每列的数据加入vatemp中
		/*
		#include <iostream>  
		#include <string>  
		#include <vector>  
		#include <fstream>  
		#include <sstream>  

		ifstream inFile("E:\\BaiduNetdiskDownload\\OSG\\qt\\4.22\\osgearth-master\\DigitalEarth\\orbit11.csv", ios::in);
		string lineStr;
		vector<vector<string>> strArray;
		double array[361][3];
		int i, j;
		i = 0;
		char* end;
		if (inFile.fail())
			cout << "读取文件失败" << endl;
		while (getline(inFile, lineStr))
		{
			j = 0;
			// 存成二维表结构  
			stringstream ss(lineStr);
			string str;
			vector<string> lineArray;
			// 按照逗号分隔  
			while (getline(ss, str, ','))
			{
				array[i][j] = static_cast<double>(strtol(str.c_str(), &end, 10));              //string -> int
				j++;
			}
			i++;
		}
		for (int i = 0; i < 361; i++)
		{
			for (int j = 0; j < 3; j++)
				cout<<array[i][j];
			cout << endl;
		}

		osg::AnimationPath *ap = new osg::AnimationPath;
		ap = CreateflyPath(vatemp);

		planeform->setUpdateCallback(new osg::AnimationPathCallback(ap, 0.0, 1.0));  //AnimationPath* ap,double timeOffset=0.0,double timeMultiplier=1.0

		//planeform->addUpdateCallback(new Trailercallback(geom, 512));

		//m_mapNode->addChild(ribbongeode);

		/*---------------------------*/
		//osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(10);
		//for (unsigned int i = 0; i < 10; ++i)
		//	(*vertices)[i].set(float(i), 0.0, 0.0);

		//osg::ref_ptr<osg::Geometry> lineGeom = new osg::Geometry;
		//lineGeom->setVertexArray(vertices.get());
		//lineGeom->addPrimitiveSet(new osg::DrawArrays(osg::DrawArrays::LINE_STRIP, 0, 10));

		////lineGeom->setInitialBound(osg::BoundingBox(osg::Vec3(-10.0, -10.0, -10.0), osg::Vec3(10.0, 10.0, 10.0)));
		//lineGeom->setUpdateCallback(new DynamicLineCallback);
		//lineGeom->setUseDisplayList(false);
		//lineGeom->setUseVertexBufferObjects(true);

		//osg::ref_ptr<osg::Geode> geode = new osg::Geode;
		//geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		//geode->getOrCreateStateSet()->setAttribute(new osg::LineWidth(2.0));
		//geode->addDrawable(lineGeom.get());

		//planeform->addChild(geode);
		/*osg::Vec3d pos1 = planeform->getMatrix().getTrans();
		std::cout << pos1 << std::endl;*/
		/*---------------------------*/
		osg::AnimationPath *ap = new osg::AnimationPath;
		ap = CreateflyPath2(vatemp);

		planeform->setUpdateCallback(new osg::AnimationPathCallback(ap, 0.0, 1.0));

		osg::ref_ptr<osgEarth::Util::EarthManipulator> emp = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		//emp->setNode(planeform);

		Viewpoint vp = emp->getViewpoint();
		vp.setNode(planeform);
		vp.range() = 5000.0;
		vp.pitch() = -90.0;
		emp->setViewpoint(vp, 2.0);

		m_mapNode->addChild(planeform);

		
		
		/*m_world->addChild(osgEarth::Util::Controls::ControlCanvas::get(ui.openGLWidget->getViewer()));
		ControlCanvas* canvas = ControlCanvas::get(ui.openGLWidget->getViewer());
		
		LabelControl* label = new LabelControl();
		label->setHorizAlign(Control::ALIGN_LEFT);
		label->setVertAlign(Control::ALIGN_TOP);
		label->setBackColor(0, 0, 0, 0.5);
		label->setMargin(10);
		label->setSize(400,50);

		canvas->addControl(label);*/

		/*labelEvent = new LabelEvent(label, m_mapNode);
		
		ui.openGLWidget->getViewer()->addEventHandler(labelEvent);*/
	}
	else
	{
		m_mapNode->removeChild(airportform);
	    m_mapNode->removeChild(ribbongeode);
		m_mapNode->removeChild(planeform);
		ui.PathRoaming->setChecked(false);
		return;
	}
}
/*---------------------------------------------------------------3D距离测量--------------------------------------------------*/
void MyOSGEarthQT::on_Distance(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		Distanceevent = new DistanceCaculator(m_mapNode->getMapSRS()->getGeographicSRS(), m_losGroup, m_mapNode);

		ui.openGLWidget->getViewer()->addEventHandler(Distanceevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(Distanceevent);
		ui.Distance->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.Area->setChecked(false);
	//ui.slopeAnalysis->setChecked(false);
	//ui.AspectAnalysis->setChecked(false);
	ui.Angle->setChecked(false);
}
/*---------------------------------------------------------------面积测量--------------------------------------------------*/
void MyOSGEarthQT::on_Area(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		Areaevent = new AreaCaculator(m_mapNode->getMapSRS()->getGeographicSRS(), m_losGroup, m_mapNode);

		ui.openGLWidget->getViewer()->addEventHandler(Areaevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(Areaevent);
		ui.Area->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.Distance->setChecked(false);
	//ui.slopeAnalysis->setChecked(false);
	//ui.AspectAnalysis->setChecked(false);
	ui.Angle->setChecked(false);
}
/*---------------------------------------------------------------角度测量--------------------------------------------------*/
void MyOSGEarthQT::on_Angle(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		Angleevent = new AngleCaculator(m_mapNode->getMapSRS()->getGeographicSRS(), m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(Angleevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(Angleevent);
		ui.Angle->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.Distance->setChecked(false);
	ui.Area->setChecked(false);
	//ui.slopeAnalysis->setChecked(false);
	//ui.AspectAnalysis->setChecked(false);
	ui.Area->setChecked(false);
}
/*---------------------------------------------------------------坡度分析--------------------------------------------------*/
struct MyScalarPrinter : public osgSim::ScalarBar::ScalarPrinter
{
	//0、0.5、1的位置显示" Bottom"、" Middle"、" Top"
	std::string printScalar(float scalar)
	{
		if (scalar == 0.0f) return osgSim::ScalarBar::ScalarPrinter::printScalar(scalar);
		else if (scalar == 0.5f) return osgSim::ScalarBar::ScalarPrinter::printScalar(scalar);
		else if (scalar == 1.0f) return osgSim::ScalarBar::ScalarPrinter::printScalar(scalar);
		else return osgSim::ScalarBar::ScalarPrinter::printScalar(scalar);
	}
};
osg::Node * createScalarBar_HUD()
{
	//osgSim::ScalarBar * geode = new osgSim::ScalarBar;
	std::vector<osg::Vec4> cs;
	cs.push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));   // B
	cs.push_back(osg::Vec4(0.0f, 1.0f, 1.0f, 1.0f));   // C
	cs.push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));   // G
	cs.push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));   // Y
	cs.push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));   // R

	osgSim::ColorRange* cr = new osgSim::ColorRange(0.0f, 1.0f, cs);
	osgSim::ScalarBar* geode = new osgSim::ScalarBar(5, 3, cr,
		"",
		osgSim::ScalarBar::HORIZONTAL,
		0.1f, new MyScalarPrinter);
	geode->setScalarPrinter(new MyScalarPrinter);

	osgSim::ScalarBar::TextProperties tp;
	tp._fontFile = "fonts/times.ttf";
	geode->setTextProperties(tp);
	osg::StateSet * stateset = geode->getOrCreateStateSet();
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	stateset->setRenderBinDetails(11, "RenderBin");

	osg::MatrixTransform * modelview = new osg::MatrixTransform;
	modelview->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	osg::Matrixd matrix(osg::Matrixd::scale(1000, 500, 1000) * osg::Matrixd::translate(120, 10, 0)); // I've played with these values a lot and it seems to work, but I have no idea why
	modelview->setMatrix(matrix);
	modelview->addChild(geode);

	osg::Projection * projection = new osg::Projection;
	projection->setMatrix(osg::Matrix::ortho2D(0, 1280, 0, 1024)); // or whatever the OSG window res is
	projection->addChild(modelview);

	return projection; //make sure you delete the return sb line
}
class SmoothingCallback : public osgDB::Registry::ReadFileCallback
{
public:
	virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) override
	{
		osgDB::ReaderWriter::ReadResult result = osgDB::Registry::instance()->readNodeImplementation(fileName, options);

		osg::Node* loadedNode = result.getNode();
		if (loadedNode)
		{
			osgUtil::SmoothingVisitor smoothing;
			loadedNode->accept(smoothing);
		}
		return result;
	}
};
void MyOSGEarthQT::on_slopeAnalysis(bool checked)
{
	if (checked)
	{
		std::string filePath = "E:/BaiduNetdiskDownload/OSG/Data/model/Block/Block.osgb";
		osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(filePath);

		node->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

		osgUtil::SmoothingVisitor smoothing;
		node->accept(smoothing);
		osgDB::Registry::instance()->setReadFileCallback(new SmoothingCallback);

		osg::ref_ptr<osg::Shader> vertShader = new osg::Shader(osg::Shader::VERTEX);
		osg::ref_ptr<osg::Shader> fragShader = new osg::Shader(osg::Shader::FRAGMENT);

		osg::Program* _slopeProgram = new osg::Program;
		vertShader->loadShaderSourceFromFile("E:/BaiduNetdiskDownload/OSG/example/Atlas-master/Atlas/resources/shaders/slope.vert");
		fragShader->loadShaderSourceFromFile("E:/BaiduNetdiskDownload/OSG/example/Atlas-master/Atlas/resources/shaders/slope.frag");
		_slopeProgram->addShader(vertShader);
		_slopeProgram->addShader(fragShader);

		node->getOrCreateStateSet()->setAttribute(_slopeProgram, osg::StateAttribute::ON);

		blxform = new osgEarth::GeoTransform;
		
		osgEarth::GeoPoint point(m_mapNode->getMap()->getSRS(), 119.26, 26, 100);
		blxform->setPosition(point);
		blxform->addChild(node);
		m_mapNode->addChild(blxform);
		//osgEarth::Registry::shaderGenerator().run(node);
		blsc = createScalarBar_HUD();
		m_mapNode->addChild(blsc);

		osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		em->setViewpoint(osgEarth::Viewpoint(NULL, 119.26, 26, 500, -2.5, -10, 3450), 2);
	}
	else
	{
		m_mapNode->removeChild(blxform);
		m_mapNode->removeChild(blsc);
		ui.slopeAnalysis->setChecked(false);//不用传入对应信号
		return;
	}
}
/*---------------------------------------------------------------坡向分析--------------------------------------------------*/
//void MyOSGEarthQT::on_AspectAnalysis(bool checked)
//{
//	if (checked)
//	{
//		
//		m_PickEvent->setActionEvent(EnumActionEvent::AspectAnalysis);
//	}
//	else
//	{
//		m_PickEvent->setActionEvent(EnumActionEvent::ActionNull);
//		ui.AspectAnalysis->setChecked(false);//不用传入对应信号
//		return;
//	}
//	ui.ViewshedAnalysis->setChecked(false);
//	ui.RadarAnalysis->setChecked(false);
//	ui.WaterAnalysis->setChecked(false);
//	ui.TerrainProfileAnalysis->setChecked(false);
//	ui.slopeAnalysis->setChecked(false);
//	ui.Distance->setChecked(false);
//	ui.Area->setChecked(false);
//}
/*---------------------------------------------------------------添加经纬线--------------------------------------------------*/
void MyOSGEarthQT::on_Addgraticule(bool checked)
{
	if (checked)
	{
		Style gridLabelStyle = gr->options().gridLabelStyle().get();
		gridLabelStyle.getOrCreate<TextSymbol>()->fill()->color() = Color::White;
		gr->options().gridLabelStyle() = gridLabelStyle;

		Style edgeLabelStyle = gr->options().edgeLabelStyle().get();
		edgeLabelStyle.getOrCreate<TextSymbol>()->fill()->color() = Color::White;
		gr->options().edgeLabelStyle() = edgeLabelStyle;

		m_mapNode->getMap()->addLayer(gr);

		ui.openGLWidget->getViewer()->getCamera()->setSmallFeatureCullingPixelSize(-1.0f);
	}
	else
	{
		m_mapNode->getMap()->removeLayer(gr);
		return;
	}
}
/*---------------------------------------------------------------添加卫星--------------------------------------------------*/
void MyOSGEarthQT::on_Addsatellite(bool checked)
{
	if (checked)
	{
		const osgEarth::SpatialReference* mapsrs5 = m_mapNode->getMapSRS();

		//近地轨道
		{
		osg::Vec3d  startWorld5;
		osg::ref_ptr<osg::Vec4Array> vertices5 = new osg::Vec4Array;
		osg::ref_ptr<osg::Vec4Array> vertices6 = new osg::Vec4Array;
		osg::ref_ptr<osg::Vec3dArray> vertices7 = new osg::Vec3dArray;
		
		for (double i = -180; i < 180; i += 2)
		{
			vertices7->push_back(osg::Vec3d(i, 0.0, 2000000.0));
			vertices5->push_back(osg::Vec4(i, 0.0, 2000000, 600000));
			if (i > -176)
			{
				vertices6->push_back(osg::Vec4(i, 0.0, 2000000, 600000));
			}
		}
		// 将经纬度转换为坐标
		for (int i = 0; i < vertices7->size(); i++)
		{
			osgEarth::GeoPoint map3(mapsrs5, vertices7->at(i).x(), vertices7->at(i).y(), vertices7->at(i).z(), osgEarth::ALTMODE_ABSOLUTE);
			map3.toWorld(startWorld5);
			vertices7->at(i) = startWorld5;
			//std::cout << startWorld5 << std::endl;
		}

		osg::ref_ptr<osg::Geometry> linesgeom = new osg::Geometry();
		linesgeom->setVertexArray(vertices7);
		linesgeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::Mode::LINE_LOOP, 0, vertices7->size()));
		//设置颜色
		osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
		linesgeom->setColorArray(colors);
		linesgeom->setColorBinding(osg::Geometry::BIND_OVERALL);
		colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 0.3f));
		//设置法线
		osg::ref_ptr<osg::Vec3Array> norms = new osg::Vec3Array;
		norms->push_back(osg::Vec3(0.0, -1.0, 0.0));
		linesgeom->setNormalArray(norms);
		linesgeom->setNormalBinding(osg::Geometry::BIND_OVERALL);
		//限制线宽
		osg::ref_ptr<osg::LineWidth> width = new osg::LineWidth;
		//设置线宽
		width->setWidth(1.0);
		geode_redOrbit->getOrCreateStateSet()->setAttributeAndModes(width, osg::StateAttribute::ON);
		osg::StateSet* ss = linesgeom->getOrCreateStateSet();
		ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		linesgeom->setStateSet(ss);
		geode_redOrbit->addDrawable(linesgeom);
		m_world->addChild(geode_redOrbit);

		osg::ref_ptr< osg::Node >  satellite = osgDB::readRefNodeFile("E:\\BaiduNetdiskDownload\\OSG\\Data\\satellite.ive");
		osgEarth::Registry::shaderGenerator().run(satellite);

		osg::ref_ptr<osg::MatrixTransform> mtFlySelf = new osg::MatrixTransform;

		osg::ref_ptr<osg::MatrixTransform> mtsatellite = new osg::MatrixTransform;
		mtFlySelf->addChild(satellite);
		mtFlySelf->setMatrix(osg::Matrixd::scale(10000, 10000, 10000));

		//osgSim::SphereSegment * ss1 = new osgSim::SphereSegment(osg::Vec3d(0.0, 0.0, 1.0),
		//	710.0f, // radius
//	osg::DegreesToRadians(0.0f),
//	osg::DegreesToRadians(360.0f),
//	osg::DegreesToRadians(-60.0f),
//	osg::DegreesToRadians(-60.0f),
//	60);

		//ss1->setAllColors(osg::Vec4(0.5f, 1.0f, 1.0f, 0.5f));
		//ss1->setSideColor(osg::Vec4(0.5f, 1.0f, 1.0f, 0.5f));

		//mtFlySelf->addChild(ss1);

		static const char* vertSource = {
		"varying vec3 pos;\n"
		"void main()\n"
		"{\n"
		"pos.x=gl_Vertex.x;\n"
		"pos.y=gl_Vertex.y;\n"
		"pos.z=gl_Vertex.z;\n"
		"gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;\n"
		"}\n"
		};
		static const char* fragSource = {
		"uniform float num; \n"
		"uniform float height; \n"
		"varying vec3 pos;\n"
		"float Alpha = 1.0; \n"
		"float f = pos.z;\n"
		"uniform float osg_FrameTime;\n"
		"void main()\n"
		"{\n"
		"if (sin(f/height*3.14*2*num+ osg_FrameTime*10) > 0)\n"
		"{\n"
		"	Alpha = 0.5;\n"
		"}\n"
		"else\n"
		"{\n"
		"	Alpha = 0;\n"
		"}\n"
		"	gl_FragColor = vec4(1,0,0,Alpha);\n"
		"}\n "
		};

		osg::ref_ptr<osg::Geode> geode = new osg::Geode();
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
		osg::ref_ptr<osg::Geometry> geom2 = new osg::Geometry();
		geode->addDrawable(geom);
		mtFlySelf->addChild(geode);

		osg::Vec3Array* vt = new osg::Vec3Array;

		vt->push_back(osg::Vec3(0, 0, 0));
		for (float i = 0; i < osg::PI * 2.5; i += osg::PI / 2.0)
		{
			float x = 150 * cos(i);
			float y = 150 * sin(i);
			vt->push_back(osg::Vec3(x, y, -500));
		}
		geom->setVertexArray(vt);
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, 0, vt->size()));

		geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		geom->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		//取消深度测试很关键，防止转动时颜色变化
		//geom->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

		osg::StateSet* stateset = geom->getOrCreateStateSet();
		osg::Program * program = new osg::Program;
		program->addShader(new osg::Shader(osg::Shader::VERTEX, vertSource));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragSource));

		osg::ref_ptr<osg::Uniform> uniform_num = new osg::Uniform("num", float(10.0));  //设置为10层
		osg::ref_ptr<osg::Uniform> uniform_height = new osg::Uniform("height", float(500.0));  //设置最大高度

		stateset->addUniform(uniform_num.get());
		stateset->addUniform(uniform_height.get());

		stateset->setAttributeAndModes(program, osg::StateAttribute::ON);
		stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		mtsatellite->addChild(mtFlySelf);

		satelliteform = new osgEarth::GeoTransform();
		satelliteform->addChild(mtsatellite);

		osg::AnimationPath *ap = new osg::AnimationPath;
		ap = CreateflyPath2(vertices5);

		satelliteform->setUpdateCallback(new osg::AnimationPathCallback(ap, 0.0, 1.0));

		/*osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		Viewpoint vp = em->getViewpoint();
		vp.setNode(satelliteform);
		vp.range() = 5000000.0;
		vp.pitch() = -45.0;
		em->setViewpoint(vp, 2.0);*/

		m_mapNode->addChild(satelliteform);
		}

		//极地轨道

		{
		osg::ref_ptr<osg::Vec4Array> vertices1 = new osg::Vec4Array;
		osg::ref_ptr<osg::Vec4Array> vertices2 = new osg::Vec4Array;
		osg::ref_ptr<osg::Vec3dArray> vertices3 = new osg::Vec3dArray;
		osg::Vec3d  startWorld1;
		
		for (double i = -90; i <= 90; i += 2)
		{
			vertices3->push_back(osg::Vec3d(0.0, i, 2200000.0));
		}
		for (double i = 90; i >= -90; i -= 2)
		{
			vertices3->push_back(osg::Vec3d(180.0, i, 2200000.0));
		}
		for (int i = 0; i < vertices3->size(); i++)
		{
			osgEarth::GeoPoint map3(mapsrs5, vertices3->at(i).x(), vertices3->at(i).y(), vertices3->at(i).z(), osgEarth::ALTMODE_ABSOLUTE);
			map3.toWorld(startWorld1);
			vertices3->at(i) = startWorld1;
		}
		
		
		osg::ref_ptr<osg::Geometry> linesgeom2 = new osg::Geometry();
		linesgeom2->setVertexArray(vertices3);
		linesgeom2->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::Mode::LINE_STRIP, 0, vertices3->size()));
		//设置颜色
		osg::ref_ptr<osg::Vec4Array> colors2 = new osg::Vec4Array;
		linesgeom2->setColorArray(colors2);
		linesgeom2->setColorBinding(osg::Geometry::BIND_OVERALL);
		colors2->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 0.3f));
		//设置法线
		osg::ref_ptr<osg::Vec3Array> norms2 = new osg::Vec3Array;
		norms2->push_back(osg::Vec3(0.0, -1.0, 0.0));
		linesgeom2->setNormalArray(norms2);
		linesgeom2->setNormalBinding(osg::Geometry::BIND_OVERALL);
		//限制线宽
		osg::ref_ptr<osg::LineWidth> width2 = new osg::LineWidth;
		//设置线宽
		width2->setWidth(1.0);
		geode_blueOrbit->getOrCreateStateSet()->setAttributeAndModes(width2, osg::StateAttribute::ON);
		osg::StateSet* ss2 = linesgeom2->getOrCreateStateSet();
		ss2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		linesgeom2->setStateSet(ss2);
		geode_blueOrbit->addDrawable(linesgeom2);
		m_world->addChild(geode_blueOrbit);
		}
		
		//GPS轨道
		//{
		//	ifstream inFile("E:\\BaiduNetdiskDownload\\OSG\\qt\\4.22\\osgearth-master\\DigitalEarth\\orbit11.csv", ios::in);
		//	string lineStr;
		//	vector<vector<string>> strArray;
		//	double array[361][3];
		//	int i, j;
		//	i = 1;
		//	char* end;
		//	if (inFile.fail())
		//		cout << "读取文件失败" << endl;
		//	while (getline(inFile, lineStr))
		//	{
		//		j = 0;
		//		// 存成二维表结构  
		//		stringstream ss(lineStr);
		//		string str;
		//		vector<string> lineArray;
		//		// 按照逗号分隔  
		//		while (getline(ss, str, ','))
		//		{
		//			array[i][j] = static_cast<double>(strtol(str.c_str(), &end, 10));              //string -> int
		//			j++;
		//		}
		//		i++;
		//	}
		//
		//	osg::ref_ptr<osg::Vec3dArray> vertices3 = new osg::Vec3dArray;
		//	for (int i = 1; i < 361; i++)
		//	{
		//		vertices3->push_back(osg::Vec3d(array[i][1], array[i][2], array[i][3]));
		//	}
		//
		//	osg::ref_ptr<osg::Geode> geode_yellowOrbit = new osg::Geode;
		//	osg::ref_ptr<osg::Geometry> linesgeom2 = new osg::Geometry();
		//	linesgeom2->setVertexArray(vertices3);
		//	linesgeom2->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::Mode::LINE_LOOP, 0, vertices3->size()));
		//	//设置颜色
		//	osg::ref_ptr<osg::Vec4Array> colors2 = new osg::Vec4Array;
		//	linesgeom2->setColorArray(colors2);
		//	linesgeom2->setColorBinding(osg::Geometry::BIND_OVERALL);
		//	colors2->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 0.3f));
		//	//设置法线
		//	osg::ref_ptr<osg::Vec3Array> norms2 = new osg::Vec3Array;
		//	norms2->push_back(osg::Vec3(0.0, -1.0, 0.0));
		//	linesgeom2->setNormalArray(norms2);
		//	linesgeom2->setNormalBinding(osg::Geometry::BIND_OVERALL);
		//	//限制线宽
		//	osg::ref_ptr<osg::LineWidth> width2 = new osg::LineWidth;
		//	//设置线宽
		//	width2->setWidth(1.0);
		//	geode_yellowOrbit->getOrCreateStateSet()->setAttributeAndModes(width2, osg::StateAttribute::ON);
		//	osg::StateSet* ss2 = linesgeom2->getOrCreateStateSet();
		//	ss2->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		//	linesgeom2->setStateSet(ss2);
		//	geode_yellowOrbit->addDrawable(linesgeom2);
		//	m_world->addChild(geode_yellowOrbit);
		//}

	}
	else
	{
		m_world->removeChild(geode_redOrbit);
		m_world->removeChild(geode_blueOrbit);
		m_mapNode->removeChild(satelliteform);

		return;
	}
}
/*---------------------------------------------------------------添加指北针--------------------------------------------------*/
void MyOSGEarthQT::on_AddCompass(bool checked)
{
	if (checked)
	{
		g_controlCanvas = new osgEarth::Util::Controls::ControlCanvas();
		m_mapNode->addChild(g_controlCanvas);

		g_compass = new Compass("E:\\BaiduNetdiskDownload\\OSG\\Data\\compass.png", g_controlCanvas);
		g_compass->setDrawView(ui.openGLWidget->getViewer());
	}
	else
	{
		m_mapNode->removeChild(g_controlCanvas);
		return;
	}
}
/*---------------------------------------------------------------添加比例尺--------------------------------------------------*/
void MyOSGEarthQT::on_AddScaleBar(bool checked)
{
	if (checked)
	{
		g_controlCanvas = new osgEarth::Util::Controls::ControlCanvas();
		m_mapNode->asGroup()->addChild(g_controlCanvas);

		g_scaleBar = new ScaleBar(m_mapNode, ui.openGLWidget->getViewer());
		osgEarth::Util::Controls::HBox* scaleBox = new osgEarth::Util::Controls::HBox(
				osgEarth::Util::Controls::Control::ALIGN_CENTER,
				osgEarth::Util::Controls::Control::ALIGN_BOTTOM,
				osgEarth::Util::Controls::Gutter(2, 2, 2, 2), 2.0f);
		scaleBox->addControl(g_scaleBar->_scaleLabel.get());
		scaleBox->addControl(g_scaleBar->_scaleBar.get());
		scaleBox->setVertFill(true);
		scaleBox->setForeColor(osg::Vec4f(0, 0, 0, 0.8));
		scaleBox->setBackColor(osg::Vec4f(1, 1, 1, 0.5));

		g_controlCanvas->addControl(scaleBox);
		ui.openGLWidget->getViewer()->addEventHandler(new ScaleBarHandler(g_scaleBar));
	}
	else
	{
		m_mapNode->removeChild(g_controlCanvas);
		return;
	}
}
/*---------------------------------------------------------------添加鹰眼图--------------------------------------------------*/
void MyOSGEarthQT::on_AddMiniMap(bool checked)
{
	if (checked)
	{
		g_controlCanvas = new osgEarth::Util::Controls::ControlCanvas();
		m_mapNode->asGroup()->addChild(g_controlCanvas);

		ImageControl* s_imageControl = 0L;

		osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile("../data/world.jpg");
		if (image.valid())
		{
			s_imageControl = new ImageControl(image.get());
			s_imageControl->setHorizAlign(Control::ALIGN_LEFT);
			s_imageControl->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_BOTTOM);
			s_imageControl->setSize(200, 100);
			s_imageControl->setFixSizeForRotation(true);
			s_imageControl->setPadding(osgEarth::Util::Controls::Control::SIDE_TOP, 60);
			s_imageControl->setPadding(osgEarth::Util::Controls::Control::SIDE_LEFT, -10);
			g_controlCanvas->addControl(s_imageControl);

			g_overviewMap = new OverviewMapControl(image);
			g_overviewMap->setWidth(200);
			g_overviewMap->setHeight(100);
			g_controlCanvas->addControl(g_overviewMap.get());

			overviewmap = new OverviewMapHandler(g_overviewMap, dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator()));
			ui.openGLWidget->getViewer()->addEventHandler(overviewmap);
		}
	}
	else
	{
		m_mapNode->removeChild(g_controlCanvas);
		ui.openGLWidget->getViewer()->removeEventHandler(overviewmap);
		return;
	}
}
/*---------------------------------------------------------------添加KML文件--------------------------------------------------*/
void MyOSGEarthQT::on_AddKml()
{
	QSettings settings;
	settings.value("currentDirectory");

	/*  第一个参数parent，用于指定父组件。注意，很多Qt组件的构造函数都会有这么一个parent参数，并提供一个默认值0；
		第二个参数caption，是对话框的标题；
		第三个参数dir，是对话框显示时默认打开的目录，"." 代表程序运行目录，"/" 代表当前盘符的根目录(Windows，Linux下/就是根目录了)，
		也可以是平台相关的，比如"C:\\"等；例如我想打开程序运行目录下的Data文件夹作为默认打开路径，这里应该写成"./Data/"，
		若想有一个默认选中的文件，则在目录后添加文件名即可："./Data/teaser.graph"

		第四个参数filter，是对话框的后缀名过滤器，比如我们使用"Image Files(*.jpg *.png)"就让它只能显示后缀名是jpg或者png的文件。
		如果需要使用多个过滤器，使用";;"分割，比如"JPEG Files(*.jpg);;PNG Files(*.png)"；

		第五个参数selectedFilter，是默认选择的过滤器；
		第六个参数options，是对话框的一些参数设定，比如只显示文件夹等等，它的取值是enum QFileDialog::Option，每个选项可以使用 | 运算组合起来。
		如果我要想选择多个文件怎么办呢？Qt提供了getOpenFileNames()函数，其返回值是一个QStringList。
		你可以把它理解成一个只能存放QString的List，也就是STL中的list<string>。*/

	QString fileName = QFileDialog::getOpenFileName(this,
		"Select File",
		settings.value("currentDirectory").toString(),
		"OpenSceneGraph (*.kml)");
	if (fileName.isEmpty())
		return;

	settings.setValue("currentDirectory", QVariant(QFileInfo(fileName).path()));//改变值

	osgEarth::Drivers::KMLOptions kml_options;
	osg::ref_ptr<osg::Node> kml = osgEarth::Drivers::KML::load(osgEarth::URI(fileName.toStdString()), m_mapNode, kml_options);
	if (!kml)
		return;

	m_mapNode->addChild(kml);

	settings.setValue("recentFile", fileName);
}
/*--------------------------------------------------------------添加Vector文件------------------------------------------------*/
void MyOSGEarthQT::on_AddVector()
{
	QSettings settings;
	settings.value("currentDirectory");

	QString fileName = QFileDialog::getOpenFileName(this,
		"Select File",
		settings.value("currentDirectory").toString(),
		"ShapeFile (*.shp)");
	if (fileName.isEmpty())
		return;

	settings.setValue("currentDirectory", QVariant(QFileInfo(fileName).path()));//改变值

	osgEarth::Map* map = new osgEarth::Map();
	osgEarth::Drivers::OGRFeatureOptions featureData;
	map = m_mapNode->getMap();
	// Configures the feature driver to load the vectors from a shapefile:
	featureData.url() = osgEarth::URI(fileName.toStdString());

	// Make a feature source layer and add it to the Map:
	osgEarth::Drivers::FeatureSourceLayerOptions ogrLayer;
	ogrLayer.name() = "vector-data";
	ogrLayer.featureSource() = featureData;
	osgEarth::Drivers::FeatureSourceLayer* featurelayer = new osgEarth::Drivers::FeatureSourceLayer(ogrLayer);
	map->addLayer(featurelayer);

	osgEarth::Symbology::Style style;

	osgEarth::Symbology::LineSymbol* ls = style.getOrCreateSymbol<osgEarth::Symbology::LineSymbol>();
	ls->stroke()->color() = osgEarth::Symbology::Color::Yellow;
	ls->stroke()->width() = 2.0f;
	ls->tessellationSize()->set(100, osgEarth::Symbology::Units::KILOMETERS);

	osgEarth::Symbology::AltitudeSymbol* alt = style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>();
	alt->clamping() = alt->CLAMP_TO_TERRAIN;
	alt->technique() = alt->TECHNIQUE_GPU;

	ls->tessellationSize()->set(100, osgEarth::Symbology::Units::KILOMETERS);

	//osgEarth::Symbology::PolygonSymbol *polygonSymbol = style.getOrCreateSymbol<osgEarth::Symbology::PolygonSymbol>();
	//polygonSymbol->fill()->color() = osgEarth::Symbology::Color(152.0f / 255, 251.0f / 255, 152.0f / 255, 0.4f); //238 230 133
	//polygonSymbol->outline() = true;

	osgEarth::Symbology::RenderSymbol* render = style.getOrCreate<osgEarth::Symbology::RenderSymbol>();
	render->depthOffset()->enabled() = true;

	osgEarth::Features::FeatureModelLayerOptions fml;
	fml.name() = "My Features";
	fml.featureSourceLayer() = "vector-data";
	fml.styles() = new osgEarth::Features::StyleSheet();
	fml.styles()->addStyle(style);
	fml.enableLighting() = false;
	osgEarth::Features::FeatureModelLayer* fmllayer = new osgEarth::Features::FeatureModelLayer(fml);
	map->addLayer(fmllayer);


	settings.setValue("recentFile", fileName);
}
/*----------------------------------------------------点标注----------------------------------------------*/
void MyOSGEarthQT::on_AddPoint(bool checked)
{
	if (checked)
	{

		m_PickEvent->setActionEvent(EnumActionEvent::AddPoint);
	}
	else
	{
		m_PickEvent->setActionEvent(EnumActionEvent::ActionNull);
		ui.AddPoint->setChecked(false);//不用传入对应信号
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.AddRectangle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------线标注----------------------------------------------*/
void MyOSGEarthQT::on_AddLine(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawlineevent = new DrawLineTool(m_mapNode, m_losGroup);
		
		ui.openGLWidget->getViewer()->addEventHandler(drawlineevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawlineevent);
		ui.AddLine->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.AddRectangle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------多边形标注----------------------------------------------*/
void MyOSGEarthQT::on_AddPolygon(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawPolygonevent = new DrawPolygonTool(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawPolygonevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawPolygonevent);
		ui.AddPolygon->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.AddRectangle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------圆弧标注----------------------------------------------*/
void MyOSGEarthQT::on_AddCircle(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawCircleevent = new DrawCircleTool(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawCircleevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawCircleevent);
		ui.AddCircle->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddRectangle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------矩形标注----------------------------------------------*/
void MyOSGEarthQT::on_AddRectangle(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawRectangleevent = new DrawRectangleTool(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawRectangleevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawRectangleevent);
		ui.AddRectangle->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------直箭头标注----------------------------------------------*/
void MyOSGEarthQT::on_StraightArrow(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawStraightArrowevent = new GeoStraightArrow(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawStraightArrowevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawStraightArrowevent);
		ui.StraightArrow->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------斜箭头标注----------------------------------------------*/
void MyOSGEarthQT::on_DiagonalArrow(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawDiagonalArrowevent = new GeoDiagonalArrow(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawDiagonalArrowevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawDiagonalArrowevent);
		ui.DiagonalArrow->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------双箭头标注----------------------------------------------*/
void MyOSGEarthQT::on_DoubleArrow(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawDoubleArrowevent = new GeoDoubleArrow(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawDoubleArrowevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawDoubleArrowevent);
		ui.DoubleArrow->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------弓形标注----------------------------------------------*/
void MyOSGEarthQT::on_GeoLune(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawLuneevent = new GeoLune(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawLuneevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawLuneevent);
		ui.Lune->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------聚居区标注----------------------------------------------*/
void MyOSGEarthQT::on_GatheringPlace(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawGatheringPlaceevent = new GeoGatheringPlace(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawGatheringPlaceevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawGatheringPlaceevent);
		ui.GatheringPlace->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.ParallelSearch->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------平行搜寻区标注----------------------------------------------*/
void MyOSGEarthQT::on_ParallelSearch(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawParallelSearchevent = new GeoParallelSearch(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawParallelSearchevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawParallelSearchevent);
		ui.ParallelSearch->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.SectorSearch->setChecked(false);
}
/*----------------------------------------------------扇形搜寻区标注----------------------------------------------*/
void MyOSGEarthQT::on_SectorSearch(bool checked)
{
	if (checked)
	{
		m_mapNode->addChild(m_losGroup);
		drawSectorSearchevent = new GeoSectorSearch(m_mapNode, m_losGroup);

		ui.openGLWidget->getViewer()->addEventHandler(drawSectorSearchevent);
	}
	else
	{
		ui.openGLWidget->getViewer()->removeEventHandler(drawSectorSearchevent);
		ui.SectorSearch->setChecked(false);
		return;
	}
	ui.ViewshedAnalysis->setChecked(false);
	ui.RadarAnalysis->setChecked(false);
	ui.WaterAnalysis->setChecked(false);
	ui.TerrainProfileAnalysis->setChecked(false);
	ui.AddPoint->setChecked(false);
	ui.AddLine->setChecked(false);
	ui.AddPolygon->setChecked(false);
	ui.AddCircle->setChecked(false);
	ui.StraightArrow->setChecked(false);
	ui.DiagonalArrow->setChecked(false);
	ui.DoubleArrow->setChecked(false);
	ui.Lune->setChecked(false);
	ui.GatheringPlace->setChecked(false);
	ui.ParallelSearch->setChecked(false);
}
/*--------------------------------------------------------------添加地球文件------------------------------------------------*/
void MyOSGEarthQT::on_AddEarth()
{
	QSettings settings;
	settings.value("currentDirectory");

	QString fileName = QFileDialog::getOpenFileName(this,
		"Select File",
		settings.value("currentDirectory").toString(),
		"OpenSceneGraph (*.earth)");
	if (fileName.isEmpty())
		return;

	settings.setValue("currentDirectory", QVariant(QFileInfo(fileName).path()));//改变值

	m_world->removeChild(m_mapNode);

	m_mapNode = osgEarth::MapNode::findMapNode(osgDB::readNodeFile(fileName.toStdString()));

	m_world->addChild(m_mapNode);

	settings.setValue("recentFile", fileName);

}
/*--------------------------------------------------------------添加高程文件------------------------------------------------*/
void MyOSGEarthQT::on_AddElevation()
{
	QSettings settings;
	settings.value("currentDirectory");

	QString fileName = QFileDialog::getOpenFileName(this,
		"Select File",
		settings.value("currentDirectory").toString(),
		"OpenSceneGraph (*.tif)");
	if (fileName.isEmpty())
		return;

	settings.setValue("currentDirectory", QVariant(QFileInfo(fileName).path()));//改变值

	osgEarth::Drivers::GDALOptions elevation;
	elevation.url() = osgEarth::URI(fileName.toStdString());
	m_mapNode->getMap()->addLayer(new ElevationLayer("elevation", elevation));

	settings.setValue("recentFile", fileName);

}
/*----------------------------------------------------填挖分析(待定)--------------------------------------------------*/
osg::Node* createtwSphere(osg::Vec3 center)
{
	osg::Geode* gnode = new osg::Geode;
	osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(center, 0.2));
	gnode->addDrawable(sd);
	gnode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	return gnode;
}
osg::Geode* CreatetwBox(osg::Vec3 from, osg::Vec3 to, float stepSize, osg::Vec4 color)
{
	osg::Geode* gnode = new osg::Geode;
	osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Box((from + to) / 2, stepSize, stepSize, std::abs(to.z() - from.z())));
	sd->setColor(color);
	gnode->addDrawable(sd);
	return gnode;
}
class TwEventHandler : public osgGA::GUIEventHandler
{
public:
	TwEventHandler(osg::Node* base, osg::Group* root, osg::Geode* mask, osg::Geometry* maskGeom, osg::DrawArrays* da, osg::Group* mark)
	{
		vertexArray = new osg::Vec3Array;
		_base = base;//地形
		_root = root;//场景根结点
		_mask = mask;//鼠标点出来的面
		_maskGeom = maskGeom;
		_da = da;
		_mark = mark;//小球

		_maskGeom->setVertexArray(vertexArray);

		osg::Vec4Array* color = new osg::Vec4Array;
		color->push_back(osg::Vec4(1.0, 0.0, 0.0, 0.4));
		_maskGeom->setColorArray(color, osg::Array::BIND_OVERALL);
	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		if (ea.getEventType() == ea.PUSH)
		{
			//是左键且左shift按下了
			if ((ea.getButton() == ea.LEFT_MOUSE_BUTTON) && (ea.getModKeyMask() == ea.MODKEY_LEFT_SHIFT))
			{
				std::cout << "push hits!" << std::endl;
				osgUtil::LineSegmentIntersector::Intersections intersections;
				osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
				if (view->computeIntersections(ea, intersections))
				{
					//有交点，先画个球
					osg::Vec3 hitPoint = intersections.begin()->getWorldIntersectPoint();
					_mark->addChild(createtwSphere(hitPoint));

					vertexArray->push_back(hitPoint);

					if (vertexArray->size() >= 3)
					{
						_da->set(GL_POLYGON, 0, vertexArray->size());
						_maskGeom->dirtyDisplayList();//redraw
					}
				}
			}
		}

		if (ea.getEventType() == ea.KEYDOWN)
		{
			if ((ea.getKey() == 'x') || (ea.getKey() == 'X'))
			{
				_da->set(GL_POLYGON, 0, 0);
				_maskGeom->dirtyDisplayList();//redraw

				_mark->removeChildren(0, _mark->getNumChildren());

				vertexArray->clear();
			}

			if ((ea.getKey() == 'b') || (ea.getKey() == 'B'))
			{
				//开始计算挖方量与填方量
				//与点围成的面先求是否有交点，有交点再求与地形交点，两个交点之间的距离就是需要挖/填的方
				//其中与面的距离在上，则是填方，与地形交点距离在上，则是挖方，使用z值大小来判断距离
				//求要求的场景的boudingbox
				_maskGeom->dirtyBound();//因为顶点改变，必须重新计算包围盒
				osg::BoundingSphere bs = _maskGeom->getBound();
				//求出xyz的最小值和最大值，然后根据间隔来求
				osg::Vec3 center = bs.center();
				float r = bs.radius();

				//以点的原点为基准面
				//挖方量
				float volumesW = 0.0;
				//填方量
				float volumesT = 0.0;

				//以最低点为基准面
				//挖方量
				float volumesWL = 0.0;
				//填方量
				float volumesTL = 0.0;
				//查找基准面的z值的最小值
				float zmin = FLT_MAX;
				for (int i = 0; i < vertexArray->size(); i++)
				{
					if (zmin > vertexArray->at(i).z())
					{
						zmin = vertexArray->at(i).z();
					}
				}

				//采样一个方向上采50份
				float stepSize = r / 50;
				for (float fromx = center.x() - r; fromx <= center.x() + r; fromx += stepSize)
				{
					for (float fromy = center.y() - r; fromy <= center.y() + r; fromy += stepSize)
					{
						//乘1000是为了确保与地形相交，与面相交不需要乘1000
						osg::Vec3 from = osg::Vec3(fromx, fromy, center.z() - r * 1000);
						osg::Vec3 to = osg::Vec3(fromx, fromy, center.z() + r * 1000);
						osgUtil::LineSegmentIntersector::Intersections intersections;
						osg::ref_ptr<osgUtil::LineSegmentIntersector> ls = new osgUtil::LineSegmentIntersector(from, to);
						osg::ref_ptr<osgUtil::IntersectionVisitor> iv = new osgUtil::IntersectionVisitor(ls);
						_mask->accept(*iv.get());

						if (ls->containsIntersections()) //与点出来的面有交点
						{
							intersections = ls->getIntersections();

							//与地形必有交点
							osgUtil::LineSegmentIntersector::Intersections intersections0;
							osg::ref_ptr<osgUtil::LineSegmentIntersector> ls0 = new osgUtil::LineSegmentIntersector(from, to);
							osg::ref_ptr<osgUtil::IntersectionVisitor> iv0 = new osgUtil::IntersectionVisitor(ls0);
							_base->accept(*iv0.get());

							if (ls0->containsIntersections()) //与地形必有交点
							{
								intersections0 = ls0->getIntersections();

								osg::Vec3 maskInter = intersections.begin()->getWorldIntersectPoint();
								osg::Vec3 terrainInter = intersections0.begin()->getWorldIntersectPoint();

								if (maskInter.z() > terrainInter.z())
								{
									volumesT += (stepSize * stepSize * std::abs(maskInter.z() - terrainInter.z()));
									_mark->addChild(CreatetwBox(maskInter, terrainInter, stepSize, osg::Vec4(0.0, 1.0, 0.0, 1.0)));
								}
								else
								{
									volumesW += (stepSize * stepSize * std::abs(maskInter.z() - terrainInter.z()));
									_mark->addChild(CreatetwBox(maskInter, terrainInter, stepSize, osg::Vec4(1.0, 0.0, 0.0, 1.0)));
								}

								maskInter.z() = zmin;
								if (maskInter.z() > terrainInter.z())
								{
									volumesTL += (stepSize * stepSize * std::abs(maskInter.z() - terrainInter.z()));
								}
								else
								{
									volumesWL += (stepSize * stepSize * std::abs(maskInter.z() - terrainInter.z()));
								}
							}
						}
					}
				}
				std::cout << "采用原点为基准面：" << std::endl;
				std::cout << "挖方量：" << volumesW << std::endl;
				std::cout << "填方量：" << volumesT << std::endl;
				std::cout << "采用最低点为基准面：" << std::endl;
				std::cout << "挖方量：" << volumesWL << std::endl;
				std::cout << "填方量：" << volumesTL << std::endl;

			}
		}
		return false;
	}

	osg::Vec3Array* vertexArray;
	osg::Node* _base;//地形
	osg::Group* _root;//场景根结点
	osg::Geode* _mask;//鼠标点出来的面
	osg::Geometry* _maskGeom;
	osg::DrawArrays* _da;
	osg::Group* _mark;//小球
};
void MyOSGEarthQT::on_Tianwa(bool checked)
{
	if (checked)
	{
		osg::ref_ptr<osg::Node> _base = new osg::Node;//地形
		//osg::ref_ptr<osg::Group> _root = new osg::Group;//场景根结点
		osg::ref_ptr<osg::Geode> _mask = new osg::Geode;//鼠标点出来的面
		osg::ref_ptr<osg::Geometry> _maskGeom = new osg::Geometry;
		osg::ref_ptr<osg::DrawArrays> _da = new osg::DrawArrays;
		osg::ref_ptr<osg::Group> _mark = new osg::Group;//小球

		_base = osgDB::readNodeFile("E:/BaiduNetdiskDownload/OSG/Data/model/Block/Block.osgb");
		_base->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		m_world->addChild(_base);

		_mask->addDrawable(_maskGeom);
		_maskGeom->addPrimitiveSet(_da);
		_mask->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		_mask->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		m_world->addChild(_mark);
		m_world->addChild(_mask);

		
		twxform = new osgEarth::GeoTransform;
		twxform->setTerrain(m_mapNode->getTerrain());
		twxform->addChild(_base);
		osgEarth::GeoPoint point(m_mapNode->getMap()->getSRS(), 116, 37, 200);    
		twxform->setPosition(point);

		m_world->addChild(twxform);
		osgEarth::Registry::shaderGenerator().run(_base);

		osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		em->setViewpoint(osgEarth::Viewpoint(NULL, 116, 37, 500, -2.5, -45, 3450), 2);

		ui.openGLWidget->getViewer()->addEventHandler(new TwEventHandler(_base, m_world, _mask, _maskGeom, _da, _mark));
	}
	else
	{
		m_world->removeChild(twxform);
		return;
	}
}
/*-------------------------------------------------动态纹理着色(墙)--------------------------------------------*/
void MyOSGEarthQT::on_Dynamictexture(bool checked)
{
	if (checked)
	{
		const SpatialReference* geoSRS = m_mapNode->getMapSRS()->getGeographicSRS();

		static const char * fragShader = {
			"varying vec4 color;\n"
			"uniform sampler2D baseTex;\n"
			"uniform int osg_FrameNumber;\n"//当前OSG程序运行的帧数；
			"uniform float osg_FrameTime;\n"//当前OSG程序的运行总时间；
			"uniform float osg_DeltaFrameTime;\n"//当前OSG程序运行每帧的间隔时间；
			"uniform mat4 osg_ViewMatrix;\n"//当前OSG摄像机的观察矩阵；
			"uniform mat4 osg_ViewMatrixInverse;\n"// 当前OSG摄像机观察矩阵的逆矩阵。
			"void main(void){\n"
			"vec2 coord = gl_TexCoord[0].xy+vec2(0,osg_FrameTime*0.1);"
			"vec4 color = texture2D(baseTex, coord);\n"
			"   gl_FragColor = color;\n"
			"}\n"
		};

		DTgeode = new osg::Geode;

		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
		geom->setVertexArray(v);
		v->push_back(osg::Vec3(115.f, 26.f, 1000.f));
		v->push_back(osg::Vec3(116.f, 26.f, 1000.f));
		v->push_back(osg::Vec3(116.f, 26.f, 5000.f));
		v->push_back(osg::Vec3(115.f, 26.f, 5000.f));
		v->push_back(osg::Vec3(116.f, 26.f, 1000.f));
		v->push_back(osg::Vec3(116.f, 27.f, 1000.f));
		v->push_back(osg::Vec3(116.f, 27.f, 5000.f));
		v->push_back(osg::Vec3(116.f, 26.f, 5000.f));
		v->push_back(osg::Vec3(116.f, 27.f, 1000.f));
		v->push_back(osg::Vec3(115.f, 27.f, 1000.f));
		v->push_back(osg::Vec3(115.f, 27.f, 5000.f));
		v->push_back(osg::Vec3(116.f, 27.f, 5000.f));
		v->push_back(osg::Vec3(115.f, 27.f, 1000.f));
		v->push_back(osg::Vec3(115.f, 26.f, 1000.f));
		v->push_back(osg::Vec3(115.f, 26.f, 5000.f));
		v->push_back(osg::Vec3(115.f, 27.f, 5000.f));

		osg::ref_ptr<osg::Vec3Array> n = new osg::Vec3Array;
		geom->setNormalArray(n);
		geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
		n->push_back(osg::Vec3(0.f, 1.f, 0.f));

		osg::Vec3d  startWorld5;

		for (int i = 0; i < v->size(); i++)
		{
			osgEarth::GeoPoint map3(geoSRS, v->at(i).x(), v->at(i).y(), v->at(i).z(), osgEarth::ALTMODE_ABSOLUTE);
			map3.toWorld(startWorld5);
			v->at(i) = startWorld5;
			//std::cout << startWorld5 << std::endl;
		}

		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 16));
		DTgeode->addDrawable(geom.get());

		osg::Vec2Array* texcoords = new osg::Vec2Array(16);
		(*texcoords)[0].set(0.0f, 0.0f);
		(*texcoords)[1].set(0.0f, 1.0f);
		(*texcoords)[2].set(1.0f, 1.0f);
		(*texcoords)[3].set(1.0f, 0.0f);
		(*texcoords)[4].set(0.0f, 0.0f);
		(*texcoords)[5].set(0.0f, 1.0f);
		(*texcoords)[6].set(1.0f, 1.0f);
		(*texcoords)[7].set(1.0f, 0.0f);
		(*texcoords)[8].set(0.0f, 0.0f);
		(*texcoords)[9].set(0.0f, 1.0f);
		(*texcoords)[10].set(1.0f, 1.0f);
		(*texcoords)[11].set(1.0f, 0.0f);
		(*texcoords)[12].set(0.0f, 0.0f);
		(*texcoords)[13].set(0.0f, 1.0f);
		(*texcoords)[14].set(1.0f, 1.0f);
		(*texcoords)[15].set(1.0f, 0.0f);
		geom->setTexCoordArray(0, texcoords);

		// set up the texture state.
		osg::Texture2D* texture = new osg::Texture2D;
		//texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
		texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
		texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
		texture->setImage(osgDB::readImageFile("colors22.png"));

		osg::StateSet* stateset = geom->getOrCreateStateSet();
		stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
		osg::Program * program = new osg::Program;
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));
		stateset->addUniform(new osg::Uniform("baseTex", 0));
		stateset->setAttributeAndModes(program, osg::StateAttribute::ON);

		geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		geom->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

		m_world->addChild(DTgeode);

		static const char* ocvertSource = {
			"void main()\n"
			"{\n"
			"    vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;\n"
			"	 gl_TexCoord[0] = gl_MultiTexCoord0;\n"
			"    gl_Position = ftransform();\n"
			"}\n"
		};
		
		static const char* ocfragSource = {
			"uniform int baseFlag;\n"//流动开关
			"uniform sampler2D baseTex;\n"//纹理0
			"uniform float osg_FrameTime;\n"//当前OSG程序的运行总时间
			"void main (void)\n"
			"{\n"
			"  vec2 Local1 = vec2(gl_TexCoord[0]);\n"
			"  vec2 Local2 = (Local1 - vec2(0.5,0.5)); \n"
			"  float Local3 = distance(gl_TexCoord[0].st,vec2(0.5,0.5));\n"
			"  float Local4 = sqrt(Local3) * 1 / max(0.0, 0.5);\n"
			"  float Local5 = 1 - Local4;\n"
			"  float Local6 = sin((Local5 + osg_FrameTime * 0.1) * 6.28318548 * 4);\n"
			"  float Local7 = pow(Local6, 8);\n"
			"  float Local8 = Local5 * Local7;\n"
			"  float d0 = floor(Local8+0.75);\n"
			"  vec2 texturecoord0 = gl_TexCoord[0].st;\n"
			"  vec4 textureColor0 = texture2D(baseTex, texturecoord0);\n"
			"  gl_FragColor = textureColor0;\n"
			"  gl_FragColor.a = d0;\n"
			"}\n"
		};
		
		
		osg::Geometry* ocgeom = new osg::Geometry;
		
		osg::Vec3Array* occoords = new osg::Vec3Array(4);
		(*occoords)[0] = osg::Vec3(115.3, 26.3, 1000);
		(*occoords)[1] = osg::Vec3(115.7, 26.3, 1000);
		(*occoords)[2] = osg::Vec3(115.7, 26.7, 1000);
		(*occoords)[3] = osg::Vec3(115.3, 26.7, 1000);
		ocgeom->setVertexArray(occoords);

		for (int i = 0; i < occoords->size(); i++)
		{
			osgEarth::GeoPoint map3(geoSRS, occoords->at(i).x(), occoords->at(i).y(), occoords->at(i).z(), osgEarth::ALTMODE_ABSOLUTE);
			map3.toWorld(startWorld5);
			occoords->at(i) = startWorld5;
		}
		
		osg::Vec2Array* tcoords = new osg::Vec2Array(4);
		(*tcoords)[0].set(0, 0);
		(*tcoords)[1].set(0, 1);
		(*tcoords)[2].set(1, 1);
		(*tcoords)[3].set(1, 0);
		ocgeom->setTexCoordArray(0, tcoords);
		
		osg::Vec4Array* colours = new osg::Vec4Array(1);
		(*colours)[0].set(1.0f, 1.0f, 1.0, 1.0f);
		ocgeom->setColorArray(colours, osg::Array::BIND_OVERALL);
		
		osg::Vec3Array* normals = new osg::Vec3Array(1);
		(*normals)[0] = osg::Vec3(0.0f, -1.0f, 0.0f);
		(*normals)[0].normalize();
		ocgeom->setNormalArray(normals, osg::Array::BIND_OVERALL);
		
		ocgeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
		
		osg::Texture2D* octexture = new osg::Texture2D;
		octexture->setDataVariance(osg::Object::DYNAMIC);
		octexture->setResizeNonPowerOfTwoHint(false);
		
		octexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		octexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		octexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
		octexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
		
		octexture->setImage(osgDB::readImageFile("E:/BaiduNetdiskDownload/OSG/Data/outcircle.png"));

		ocnode = new osg::Geode;
		ocnode->addDrawable(ocgeom);
		ocnode->getOrCreateStateSet()->setTextureAttributeAndModes(0, octexture, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		ocnode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		ocnode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		ocnode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		
		osg::ref_ptr<osg::Program> ocprogram = new osg::Program;
		ocprogram->addShader(new osg::Shader(osg::Shader::VERTEX, ocvertSource));
		ocprogram->addShader(new osg::Shader(osg::Shader::FRAGMENT, ocfragSource));
		ocnode->getOrCreateStateSet()->setAttributeAndModes(ocprogram);
		
		ocnode->getOrCreateStateSet()->addUniform(new osg::Uniform("baseTex", 0));
		
		m_world->addChild(ocnode);

		osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		em->setViewpoint(osgEarth::Viewpoint(NULL, 115.5, 26.5, 20600.66, -2.5, -45, 34500), 2);

	}
	else
	{
		m_world->removeChild(DTgeode);
		m_world->removeChild(ocnode);
		return;
	}
}
/*-------------------------------------------------飞行迹线--------------------------------------------------*/
osg::Vec3Array* getpoint(osg::Vec3 p1, osg::Vec3 p2, float h)
{
	osg::Vec3Array* result = new osg::Vec3Array;

	h = h > 5000 ? h : 5000;

	float L = abs(p1.x() - p2.x()) > abs(p1.y() - p2.y()) ? abs(p1.x() - p2.x()) : abs(p1.y() - p2.y());

	int num = 50;

	float dlt = L / num;

	if (abs(p1.x() - p2.x()) > abs(p1.y() - p2.y())) {//以lon为基准

		float delLat = (p2.y() - p1.y()) / num;

		if (p1.x() - p2.x() > 0) {

			dlt = -dlt;

		}

		for (int i = 0; i < num; i++) {

			float tempH = h - pow((-0.5 * L + abs(dlt) * i), 2) * 4 * h / pow(L, 2);

			float lon = p1.x() + dlt * i;

			float lat = p1.y() + delLat * i;

			result->push_back(osg::Vec3(lon, lat, tempH));

		}

	}
	else {//以lat为基准

		float delLon = (p2.x() - p1.x()) / num;

		if (p1.y() - p2.y() > 0) {

			dlt = -dlt;

		}

		for (int i = 0; i < num; i++) {

			float tempH = h - pow((-0.5 * L + abs(dlt) * i), 2) * 4 * h / pow(L, 2);

			float lon = p1.x() + delLon * i;

			float lat = p1.y() + dlt * i;

			result->push_back(osg::Vec3(lon, lat, tempH));

		}

	}

	return result;
}
osg::Geode* getgeode(osg::Vec3 to, osg::Vec3 from, const SpatialReference* srs)
{
	static const char * fragShader = {
		"varying vec4 color;\n"
		"uniform sampler2D baseTex;\n"
		"uniform int osg_FrameNumber;\n"//当前OSG程序运行的帧数；
		"uniform float osg_FrameTime;\n"//当前OSG程序的运行总时间；
		"uniform float osg_DeltaFrameTime;\n"//当前OSG程序运行每帧的间隔时间；
		"uniform mat4 osg_ViewMatrix;\n"//当前OSG摄像机的观察矩阵；
		"uniform mat4 osg_ViewMatrixInverse;\n"// 当前OSG摄像机观察矩阵的逆矩阵。
		"void main(void){\n"
		"vec2 coord = gl_TexCoord[0].xy+vec2(0,osg_FrameTime*0.3);"
		"vec4 color = texture2D(baseTex, coord);\n"
		"   gl_FragColor = color;\n"
		"}\n"
	};

	osg::Geode* geode = new osg::Geode;

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
	v = getpoint(to, from, 5000);
	geom->setVertexArray(v);

	osg::ref_ptr<osg::Vec3Array> n = new osg::Vec3Array;
	geom->setNormalArray(n);
	geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
	n->push_back(osg::Vec3(0.f, 1.f, 0.f));

	osg::Vec3d  startWorld5;

	for (int i = 0; i < v->size(); i++)
	{
		osgEarth::GeoPoint map3(srs, v->at(i).x(), v->at(i).y(), v->at(i).z(), osgEarth::ALTMODE_ABSOLUTE);
		map3.toWorld(startWorld5);
		v->at(i) = startWorld5;
	}

	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 50));
	geode->addDrawable(geom.get());

	osg::Vec2Array* texcoords = new osg::Vec2Array(50);

	for (int i = 0; i < texcoords->size(); i++)
	{
		(*texcoords)[i].set(0.0f, i / 50.0);
	}
	geom->setTexCoordArray(0, texcoords);

	// set up the texture state.
	osg::Texture2D* texture = new osg::Texture2D;
	//texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
	texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
	texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
	texture->setImage(osgDB::readImageFile("E:/BaiduNetdiskDownload/OSG/Data/colors11.png"));

	osg::StateSet* stateset = geom->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
	osg::Program * program = new osg::Program;
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));
	stateset->addUniform(new osg::Uniform("baseTex", 0));
	stateset->setAttributeAndModes(program, osg::StateAttribute::ON);

	geom->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	geom->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	osg::ref_ptr<osg::LineWidth> lineWid = new osg::LineWidth(2.0f);
	stateset->setAttribute(lineWid);

	return geode;

}
void MyOSGEarthQT::on_TrailLine(bool checked)
{
	if (checked)
	{
		TrailLine = new osg::Group();
		const SpatialReference* geoSRS = m_mapNode->getMapSRS()->getGeographicSRS();
		osg::Vec3f fuzhou = osg::Vec3f(119.1724f, 26.0339f, 0.f);
		osg::Vec3f longyan = osg::Vec3f(117.0373f, 25.0984f, 0.f);
		osg::Vec3f zhangzhou = osg::Vec3f(117.7098f, 24.5033f, 0.f);
		osg::Vec3f quanzhou = osg::Vec3f(118.6130f, 24.8894f, 0.f);
		osg::Vec3f ningde = osg::Vec3f(119.5262f, 26.6602f, 0.f);
		osg::Vec3f sanming = osg::Vec3f(117.6458f, 26.2717f, 0.f);
		osg::Vec3f nanping = osg::Vec3f(118.1204f, 27.3317f, 0.f);
		osg::Vec3f xiamen = osg::Vec3f(118.0339f, 24.4841f, 0.f);
		osg::Vec3f putian = osg::Vec3f(118.9947f, 25.4188f, 0.f);
		osg::Vec3f taibei = osg::Vec3f(121.5644f, 25.0337f, 0.f);

		//地点
		{
			std::string name1 = "福州";
			std::string name2 = "龙岩";
			std::string name3 = "漳州";
			std::string name4 = "泉州";
			std::string name5 = "宁德";
			std::string name6 = "三明";
			std::string name7 = "南平";
			std::string name8 = "厦门";
			std::string name9 = "莆田";
			std::string name10 = "台北";
			std::string _strWideName1;
			std::string _strWideName2;
			std::string _strWideName3;
			std::string _strWideName4;
			std::string _strWideName5;
			std::string _strWideName6;
			std::string _strWideName7;
			std::string _strWideName8;
			std::string _strWideName9;
			std::string _strWideName10;
			gb2312ToUtf8(name1, _strWideName1);
			gb2312ToUtf8(name2, _strWideName2);
			gb2312ToUtf8(name3, _strWideName3);
			gb2312ToUtf8(name4, _strWideName4);
			gb2312ToUtf8(name5, _strWideName5);
			gb2312ToUtf8(name6, _strWideName6);
			gb2312ToUtf8(name7, _strWideName7);
			gb2312ToUtf8(name8, _strWideName8);
			gb2312ToUtf8(name9, _strWideName9);
			gb2312ToUtf8(name9, _strWideName10);

			Style pm;
			pm.getOrCreate<osgEarth::Symbology::TextSymbol>()->font() = "C:/Windows/Fonts/simhei.ttf";//指定中文字体路径
			pm.getOrCreate<osgEarth::Symbology::TextSymbol>()->encoding() = osgEarth::Symbology::TextSymbol::ENCODING_UTF8;
			pm.getOrCreate<TextSymbol>()->size() = 15;
			pm.getOrCreate<TextSymbol>()->fill()->color() = Color::Yellow;
			pm.getOrCreate<IconSymbol>()->url()->setLiteral("E:/BaiduNetdiskDownload/OSG/Data/data/city.png");
			pm.getOrCreate<IconSymbol>()->declutter() = true;
			pm.getOrCreate<IconSymbol>()->scale() = 0.1;

			// test with an LOD:
			osg::LOD* lod = new osg::LOD();

			// bunch of pins:
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 119.1724, 26.0339), _strWideName1, pm), 0.0, 2e6);
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 117.0373, 25.0984), _strWideName2, pm), 0.0, 2e6);
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 117.7098, 24.5033), _strWideName3, pm), 0.0, 2e6);
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 118.6130, 24.8894), _strWideName4, pm), 0.0, 2e6);
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 119.5262, 26.6602), _strWideName5, pm), 0.0, 2e6);
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 117.6458, 26.2717), _strWideName6, pm), 0.0, 2e6);
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 118.1204, 27.3317), _strWideName7, pm), 0.0, 2e6);
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 118.0339, 24.4841), _strWideName8, pm), 0.0, 2e6);
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 118.9947, 25.4188), _strWideName9, pm), 0.0, 2e6);
			lod->addChild(new PlaceNode(GeoPoint(geoSRS, 121.5644, 25.0337), _strWideName10, pm), 0.0, 2e6);
			TrailLine->addChild(lod);
		}
		
		TrailLine->addChild(getgeode(fuzhou, longyan, geoSRS));
		TrailLine->addChild(getgeode(fuzhou, zhangzhou, geoSRS));
		TrailLine->addChild(getgeode(fuzhou, quanzhou, geoSRS));
		TrailLine->addChild(getgeode(fuzhou, ningde, geoSRS));
		TrailLine->addChild(getgeode(fuzhou, sanming, geoSRS));
		TrailLine->addChild(getgeode(fuzhou, nanping, geoSRS));
		TrailLine->addChild(getgeode(fuzhou, xiamen, geoSRS));
		TrailLine->addChild(getgeode(fuzhou, putian, geoSRS));
		TrailLine->addChild(getgeode(fuzhou, taibei, geoSRS));

		m_world->addChild(TrailLine);

		osg::ref_ptr<osgEarth::Util::EarthManipulator> em = dynamic_cast<osgEarth::Util::EarthManipulator*>(ui.openGLWidget->getViewer()->getCameraManipulator());
		em->setViewpoint(osgEarth::Viewpoint(NULL, 119, 26, 7000, -2.5, -45, 250000), 2);
	}
	else
	{
		m_world->removeChild(TrailLine);
		return;
	}
}
/*-------------------------------------------------更改时间--------------------------------------------*/
struct Changetime : public ControlEventHandler
{
	Changetime(SkyNode* sky , int time):_sky(sky), _time(time){}

	void onValueChanged(Control* control, float value)
	{
		_time = value;
		_sky->setDateTime(osgEarth::DateTime(2021, 4, 21, _time));
	}
	SkyNode* _sky;
	int _time;
};
void MyOSGEarthQT::on_ChangeTime(bool checked)
{
	if (checked)
	{
		int i = 0;
		timecanvas = new ControlCanvas();
		m_mapNode->addChild(timecanvas);

		Grid* timeBox = timecanvas->addControl(new Grid());
		timeBox->setBackColor(0, 0, 0, 0.5);
		timeBox->setMargin(10);
		timeBox->setPadding(10);
		timeBox->setChildSpacing(10);
		timeBox->setChildVertAlign(Control::ALIGN_CENTER);
		timeBox->setAbsorbEvents(true);
		timeBox->setVertAlign(Control::ALIGN_TOP);

		LabelControl* hLabel = new LabelControl("Hours:");
		hLabel->setVertAlign(Control::ALIGN_CENTER);
		timeBox->setControl(0, 0, hLabel);

		HSliderControl* ct = new HSliderControl(0.0f, 24.0f, 0.0f, new Changetime(skyNode, i));
		ct->setWidth(240);
		ct->setHeight(12);
		ct->setVertAlign(Control::ALIGN_CENTER);
		timeBox->setControl(1, 0, ct);
		timeBox->setControl(2, 0, new LabelControl(ct));
		
	}
	else
	{
		m_mapNode->removeChild(timecanvas);
		ui.ChangeTime->setChecked(false);//不用传入对应信号
		return;
	}
	

}