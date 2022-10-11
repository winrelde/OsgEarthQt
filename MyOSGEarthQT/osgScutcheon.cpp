#include <Windows.h>

#include "osgScutcheon.h"


osgScutcheon::osgScutcheon()
{

}

osgScutcheon::osgScutcheon(const QString& titleText, osg::ref_ptr<osg::MatrixTransform> targetObj) :
	osgWidget::Box("osgScutcheon", osgWidget::Box::VERTICAL),
	mTargetObj(targetObj),
	mIndex(0),
	mIsVisible(true),
	mIsItemVisible(true),
	mIsMouseDrag(false),
	mIsMousePush(false)
{

	getBackground()->setColor(1.0, 1.0, 1.0, 0.5);
	setEventMask(osgWidget::EVENT_ALL);
	attachMoveCallback();

	mTitleText = titleText;
	addLabelTitle(titleText);

}

osgScutcheon::~osgScutcheon()
{

}

void osgScutcheon::addLabelTitle(const QString &titleText)
{

	osg::ref_ptr<osgWidget::Label> labelTitle = new osgWidget::Label("labelTitle");
	labelTitle->setFont(FONT_TEXT);
	labelTitle->setFontSize(14);
	labelTitle->setFontColor(1.0f, 1.0f, 0.0f, 1.0f);
	labelTitle->setColor(1.0f, 1.0f, 1.0f, 0.3f);
	labelTitle->addSize(128.0f, 32.0f);
	//labelTitle->setEventMask(osgWidget::EVENT_MOUSE_DRAG);//设置完无法拖动
	//labelTitle->addCallback(new osgWidget::Callback(&osgScutcheon::callbackMousePush, this, osgWidget::EVENT_MOUSE_DRAG));

	//labelTitle->setShadow(0.08f);
	labelTitle->setCanFill(true);
	labelTitle->setImage(TITLE_IMAGE);
	labelTitle->setTexCoord(0.0f, 0.0f, osgWidget::Widget::LOWER_LEFT);
	labelTitle->setTexCoord(1.0f, 0.0f, osgWidget::Widget::LOWER_RIGHT);
	labelTitle->setTexCoord(1.0f, 1.0f, osgWidget::Widget::UPPER_RIGHT);
	labelTitle->setTexCoord(0.0f, 1.0f, osgWidget::Widget::UPPER_LEFT);
	//CommonFunction com;
	//osgText::String labelString = com.qstringToOsgTextString(titleText);
	//labelTitle->setLabel(labelString);
	labelTitle->setLabel(titleText.toStdString());
	addWidget(labelTitle);//谁先add谁在标牌最下面
}


void osgScutcheon::addLabelItem(const QString &labelText)
{
	++mIndex;
	osgScutcheonMenu* labelItem = new osgScutcheonMenu(labelText, this);
	labelItem->setIndex(mIndex);
	mLabelItemManager.push_back(labelItem);
	addWidget(labelItem);//谁先add谁在标牌最下面
}

void osgScutcheon::addLabelItem(osgScutcheonMenu &labelItem)
{
	++mIndex;
	labelItem.setIndex(mIndex);
	mLabelItemManager.push_back(&labelItem);
	if (mIsItemVisible)
	{
		addWidget(&labelItem);//谁先add谁在标牌最下面
	}

}

void osgScutcheon::moveLabelItem()
{
	int x = this->getX();
	int y = this->getY();

	for (int i = 0; i < mLabelItemManager.size(); ++i)
	{
		osgScutcheonMenu *labelItem = mLabelItemManager.at(i);
		labelItem->moveTo(x, y);
	}
}

bool osgScutcheon::callbackMouseDrag(osgWidget::Event &ev)
{

	return true;
}

bool osgScutcheon::mouseDrag(double, double, const osgWidget::WindowManager *)
{
	qDebug() << "osgScutcheon mouseDrag called";
	mIsMouseDrag = true;
	return true;
}

bool osgScutcheon::mousePush(double, double, const osgWidget::WindowManager *)
{
	mIsMousePush = true;
	qDebug() << "osgScutcheon mousePush called" << mIsMousePush << ":" << mTitleText;
	return true;
}

bool osgScutcheon::mouseRelease(double, double, const osgWidget::WindowManager *)
{
	qDebug() << "osgScutcheon mouseRelease called" << mIsMouseDrag << ":" << mTitleText;

	if (!mIsMouseDrag)
	{
		if (mIsMousePush && mIsItemVisible)
		{
			std::vector<osg::ref_ptr<osgScutcheonMenu> >::const_iterator it;
			for (it = mLabelItemManager.begin(); it != mLabelItemManager.end(); ++it)
			{
				(*it)->getChildMenu()->hide();
				this->removeWidget((*it));
				qDebug() << "item removed!" << (*it)->getIndex();
			}
		}
		else
		{
			if (this->getNumChildren() <= mLabelItemManager.size())
			{
				std::vector<osg::ref_ptr<osgScutcheonMenu> >::const_iterator it;
				for (it = mLabelItemManager.begin(); it != mLabelItemManager.end(); ++it)
				{
					this->addWidget((*it));
					qDebug() << "item added!" << (*it)->getIndex();
				}
			}
		}
		mIsItemVisible = !mIsItemVisible;
	}
	mIsMousePush = false;
	mIsMouseDrag = false;

	return true;
}

