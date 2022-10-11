#pragma once
#include <Windows.h>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QFileDialog>
#include <QtCore/QSettings>

#include "ui_MyOSGEarthQT.h"
#include "PickEvent.h"
#include "SetLosHeight.h"
#include "ViewshedPara.h"
#include "kfz.h"
#include "TPGEvent.h"
#include "RTTPicker.h"
#include "Compass.h"
#include "ScaleBar.h"
#include "OverviewMap.h"
#include "DrawLineTool.h"
#include "DrawPolygonTool.h"
#include "DrawCircleTool.h"
#include "DrawRectangleTool.h"
#include "GeoStraightArrow.h"
#include "GeoDoubleArrow.h"
#include "GeoDiagonalArrow.h"
#include "GeoGatheringPlace.h"
#include "GeoLune.h"
#include "GeoParallelSearch.h"
#include "GeoSectorSearch.h"
#include "AngleCaculator.h"
#include "DistanceCaculator.h"
#include "AreaCaculator.h"
#include "LabelEvent.h"

#include <osgEarth/Map>
#include <osgEarth/MapNode>
#include <osgEarthUtil/Sky>
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthUtil/Shadowing>

#include <osg/Notify>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgEarth/ImageLayer>
#include <osgEarth/ModelLayer>
#include <osgEarth/GLUtils>
#include <osgEarthUtil/ExampleResources>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/Fog>
#include <osg/Fog>

#include <osgEarthSymbology/Style>
#include <osgEarthFeatures/FeatureModelLayer>

#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>
#include <osg/TexGen>

#include <osg/CullFace>
#include <osg/ClipNode>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgEarth/VirtualProgram>
#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarth/ShaderUtils>
#include <osgEarth/FileUtils>
#include <osgEarth/GLUtils>

#include <osgParticle/PrecipitationEffect>

#include <osgEarthUtil/LogarithmicDepthBuffer>
#include <osgEarthUtil/GeodeticGraticule>
#include <osgSim/SphereSegment>

#include <osgEarthFeatures/FeatureModelLayer>

#include <osgEarthDrivers/tms/TMSOptions>
#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>
#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>
#include <osgEarthDrivers/engine_rex/RexTerrainEngineOptions>

#include <osgEarthAnnotation/PlaceNode>
#include <osgEarthAnnotation/LabelNode>

#include <osgGA/GUIEventHandler>

#include <osgEarth/ThreadingUtils>
#include <osgEarth/Metrics>
#include <iostream>
#include <osgEarthUtil/Controls>

#include <osgEarthDrivers/kml/KML>
#include <osgEarthDrivers/kml/KMLOptions>

#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Texture2D>
#include <fstream>
#include "CloudBlock.h"

#include <osgSim/ScalarsToColors>
#include <osgSim/ColorRange>
#include <osgSim/ScalarBar>


class MyOSGEarthQT : public QMainWindow
{
	Q_OBJECT

public:

	MyOSGEarthQT(QWidget *parent = Q_NULLPTR);
	~MyOSGEarthQT() {}
	void sendLosHeight(float height);
	void sendViewshedPara(int numSpokes, int numSegment);

private slots:

	void on_VisibilityAnalysis(bool checked);
	void on_ViewshedAnalysis(bool checked);
	void on_RadarAnalysis(bool checked);
	void on_ClearAnalysis();
	void on_SetLosHeight();
	void on_SetViewshedPara();
	void on_kfz();
	void on_WaterAnalysis(bool checked);
	void on_TerrainProfileAnalysis(bool checked);
	void on_shengjiexian(bool checked);
	void on_Fuzhou();
	void on_Boston();
	void on_Airport();
	void on_DongBaoShan();
	void on_SciencePark();
	void on_FeaturesQuery(bool checked);
	void on_AddModel(bool checked);
	void on_FogEffect(bool checked);
	void on_Rain(bool checked);
	void on_Snow(bool checked);
	void on_Cloud(bool checked);
	void on_Fire(bool checked);
	void on_AddCityModel(bool checked);
	void on_PathRoaming(bool checked);
	void  on_Distance(bool checked);
	void  on_Area(bool checked);
	void  on_Angle(bool checked);
	void  on_slopeAnalysis(bool checked);
	//void  on_AspectAnalysis(bool checked);
	void  on_AddTiltphotographymodel(bool checked);
	void  on_Addgraticule(bool checked);
	void  on_Addsatellite(bool checked);
	void  on_AddCompass(bool checked);
	void  on_AddScaleBar(bool checked);
	void  on_AddMiniMap(bool checked);
	void  on_AddKml();
	void  on_AddVector();
	void  on_AddPoint(bool checked);
	void  on_AddLine(bool checked);
	void  on_AddPolygon(bool checked);
	void  on_AddCircle(bool checked);
	void  on_AddRectangle(bool checked);
	void  on_StraightArrow(bool checked); 
	void  on_DiagonalArrow(bool checked);
	void  on_DoubleArrow(bool checked);
	void  on_GeoLune(bool checked);
	void  on_GatheringPlace(bool checked);
	void  on_ParallelSearch(bool checked);
	void  on_SectorSearch(bool checked);
	void  on_AddEarth();
	void  on_AddElevation(); 
	void  on_Tianwa(bool checked);
	void  on_Dynamictexture(bool checked); 
	void  on_TrailLine(bool checked);
	void  on_ChangeTime(bool checked);

private:

