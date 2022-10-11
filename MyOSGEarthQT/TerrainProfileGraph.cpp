#include "TerrainProfileGraph.h"



TerrainProfileGraph::TerrainProfileGraph(TerrainProfileCalculator* profileCalculator, double graphWidth = 200, double graphHeight = 100) :
	_profileCalculator(profileCalculator),
	_graphWidth(graphWidth),
	_graphHeight(graphHeight),
	_color(1.0f, 1.0f, 0.0f, 1.0f),
	_backcolor(0.0f, 0.0f, 0.0f, 0.5f)
{
	_graphChangedCallback = new GraphChangedCallback(this);
	_profileCalculator->addChangedCallback(_graphChangedCallback.get());

	float textSize = 8;
	osg::ref_ptr< osgText::Font> font = osgEarth::Registry::instance()->getDefaultFont();

	osg::Vec4 textColor = osg::Vec4f(1, 0, 0, 1);

	_distanceMinLabel = new osgText::Text();
	_distanceMinLabel->setCharacterSize(textSize);
	_distanceMinLabel->setFont(font.get());
	_distanceMinLabel->setAlignment(osgText::TextBase::LEFT_BOTTOM);
	_distanceMinLabel->setColor(textColor);

	_distanceMaxLabel = new osgText::Text();
	_distanceMaxLabel->setCharacterSize(textSize);
	_distanceMaxLabel->setFont(font.get());
	_distanceMaxLabel->setAlignment(osgText::TextBase::RIGHT_BOTTOM);
	_distanceMaxLabel->setColor(textColor);

	_elevationMinLabel = new osgText::Text();
	_elevationMinLabel->setCharacterSize(textSize);
	_elevationMinLabel->setFont(font.get());
	_elevationMinLabel->setAlignment(osgText::TextBase::RIGHT_BOTTOM);
	_elevationMinLabel->setColor(textColor);

	_elevationMaxLabel = new osgText::Text();
	_elevationMaxLabel->setCharacterSize(textSize);
	_elevationMaxLabel->setFont(font.get());
	_elevationMaxLabel->setAlignment(osgText::TextBase::RIGHT_TOP);
	_elevationMaxLabel->setColor(textColor);
}

TerrainProfileGraph::~TerrainProfileGraph()
{
	_profileCalculator->removeChangedCallback(_graphChangedCallback.get());
}

void TerrainProfileGraph::setTerrainProfile(const TerrainProfile& profile)
{
	_profile = profile;
	redraw();
}

//重绘场景图
//Redraws the graph
void TerrainProfileGraph::redraw()
{
	removeChildren(0, getNumChildren());

	addChild(createBackground(_graphWidth, _graphHeight, _backcolor));

	osg::Geometry* geom = new osg::Geometry;
	geom->setUseVertexBufferObjects(true);

	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->reserve(_profile.getNumElevations());
	geom->setVertexArray(verts);
	if (verts->getVertexBufferObject())
		verts->getVertexBufferObject()->setUsage(GL_STATIC_DRAW_ARB);

	osg::Vec4Array* colors = new osg::Vec4Array(osg::Array::BIND_OVERALL);
	colors->push_back(_color);
	geom->setColorArray(colors);

	double minElevation, maxElevation;
	_profile.getElevationRanges(minElevation, maxElevation);
	double elevationRange = maxElevation - minElevation;

	double totalDistance = _profile.getTotalDistance();

	for (unsigned int i = 0; i < _profile.getNumElevations(); i++)
	{
		double distance = _profile.getDistance(i);
		double elevation = _profile.getElevation(i);

		double x = (distance / totalDistance) * _graphWidth;
		double y = ((elevation - minElevation) / elevationRange) * _graphHeight;
		verts->push_back(osg::Vec3(x, y, 0));
	}

	geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, verts->size()));
	osg::Geode* graphGeode = new osg::Geode;
	graphGeode->addDrawable(geom);
	addChild(graphGeode);

	osg::Geode* labelGeode = new osg::Geode;
	labelGeode->addDrawable(_distanceMinLabel.get());
	labelGeode->addDrawable(_distanceMaxLabel.get());
	labelGeode->addDrawable(_elevationMinLabel.get());
	labelGeode->addDrawable(_elevationMaxLabel.get());

	_distanceMinLabel->setPosition(osg::Vec3(0, 0, 0));
	_distanceMaxLabel->setPosition(osg::Vec3(_graphWidth - 15, 0, 0));
	_elevationMinLabel->setPosition(osg::Vec3(_graphWidth - 5, 10, 0));
	_elevationMaxLabel->setPosition(osg::Vec3(_graphWidth - 5, _graphHeight, 0));

	_distanceMinLabel->setText("0m");
	_distanceMaxLabel->setText(toString<int>((int)totalDistance) + std::string("m"));

	_elevationMinLabel->setText(toString<int>((int)minElevation) + std::string("m"));
	_elevationMaxLabel->setText(toString<int>((int)maxElevation) + std::string("m"));

	addChild(labelGeode);

}

osg::Node* TerrainProfileGraph::createBackground(double width, double height, const osg::Vec4f& backgroundColor)
{
	//Create a background quad
	osg::Geometry* geometry = new osg::Geometry();
	geometry->setUseVertexBufferObjects(true);

	osg::Vec3Array* verts = new osg::Vec3Array();
	verts->reserve(4);
	verts->push_back(osg::Vec3(0, 0, 0));
	verts->push_back(osg::Vec3(width, 0, 0));
	verts->push_back(osg::Vec3(width, height, 0));
	verts->push_back(osg::Vec3(0, height, 0));
	geometry->setVertexArray(verts);
	if (verts->getVertexBufferObject())
		verts->getVertexBufferObject()->setUsage(GL_STATIC_DRAW_ARB);

	osg::Vec4Array* colors = new osg::Vec4Array(osg::Array::BIND_OVERALL);
	colors->push_back(backgroundColor);
	geometry->setColorArray(colors);

	geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));

	osg::Geode* geode = new osg::Geode;
	geode->addDrawable(geometry);

	return geode;

}

