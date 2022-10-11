#pragma once

#ifndef OVERVIEWMAP_H
#define OVERVIEWMAP_H 1

#include <osgEarthUtil/Controls>
#include <osgEarthUtil/EarthManipulator>
#include <osg/Texture2D>
#include<osg/Image>

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

class  OverviewMapControl : public Control
{
public:
	OverviewMapControl(osg::Image* image = 0L);

	/** dtor */
	virtual ~OverviewMapControl() { }

	void setImage(osg::Image* image);
	osg::Image* getImage() const { return _image.get(); }

	/** Rotates the image. */
	void setRotation(const Angular& angle);
	const Angular& getRotation() const { return _rotation; }

	/** Tells the control to fix its minimum size to account to rotation. Otherwise the
		control will auto-size its width/height based on the rotation angle. */
	void setFixSizeForRotation(bool value);
	bool getFixSizeForRotation() const { return _fixSizeForRot; }

	osg::Geometry* getOrCreateCross();

	osg::Geode* getGeode() { return Control::getGeode(); }


	osg::Geometry* getOrCreateRedPoints();
	osg::Geometry* getOrCreateBluePoints();

	virtual void setVisible(bool value);

public: // Control
	virtual void calcSize(const ControlContext& context, osg::Vec2f& out_size);
	virtual void draw(const ControlContext& cx);

	osg::Vec3 convertXYZ2UV(const osg::Vec3 &v3);

private:
	friend class OverviewMapHandler;
	osg::ref_ptr<osg::Image> _image;
	Angular _rotation;
	bool _fixSizeForRot;
	//      osg::Geometry* _geom;
	osg::ref_ptr<osg::Geometry>  _cross;
	osg::ref_ptr<osg::Geometry> _redPts;
	osg::ref_ptr<osg::Geometry> _bluePts;
	osg::ref_ptr<osg::MatrixTransform> _xform;
	float _opacity;

};

struct OverviewMapHandler : public osgGA::GUIEventHandler {
	OverviewMapHandler(OverviewMapControl* om, osgEarth::Util::EarthManipulator* em)
		: om_(om), em_(em), clicked_(false) { }
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	bool isInside(const osg::Vec3& pos);
	bool clicked_;
	osg::Vec3 clickPos_;
	osg::Vec3 startingPos_;
	OverviewMapControl* om_;
	osgEarth::Util::EarthManipulator* em_;
	void processDrag(const osg::Vec3& newMousePos);

};
#endif