	Ui::MyOSGEarthQTClass	ui;
	SetLosHeight*			sub_setLosHeight;
	ViewshedPara*			sub_viewshedPara;
	kfz*			sub_information;

	osg::Camera*					m_hud;
	osg::ref_ptr<osgEarth::MapNode> m_mapNode;
	osg::ref_ptr<osg::Group>		m_world;
	osg::ref_ptr<osg::Group>		m_losGroup;
	osg::ref_ptr<osgEarth::Util::SkyNode> skyNode;

	PickEvent*	m_PickEvent;

	LabelEvent*	labelEvent;
	ControlCanvas* ballooncanvas;

	osgEarth::GeoTransform* waterxform = new osgEarth::GeoTransform();

	TerrainProfileEventHandler* tp;
	osg::ref_ptr< TerrainProfileCalculator > calculator;
	osg::Camera* tp_hud;
	osg::Group* profileNode;
	osgEarth::Util::Controls::LabelControl* fidLabel;
	osgEarth::Util::Controls::LabelControl* nameLabel;

	GeodeticGraticule* gr = new GeodeticGraticule();

	ControlCanvas* feaquerycanvas;
	osgEarth::Util::RTTPicker* picker;
	osgViewer::Viewer* viewer;

	VBox* vbox;
	
	osg::Node* plane3;
	RadialLineOfSightNode* tetheredRadial;
	osg::Group* tankplanelos;
	osgEarth::GeoTransform* tankform;

	osgEarth::GeoTransform* planeform;
	osg::ref_ptr<osg::Geode> ribbongeode;
	osgEarth::GeoTransform* airportform;

	osg::Group* haze;
	osg::Group* g;
	osgParticle::PrecipitationEffect* pre = new osgParticle::PrecipitationEffect;
	osgParticle::PrecipitationEffect* rainpre = new osgParticle::PrecipitationEffect;

	osg::ref_ptr<osgEarth::GeoTransform> xform = new osgEarth::GeoTransform;
	osg::ref_ptr<osgEarth::GeoTransform> tpxform = new osgEarth::GeoTransform;
	osg::ref_ptr<osgEarth::GeoTransform> snowxform = new osgEarth::GeoTransform;
	osg::ref_ptr<osgEarth::GeoTransform> rainxform = new osgEarth::GeoTransform;
	osg::ref_ptr<osgEarth::GeoTransform> cloudxform = new osgEarth::GeoTransform;

	osg::ref_ptr<osgEarth::Util::Controls::ControlCanvas> g_controlCanvas;
	osg::ref_ptr<Compass> g_compass;
	osg::ref_ptr<ScaleBar> g_scaleBar; 
	osg::ref_ptr<OverviewMapControl> g_overviewMap;
	osgGA::GUIEventHandler* overviewmap;

	osg::ref_ptr<osg::Geode> geode_redOrbit = new osg::Geode;
	osg::ref_ptr<osg::Geode> geode_blueOrbit = new osg::Geode;
	osgEarth::GeoTransform* satelliteform;

	osg::ref_ptr<osgGA::GUIEventHandler> drawlineevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawPolygonevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawCircleevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawRectangleevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawStraightArrowevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawDiagonalArrowevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawDoubleArrowevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawLuneevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawGatheringPlaceevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawParallelSearchevent;
	osg::ref_ptr<osgGA::GUIEventHandler> drawSectorSearchevent;

	osg::ref_ptr<osgGA::GUIEventHandler> Angleevent;
	osg::ref_ptr<osgGA::GUIEventHandler> Distanceevent;
	osg::ref_ptr<osgGA::GUIEventHandler> Areaevent;

	osg::ref_ptr<osg::Geode> DTgeode;
	osg::ref_ptr<osg::Geode> ocnode;
	osg::Group* TrailLine;

	osgEarth::GeoTransform* blxform;
	osg::Node* blsc;

	osg::ref_ptr<osgEarth::GeoTransform> twxform;

	ControlCanvas* timecanvas;
};

