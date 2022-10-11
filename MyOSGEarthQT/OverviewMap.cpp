#include <Windows.h>

#include "OverviewMap.h"
#include <osg/LineWidth>
#include <osg/Point>
#include <osgEarthSymbology/Color>


namespace {

	void convertXY2LatLon(float w, float h, float x, float y, float& lat, float& lon)
	{
		float u = (x + 0.5) / w;
		float v = (y + 0.5) / h;
		lat = (u - 0.5) * (2 * osg::PI);
		lon = (0.5 - v) * osg::PI;
	}

	bool worldToScreen(osgViewer::View* viewer, const osg::Vec3d& world, osg::Vec3d *screen, bool invertY)
	{
		if (!viewer) {
			//        ERROR("No viewer");
			return false;
		}
		osg::Camera *cam = viewer->getCamera();
		osg::Matrixd MVP = cam->getViewMatrix() * cam->getProjectionMatrix();
		// Get clip coords
		osg::Vec4d pt;
		pt = osg::Vec4d(world, 1.0) * MVP;
		// Clip
		if (pt.x() < -pt.w() ||
			pt.x() > pt.w() ||
			pt.y() < -pt.w() ||
			pt.y() > pt.w() ||
			pt.z() < -pt.w() ||
			pt.z() > pt.w()) {
			// Outside frustum
	//        TRACE("invalid pt: %g,%g,%g,%g", pt.x(), pt.y(), pt.z(), pt.w());
			return false;
		}
		//    TRACE("clip pt: %g,%g,%g,%g", pt.x(), pt.y(), pt.z(), pt.w());
			// Perspective divide: now NDC
		pt /= pt.w();
		const osg::Viewport *viewport = cam->getViewport();
#if 1
		screen->x() = viewport->x() + viewport->width() * 0.5 + pt.x() * viewport->width() * 0.5;
		screen->y() = viewport->y() + viewport->height() * 0.5 + pt.y() * viewport->height() * 0.5;
		//double near = 0;
		//double far = 1;
		//screen->z() = (far + near) * 0.5 + (far - near) * 0.5 * pt.z();
		screen->z() = 0.5 + 0.5 * pt.z();
#else
		*screen = osg::Vec3d(pt.x(), pt.y(), pt.z()) * cam->getViewport()->computeWindowMatrix();
#endif
		if (invertY) {
			screen->y() = viewport->height() - screen->y();
		}
		//    TRACE("screen: %g,%g,%g", screen->x(), screen->y(), screen->z());
		return true;
	}

	void calculateRotatedSize(float w, float h, float angle_rad, float& out_w, float& out_h)
	{
		float x1 = -w / 2, x2 = w / 2, x3 = w / 2, x4 = -w / 2;
		float y1 = h / 2, y2 = h / 2, y3 = -h / 2, y4 = -h / 2;

		float cosa = cos(angle_rad);
		float sina = sin(angle_rad);

		float
			x11
			= x1 * cosa + y1 * sina,
			y11 = -x1 * sina + y1 * cosa,
			x21 = x2 * cosa + y2 * sina,
			y21 = -x2 * sina + y2 * cosa,
			x31 = x3 * cosa + y3 * sina,
			y31 = -x3 * sina + y3 * cosa,
			x41 = x4 * cosa + y4 * sina,
			y41 = -x4 * sina + y3 * cosa;

		float xmin = (std::min)(x11, (std::min)(x21, (std::min)(x31, x41)));
		float ymin = (std::min)(y11, (std::min)(y21, (std::min)(y31, y41)));

		float xmax = (std::max)(x11, (std::max)(x21, (std::max)(x31, x41)));
		float ymax = (std::max)(y11, (std::max)(y21, (std::max)(y31, y41)));

		out_w = xmax - xmin;
		out_h = ymax - ymin;
	}

	void rot(float x, float y, const osg::Vec2f& c, float angle_rad, osg::Vec3f& out)
	{
		float cosa = cos(angle_rad);
		float sina = sin(angle_rad);
		out.x() = (c.x() - x) * cosa - (c.y() - y) * sina + c.x();
		out.y() = (c.y() - y) * cosa + (c.x() - x) * sina + c.y();
		out.z() = 0.0f;
	}

	// Convenience method to create safe Control geo.
	// Since Control geo can change, we need to always set it
	// to DYNAMIC data variance.
	osg::Geometry* newGeometry()
	{
		osg::Geometry* geom = new osg::Geometry();
		geom->setUseVertexBufferObjects(true);
		geom->setUseDisplayList(false);
		geom->setDataVariance(osg::Object::DYNAMIC);
		return geom;
	}

}

