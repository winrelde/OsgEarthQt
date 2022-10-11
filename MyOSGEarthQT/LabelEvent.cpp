#include <Windows.h>
#include "LabelEvent.h"

LabelEvent::LabelEvent(osgEarth::Util::Controls::LabelControl* flyLabel1,
	osgEarth::Util::Controls::LabelControl* flyLabel2,
	osgEarth::Util::Controls::LabelControl* flyLabel3,
	osgEarth::Util::Controls::LabelControl* flyLabel4)
{
	flyCoords1 = flyLabel1;
	flyCoords2 = flyLabel2;
	flyCoords3 = flyLabel3;
	flyCoords4 = flyLabel4;
}
LabelEvent::~LabelEvent(void)
{

}


bool LabelEvent::handle(const osgGA::GUIEventAdapter&ea, osgGA::GUIActionAdapter&aa)
{
	osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
	if (viewer)
	{
		if (ea.getEventType() == ea.FRAME)
		{
			osgEarth::Util::EarthManipulator* em = dynamic_cast<osgEarth::Util::EarthManipulator*>(viewer->getCameraManipulator());
			if (em)
			{
				osg::Node* node = em->getViewpoint().getNode();
				
				if (node)
				{
					osg::MatrixTransform* mtf = dynamic_cast<osg::MatrixTransform*>(node);
					if (mtf)
					{
						osg::Vec3d pos1 = mtf->getMatrix().getTrans();

						osg::EllipsoidModel* emm = new osg::EllipsoidModel();
						osg::Vec3d lonLatAlt;
						emm->convertXYZToLatLongHeight(pos1.x(), pos1.y(), pos1.z(),
							lonLatAlt.y(), lonLatAlt.x(), lonLatAlt.z());

						lonLatAlt.x() = osg::RadiansToDegrees(lonLatAlt.x());
						lonLatAlt.y() = osg::RadiansToDegrees(lonLatAlt.y());
						float areas = (1.854*1.374*lonLatAlt.z()) / 0.807;

						char wstc1[512];
						char wstc2[512];
						char wstc3[512];
						char wstc4[512];

						//sprintf(wstc, "经度:%.2f\n纬度:%.2f\n高度:%.2fm\n面积:%.2fm^2", lonLatAlt.x(), lonLatAlt.y(), lonLatAlt.z(), areas);
						sprintf(wstc1, "%.2f", lonLatAlt.x());
						sprintf(wstc2, "%.2f", lonLatAlt.y());
						sprintf(wstc3, "%.2fm", lonLatAlt.z());
						sprintf(wstc4, "%.2fm^2", areas);
						flyCoords1->setText(wstc1);
						flyCoords2->setText(wstc2);
						flyCoords3->setText(wstc3);
						flyCoords4->setText(wstc4);
						
					}
					else
					{
						flyCoords1->setText("asodh shs a");
					}
				}
				else
				{
					flyCoords1->setText("no theron");
				}
			}
			
		}

		/*if (ea.getEventType() == ea.KEYDOWN && ea.getKey() == ea.KEY_C)
		{
			osgEarth::Util::EarthManipulator* em = dynamic_cast<osgEarth::Util::EarthManipulator*>(viewer->getCameraManipulator());
			if (em)
			{
				osg::Node* node = em->getViewpoint().getNode();

				if (node)
				{
					osg::MatrixTransform* mtf = dynamic_cast<osg::MatrixTransform*>(node);
					if (mtf)
					{
						osg::Node* ssg = mtf->getChild(0)->asGroup()->getChild(1);
						ssg->setNodeMask(0);
						
					}

				}
			}
		}

		if (ea.getEventType() == ea.KEYDOWN && ea.getKey() == ea.KEY_B)
		{
			osgEarth::Util::EarthManipulator* em = dynamic_cast<osgEarth::Util::EarthManipulator*>(viewer->getCameraManipulator());
			if (em)
			{
				osg::Node* node = em->getViewpoint().getNode();

				if (node)
				{
					osg::MatrixTransform* mtf = dynamic_cast<osg::MatrixTransform*>(node);
					if (mtf)
					{
						osg::Node* ssg = mtf->getChild(0)->asGroup()->getChild(1);
						ssg->setNodeMask(1);

					}

				}
			}
		}*/
		/*if (ea.getEventType() == ea.KEYDOWN && ea.getKey() == '>')
		{
			osgEarth::Util::EarthManipulator* em = dynamic_cast<osgEarth::Util::EarthManipulator*>(viewer->getCameraManipulator());
			if (em)
			{
				osg::Node* node = em->getViewpoint().getNode();

				if (node)
				{
					osg::MatrixTransform* mtf = dynamic_cast<osg::MatrixTransform*>(node);
					if (mtf)
					{
						osg::Node* ssg = mtf->getChild(0)->asGroup()->getChild(1);
						osgSim::SphereSegment* ss = dynamic_cast<osgSim::SphereSegment*>(ssg);
						
						float azMin, azMax, elevMin, elevMax;
						ss->getArea(azMin, azMax, elevMin, elevMax);

						ss->setArea(azMin, azMax, elevMin*1.1f, elevMax*1.1f);
						return true;
					}

				}
			}
		}
		return false;*/
	}
	return false;
}