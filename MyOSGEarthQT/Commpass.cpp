#include <Windows.h>
#include <osgEarthUtil/Controls>
#include <osgDB/ReadFile>
#include <osg/Camera>
#include <osgEarthSymbology/Color>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/Controls>
#include <osgEarth/Units>
#include "Compass.h"
#include <assert.h>

namespace ui = osgEarth::Util::Controls;

#define M_PI 3.14159265358979323846  /* mathematical constant pi */

///Radian to degree conversion factor
static const double RAD2DEG = 180.0 / M_PI;
///Degree to radian conversion factor
static const double DEG2RAD = M_PI / 180.0;

/**
* Adjusts incoming angle to fit the range [0, 360)
* @param[in ] in angle (deg)
* @return equivalent angle between 0 and 360 (deg)
*/
inline double angFix360(double in)
{
	// According to the Intel HotSpot, the call to fmod is expensive, so only make the call if necessary
	if ((in < 0.0) || (in >= 360.0))
	{
		in = fmod(in, 360.0);

		if (in < 0.0)
			in += 360.0;
	}
	return in;
}

/**
* Checks the equality of two values based on a tolerance
* @param[in ] a First value to compare
* @param[in ] b Second value to compare
* @param[in ] t Comparison tolerance
* @return the equality of two values based on a tolerance
*/
inline bool areEqual(double a, double b, double t = 1.0e-6)
{
	return fabs(a - b) < t;
}


/**
 * Callback handler for frame updates, for viewpoint/heading changes
 */
class Compass::FrameEventHandler : public osgGA::GUIEventHandler
{
public:
	/** Constructs a new FrameEventHandler */
	explicit FrameEventHandler(Compass* compass) : compass_(compass)
	{}
	/** Handles frame updates and returns false so other handlers can process as well */
	bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
	{
		if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
			compass_->update_();

		return false;
	}

protected:
	virtual ~FrameEventHandler() {}
private:
	Compass* compass_;
};

Compass::Compass(const std::string& compassFilename, osgEarth::Util::Controls::ControlCanvas* canvas) :
	drawView_(NULL),
	activeView_(NULL),
	compass_(NULL),
	readout_(NULL),
	pointer_(NULL),
	compassUpdateEventHandler_(NULL),
	canvas_(canvas)
{
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile(compassFilename);
	if (image)
	{
		compass_ = new osgEarth::Util::Controls::ImageControl(image);
		compass_->setAbsorbEvents(false);
		compass_->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_RIGHT);
		compass_->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_TOP);
		compass_->setFixSizeForRotation(true);
		compass_->setName("Compass Image");

		// get the compass size to place text properly
		const float compassSize = static_cast<float>(image->t());
		// font size will be 12% size of the total image
		const int fontSize = static_cast<int>(image->t() * 0.12);

		// using default font and color
		readout_ = new osgEarth::Util::Controls::LabelControl("0.0", static_cast<float>(fontSize));
		readout_->setAbsorbEvents(false);
		readout_->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_RIGHT);
		readout_->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_TOP);
		// set the text to appear in the upper middle of the compass image, 79% up, 53% across
		readout_->setPadding(osgEarth::Util::Controls::Control::SIDE_TOP, compassSize * 0.7);
		readout_->setPadding(osgEarth::Util::Controls::Control::SIDE_RIGHT, compassSize * 0.5);
		readout_->setHaloColor(osgEarth::Symbology::Color::Black);
		readout_->setName("Compass Readout");

		// pointer is a text character, using default font
		pointer_ = new osgEarth::Util::Controls::LabelControl("|", osg::Vec4f(1, 0, 0, 1), static_cast<float>(fontSize));
		pointer_->setAbsorbEvents(false);
		pointer_->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_RIGHT);
		pointer_->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_TOP);
		// set the pointer to appear near the top middle of the compass image, 99% up, 69% across
		pointer_->setPadding(osgEarth::Util::Controls::Control::SIDE_TOP, compassSize * 0.25);
		pointer_->setPadding(osgEarth::Util::Controls::Control::SIDE_RIGHT, compassSize * 0.685);
		pointer_->setName("Compass Pointer");

		compassUpdateEventHandler_ = new FrameEventHandler(this);
	}
}

