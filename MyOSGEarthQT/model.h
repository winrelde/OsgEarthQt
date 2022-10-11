#pragma once

#ifndef MODEL_H
#define MODEL_H
#include <iostream>
#include <QObject>
#include <qmath.h>
#include "osgScutcheon.h"
#include <QMutexLocker>


class ModelScutcheonCallback;
class Model : public QObject, public osg::Referenced
{
	Q_OBJECT
public:
	explicit Model(QObject *parent = nullptr);
	~Model();


	/**
	 * @brief addModel ���ģ��
	 * @param modelPath ģ���ļ�����·��
	 * @param modelName ģ������
	 */
	void addModelNode(QString modelPath, QString modelName);


	/**
	 *��������
	 * @brief createScutcheon
	 */
	void createScutcheon();

	/**
	 *���ر���
	 * @brief hideScutcheon
	 */
	void hideScutcheon();
	/**
	 *��ʾ����
	 * @brief showScutcheon
	 */
	void showScutcheon();
	/**
	 *���ģ�ͱ��ƵĻص��¼����±���
	 * @brief addModelScaleCallback
	 */
	void addModelScutcheonCallback();



private:

	//һ��������ģ��Model����һ��ģ�͸��ڵ㣻
	//ģ�͸��ڵ�:����һ��ģ�����ͽڵ㣻
	//ģ�����ͽڵ㣺����ģ�ͽڵ㡢ģ�͹켣�߽ڵ�ȣ�
	//ģ�ͽڵ㣺���ģ�Ͳ���Ҫ������ת�Ȳ�������ģ�ͽڵ��°���ԭʼģ�ͽڵ㣻
	//�����������������������Ҫ���ŵȲ��������հ�����Scale/Rotate/Translate˳��ģ�ͽڵ��°���ƽ�ƽڵ㣬
	//������������������ƽ�ƽڵ��°�����ת�ڵ㣬��ת�ڵ��°������Žڵ㣬���Žڵ��²Ű���ԭʼģ�ͽڵ㣻
	//���ƽڵ㣺���ƽڵ����Ӧ��ģ�ͽڵ�󶨣���ʾ�Ǿ���ĳ��ģ�ͽڵ�ı��ơ�һ�����ƽڵ�������һ�����Ʋ˵���ÿ��һ���˵��ֿ��԰�����������˵�
	//���ƹ������ڵ㣺���ƹ������ڵ�ͳһ�������б��ƣ����ƹ�����ȫ��Ψһ
	//GraphicsView* m_pMap3D;
	//ModelConfig m_modelConfig;
	int m_modelID;
	osg::ref_ptr<osg::Node> m_pNode;//ԭʼģ�ͽڵ�
	osg::ref_ptr<osg::Group> m_pRootModel;//ģ�͸��ڵ�

	osg::ref_ptr<osg::MatrixTransform> m_pModelNode;//ģ�ͽڵ�

	osg::ref_ptr<osgScutcheonManager> m_pScutcheonManager;//���ƹ�������һ�����ƹ����������������
	osg::ref_ptr<osgScutcheon> m_pScutcheon;//ģ�ͱ��ƽڵ�

};

/**
 *���ģ�ͱ��ƵĻص��¼����±���
*/
class ModelScutcheonCallback : public osg::NodeCallback
{
public:
	ModelScutcheonCallback(int menuIndex) :
		m_menuIndex(menuIndex)
	{
		m_pNewScutcheonMenu = nullptr;
		m_pScutcheon = nullptr;
	}
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		m_pScutcheon = dynamic_cast<osgScutcheon*>(node);
		if (m_pScutcheon->getVisibility())
		{
			m_pScutcheon->setVisibility(true);
			m_pNewScutcheonMenu = m_pScutcheon->getLabelItemManager().at(m_menuIndex);//�ҵ����µ�һ�����Ʋ˵�
			updateScutcheonText();
		}
		else
		{
			m_pScutcheon->setVisibility(false);
		}

