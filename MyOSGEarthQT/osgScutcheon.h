#pragma once

#ifndef OSGSCUTCHEON_H
#define OSGSCUTCHEON_H


#include <QObject>
#include <iostream>
#include <sstream>
#include <QDebug>
#include <QTextCodec>
//#include "qCommonFunction.h"
#include <QVector>
#include <QThread>
//#include "3DEngine/graphicsview.h"
#include <osgWidget/Box>
#include <osgWidget/Label>
#include <osgWidget/WindowManager>

#define FONT_TEXT "fonts/simhei.ttf"
#define TITLE_IMAGE "../publish/data/texture/mark/label_title.png"
#define ITEM_IAMGE "../publish/data/texture/mark/label_normal_01.png"
//���ƶ����˵�
class ImageLabel;
//����һ���˵�
class osgScutcheonMenu;

/**����
 * @brief The osgScutcheon class
 */
class osgScutcheon : public osgWidget::Box
{

public:
	osgScutcheon();
	///
	/// \brief osgScutcheon���캯��
	/// \param titleText ���Ƶı�������
	/// \param targetObj ���ư󶨵Ľڵ�
	///
	osgScutcheon(const QString& titleText, osg::ref_ptr<osg::MatrixTransform> targetObj);
	~osgScutcheon();

public:

	void addLabelTitle(const QString &titleText);
	void addLabelItem(const QString &labelText);
	void addLabelItem(osgScutcheonMenu &labelItem);

	void hideOtherLabelItem();
	void moveLabelItem();

	void createLine(const osg::Vec3& startPt = osg::Vec3(0, 0, 0), const osg::Vec3& endPt = osg::Vec3(0, 0, 0));

	void onMouseEvent(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer *viewer);

	osg::Geometry* getLine() const { return m_line; }

	int getItemCount();

	void setVisibility(bool b);
	bool getVisibility() { return mIsVisible; }

	void setPos(osg::Vec3 pos);
	void setOffset(osg::Vec3 offset);

	//����ģ�͸��±��ƻص�������ʹ��
	osg::ref_ptr<osg::MatrixTransform> getTargetObject();
	std::vector<osg::ref_ptr<osgScutcheonMenu> > getLabelItemManager() const;

private:
	bool callbackMouseDrag(osgWidget::Event& ev);

protected:
	bool mouseDrag(double, double, const osgWidget::WindowManager*);
	bool mousePush(double, double, const osgWidget::WindowManager*);
	bool mouseRelease(double, double, const osgWidget::WindowManager*);

public:
	QString mTitleText;
	std::vector<osg::ref_ptr<osgScutcheonMenu> > mLabelItemManager;

	int mIndex;

	osg::ref_ptr<osg::MatrixTransform> mTargetObj;
	osg::ref_ptr<osg::Geometry> m_line;

	bool mIsVisible;
	bool mIsItemVisible;
	bool mIsMousePush;
	bool mIsMouseDrag;


	osg::Vec3 m_pos;
	osg::Vec3 m_offset;

	osg::Vec3 m_LastPt;
	osg::Vec3 m_LBDownPt;

};

/**
  ���ƹ�������ȫ��Ψһ��һ�����ƹ����������������
 * @brief The osgScutcheonManager class
 */
class osgScutcheonManager : public osgGA::GUIEventHandler
{
protected:
	osgScutcheonManager(osgViewer::Viewer* pViewer, osg::Group* pScreneRoot);
	~osgScutcheonManager();

public:
	static osgScutcheonManager *instance();
	static void destroy();
	bool handle(const osgGA::GUIEventAdapter&, osgGA::GUIActionAdapter&);
	int addLabel(osgScutcheon* label);
	int delLabel(osgScutcheon*& label);
	int delLabel(osg::MatrixTransform* tethernode);
	void clear();
	void lock(bool b);

	void setVisibility(bool b);
	void setVisibilityByID(unsigned int ID, bool b);
	bool getVisibilityByID(unsigned int ID);
	bool getVisibility() const { return m_bVisible; }

	void setLabelTxtColor(const osg::Vec4f& color);
	void setLabelBgColor(const osg::Vec4f& color);
	const osg::Vec4f& getLabelTxtColor() const { return m_clrTxt; }
	const osg::Vec4f& getLabelBgColor() const { return m_clrBg; }

