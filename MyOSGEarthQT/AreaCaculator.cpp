#include <Windows.h>
#include "AreaCaculator.h"


AreaCaculator::AreaCaculator(const SpatialReference* srs, osg::Group* annoGroup, osgEarth::MapNode* mapNode) :_srs(srs), _annoGroup(annoGroup), _mapnode(mapNode)
{
	_pFeature = NULL;
	_pFeatureNode = NULL;
	_label = NULL;
	_mapnode = NULL;
	_vecPoint.clear();

	Style labelStyle;
	TextSymbol* text = labelStyle.getOrCreate<TextSymbol>();
	text->declutter() = false;
	text->pixelOffset()->set(0, 20);
	text->alignment() = TextSymbol::ALIGN_CENTER_BOTTOM;
	text->size() = text->size()->eval() + 2.0f;
	text->fill()->color() = Color(Color::Yellow, 0.8);
	//set font to show Chinese
	text->font() = "simhei.ttf";
	_label = new LabelNode("", labelStyle);
	_label->setDynamic(true);
	_label->setHorizonCulling(false);
	_annoGroup->addChild(_label);
}


AreaCaculator::~AreaCaculator()
{
}

bool AreaCaculator::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	//if (!_isEnable) return false;
	osg::Vec3d vecPos;
	osg::Vec3d pos = getPos(ea, aa, vecPos);
	switch (ea.getEventType())
	{
	case osgGA::GUIEventAdapter::PUSH:
	{
		if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
		{
			slotPicked(vecPos);
		}
		else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)
		{
			slotRightHandle();
		}
		break;
	}
	case osgGA::GUIEventAdapter::MOVE:
	{
		slotMoving(vecPos);
		break;
	}
	}

	return false;
}

osg::Vec3d AreaCaculator::getPos(const osgGA::GUIEventAdapter& ea,
	osgGA::GUIActionAdapter& aa, osg::Vec3d& pos)
{
	pos = osg::Vec3d(0, 0, 0);
	osgViewer::Viewer* pViewer = dynamic_cast<osgViewer::Viewer*>(&aa);
	if (pViewer == NULL)
	{
		return osg::Vec3d(0, 0, 0);
	}
	osgUtil::LineSegmentIntersector::Intersections intersection;
	double x = ea.getX();
	double y = ea.getY();
	pViewer->computeIntersections(ea.getX(), ea.getY(), intersection);
	osgUtil::LineSegmentIntersector::Intersections::iterator iter
		= intersection.begin();
	if (iter != intersection.end())
	{
		_srs->getGeodeticSRS()->getEllipsoid()->convertXYZToLatLongHeight(
			iter->getWorldIntersectPoint().x(), iter->getWorldIntersectPoint().y(), iter->getWorldIntersectPoint().z(),
			pos.y(), pos.x(), pos.z());
		pos.x() = osg::RadiansToDegrees(pos.x());
		pos.y() = osg::RadiansToDegrees(pos.y());
		return iter->getWorldIntersectPoint();
	}
	return osg::Vec3d(0, 0, 0);
}

void AreaCaculator::slotPicked(osg::Vec3d pos)
{
	_vecPoint.push_back(pos);

	if (_pFeatureNode == NULL)
	{
		Style polygonStyle;
		polygonStyle.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Blue, 0.3);
		polygonStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_TO_TERRAIN;
		polygonStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_DRAPE;

		_pFeature = new Feature(new osgEarth::Annotation::Polygon, _srs);
		_pFeatureNode = new FeatureNode(_pFeature, polygonStyle);
		_annoGroup->addChild(_pFeatureNode);
	}
	_pFeature->getGeometry()->clear();
	for (int i = 0; i < _vecPoint.size(); ++i)
	{
		_pFeature->getGeometry()->push_back(_vecPoint[i]);
	}
	_pFeatureNode->dirty();
}

void AreaCaculator::slotMoving(osg::Vec3d pos)
{
	if (_vecPoint.size() <= 0)
	{
		return;
	}
	_pFeature->getGeometry()->clear();
	for (int i = 0; i < _vecPoint.size(); ++i)
	{
		_pFeature->getGeometry()->push_back(_vecPoint[i]);
	}
	_pFeature->getGeometry()->push_back(pos);
	_pFeatureNode->dirty();

	const Geometry *geo = _pFeature->getGeometry();
	if (geo->size() > 2)
	{
		_label->setPosition(GeoPoint(_srs, geo->getBounds().center()));
		double area = caculateArea(geo);
		//double area = calcArea(geo->asVector(),_mapnode);
		//double area = calcSurfaceArea(geo->asVector(), _mapnode);
		_label->setText(toString(area));
	}
}

