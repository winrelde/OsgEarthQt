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

	//ģ�ͳ�ʼָ�����Ϊ�������򣬲�����������ı���ͨ����תZ��(�����)����Ϊ����
	m_pNode = osgDB::readNodeFile(modelPath.toLocal8Bit().data());
	if (!m_pNode)
	{
		qDebug() << "Error:addModel failure! Please check model path:" << modelPath;
		return;
	}
	//�������߸������ţ�����ģ�����ź���
	m_pNode->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);
	m_pNode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

	m_pModelNode = new osg::MatrixTransform;
	m_pModelNode->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);
	m_pModelNode->addChild(m_pNode);
	m_pModelNode->setName(modelName.toStdString());
	m_pRootModel->addChild(m_pModelNode.get());

	//��������
	createScutcheon();

}

void Model::createScutcheon()
{
	if (!m_pModelNode)
		return;
	if (m_pScutcheon == nullptr)
	{
		qDebug() << "m_pScutcheon";
		//���ƹ�������ȫ��Ψһ��һ�����ƹ����������������
		m_pScutcheonManager = osgScutcheonManager::instance();
		//���ƣ�ÿ�����ư������һ���˵�
		QString scutTitleText = QString::fromStdString(m_pModelNode->getName());
		m_pScutcheon = new osgScutcheon(scutTitleText, m_pModelNode);
		m_pScutcheonManager->addLabel(m_pScutcheon.get());

		//һ�����Ʋ˵���ÿ��һ���˵�������������˵�
		osg::ref_ptr<osgScutcheonMenu> labelItemMenu1 = new osgScutcheonMenu(QStringLiteral("������Ϣ"), m_pScutcheon);
		QString labelText = QStringLiteral("����:") + QString::fromStdString(m_pModelTypeNode->getName());
		QString speed = QStringLiteral("�ٶ�:1550km/h");
		QString dis = QStringLiteral("���:5000km");
		labelItemMenu1->addChildMenu(labelText);//�������Ʋ˵�
		labelItemMenu1->addChildMenu(speed);//�������Ʋ˵�
		labelItemMenu1->addChildMenu(dis);//�������Ʋ˵�


		QString lon = QStringLiteral("����:") + QString::number(0.000000, 'f', 6);
		QString lan = QStringLiteral("γ��:") + QString::number(0.000000, 'f', 6);
		QString alt = QStringLiteral("�߶�:") + QString::number(0.000000, 'f', 6);
		QString heading = QStringLiteral("����:") + QString::number(0.000000, 'f', 6);
		QString pitch = QStringLiteral("����:") + QString::number(0.000000, 'f', 6);
		QString roll = QStringLiteral("��ת:") + QString::number(0.000000, 'f', 6);
		osg::ref_ptr<osgScutcheonMenu> labelItemMenu2 = new osgScutcheonMenu(QStringLiteral("λ����Ϣ"), m_pScutcheon);
		labelItemMenu2->addChildMenu(lon);
		labelItemMenu2->addChildMenu(lan);
		labelItemMenu2->addChildMenu(alt);
		labelItemMenu2->addChildMenu(heading);
		labelItemMenu2->addChildMenu(pitch);
		labelItemMenu2->addChildMenu(roll);

		m_pScutcheon->addLabelItem(*labelItemMenu1);//��ʾ��0�����Ʋ˵�
		m_pScutcheon->addLabelItem(*labelItemMenu2);//��ʾ��1�����Ʋ˵�
	}

	//��ӱ��Ƹ��»ص��¼�
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
		m_pModelScutcheonCallback = new ModelScutcheonCallback(1);//1��ʾ��1�����Ʋ˵���Ҫ��������
		m_pScutcheon->setUpdateCallback(m_pModelScutcheonCallback);
	}
}