		traverse(node, nv);
	}

	void updateScutcheonText()
	{
		if (!m_pScutcheon || !m_pNewScutcheonMenu)
			return;
		m_pModelNode = m_pScutcheon->getTargetObject();
		osg::Vec3d currentPos = getModelPosition(m_pModelNode);

		osg::Vec3d scale = osg::Vec3d();
		osg::Quat so;
		osg::Vec3d translate = osg::Vec3d();
		//�������ź��ģ�����ʹ��decompose����ֽ⣬��ȻgetRotate������ת�����ܲ���ȷ
		osg::Quat rotation;
		m_pModelNode->getMatrix().decompose(translate, rotation, scale, so);
		//        osg::Quat rotation = m_pModelNode->getMatrix().getRotate();
		//�·���
		//Quaternion q(rotation.x(), rotation.y(), rotation.z(), rotation.w());
		//q.normalize();
		double res[3];
		//q.quaternion2Euler(q, res, Quaternion::zyx);
		double pitchAngle = res[0];
		double headingAngle = res[1];
		double rollAngle = res[2];
		//�Ϸ���
		//        double headingAngle = 0.0;
		//        double pitchAngle = 0.0;
		//        double rollAngle = 0.0;
		//        QuatToHPR(rotation, headingAngle, pitchAngle, rollAngle);

		QString lon = QStringLiteral("����:") + QString::number(currentPos.x(), 'f', 6);
		QString lan = QStringLiteral("γ��:") + QString::number(currentPos.y(), 'f', 6);
		QString alt = QStringLiteral("�߶�:") + QString::number(currentPos.z(), 'f', 6);
		QString heading = QStringLiteral("����:") + QString::number(osg::RadiansToDegrees(headingAngle), 'f', 6);
		QString pitch = QStringLiteral("����:") + QString::number(osg::RadiansToDegrees(pitchAngle), 'f', 6);
		QString roll = QStringLiteral("��ת:") + QString::number(osg::RadiansToDegrees(rollAngle), 'f', 6);

		int menuItemIndex = 0;
		m_pNewScutcheonMenu->updateChildMenuText(menuItemIndex++, lon);//���¶����˵�
		m_pNewScutcheonMenu->updateChildMenuText(menuItemIndex++, lan);
		m_pNewScutcheonMenu->updateChildMenuText(menuItemIndex++, alt);
		m_pNewScutcheonMenu->updateChildMenuText(menuItemIndex++, heading);
		m_pNewScutcheonMenu->updateChildMenuText(menuItemIndex++, pitch);
		m_pNewScutcheonMenu->updateChildMenuText(menuItemIndex++, roll);

	}

	osg::Vec3d getModelPosition(osg::ref_ptr<osg::MatrixTransform> m_pModelNode)
	{
		osg::Vec3d posXYZ = osg::Vec3d();
		osg::Vec3d currentPosition = osg::Vec3d();
		if (m_pModelNode)
		{
			osg::Matrix mat = m_pModelNode->getMatrix();
			posXYZ = posXYZ * mat;
			//            posXYZ = m_pModelNode->getMatrix().getTrans();
		}

		//GraphicsView::instance()->getSRS()->getGeodeticSRS()->getEllipsoid()->convertXYZToLatLongHeight(posXYZ.x(), posXYZ.y(), posXYZ.z(),
		//	currentPosition.y(), currentPosition.x(),
		//	currentPosition.z());

		currentPosition.x() = osg::RadiansToDegrees(currentPosition.x());
		currentPosition.y() = osg::RadiansToDegrees(currentPosition.y());
		return currentPosition;
	}
	//ZXY
	void QuatToHPR(osg::Quat q, double& heading, double& pitch, double& roll)
	{
		double test = q.y() * q.z() + q.x() * q.w();

		if (test > 0.4999)
		{ // singularity at north pole
			heading = 2.0 * atan2(q.y(), q.w());
			pitch = osg::PI_2;
			roll = 0.0;
			return;
		}
		if (test < -0.4999)
		{ // singularity at south pole
			heading = 2.0 * atan2(q.y(), q.w());
			pitch = -osg::PI_2;
			roll = 0.0;
			return;
		}
		double sqx = q.x() * q.x();
		double sqy = q.y() * q.y();
		double sqz = q.z() * q.z();
		heading = atan2(2.0 * q.z() * q.w() - 2.0 * q.y() * q.x(), 1.0 - 2.0 * sqz - 2.0 * sqx);
		pitch = asin(2.0 * test);
		roll = atan2(2.0 * q.y() * q.w() - 2.0 * q.z() * q.x(), 1.0 - 2.0 * sqy - 2.0 * sqx);
	}

public:
	int m_menuIndex;//����һ���˵���ʶ���ж����ĸ��˵���Ҫ����
	osg::ref_ptr<osgScutcheon> m_pScutcheon;//��Ҫ������Ϣ�ı���
	osg::ref_ptr<osgScutcheonMenu> m_pNewScutcheonMenu;//Ҫ���µı��Ʋ˵�
	osg::ref_ptr<osg::MatrixTransform> m_pModelNode;//���ư󶨵�ģ�ͽڵ�
};

#endif // MODEL_H
