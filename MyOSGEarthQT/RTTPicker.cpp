#include "RTTPicker.h"

#include <osgGA/GUIEventHandler>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgEarth/MapNode>
#include <osgEarth/Registry>
#include <osgEarth/ObjectIndex>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ExampleResources>
#include <osgEarthUtil/Controls>
#include <osgEarthUtil/RTTPicker>
#include <osgEarthFeatures/Feature>
#include <osgEarthFeatures/FeatureIndex>


using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;
using namespace osgEarth::Features;


//const char* highlightVert =
//"#version " GLSL_VERSION_STR "\n"
//"uniform uint objectid_to_highlight; \n"
//"uint oe_index_objectid;      // Stage global containing object id \n"
//"flat out int selected; \n"
//"void checkForHighlight(inout vec4 vertex) \n"
//"{ \n"
//"    selected = (objectid_to_highlight > 1u && objectid_to_highlight == oe_index_objectid) ? 1 : 0; \n"
//"} \n";
//
//const char* highlightFrag =
//"#version " GLSL_VERSION_STR "\n"
//"flat in int selected; \n"
//"void highlightFragment(inout vec4 color) \n"
//"{ \n"
//"    if ( selected == 1 ) \n"
//"        color.rgb = mix(color.rgb, clamp(vec3(0.5,2.0,2.0)*(1.0-color.rgb), 0.0, 1.0), 0.5); \n"
//"} \n";
//
//void installHighlighter()
//{
//	osg::ref_ptr<osgEarth::MapNode> mapNode = new osgEarth::MapNode;
//	osg::Uniform*     highlightUniform;
//
//	osg::StateSet* stateSet = mapNode->getOrCreateStateSet();
//	int attrLocation = Registry::objectIndex()->getObjectIDAttribLocation();
//
//	// This shader program will highlight the selected object.
//	VirtualProgram* vp = VirtualProgram::getOrCreate(stateSet);
//	vp->setFunction("checkForHighlight", highlightVert, ShaderComp::LOCATION_VERTEX_CLIP);
//	vp->setFunction("highlightFragment", highlightFrag, ShaderComp::LOCATION_FRAGMENT_COLORING);
//
//	// Since we're accessing object IDs, we need to load the indexing shader as well:
//	Registry::objectIndex()->loadShaders(vp);
//
//	// A uniform that will tell the shader which object to highlight:
//	highlightUniform = new osg::Uniform("objectid_to_highlight", 0u);
//	stateSet->addUniform(highlightUniform);
//}

ReadoutCallback::ReadoutCallback(ControlCanvas* container) : _lastFID(~0)
	{
		_grid = new Grid();
		_grid->setBackColor(osg::Vec4(0, 0, 0, 0.7f));
		container->addControl(_grid);
	}

void ReadoutCallback::onHit(ObjectID id)
{
	FeatureIndex* index = Registry::objectIndex()->get<FeatureIndex>(id).get();
	Feature* feature = index ? index->getFeature(id) : 0L;
	if (feature && feature->getFID() != _lastFID)
	{
		_grid->clearControls();
		unsigned r = 0;

		_grid->setControl(0, r, new LabelControl("FID", Color::Red));
		_grid->setControl(1, r, new LabelControl(Stringify() << feature->getFID(), Color::White));
		++r;

		const AttributeTable& attrs = feature->getAttrs();
		for (AttributeTable::const_iterator i = attrs.begin(); i != attrs.end(); ++i, ++r)
		{
			_grid->setControl(0, r, new LabelControl(i->first, 14.0f, Color::Yellow));
			_grid->setControl(1, r, new LabelControl(i->second.getString(), 14.0f, Color::White));
		}
		if (!_grid->visible())
			_grid->setVisible(true);

		_lastFID = feature->getFID();
	}
	//highlightUniform->set(id);
}

void ReadoutCallback::onMiss()
{
	_grid->setVisible(false);
	_lastFID = 0u;
	//highlightUniform->set(0u);
}

bool ReadoutCallback::accept(const osgGA::GUIEventAdapter& ea, const osgGA::GUIActionAdapter& aa)
{
	return ea.getEventType() == ea.RELEASE; // click
}

void ReadoutCallback::removecontrols()
{
	viewer->stopThreading();

	
	viewer->removeEventHandler(picker);
	picker = 0L;

	viewer->startThreading();
	std::cout << "test" << std::endl;
}