OverviewMapControl::OverviewMapControl(osg::Image* image)
	: _rotation(0.0, Units::RADIANS)
	, _fixSizeForRot(false)
	, _opacity(0.5f)
{
	setImage(image);
	//setAlign(Control::ALIGN_LEFT, Control::ALIGN_BOTTOM);
	setAlign(Control::ALIGN_LEFT, Control::ALIGN_BOTTOM);
}

osg::Geometry* OverviewMapControl::getOrCreateCross()
{
	if (!_cross.valid()) {
		_cross = new osg::Geometry;
		_cross->setUseVertexBufferObjects(true);
		osg::ref_ptr<osg::Vec3dArray> crossVt = new osg::Vec3dArray;
		_cross->setVertexArray(crossVt);
		osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
		normal->push_back(osg::Vec3(0, 0, 1));
		_cross->setNormalArray(normal, osg::Array::BIND_OVERALL);
		osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
		color->push_back(osg::Vec4(0.8, 0.8, 0.8, 1));
		_cross->setColorArray(color, osg::Array::BIND_OVERALL);
		_cross->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 4));
		_cross->getOrCreateStateSet()->setAttribute(
			new osg::LineWidth(1.0), osg::StateAttribute::ON);
	}
	return _cross.get();
}

osg::Vec3 OverviewMapControl::convertXYZ2UV(const osg::Vec3& v3)
{
	double w = width().get();
	double h = height().get();
	double x = (v3.x() + 180.0) * w / 360.0;
	double y = (v3.y() + 90.0) * h / 180.0;
	return osg::Vec3(x, y, 0.0);
}


osg::Geometry* createPoints(const osg::Vec4& color)
{
	osg::Geometry* pts = new osg::Geometry;
	pts->setUseVertexBufferObjects(true);
	osg::ref_ptr<osg::Vec3dArray> ptVt = new osg::Vec3dArray;
	pts->setVertexArray(ptVt);
	pts->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, ptVt->size()));

	// bind color
	osg::ref_ptr<osg::Vec4Array> colorArry = new osg::Vec4Array;
	colorArry->push_back(color);
	pts->setColorArray(colorArry, osg::Array::BIND_OVERALL);
	// bind point state set
	osg::StateSet* stateset = new osg::StateSet;
	osg::Point* point = new osg::Point;
	// set point size
	point->setSize(5.0f);
	stateset->setAttribute(point);

	pts->setStateSet(stateset);

	return pts;
}

osg::Geometry* OverviewMapControl::getOrCreateRedPoints()
{
	if (!_redPts.valid()) {
		_redPts = createPoints(osgEarth::Symbology::Color(0xFF0000FF, osgEarth::Symbology::Color::RGBA));
	}
	return _redPts.get();
}

osg::Geometry *OverviewMapControl::getOrCreateBluePoints()
{
	if (!_bluePts.valid()) {
		_bluePts = createPoints(osgEarth::Symbology::Color(0x1C86EFFF, osgEarth::Symbology::Color::RGBA));
	}
	return _bluePts.get();
}

void OverviewMapControl::setVisible(bool value) {
	Control::setVisible(value);
	if (_xform.valid()) {
		_xform->setNodeMask(value ? ~0 : 0);
	}
}

void OverviewMapControl::setImage(osg::Image* image)
{
	if (image != _image.get()) {
		_image = image;
		dirty();
	}
}

void OverviewMapControl::setRotation(const Angular& angle)
{
	if (angle != _rotation) {
		_rotation = angle;
		dirty();
	}
}

void OverviewMapControl::setFixSizeForRotation(bool value)
{
	if (_fixSizeForRot != value) {
		_fixSizeForRot = value;
		dirty();
	}
}

void OverviewMapControl::calcSize(const ControlContext& cx, osg::Vec2f& out_size)
{
	if (visible() == true) {
		_renderSize.set(0, 0);

		//First try the explicit settings
		if (width().isSet() && height().isSet()) {
			_renderSize.set(width().value(), height().value());
		}
		//Second try the size of the image itself
		else if (_image.valid()) {
			_renderSize.set(_image->s(), _image->t());
		}
		//Lastly just use the default values for width and height
		else {
			_renderSize.set(width().value(), height().value());
		}

		//if there's a rotation angle, rotate
		float rot = _fixSizeForRot ? osg::PI_4 : _rotation.as(Units::RADIANS);
		if (rot != 0.0f) {
			calculateRotatedSize(
				_renderSize.x(), _renderSize.y(),
				rot,
				_renderSize.x(), _renderSize.y());
		}

		out_size.set(
			margin().left() + margin().right() + _renderSize.x(),
			margin().top() + margin().bottom() + _renderSize.y());

		//_dirty = false;
	}
	else {
		out_size.set(0, 0);
	}
}

