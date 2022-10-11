#pragma once
#include <QtWidgets/QWidget>
#include <QtCore/QTimer>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/MultiTouchTrackballManipulator>
#include <osgDB/ReadFile>
#include <osgQt/GraphicsWindowQt>

#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ViewFitter>

class osgQtView : public QWidget, public osgViewer::CompositeViewer
{
public:

	osgQtView(QWidget* parent = 0, Qt::WindowFlags f = 0, osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::CompositeViewer::SingleThreaded);
	~osgQtView() {}

	QWidget* addViewWidget(osgQt::GraphicsWindowQt* gw);
	osgQt::GraphicsWindowQt* createGraphicsWindow(int x, int y, int w, int h, const std::string& name = "", bool windowDecoration = false);
	osg::ref_ptr<osgViewer::Viewer> getViewer();
	osg::ref_ptr<osg::Camera> getCamera();
	
private:

	virtual void update() { QWidget::update(); }
	virtual void paintEvent(QPaintEvent* event);
	QTimer							m_timer;
	QWidget*						m_viewwidget;
	osg::ref_ptr<osgViewer::Viewer> m_viewer;
	QGridLayout*					m_gridlayout;
};