void AreaCaculator::slotRightHandle()
{
	_vecPoint.clear();
}

double AreaCaculator::caculateArea(const Geometry *geo)
{
	double dLowX = 0, dLowY = 0, dMiddleX = 0, dMiddleY = 0, dHighX = 0, dHighY = 0;
	double dSum1 = 0, dSum2 = 0, iCount1 = 0, iCount2 = 0;

	for (int i = 0; i < geo->size(); i++)
	{
		if (i == 0)
		{
			dLowX = osg::DegreesToRadians(geo->at(geo->size() - 1).x());
			dLowY = osg::DegreesToRadians(geo->at(geo->size() - 1).y());
			dMiddleX = osg::DegreesToRadians(geo->at(0).x());
			dMiddleY = osg::DegreesToRadians(geo->at(0).y());
			dHighX = osg::DegreesToRadians(geo->at(1).x());
			dHighY = osg::DegreesToRadians(geo->at(1).y());
		}
		else if (i == geo->size() - 1)
		{
			dLowX = osg::DegreesToRadians(geo->at(geo->size() - 2).x());
			dLowY = osg::DegreesToRadians(geo->at(geo->size() - 2).y());
			dMiddleX = osg::DegreesToRadians(geo->at(geo->size() - 1).x());
			dMiddleY = osg::DegreesToRadians(geo->at(geo->size() - 1).y());
			dHighX = osg::DegreesToRadians(geo->at(0).x());
			dHighY = osg::DegreesToRadians(geo->at(0).y());
		}
		else
		{
			dLowX = osg::DegreesToRadians(geo->at(i - 1).x());
			dLowY = osg::DegreesToRadians(geo->at(i - 1).y());
			dMiddleX = osg::DegreesToRadians(geo->at(i).x());
			dMiddleY = osg::DegreesToRadians(geo->at(i).y());
			dHighX = osg::DegreesToRadians(geo->at(i + 1).x());
			dHighY = osg::DegreesToRadians(geo->at(i + 1).y());
		}
		double AM = cos(dMiddleY) * cos(dMiddleX);
		double BM = cos(dMiddleY) * sin(dMiddleX);
		double CM = sin(dMiddleY);
		double AL = cos(dLowY) * cos(dLowX);
		double BL = cos(dLowY) * sin(dLowX);
		double CL = sin(dLowY);
		double AH = cos(dHighY) * cos(dHighX);
		double BH = cos(dHighY) * sin(dHighX);
		double CH = sin(dHighY);

		double dCoefficientL = (AM*AM + BM*BM + CM*CM) / (AM*AL + BM*BL + CM*CL);
		double dCoefficientH = (AM*AM + BM*BM + CM*CM) / (AM*AH + BM*BH + CM*CH);

		double dALtangent = dCoefficientL * AL - AM;
		double dBLtangent = dCoefficientL * BL - BM;
		double dCLtangent = dCoefficientL * CL - CM;
		double dAHtangent = dCoefficientH * AH - AM;
		double dBHtangent = dCoefficientH * BH - BM;
		double dCHtangent = dCoefficientH * CH - CM;

		double dAngleCos = (dAHtangent * dALtangent + dBHtangent * dBLtangent + dCHtangent * dCLtangent) /
			(sqrt(dAHtangent * dAHtangent + dBHtangent * dBHtangent + dCHtangent * dCHtangent) *
				sqrt(dALtangent * dALtangent + dBLtangent * dBLtangent + dCLtangent * dCLtangent));

		dAngleCos = acos(dAngleCos);

		double dANormalLine = dBHtangent * dCLtangent - dCHtangent * dBLtangent;
		double dBNormalLine = 0 - (dAHtangent * dCLtangent - dCHtangent * dALtangent);
		double dCNormalLine = dAHtangent * dBLtangent - dBHtangent * dALtangent;

		double dOrientationValue = 0;
		if (AM != 0)
		{
			dOrientationValue = dANormalLine / AM;
		}
		else if (BM != 0)
		{
			dOrientationValue = dBNormalLine / BM;
		}
		else
		{
			dOrientationValue = dCNormalLine / CM;
		}

		if (dOrientationValue > 0)
		{
			dSum1 += dAngleCos;
			iCount1++;
		}
		else
		{
			dSum2 += dAngleCos;
			iCount2++;
		}
	}
	double dSum = 0;
	if (dSum1 > dSum2)
	{
		dSum = dSum1 + (2 * osg::PI*iCount2 - dSum2);
	}
	else
	{
		dSum = (2 * osg::PI*iCount1 - dSum1) + dSum2;
	}

	double dTotalArea = (dSum - (geo->size() - 2)*osg::PI)* osg::WGS_84_RADIUS_EQUATOR *
		osg::WGS_84_RADIUS_EQUATOR;
	return dTotalArea;
}