	osg::ref_ptr<osgWidget::WindowManager> getWindowManager() const;
public:
	static osgScutcheonManager *m_pInstance;
	osg::ref_ptr<osgWidget::WindowManager> mWindowManager;
	//osgWidget::WindowManager* mWindowManager;
	std::vector<osgScutcheon*> m_LabelList;
	bool m_bStopGUI;
	bool m_bGUIStoped;
	bool m_bVisible;
	osg::Vec4f m_clrTxt;
	osg::Vec4f m_clrBg;

};



//����һ���˵�
class osgScutcheonMenu : public osgWidget::Label
{

public:
	osgScutcheonMenu(const QString &label, osg::ref_ptr<osgScutcheon> parentMenu) :
		osgWidget::Label("menu1"),
		mChildMenu(NULL),
		mParentMenu(parentMenu),
		mHasChildMenu(false),
		mIndex(0),
		mMenuItemCount(0)
	{
		initLabel(label);//��ʼ��һ���˵�
	}

	//��Ӷ����˵�
	void addChildMenu(QString &menuText)
	{
		ImageLabel *childMenuContent = new ImageLabel(menuText);
		if (mChildMenu == NULL)
		{
			mChildMenu = new osgWidget::Box("childMenu", osgWidget::Box::VERTICAL, true);
			mChildMenu->addWidget(childMenuContent);
			mMenuItemManager.push_back(childMenuContent);
			mChildMenu->getBackground()->setColor(1.0f, 1.0f, 1.0f, 0.6f);
			mChildMenu->resize();
			mChildMenu->hide();
			mHasChildMenu = true;
			//�˾䲻�ܼӣ���Ȼ��һ���ӿ���ʾmChildMenu����ΪmParent����ʾ��
			//mParentMenu->addChild(mChildMenu);
			mParentMenu->getWindowManager()->addChild(mChildMenu.get());

		}
		else
		{
			mChildMenu->addWidget(childMenuContent);
			mMenuItemManager.push_back(childMenuContent);
		}

		++mMenuItemCount;
	}

	//���¶����˵�����
	void updateChildMenuText(int index, QString labelText)
	{
		if (index >= 0 && index < getMenuItemCount())
		{
			mMenuItemManager.at(index)->setLabelText(labelText);
		}
	}

	//��ʼ��һ���˵�
	void initLabel(const QString &labelText)
	{
		setFont(FONT_TEXT);
		setFontSize(15);
		setFontColor(0.0f, 0.0f, 1.0f, 1.0f);
		addSize(128.0f, 32.0f);
		setColor(1.0f, 1.0f, 0.0f, 0.6f);
		//setShadow(0.08f);
		setCanFill(true);
		setEventMask(osgWidget::EVENT_ALL);
		setImage(ITEM_IAMGE);
		setTexCoord(0.0f, 0.0f, osgWidget::Widget::LOWER_LEFT);
		setTexCoord(1.0f, 0.0f, osgWidget::Widget::LOWER_RIGHT);
		setTexCoord(1.0f, 1.0f, osgWidget::Widget::UPPER_RIGHT);
		setTexCoord(0.0f, 1.0f, osgWidget::Widget::UPPER_LEFT);
		//�����ı�������󣬲�Ȼ���ƴ�С��ı�
		//CommonFunction com;
		//osgText::String labelString = com.qstringToOsgTextString(labelText);
		//setLabel(labelString);
		setLabel(labelText.toStdString());
	}



	void setIndex(int index)
	{
		mIndex = index;
	}

	void moveTo(int parentX, int parentY)
	{
		if (mChildMenu)
		{
			mChildMenu->setOrigin(parentX + 128.0, parentY + 32.0 * mIndex);
			mChildMenu->update();
		}
	}

	bool isHasChildMenu()
	{
		return mHasChildMenu;
	}

	osg::ref_ptr<osgWidget::Window> getChildMenu()
	{
		return mChildMenu;
	}

	int getIndex()
	{
		return mIndex;
	}

	int getMenuItemCount()
	{
		return mMenuItemManager.size();
	}

	//���ص�ǰ������������Ӳ˵�
	void hideOtherChildMenu()
	{
		for (int i = 0; i < mParentMenu->getLabelItemManager().size(); i++)
		{
			osg::ref_ptr<osgScutcheonMenu> childItem = mParentMenu->getLabelItemManager().at(i);
			if (childItem->getChildMenu() != mChildMenu)
			{
				childItem->getChildMenu()->hide();
			}
			qDebug() << "hideOtherChildMenu";
		}
	}
	//�������б�����������Ӳ˵�
	void hideAllChildMenu(osgWidget::WindowManager* wm)
	{
		osgWidget::Window *tmp = 0;
		unsigned int count = wm->getNumChildren();
		for (unsigned int i = 0; i < count; ++i)
		{
			tmp = dynamic_cast<osgWidget::Window*>(wm->getChild(i));
			if (tmp)
			{
				QString name = QString::fromStdString(tmp->getName());
				if (tmp != mChildMenu.get() && name == "childMenu")
				{
					if (tmp->isVisible())
					{
						tmp->hide();
					}
				}
			}
		}
	}

