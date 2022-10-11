#include "model.h"

Model::Model(QObject *parent) : QObject(parent),
m_pMap3D(GraphicsView::instance())
{
	m_pRootModel = new osg::Group;
	m_pRootModel->setName("RootModel");
	m_pMap3D->getRoot()->addChild(m_pRootModel);

	this->m_pModelNode = nullptr;

	this->m_pModelScutcheonCallback = nullptr;
	this->m_pScutcheon = nullptr;
	this->m_pScutcheonManager = nullptr;

}


Model::~Model()
{

}


void Model::addModelNode(QString modelPath, QString modelName)
{

	//模型初始指向必须为正北方向，不是正北方向的必须通过旋转Z轴(航向角)调整为正北
	m_pNode = osgDB::readNodeFile(modelPath.toLocal8Bit().data());
	if (!m_pNode)
	{
		qDebug() << "Error:addModel failure! Please check model path:" << modelPath;
		return;
	}
	//开启法线跟随缩放，避免模型缩放后变黑
	m_pNode->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);
	m_pNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

	m_pModelNode = new osg::MatrixTransform;
	m_pModelNode->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);
	m_pModelNode->addChild(m_pNode);
	m_pModelNode->setName(modelName.toStdString());
	m_pRootModel->addChild(m_pModelNode.get());

	//创建标牌
	createScutcheon();

}

void Model::createScutcheon()
{
	if (!m_pModelNode)
		return;
	if (m_pScutcheon == nullptr)
	{
		qDebug() << "m_pScutcheon";
		//标牌管理器，全局唯一，一个标牌管理器包含多个标牌
		m_pScutcheonManager = osgScutcheonManager::instance();
		//标牌，每个标牌包含多个一级菜单
		QString scutTitleText = QString::fromStdString(m_pModelNode->getName());
		m_pScutcheon = new osgScutcheon(scutTitleText, m_pModelNode);
		m_pScutcheonManager->addLabel(m_pScutcheon.get());

		//一级标牌菜单，每个一级菜单包含多个二级菜单
		osg::ref_ptr<osgScutcheonMenu> labelItemMenu1 = new osgScutcheonMenu(QStringLiteral("基础信息"), m_pScutcheon);
		QString labelText = QStringLiteral("类型:") + QString::fromStdString(m_pModelTypeNode->getName());
		QString speed = QStringLiteral("速度:1550km/h");
		QString dis = QStringLiteral("射程:5000km");
		labelItemMenu1->addChildMenu(labelText);//二级标牌菜单
		labelItemMenu1->addChildMenu(speed);//二级标牌菜单
		labelItemMenu1->addChildMenu(dis);//二级标牌菜单


		QString lon = QStringLiteral("经度:") + QString::number(0.000000, 'f', 6);
		QString lan = QStringLiteral("纬度:") + QString::number(0.000000, 'f', 6);
		QString alt = QStringLiteral("高度:") + QString::number(0.000000, 'f', 6);
		QString heading = QStringLiteral("航向:") + QString::number(0.000000, 'f', 6);
		QString pitch = QStringLiteral("俯仰:") + QString::number(0.000000, 'f', 6);
		QString roll = QStringLiteral("滚转:") + QString::number(0.000000, 'f', 6);
		osg::ref_ptr<osgScutcheonMenu> labelItemMenu2 = new osgScutcheonMenu(QStringLiteral("位置信息"), m_pScutcheon);
		labelItemMenu2->addChildMenu(lon);
		labelItemMenu2->addChildMenu(lan);
		labelItemMenu2->addChildMenu(alt);
		labelItemMenu2->addChildMenu(heading);
		labelItemMenu2->addChildMenu(pitch);
		labelItemMenu2->addChildMenu(roll);

		m_pScutcheon->addLabelItem(*labelItemMenu1);//表示第0个标牌菜单
		m_pScutcheon->addLabelItem(*labelItemMenu2);//表示第1个标牌菜单
	}

	//添加标牌更新回调事件
	addModelScutcheonCallback();

}

void Model::hideScutcheon()
{
	if (!m_pScutcheon)
		return;
	m_pScutcheon->setVisibility(false);
}

void Model::showScutcheon()
{
	if (!m_pScutcheon)
		return;
	m_pScutcheon->setVisibility(true);
}

void Model::addModelScutcheonCallback()
{

	if (!m_pScutcheon)
		return;
	if (!m_pModelScutcheonCallback)
	{
		qDebug() << "m_pModelScutcheonCallback";
		m_pModelScutcheonCallback = new ModelScutcheonCallback(1);//1表示第1个标牌菜单需要更新文字
		m_pScutcheon->setUpdateCallback(m_pModelScutcheonCallback);
	}
}