class TriangleResult
{
public:

	TriangleResult()
	{

	}

public:

	void operator() (const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3)
	{
		std::vector<osg::Vec3d> triangle;
		triangle.resize(3);
		triangle[0] = v1;
		triangle[1] = v2;
		triangle[2] = v3;
		_bigtriangles.push_back(triangle);
	}

public:

	void createtriangles(std::vector<std::vector<osg::Vec3d>>& triangles, double minarea)
	{
		std::vector<std::vector<std::vector<osg::Vec3d>>> results;
		results.resize(_bigtriangles.size());
		double minarea2 = minarea * minarea;
#pragma omp parallel for
		for (size_t i = 0; i < _bigtriangles.size(); i++)
		{
			DecomposeTriangleArea(_bigtriangles[i][0], _bigtriangles[i][1], _bigtriangles[i][2], results[i], minarea);
		}

		for (size_t i = 0; i < results.size(); i++)
		{
			triangles.insert(triangles.end(), results[i].begin(), results[i].end());
		}
	}

private:

	double getarea2(osg::Vec3d& v1, osg::Vec3d& v2, osg::Vec3d& v3) const
	{
		double a = (v1 - v2).length2();
		double b = (v1 - v3).length2();
		double c = (v3 - v2).length2();
		return ((c*a) - pow((c + a - b) / 2, 2)) / 4;
	}

	void DecomposeTriangleArea(osg::Vec3d& p1, osg::Vec3d& p2, osg::Vec3d& p3, std::vector<std::vector<osg::Vec3d>>& result, double minarea2)
	{

		double area2 = getarea2(p1, p2, p3);
		if (area2 < minarea2)
		{
			std::vector<osg::Vec3d> triangle;
			triangle.resize(3);
			triangle[0] = p1;
			triangle[1] = p2;
			triangle[2] = p3;
			result.push_back(triangle);
		}
		else
		{
			double d12 = (p1 - p2).length2();
			double d13 = (p1 - p3).length2();
			double d23 = (p3 - p2).length2();

			double maxd = max(d23, max(d12, d13));
			if (osg::equivalent(d12, maxd))
			{
				osg::Vec3d pc;
				pc = (p1 + p2) / 2.0;
				DecomposeTriangleArea(p1, pc, p3, result, minarea2);
				DecomposeTriangleArea(pc, p2, p3, result, minarea2);
			}
			else if (osg::equivalent(d13, maxd))
			{
				osg::Vec3d pc;
				pc = (p1 + p3) / 2.0;
				DecomposeTriangleArea(p2, pc, p1, result, minarea2);
				DecomposeTriangleArea(pc, p2, p3, result, minarea2);
			}
			else
			{
				osg::Vec3d pc;
				pc = (p2 + p3) / 2.0;
				DecomposeTriangleArea(p1, pc, p3, result, minarea2);
				DecomposeTriangleArea(pc, p1, p2, result, minarea2);
			}
		}
	}

private:

	std::vector<std::vector<osg::Vec3d>> _bigtriangles;
};

void AreaCaculator::trianglation(const std::vector<osg::Vec3d>& points, std::vector<std::vector<osg::Vec3d>>& triangles, double minarea)
{
	if (minarea < 0)
		return;
	osg::ref_ptr<osg::Geometry> polyGeom = new osg::Geometry();
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	vertices->resize(points.size());
	for (size_t i = 0; i < points.size(); i++)
	{
		(*vertices)[i] = osg::Vec3d(points[i].x(), points[i].y(), 0.0);
	}
	polyGeom->setVertexArray(vertices);
	polyGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, points.size()));

	osg::ref_ptr<osgUtil::Tessellator> tscx = new osgUtil::Tessellator;
	tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
	tscx->setBoundaryOnly(false);
	tscx->setWindingType(osgUtil::Tessellator::TESS_WINDING_ODD);
	tscx->retessellatePolygons(*polyGeom);

	osg::TriangleFunctor<TriangleResult> tf;
	polyGeom->accept(tf);
	tf.createtriangles(triangles, minarea);
}