Compass::~Compass()
{
	if (drawView_.valid())
	{
		drawView_.get()->removeEventHandler(compassUpdateEventHandler_);
	}
}

void Compass::setListener(CompassUpdateListenerPtr listener)
{
	compassUpdateListener_ = listener;
}

void Compass::removeListener(const CompassUpdateListenerPtr& listener)
{
	if (compassUpdateListener_ == listener)
		compassUpdateListener_.reset();
}

void Compass::setDrawView(osgViewer::View* drawView)
{
	if (drawView == NULL)
	{
		removeFromView();
		return;
	}
	assert(!drawView_.valid());
	if (compass_ && drawView && !drawView_.valid())
	{
		drawView_ = drawView;
		canvas_->addControl<ui::ImageControl>(compass_);
		canvas_->addControl<ui::LabelControl>(readout_);
		canvas_->addControl<ui::LabelControl>(pointer_);

		// set up the callback for frame updates
		drawView->addEventHandler(compassUpdateEventHandler_);
	}
}

osgViewer::View* Compass::drawView() const
{
	return drawView_.get();
}

void Compass::removeFromView()
{
	if (compass_ && drawView_.valid())
	{
		canvas_->removeControl(compass_);
		canvas_->removeControl(readout_);
		canvas_->removeControl(pointer_);

		// stop callbacks for frame updates
		drawView_.get()->removeEventHandler(compassUpdateEventHandler_);
		drawView_ = NULL;
	}
}

void Compass::setActiveView(osgViewer::View* activeView)
{
	activeView_ = activeView;
}

int Compass::size() const
{
	if (compass_ == NULL)
		return 0;

	osg::Image* image = compass_->getImage();
	return image != NULL ? image->t() : 0;
}

void Compass::update_()
{
	const double TWO_DECIMAL_PLACES = 1e-02;
	if (!drawView_.valid())
	{
		return;
	}
	// if activeView not already set, or if it went away, set the active view to the draw view
	if (!activeView_.valid())
	{
		activeView_ = drawView_.get();
	}

	double heading = 0.0; // degrees
	if (true)
	{
		// Figure out the camera heading; use EarthManipulator to account for tether mode rotations
		const osgEarth::Util::EarthManipulator* manip = dynamic_cast<const osgEarth::Util::EarthManipulator*>(activeView_->getCameraManipulator());
		if (manip != NULL)
		{
			manip->getCompositeEulerAngles(&heading);
			// Convert to degrees
			heading = angFix360(heading * RAD2DEG);
		}
		else
		{
			// Fall back to the viewpoint's heading
			heading = angFix360(manip->getViewpoint().heading()->as(osgEarth::Units::DEGREES));
		}

		// make sure that anything equivalent to 0.00 is displayed as 0.00
		if (areEqual(heading, 0.0, TWO_DECIMAL_PLACES) || areEqual(heading, 360.0, TWO_DECIMAL_PLACES))
			heading = 0.0;
	}

	// check to see if this is a change of heading; note that compass rotation is -heading
	if (compass_ && !areEqual(compass_->getRotation().as(osgEarth::Units::DEGREES), -heading, TWO_DECIMAL_PLACES))
	{
		compass_->setRotation(-heading);
		// test to make sure -that a negative rotation is not converted to a positive 360+rotation
		assert(areEqual(compass_->getRotation().as(osgEarth::Units::DEGREES), -heading));

		if (readout_ && readout_.valid())
		{
			std::stringstream str;
			str << std::fixed << std::setprecision(2) << heading;
			std::string dirStr = str.str();
			if (dirStr.empty())
				return;
			if (readout_->text() != dirStr)
				readout_->setText(dirStr);
		}

		// if we have a listener, notify that we have updated
		if (compassUpdateListener_)
			compassUpdateListener_->onUpdate(heading);
	}
}