#undef IMAGECONTROL_TEXRECT

void OverviewMapControl::draw(const ControlContext& cx)
{
	Control::draw(cx);

	if (visible() && parentIsVisible() && _image.valid()) {
		//TODO: this is not precisely correct..images get deformed slightly..
		osg::Geometry* g = newGeometry();

		float rx = osg::round(_renderPos.x());
		float ry = osg::round(_renderPos.y());
		float vph = cx._vp->height();

		osg::Vec3Array* verts = new osg::Vec3Array(6);
		g->setVertexArray(verts);

		if (_rotation.as(Units::RADIANS) != 0.0f || _fixSizeForRot == true) {
			osg::Vec2f rc(rx + _renderSize.x() / 2, (vph - ry) - _renderSize.y() / 2);
			float ra = osg::PI - _rotation.as(Units::RADIANS);

			rx += 0.5 * _renderSize.x() - 0.5 * (float)_image->s();
			ry += 0.5 * _renderSize.y() - 0.5 * (float)_image->t();

			rot(rx, vph - ry, rc, ra, (*verts)[0]);
			rot(rx, vph - ry - _image->t(), rc, ra, (*verts)[1]);
			rot(rx + _image->s(), vph - ry - _image->t(), rc, ra, (*verts)[2]);
			(*verts)[3].set((*verts)[2]);
			rot(rx + _image->s(), vph - ry, rc, ra, (*verts)[4]);
			(*verts)[5].set((*verts)[0]);
		}
		else {
			(*verts)[0].set(rx, vph - ry, 0);
			(*verts)[1].set(rx, vph - ry - _renderSize.y(), 0);
			(*verts)[2].set(rx + _renderSize.x(), vph - ry - _renderSize.y(), 0);
			(*verts)[3].set((*verts)[2]);
			(*verts)[4].set(rx + _renderSize.x(), vph - ry, 0);
			(*verts)[5].set((*verts)[0]);
		}

		osg::ref_ptr<osg::Vec3Array> normal = new osg::Vec3Array;
		normal->push_back(osg::Vec3(0, -1, 0));
		g->setNormalArray(normal, osg::Array::BIND_OVERALL);

		g->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 6));

		osg::Vec4Array* c = new osg::Vec4Array(1);
		(*c)[0] = osg::Vec4f(1, 1, 1, 1);
		g->setColorArray(c);
		g->setColorBinding(osg::Geometry::BIND_OVERALL);

		bool flip = _image->getOrigin() == osg::Image::TOP_LEFT;

		osg::Vec2Array* t = new osg::Vec2Array(6);

#ifdef IMAGECONTROL_TEXRECT

		(*t)[0].set(0, flip ? 0 : _image->t() - 1);
		(*t)[1].set(0, flip ? _image->t() - 1 : 0);
		(*t)[2].set(_image->s() - 1, flip ? _image->t() - 1 : 0);
		(*t)[3].set((*t)[2]);
		(*t)[4].set(_image->s() - 1, flip ? 0 : _image->t() - 1);
		(*t)[5].set((*t)[0]);
		osg::TextureRectangle* tex = new osg::TextureRectangle(_image.get());

#else

		(*t)[0].set(0, flip ? 0 : 1);
		(*t)[1].set(0, flip ? 1 : 0);
		(*t)[2].set(1, flip ? 1 : 0);
		(*t)[3].set((*t)[2]);
		(*t)[4].set(1, flip ? 0 : 1);
		(*t)[5].set((*t)[0]);
		
#endif
		osg::Texture2D* tex = new osg::Texture2D;
		if (_image.valid())
			tex = new osg::Texture2D(_image.get());

		g->setTexCoordArray(0, t);

		tex->setResizeNonPowerOfTwoHint(false);

		tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		g->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
		g->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		g->getOrCreateStateSet()->setAttribute(new osg::LineWidth(2), osg::StateAttribute::ON);
		/*osg::TexEnv* texenv = new osg::TexEnv( osg::TexEnv::MODULATE );
		g->getStateSet()->setTextureAttributeAndModes( 0, texenv, osg::StateAttribute::ON );
		 */
		 //        getGeode()->addDrawable(g);

		 //        getGeode()->addDrawable(getOrCreateCross());
		 //        getGeode()->addDrawable(getOrCreatePoints());
		if (!_xform.valid()) {

			_xform = new osg::MatrixTransform;
			//_xform->addChild(g);
			_xform->addChild(getOrCreateCross());
			_xform->addChild(getOrCreateRedPoints());
			_xform->addChild(getOrCreateBluePoints());
			addChild(_xform);
		}

		_dirty = false;
	}
}

