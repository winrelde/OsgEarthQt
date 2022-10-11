#pragma once

#include <osgGA/GUIEventHandler>
#include <osgEarth/MapNode>
#include <osgViewer/Viewer>
#include <osgEarthUtil/Controls>
#include <osgEarthUtil/EarthManipulator>
#include <osgSim/SphereSegment>

/*-----------------------------------------------信息交互事件-----------------------------------*/

class LabelEvent :public osgGA::GUIEventHandler
{
public:
	LabelEvent(osgEarth::Util::Controls::LabelControl* flyLabel1, 
		osgEarth::Util::Controls::LabelControl* flyLabel2, 
		osgEarth::Util::Controls::LabelControl* flyLabel3, 
		osgEarth::Util::Controls::LabelControl* flyLabel4);
	~LabelEvent(void);

	bool handle(const osgGA::GUIEventAdapter&ea, osgGA::GUIActionAdapter&aa);

private:
	osgEarth::Util::Controls::LabelControl* flyCoords1;
	osgEarth::Util::Controls::LabelControl* flyCoords2;
	osgEarth::Util::Controls::LabelControl* flyCoords3;
	osgEarth::Util::Controls::LabelControl* flyCoords4;

};
