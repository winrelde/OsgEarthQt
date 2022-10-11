#include <Windows.h>
#include "Common.h"

#include "osgQtView.h"
#include <osgEarthUtil/ExampleResources>
#include <osgGA/StateSetManipulator>

//osg控件
osgQtView::osgQtView(QWidget *parent, Qt::WindowFlags f, osgViewer::ViewerBase::ThreadingModel threadingModel) : 
	QWidget(parent, f)
{
	setThreadingModel(threadingModel);
	setKeyEventSetsDone(0);
	m_viewwidget = dynamic_cast<QWidget*>(addViewWidget(createGraphicsWindow(0, 0, 1920, 1080, "", true)));
	m_gridlayout = new QGridLayout(this);
	m_gridlayout->addWidget(m_viewwidget);

	m_viewwidget->show();

	connect(&m_timer, &QTimer::timeout, this, &osgQtView::update);
	m_timer.start(1);
}

//增加视口
QWidget *osgQtView::addViewWidget(osgQt::GraphicsWindowQt *gw)
{
	m_viewer = new osgViewer::Viewer;
	this->addView(m_viewer);
	m_viewer->setCameraManipulator(new osgEarth::Util::EarthManipulator);//默认地球控制器
	osgEarth::Util::MapNodeHelper().configureView(m_viewer);
	m_viewer->addEventHandler(new osgGA::StateSetManipulator(m_viewer->getCamera()->getOrCreateStateSet()));

	osg::Camera* camera = m_viewer->getCamera();
	camera->setGraphicsContext(gw);
	camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.6f, 1.0));
	camera->setDrawBuffer(GL_BACK);
	camera->setReadBuffer(GL_BACK);

	const osg::GraphicsContext::Traits* traits = gw->getTraits();
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	camera->setProjectionMatrixAsPerspective(30.0, static_cast<double>(traits->width) / static_cast<double>(traits->height), 1.0, 10000.0);

	gw->setTouchEventsEnabled(true);//可触摸
	return gw->getGLWidget();
}

//创造图形窗口
osgQt::GraphicsWindowQt *osgQtView::createGraphicsWindow(int x, int y, int w, int h, const std::string &name, bool windowDecoration)
{
	osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();//为运行时要查询的对象维护DisplaySettings单例
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->windowName = name;
	traits->windowDecoration = windowDecoration;
	traits->x = x;
	traits->y = y;
	traits->width = w;
	traits->height = h;
	traits->doubleBuffer = true;
	traits->alpha = ds->getMinimumNumAlphaBits();
	traits->stencil = ds->getMinimumNumStencilBits();
	traits->sampleBuffers = ds->getMultiSamples();
	traits->samples = ds->getNumMultiSamples();

	return new osgQt::GraphicsWindowQt(traits.get());
}


osg::ref_ptr<osgViewer::Viewer> osgQtView::getViewer()
{
	return m_viewer.get();
}

osg::ref_ptr<osg::Camera> osgQtView::getCamera()
{
	return m_viewer->getCamera();
}

void osgQtView::paintEvent(QPaintEvent * event)
{
	frame();
}
