#pragma once
#include <osg/Notify>
#include <osgGA/StateSetManipulator>
#include <osgGA/GUIEventHandler>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgEarth/MapNode>
#include <osgEarth/XmlUtils>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthUtil/TerrainProfile>
#include <osgEarth/GeoMath>
#include <osgEarth/Registry>
#include <osgEarth/FileUtils>
#include <osgEarth/GLUtils>
#include <osgEarthFeatures/Feature>
#include <osgEarthAnnotation/FeatureNode>
#include <osgText/Text>
#include <osgText/Font>
#include <osg/io_utils>

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

class TerrainProfileGraph : public osg::Group
{
public:
	struct GraphChangedCallback : public TerrainProfileCalculator::ChangedCallback
	{
		
			GraphChangedCallback(TerrainProfileGraph* graph) :
				_graph(graph)
			{
			}
			//*计算两点之间的TerrainProfile； 监视场景图以更改高程并更新轮廓
			virtual void onChanged(const osgEarth::Util::TerrainProfileCalculator* sender)
			{
				_graph->setTerrainProfile(sender->getProfile());
			}

			TerrainProfileGraph* _graph;
		
	};

	TerrainProfileGraph(osgEarth::Util::TerrainProfileCalculator* profileCalculator, double graphWidth, double graphHeight);
	osg::Node* createBackground(double width, double height, const osg::Vec4f& backgroundColor);
	void setTerrainProfile(const TerrainProfile& profile);
	void redraw();

	~TerrainProfileGraph();

private:
	osg::ref_ptr<osgText::Text> _distanceMinLabel, _distanceMaxLabel, _elevationMinLabel, _elevationMaxLabel;

	osg::Vec4f _backcolor;
	osg::Vec4f _color;
	TerrainProfile _profile;
	osg::ref_ptr< osgEarth::Util::TerrainProfileCalculator > _profileCalculator;
	double _graphWidth, _graphHeight;
	osg::ref_ptr< GraphChangedCallback > _graphChangedCallback;

};