bool OverviewMapHandler::isInside(const osg::Vec3& pos)
{
	osg::Vec3 trans = om_->_xform->getMatrix().getTrans();
	// Clicked below or left
	if (pos.x() < trans.x() || pos.y() < trans.y())
		return false;
	// Clicked right
	if (pos.x() > trans.x() + om_->width().value())
		return false;
	// Clicked above
	if (pos.y() > trans.y() + om_->height().value())
		return false;
	return true;

	//return (x > om_->x().value() && y > om_->y().value() && x < om_->x().value() + om_->width().value() && y < om_->y().value() + om_->height().value());
}

void OverviewMapHandler::processDrag(const osg::Vec3& newMousePos)
{
	const osg::Vec3 delta = (newMousePos - clickPos_);
	const osg::Vec3 newPos = startingPos_ + delta;
	om_->_xform->setMatrix(osg::Matrix::translate(newPos));
}


bool OverviewMapHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
	if (ea.getEventType() == ea.FRAME) {
		if (em_) {
			osgEarth::Viewpoint vp = em_->getViewpoint();
			osgEarth::GeoPoint pt = vp.focalPoint().get();
			double mapWidth = om_->width().get();
			double mapHeight = om_->height().get();
			double x = (pt.vec3d().x() + 180.0) * mapWidth / 360.0;
			double y = (pt.vec3d().y() + 90.0) * mapHeight / 180.0;
			if (om_) {
				osg::Vec3dArray* crossVt = dynamic_cast<osg::Vec3dArray*>(om_->getOrCreateCross()->getVertexArray());
				if (crossVt) {
					crossVt->clear();
					crossVt->push_back(osg::Vec3d(x, y - 5, 0));
					crossVt->push_back(osg::Vec3d(x, y + 5, 0));
					crossVt->push_back(osg::Vec3d(x - 5, y, 0));
					crossVt->push_back(osg::Vec3d(x + 5, y, 0));
					crossVt->dirty();
				}
			}
		}
	}
	else if (ea.getEventType() == ea.PUSH && ea.getButton() == ea.MIDDLE_MOUSE_BUTTON) {
		clickPos_ = osg::Vec3f(ea.getX(), ea.getY(), 0.0);
		if (isInside(clickPos_)) {
			startingPos_ = om_->_xform->getMatrix().getTrans();
			clicked_ = true;
			// Turn off the outline
			return true;
		}
	}
	else if (ea.getEventType() == ea.RELEASE) {
		if (clicked_) {
			processDrag(osg::Vec3f(ea.getX(), ea.getY(), 0.0));
			clicked_ = false;
			return true;
		}

	}
	else if (ea.getEventType() == ea.DRAG) {
		if (clicked_) {
			processDrag(osg::Vec3f(ea.getX(), ea.getY(), 0.0));
			return true;
		}
	}
	else if (ea.getEventType() == ea.MOVE) {
		if (isInside(osg::Vec3f(ea.getX(), ea.getY(), 0.0))) {
		}
		else {
		}

	}
	else if (ea.getEventType() == ea.PUSH && ea.getButton() == ea.LEFT_MOUSE_BUTTON) {
		float x = ea.getX();
		float y = ea.getY();
		float w = om_->width().value();
		float h = om_->height().value();
		float lat, lon;

		if (isInside(osg::Vec3f(x, y, 0.0))) {
			osg::Vec3 clickPos(ea.getX(), ea.getY(), 0.0);
			osg::Vec3 omPos(om_->_xform->getMatrix().getTrans());
			osg::Vec3 delta = clickPos - omPos;
			convertXY2LatLon(w, h, delta.x(), h - delta.y(), lat, lon);
			Viewpoint vp;
			vp.focalPoint() = GeoPoint(SpatialReference::get("wgs84"), osg::RadiansToDegrees(lat), osg::RadiansToDegrees(lon), 0, ALTMODE_ABSOLUTE);
			vp.heading()->set(0.0, Units::DEGREES);
			vp.pitch()->set(-89.0, Units::DEGREES);
			vp.range()->set(SpatialReference::get("wgs84")->getEllipsoid()->getRadiusEquator()*0.5, Units::METERS);
			vp.positionOffset()->set(0, 0, 0);
			em_->setViewpoint(vp);
			return true;
		}
	}

	return false;
}