	void managed(osgWidget::WindowManager* wm)
	{
		osgWidget::Label::managed(wm);
		if (mChildMenu)
		{
			mChildMenu->hide();
		}
	}

	void positioned()
	{
		osgWidget::Label::positioned();
		if (mChildMenu)
		{
			//qDebug() << "mChildMenu not NULL!!" << mIndex;
			mChildMenu->setOrigin(mParentMenu->getX() + 128.0, mParentMenu->getY() + 32.0 * mIndex);
		}
	}

	bool mousePush(double, double, const osgWidget::WindowManager*wm)
	{
		hideOtherChildMenu();
		//hideAllChildMenu(mParentMenu->getWindowManager());

		if (!mChildMenu->isVisible())
		{
			mChildMenu->show();
			qDebug() << "hideOtherChildMenu show";
		}
		else
		{
			mChildMenu->hide();
			qDebug() << "hideOtherChildMenu hide";
		}
		return true;
	}

	bool mouseEnter(double, double, const osgWidget::WindowManager*)
	{
		setColor(1.0f, 1.0f, 1.0f, 0.3f);
		return true;
	}

	bool mouseLeave(double, double, const osgWidget::WindowManager*)
	{
		setColor(1.0f, 1.0f, 1.0f, 0.6f);
		return true;
	}

public:
	QVector<osg::ref_ptr<ImageLabel> > mMenuItemManager;

private:
	osg::ref_ptr<osgWidget::Box> mChildMenu;
	osg::ref_ptr<osgScutcheon> mParentMenu;

	int mIndex;
	int mMenuItemCount;
	bool mHasChildMenu;

};

//���ƶ����˵�
class ImageLabel : public osgWidget::Label
{

public:
	ImageLabel(QString &labelText) : osgWidget::Label("menu1")
	{
		setFont(FONT_TEXT);
		setFontSize(13);
		setFontColor(0.0f, 0.2f, 1.0f, 1.0f);
		addSize(128.0f, 32.0f);
		setColor(1.0f, 1.0f, 1.0f, 0.6f);
		//setPadding(1.0f);
		//setShadow(0.08f);
		setCanFill(true);
		setEventMask(osgWidget::EVENT_ALL);
		setImage(ITEM_IAMGE);

		setLabelText(labelText);

	}

	void setLabelText(QString labelText)
	{
		setAlignHorizontal(osgWidget::Widget::HA_LEFT);
		setTexCoord(0.0f, 0.0f, osgWidget::Widget::LOWER_LEFT);
		setTexCoord(1.0f, 0.0f, osgWidget::Widget::LOWER_RIGHT);
		setTexCoord(1.0f, 1.0f, osgWidget::Widget::UPPER_RIGHT);
		setTexCoord(0.0f, 1.0f, osgWidget::Widget::UPPER_LEFT);

		//CommonFunction com;
		//osgText::String labelString = com.qstringToOsgTextString(labelText);
		//setLabel(labelString);
		setLabel(labelText.toStdString());
	}

	bool mousePush(double, double, const osgWidget::WindowManager*)
	{
		return true;
	}


	bool mouseDrag(double, double, const osgWidget::WindowManager*)
	{
		//osg::notify(osg::NOTICE) << _name << " > mouseDrag called" << std::endl;
		return false;
	}

	bool mouseEnter(double, double, const osgWidget::WindowManager*)
	{
		//setColor(1.0f, 1.0f, 1.0f, 1.0f);
		setFontColor(0.0f, 0.0f, 1.0f, 1.0f);
		return true;
	}

	bool mouseLeave(double, double, const osgWidget::WindowManager*)
	{
		//setColor(1.0f, 1.0f, 1.0f, 0.8f);
		setFontColor(0.0f, 0.2f, 1.0f, 1.0f);
		return true;
	}

	bool mouseOver(double, double, const osgWidget::WindowManager*)
	{
		return true;
	}
};

#endif // OSGSCUTCHEON_H
