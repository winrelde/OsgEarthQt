#include "TPGEvent.h"

TerrainProfileEventHandler::TerrainProfileEventHandler(osgEarth::MapNode* mapNode, osg::Group* root, osgEarth::Util::TerrainProfileCalculator* profileCalculator) :
	_mapNode(mapNode),
	_root(root),
	_startValid(false),
	_profileCalculator(profileCalculator)
{
	_start = profileCalculator->getStart().vec3d();
	_end = profileCalculator->getEnd().vec3d();
	compute();
}

bool TerrainProfileEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	if (ea.getEventType() == ea.PUSH && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
	{
		osg::Vec3d world;
		if (_mapNode->getTerrain()->getWorldCoordsUnderMouse(aa.asView(), ea.getX(), ea.getY(), world))
		{
			GeoPoint mapPoint;
			mapPoint.fromWorld(_mapNode->getMapSRS(), world);
			//_mapNode->getMap()->worldPointToMapPoint( world, mapPoint );

			if (!_startValid)
			{
				_startValid = true;
				_start = mapPoint.vec3d();
				if (_featureNode.valid())
				{
					_root->removeChild(_featureNode.get());
					_featureNode = 0;
				}
			}
			else
			{
				_end = mapPoint.vec3d();
				compute();
				_startValid = false;
			}
		}
	}
	return false;
}
//计算
void TerrainProfileEventHandler::compute()
{
	//Tell the calculator about the new start/end points
	_profileCalculator->setStartEnd(GeoPoint(_mapNode->getMapSRS(), _start.x(), _start.y()),
		GeoPoint(_mapNode->getMapSRS(), _end.x(), _end.y()));

	//移除之前做过的要素
	if (_featureNode.valid())
	{
		_root->removeChild(_featureNode.get());
		_featureNode = 0;
	}

	LineString* line = new LineString();
	line->push_back(_start);
	line->push_back(_end);
	Feature* feature = new Feature(line, _mapNode->getMapSRS());
	feature->geoInterp() = GEOINTERP_GREAT_CIRCLE;

	//Define a style for the line
	Style style;
	LineSymbol* ls = style.getOrCreateSymbol<LineSymbol>();
	ls->stroke()->color() = Color::Yellow;
	ls->stroke()->width() = 3.0f;
	ls->tessellationSize()->set(100.0, Units::KILOMETERS);

	AltitudeSymbol* alt = style.getOrCreate<AltitudeSymbol>();
	alt->clamping() = alt->CLAMP_TO_TERRAIN;
	alt->technique() = alt->TECHNIQUE_DRAPE;

	RenderSymbol* render = style.getOrCreate<RenderSymbol>();
	render->lighting() = false;

	feature->style() = style;
	_featureNode = new FeatureNode(feature);
	_featureNode->setMapNode(_mapNode);
	_root->addChild(_featureNode.get());

}

void TerrainProfileEventHandler::Remove()
{
	_root->removeChild(_featureNode.get());

}

osg::Camera* createHud(double width, double height)
{
	osg::Camera* hud = new osg::Camera;
	hud->setProjectionMatrix(osg::Matrix::ortho2D(0, width, 0, height));
	hud->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	hud->setViewMatrix(osg::Matrix::identity());
	hud->setClearMask(GL_DEPTH_BUFFER_BIT);
	hud->setRenderOrder(osg::Camera::POST_RENDER);
	hud->setAllowEventFocus(false);//不允许动作产生
	osg::StateSet* hudSS = hud->getOrCreateStateSet();
	GLUtils::setLighting(hudSS, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	hudSS->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	hudSS->setMode(GL_BLEND, osg::StateAttribute::ON);

	return hud;
}