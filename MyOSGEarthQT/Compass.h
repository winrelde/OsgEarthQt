#pragma once
#ifndef COMPASS_H
#define COMPASS_H

#include <osg/ref_ptr>
#include <osg/observer_ptr>
#include <osgViewer/View>

#include <memory>

namespace osgEarth {
	namespace Util {
		namespace Controls {
			class ImageControl;
			class LabelControl;
			class ControlCanvas;
		}
	}
}



/// define an interface for listeners for compass heading updates
class  CompassUpdateListener
{
public:
	CompassUpdateListener() {}
	virtual ~CompassUpdateListener() {}

	/** Executed when the compass heading changes, passes in heading in degrees */
	virtual void onUpdate(double heading) = 0;
};

/// Shared pointer to a CompassUpdateListener
typedef std::shared_ptr<CompassUpdateListener> CompassUpdateListenerPtr;

/**
  * Creates a Compass which can be displayed as a HUD control in a single view.  The
  * Compass is drawn on a single view, but may reflect the heading of a different view.
  * The view on which it is drawn is the Draw View (setDrawView(), drawView(), and
  * removeFromView()).  The view from which it pulls heading values is the Active view
  * (setActiveView()).  In single-view situations, these are often the same.  When
  * using insets, they may differ.  See the CompassFocusManagerAdapter class for an
  * easy way to tie focus-view changes to the setActiveView() method.
  */
class  Compass : public osg::Referenced
{
public:

	/** Constructs a new Compass */
	explicit Compass(const std::string& compassFilename, osgEarth::Util::Controls::ControlCanvas* canvas);

	/**
	* Display the Compass controls as an overlay in the specified view
	* @param drawView View on which the compass is drawn (in lower right corner).  May
	*   be different than the active view, which feeds the heading values for compass.
	*   Passing in NULL is equivalent to calling removeFromView().
	*/
	void setDrawView(osgViewer::View* drawView);

	/**
	* Remove the Compass controls from the draw view, hiding it.  No effect if the
	* compass is not currently being drawn.
	*/
	void removeFromView();

	/** Retrieves the current draw view (may be NULL) */
	osgViewer::View* drawView() const;

	/**
	* Tell the Compass to show the specified view's heading
	* @param activeView View whose compass values to display (not necessarily the
	*   same as the view in which the compass is drawn)
	*/
	void setActiveView(osgViewer::View* activeView);

	/**
	* Set our listener
	* @param listener Observer for the compass updates
	*/
	void setListener(CompassUpdateListenerPtr listener);

	/**
	* Unset our listener
	* @param listener Observer for the compass updates
	*/
	void removeListener(const CompassUpdateListenerPtr& listener);

	/**
	* Get the width/height size of the image in pixels. Width and height are the same
	* @return size of the compass image in pixels, returns 0 if no image found
	*/
	int size() const;

protected:
	/** Destructor */
	virtual ~Compass();

private:
	/** Update the compass display */
	void update_();

private:
	class FrameEventHandler;

	osg::observer_ptr<osgViewer::View> drawView_;                      ///< Reference to the view on which to overlay the compass
	osg::observer_ptr<osgViewer::View> activeView_;                    ///< Reference to the view whose data the compass is showing
	osg::observer_ptr<osgEarth::Util::Controls::ControlCanvas> canvas_;
	osg::ref_ptr<osgEarth::Util::Controls::ImageControl> compass_;  ///< compass image control
	osg::ref_ptr<osgEarth::Util::Controls::LabelControl> readout_;  ///< compass readout control
	osg::ref_ptr<osgEarth::Util::Controls::LabelControl> pointer_;  ///< compass pointer control

	osg::ref_ptr<FrameEventHandler> compassUpdateEventHandler_;     ///< Reference to the update event handler
	CompassUpdateListenerPtr compassUpdateListener_;        ///< Listener for our updates, if any

};



#endif // COMPASS_H