std::vector<osg::ref_ptr<osgScutcheonMenu> > osgScutcheon::getLabelItemManager() const
{
	return mLabelItemManager;
}


void osgScutcheon::onMouseEvent(const osgGA::GUIEventAdapter &ea, osgViewer::Viewer *viewer)
{
	int etype = ea.getEventType();
	if (etype == ea.FRAME)
	{
		if (mTargetObj)
		{
			osg::Vec3 position = mTargetObj->getMatrix().getTrans() + osg::Vec3d(0, 0, 0);
			osgViewer::Renderer* renderer = dynamic_cast<osgViewer::Renderer*>(viewer->getCamera()->getRenderer());
			osg::Vec3 renderPos;
			renderer->getSceneView(0)->projectObjectIntoWindow(position, renderPos);

			float x = this->getX();//renderpos.x
			float y = this->getY();
			float w = this->getWidth();//rendersize.x
			float h = this->getHeight();

			float offset = 0.0;
			osg::Vec3 stPt(x + 0.5 * w, y + 0.5 * h, 0);
			if (stPt.y() - renderPos.y() > 0.5*h)
			{
				stPt[1] = stPt.y() - 0.5*h + offset;
			}
			else if (stPt.y() - renderPos.y() < -0.5*h)
			{
				stPt[1] = stPt.y() + 0.5*h + offset;
			}
			if (stPt.x() - renderPos.x() > 0.5*w)
			{
				stPt[0] = stPt.x() - 0.5*w - offset;
			}
			else if (stPt.x() - renderPos.x() < -0.5*w)
			{
				stPt[0] = stPt.x() + 0.5*w - offset;
			}
			setPos(renderPos + osg::Vec3d(50, 50, 50));//设置标牌初始位置距离目标的偏移为50
			createLine(stPt, renderPos);

		}
	}
	if ((etype == ea.PUSH) && ea.getButtonMask() == ea.LEFT_MOUSE_BUTTON)
	{
		if (mIsMousePush)
		{
			//qDebug() << "mousePush called" ;
			m_LBDownPt.set(ea.getX(), ea.getY(), 0);
			m_LastPt = m_LBDownPt;
		}
	}
	if ((etype == ea.DRAG))
	{
		//qDebug() << "mouseDrag called" ;
		if (mIsMouseDrag)
		{
			osg::Vec3 pt(ea.getX() - m_LastPt[0], ea.getY() - m_LastPt[1], 0);
			setOffset(m_offset + pt);
			m_LastPt.set(ea.getX(), ea.getY(), 0);
			ea.setHandled(true);
		}
	}
	if ((etype == ea.RELEASE))
	{
		//qDebug() << "mouseDrag release" ;
	}
}

void osgScutcheon::setPos(osg::Vec3 pos)
{
	m_pos = pos;
	pos = m_pos + m_offset;
	this->setOrigin(pos.x(), pos.y());
	this->update();
	moveLabelItem();
}

void osgScutcheon::setOffset(osg::Vec3 offset)
{
	m_offset = offset;
	offset = m_pos + m_offset;
	this->setOrigin(offset.x(), offset.y());
	this->update();
	moveLabelItem();
}

osg::ref_ptr<osg::MatrixTransform> osgScutcheon::getTargetObject()
{
	return mTargetObj;
}
int osgScutcheon::getItemCount()
{
	return mLabelItemManager.size();
}

void osgScutcheon::setVisibility(bool b)
{
	mIsVisible = b;
	if (mIsVisible)
	{
		this->show();
		m_line->setNodeMask(1);
	}
	else
	{
		this->hide();
		m_line->setNodeMask(0);
		for (int i = 0; i < mLabelItemManager.size(); i++)
		{
			mLabelItemManager.at(i)->getChildMenu()->hide();
		}
	}

}

void osgScutcheon::createLine(const osg::Vec3 &startPt, const osg::Vec3 &endPt)
{
	if (NULL == m_line)
	{	//创建标牌和模型之间的连接线
		m_line = new osg::Geometry;
		osg::Vec3Array* vertices = new osg::Vec3Array;
		vertices->push_back(startPt);
		vertices->push_back(endPt);
		m_line->setVertexArray(vertices);

		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
		m_line->setNormalArray(normals);
		m_line->setNormalBinding(osg::Geometry::BIND_OVERALL);

		osg::Vec4Array* colors = new osg::Vec4Array;

		colors->push_back(osg::Vec4(1.0, 1.0, 0.0, 1.0));
		m_line->setColorArray(colors);
		m_line->setColorBinding(osg::Geometry::BIND_OVERALL);

		m_line->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		m_line->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
		m_line->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(1.0f), osg::StateAttribute::ON);
		m_line->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		osg::Geode* geode = new osg::Geode();
		geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		geode->addDrawable(m_line);
		//this->addChild(geode);
		this->getWindowManager()->addChild(geode);
	}
	else
	{   //动态更新线长
		osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(m_line->getVertexArray());
		(*vertices)[0].set(startPt.x(), startPt.y(), 0.0);
		(*vertices)[1].set(endPt.x(), endPt.y(), 0.0);
		vertices->dirty();
		m_line->setVertexArray(vertices);
		m_line->dirtyDisplayList();
	}
}