double AreaCaculator::calcArea(const std::vector<osg::Vec3d>& points, osgEarth::MapNode* mapnode)
{
	if (mapnode && mapnode->isGeocentric())
	{
		//https://github.com/kestiny/OSG3D/blob/master/OSGEarth/calcmath.cpp

		double dTotalArea = 0.0;
		int iCount = points.size();
		if (iCount > 2)
		{
			double dLowX = 0.0;
			double dLowY = 0.0;
			double dMiddleX = 0.0;
			double dMiddleY = 0.0;
			double dHighX = 0.0;
			double dHighY = 0.0;
			double AM = 0.0;
			double BM = 0.0;
			double CM = 0.0;
			double AL = 0.0;
			double BL = 0.0;
			double CL = 0.0;
			double AH = 0.0;
			double BH = 0.0;
			double CH = 0.0;

			double dCoefficientL = 0.0;
			double dCoefficientH = 0.0;

			double dALtangent = 0.0;
			double dBLtangent = 0.0;
			double dCLtangent = 0.0;
			double dAHtangent = 0.0;
			double dBHtangent = 0.0;
			double dCHtangent = 0.0;

			double dANormalLine = 0.0;
			double dBNormalLine = 0.0;
			double dCNormalLine = 0.0;

			double dOrientationValue = 0.0;
			double dAngleCos = 0.0;

			double dSum1 = 0.0;
			double dSum2 = 0.0;
			int iCount2 = 0;
			int iCount1 = 0;

			double dSum = 0.0;

			for (int i = 0; i < iCount; i++)
			{
				if (i == 0)
				{
					dLowX = osg::DegreesToRadians(points.at(iCount - 1).x());
					dLowY = osg::DegreesToRadians(points.at(iCount - 1).y());
					dMiddleX = osg::DegreesToRadians(points.at(0).x());
					dMiddleY = osg::DegreesToRadians(points.at(0).y());
					dHighX = osg::DegreesToRadians(points.at(1).x());
					dHighY = osg::DegreesToRadians(points.at(1).y());
				}
				else if (i == iCount - 1)
				{
					dLowX = osg::DegreesToRadians(points.at(iCount - 2).x());
					dLowY = osg::DegreesToRadians(points.at(iCount - 2).y());
					dMiddleX = osg::DegreesToRadians(points.at(iCount - 1).x());
					dMiddleY = osg::DegreesToRadians(points.at(iCount - 1).y());
					dHighX = osg::DegreesToRadians(points.at(0).x());
					dHighY = osg::DegreesToRadians(points.at(0).y());
				}
				else
				{
					dLowX = osg::DegreesToRadians(points.at(i - 1).x());
					dLowY = osg::DegreesToRadians(points.at(i - 1).y());
					dMiddleX = osg::DegreesToRadians(points.at(i).x());
					dMiddleY = osg::DegreesToRadians(points.at(i).y());
					dHighX = osg::DegreesToRadians(points.at(i + 1).x());
					dHighY = osg::DegreesToRadians(points.at(i + 1).y());
				}
				AM = cos(dMiddleY) * cos(dMiddleX);
				BM = cos(dMiddleY) * sin(dMiddleX);
				CM = sin(dMiddleY);
				AL = cos(dLowY) * cos(dLowX);
				BL = cos(dLowY) * sin(dLowX);
				CL = sin(dLowY);
				AH = cos(dHighY) * cos(dHighX);
				BH = cos(dHighY) * sin(dHighX);
				CH = sin(dHighY);

				dCoefficientL = (AM*AM + BM * BM + CM * CM) / (AM*AL + BM * BL + CM * CL);
				dCoefficientH = (AM*AM + BM * BM + CM * CM) / (AM*AH + BM * BH + CM * CH);

				dALtangent = dCoefficientL * AL - AM;
				dBLtangent = dCoefficientL * BL - BM;
				dCLtangent = dCoefficientL * CL - CM;
				dAHtangent = dCoefficientH * AH - AM;
				dBHtangent = dCoefficientH * BH - BM;
				dCHtangent = dCoefficientH * CH - CM;

				dAngleCos = (dAHtangent * dALtangent + dBHtangent * dBLtangent + dCHtangent * dCLtangent) /
					(sqrt(dAHtangent * dAHtangent + dBHtangent * dBHtangent + dCHtangent * dCHtangent) *
						sqrt(dALtangent * dALtangent + dBLtangent * dBLtangent + dCLtangent * dCLtangent));

				dAngleCos = acos(dAngleCos);

				dANormalLine = dBHtangent * dCLtangent - dCHtangent * dBLtangent;
				dBNormalLine = 0 - (dAHtangent * dCLtangent - dCHtangent * dALtangent);
				dCNormalLine = dAHtangent * dBLtangent - dBHtangent * dALtangent;

				if (AM != 0)
				{
					dOrientationValue = dANormalLine / AM;
				}
				else if (BM != 0)
				{
					dOrientationValue = dBNormalLine / BM;
				}
				else
				{
					dOrientationValue = dCNormalLine / CM;
				}
				if (dOrientationValue > 0)
				{
					dSum1 += dAngleCos;
					iCount1++;
				}
				else
				{
					dSum2 += dAngleCos;
					iCount2++;
				}
			}
			if (dSum1 > dSum2)
			{
				dSum = dSum1 + (2 * osg::PI*iCount2 - dSum2);
			}
			else
			{
				dSum = (2 * osg::PI*iCount1 - dSum1) + dSum2;
			}

			double r = mapnode->getMapSRS()->getEllipsoid()->getRadiusEquator();
			dTotalArea = (dSum - (iCount - 2)*osg::PI)* r * r;
		}
		return dTotalArea;
	}
	else
	{
		osg::ref_ptr<osgEarth::Features::Polygon> ply = new osgEarth::Features::Polygon(&points);
		return osgEarth::Features::GeometryUtils::getGeometryArea(ply);
	}
}