///////////////////////////////////////////////////////////////
/// \brief osgScutcheonManager::osgScutcheonManager
/// 标牌管理器
/// ///////////////////////////////////////////////////////////
osgScutcheonManager* osgScutcheonManager::m_pInstance = NULL;
osgScutcheonManager::osgScutcheonManager(osgViewer::Viewer* pViewer, osg::Group* pScreneRoot)
{
	m_bVisible = true;


	//0xF0000000表示标牌显示 0x00000000表示标牌隐藏 WM_PICK_DEBUG显示调试信息
	mWindowManager = new osgWidget::WindowManager(pViewer, 50.0f, 50.0f, 1, /*0xF0000000*/
		osgWidget::WindowManager::WM_PICK_DEBUG);
	osg::Camera* camera = mWindowManager->createParentOrthoCamera();
	pScreneRoot->addChild(camera);

	//pViewer->addEventHandler(new osgWidget::MouseHandler(mWindowManager));
	//pViewer->addEventHandler(new osgWidget::KeyboardHandler(mWindowManager));
	//pViewer->addEventHandler(new osgWidget::ResizeHandler(mWindowManager, camera));
	//pViewer->addEventHandler(new osgWidget::CameraSwitchHandler(mWindowManager, camera));

}

osgScutcheonManager::~osgScutcheonManager()
{
	m_bGUIStoped = true;
	clear();
}

//osgScutcheonManager *osgScutcheonManager::instance()
//{
//	if (NULL == m_pInstance)
//	{
//		//osgViewer::Viewer* getOSGViewer(){ return m_pViewer; }
//		//osg::Group* getRoot(){ return m_pRoot.get(); }
//		m_pInstance = new osgScutcheonManager(Viewer,
//			GraphicsView::instance()->getRoot());
//		GraphicsView::instance()->getOSGViewer()->addEventHandler(m_pInstance);
//	}
//	return m_pInstance;
//}

void osgScutcheonManager::destroy()
{
	if (NULL != m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

bool osgScutcheonManager::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
	if (m_bStopGUI)
	{
		m_bGUIStoped = true;
	}
	else
	{
		osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
		int nCount = m_LabelList.size();
		for (int i = 0; i < nCount; i++)
		{
			m_LabelList[i]->onMouseEvent(ea, viewer);
		}
	}
	return false;
}

int osgScutcheonManager::addLabel(osgScutcheon *label)
{
	m_bVisible = true;
	if (label == NULL) { return 0; }
	lock(true);
	//    while (!m_bStopGUI) QThread::sleep(100);
	int nCount = m_LabelList.size();
	for (int i = 0; i < nCount; i++)
	{
		if (m_LabelList[i] == label)
		{
			lock(false);
			return 0;
		}
	}
	mWindowManager->addChild(label);
	mWindowManager->resizeAllWindows();
	m_LabelList.push_back(label);

	lock(false);
	return m_LabelList.size();
}

int osgScutcheonManager::delLabel(osgScutcheon *&label)
{
	lock(true);
	while (!m_bStopGUI) QThread::sleep(100);
	for (std::vector<osgScutcheon*>::iterator vit = m_LabelList.begin(); vit != m_LabelList.end(); vit++)
	{
		if ((*vit) == label)
		{
			m_LabelList.erase(vit);
			//label->setVisibility(false);//TODO 清除界面上已经显示的标牌
			delete label;
			label = NULL;
			lock(false);
			return m_LabelList.size();
		}
	}
	lock(false);
	return 0;
}

int osgScutcheonManager::delLabel(osg::MatrixTransform *tethernode)
{
	return 0;
}

void osgScutcheonManager::clear()
{
	lock(true);
	//while (!m_bGUIStoped) Sleep(200);
	while (m_LabelList.size() > 0)
	{
		osgScutcheon* back = m_LabelList.back();
		delLabel(back);
	}
	lock(false);
}

void osgScutcheonManager::lock(bool b)
{
	if (b)
	{
		m_bStopGUI = true;
		m_bGUIStoped = false;
	}
	else
	{
		m_bStopGUI = false;
		m_bGUIStoped = true;
	}
}

osg::ref_ptr<osgWidget::WindowManager> osgScutcheonManager::getWindowManager() const
{
	return mWindowManager;
}