double AreaCaculator::calcSurfaceArea(const std::vector<osg::Vec3d>& points, osgEarth::MapNode* mapnode)
{
	double minarea = calcArea(points, 0L);
	minarea /= 100;
	std::vector<std::vector<osg::Vec3d>> triangles;
	trianglation(points, triangles, minarea);

	double surfacearea = 0.0;
	for (size_t i = 0; i < triangles.size(); i++)
	{
		osg::Vec3d& p0 = triangles[i][0];
		osg::Vec3d& p1 = triangles[i][1];
		osg::Vec3d& p2 = triangles[i][2];
		/*mapnode->getTerrain()->getHeight(NULL, p0.x(), p0.y(), &p0.z(), NULL);
		mapnode->getTerrain()->getHeight(NULL, p1.x(), p1.y(), &p1.z(), NULL);
		mapnode->getTerrain()->getHeight(NULL, p2.x(), p2.y(), &p2.z(), NULL);
		p0 = latlnghigh2xyz(mapnode, p0);
		p1 = latlnghigh2xyz(mapnode, p1);
		p2 = latlnghigh2xyz(mapnode, p2);*/


		surfacearea += caltrianglearea(p0, p1, p2);
	}

	return surfacearea;
}

osg::Vec3d AreaCaculator::latlnghigh2xyz(osgEarth::MapNode* mapnode, const osg::Vec3d& llh)
{
	osg::Vec3d res;
	double lat = osg::DegreesToRadians(llh.y());
	double lng = osg::DegreesToRadians(llh.x());;
	double height = llh.z();
	mapnode->getMap()->getProfile()->getSRS()->getEllipsoid()->convertLatLongHeightToXYZ(
		lat, lng, height, res.x(), res.y(), res.z());
	return res;
}

double AreaCaculator::caltrianglearea(const osg::Vec3d& a, const osg::Vec3d& b, const osg::Vec3d& c)
{
	double area = 0;

	double side[3];

	side[0] = sqrt(pow(a.x() - b.x(), 2) + pow(a.y() - b.y(), 2) + pow(a.z() - b.z(), 2));
	side[1] = sqrt(pow(a.x() - c.x(), 2) + pow(a.y() - c.y(), 2) + pow(a.z() - c.z(), 2));
	side[2] = sqrt(pow(c.x() - b.x(), 2) + pow(c.y() - b.y(), 2) + pow(c.z() - b.z(), 2));

	if (side[0] + side[1] <= side[2] || side[0] + side[2] <= side[1] || side[1] + side[2] <= side[0]) return area;

	double p = (side[0] + side[1] + side[2]) / 2;
	area = sqrt(p*(p - side[0])*(p - side[1])*(p - side[2]));

	return area;
}