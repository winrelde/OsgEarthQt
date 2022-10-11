#include <Windows.h>
#include "GeoDiagonalArrow.h"
#include "PlottingMath.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

GeoDiagonalArrow::GeoDiagonalArrow(osgEarth::MapNode *mapNode, osg::Group *drawGroup)
    : DrawTool(mapNode, drawGroup)
    , _ratio(6.0)
{
    _polygonStyle.getOrCreate<PolygonSymbol>()
        ->fill()
        ->color()
        = Color(Color::Yellow, 0.25);

    LineSymbol* ls = _polygonStyle.getOrCreate<LineSymbol>();
    ls->stroke()->color() = Color::White;
    ls->stroke()->width() = 2.0f;
    ls->stroke()->widthUnits() = osgEarth::Units::PIXELS;

    _polygonStyle.getOrCreate<AltitudeSymbol>()
        ->clamping()
        = AltitudeSymbol::CLAMP_TO_TERRAIN;
    _polygonStyle.getOrCreate<AltitudeSymbol>()
        ->technique()
        = AltitudeSymbol::TECHNIQUE_DRAPE;
    _polygonStyle.getOrCreate<AltitudeSymbol>()
        ->verticalOffset()
        = 0.1;
}

void GeoDiagonalArrow::beginDraw(const osg::Vec3d &lla)
{
    _controlPoints.push_back(osg::Vec2(lla.x(), lla.y()));
    if (_controlPoints.empty() || _controlPoints.size() < 2)
        return;
    if (_controlPoints.size() == 2 && _controlPoints[0]==_controlPoints[1])
        return;
     _drawParts.clear();
	 if (_controlPoints.size() == 2)
		 _drawParts = calculateTwoPoints(_controlPoints);
	 else
		 // _drawParts = calculateMorePoints(_controlPoints);
		 return;

     if (!_featureNode.valid()) {
          Feature* feature = new Feature(new osgEarth::Symbology::Polygon, getMapNode()->getMapSRS(), _polygonStyle);
         _featureNode = new FeatureNode(feature);
          drawCommand(_featureNode);
     }

     Geometry* geom = _featureNode->getFeature()->getGeometry();
     geom->clear();
     for (auto& n : _drawParts) {
         geom->push_back(osg::Vec3(n.x(), n.y(), 0));
     }
     _featureNode->init();
}

void GeoDiagonalArrow::moveDraw(const osg::Vec3d &lla)
{
    if (_controlPoints.empty())
        return;
    if (!_featureNode.valid()) {
         Feature* feature = new Feature(new osgEarth::Symbology::Polygon, getMapNode()->getMapSRS(), _polygonStyle);
        _featureNode = new FeatureNode(feature);
        drawCommand(_featureNode);
    }

    if (_featureNode.valid()) {
        std::vector<osg::Vec2> ctrlPts = _controlPoints;
        std::vector<osg::Vec2> drawPts;
        ctrlPts.push_back(osg::Vec2(lla.x(), lla.y()));

		if (ctrlPts.size() == 2)
			drawPts = calculateTwoPoints(ctrlPts);
		else
			//drawPts = calculateMorePoints(ctrlPts);
			return ;

        Geometry* geom = _featureNode->getFeature()->getGeometry();
        geom->clear();
        for (auto& n : drawPts) {
            geom->push_back(osg::Vec3(n.x(), n.y(), 0));
        }
        _featureNode->init();
    }
}

void GeoDiagonalArrow::endDraw(const osg::Vec3d &lla)
{

}

void GeoDiagonalArrow::resetDraw()
{
    _controlPoints.clear();
    _featureNode = NULL;
}

std::vector<osg::Vec2> GeoDiagonalArrow::calculateTwoPoints(const std::vector<osg::Vec2> &ctrlPts)
{
	osg::Vec2 pointS, pointE;
    //取出首尾两个点

    pointS = ctrlPts[0];
    pointE = ctrlPts[1];

	float w;
    //计算箭头总长度

    float l = sqrtf((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
    //计算直箭头的宽

    w = l/_ratio;

	float x_;
    //计算三角形的底边中心点坐标

    x_ = pointS.x() + (pointE.x() - pointS.x())*(_ratio-1)/_ratio;
    float y_ = pointS.y() + (pointE.y() - pointS.y())*(_ratio-1)/_ratio;
    osg::Vec2 point_o(x_, y_);

    //计算

    std::vector<osg::Vec2> v_lr_ = Math::calculateVector(osg::Vec2(pointE.x()-pointS.x(), pointE.y()-pointS.y()), osg::PI/2, w/2);
    //获取左边尾部向量

    osg::Vec2 v_l_ = v_lr_[0];
    //获取右边尾部向量

    osg::Vec2 v_r_ = v_lr_[1];
    //获取左边尾部点

    osg::Vec2 point_l(v_l_.x()+pointS.x(), v_l_.y()+pointS.y());
    //获取右边尾部点

    osg::Vec2 point_r(v_r_.x()+pointS.x(), v_r_.y()+pointS.y());

    osg::Vec2 point_h_l(v_l_.x()/_ratio+x_, v_l_.y()/_ratio+y_);
    osg::Vec2 point_h_r(v_r_.x()/_ratio+x_, v_r_.y()/_ratio+y_);

    //计算三角形左边点

    osg::Vec2 point_a_l((point_h_l.x()*2-point_h_r.x()), point_h_l.y()*2-point_h_r.y());
    //计算三角形右边点

    osg::Vec2 point_a_r(point_h_r.x()*2-point_h_l.x(), point_h_r.y()*2-point_h_l.y());
    return std::vector<osg::Vec2>{point_l,point_h_l,point_a_l,pointE,point_a_r,point_h_r,point_r};

}

//std::vector<osg::Vec2> GeoDiagonalArrow::calculateMorePoints(const std::vector<osg::Vec2> &ctrlPts)
//{
//    //计算箭头总长度
//    float l = 0;
//    //计算直箭头的宽
//    float w = 0;
//    for (int i = 0; i < ctrlPts.size()-1; i++) {
//        //取出首尾两个点
//        osg::Vec2 pointS = ctrlPts[i];
//        osg::Vec2 pointE = ctrlPts[i+1];
//        l += sqrtf((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
//    }
//    w = l/_ratio;
//    float a = atanf(w/(2.0*l));
//
//    //定义左右控制点集合
//    std::vector<osg::Vec2> points_C_l;
//    std::vector<osg::Vec2> points_C_r;
//    //定义尾部左右的起始点
//    osg::Vec2 point_t_l;
//    osg::Vec2 point_t_r;
//
//    //计算中间的所有交点
//    for (int j=0; j < ctrlPts.size()-2; j++) {
//        osg::Vec2 pointU_1 = ctrlPts[j]; //第一个用户传入的点
//        osg::Vec2 pointU_2 = ctrlPts[j+1]; //第二个用户传入的点
//        osg::Vec2 pointU_3 = ctrlPts[j+2]; //第三个用户传入的点
//
//        //计算向量
//        osg::Vec2 v_U_1_2(pointU_2.x()-pointU_1.x(), pointU_2.y()-pointU_1.y());
//        osg::Vec2 v_U_2_3(pointU_3.x()-pointU_2.x(), pointU_3.y()-pointU_2.y());
//
//        //定义左边第一个控制点
//        osg::Vec2 point_l_1;
//        //定义右边第一个控制点
//        osg::Vec2 point_r_1;
//        //如果j=0时，左右第一个控制点需要计算
//        if (j == 0) {
//            std::vector<osg::Vec2> v_lr_= Math::calculateVector(v_U_1_2, osg::PI_2, w/2);
//            //获取左边尾部点
//            osg::Vec2 v_l_ = v_lr_[0];
//            //获取右边尾部点
//            osg::Vec2 v_r_ = v_lr_[1];
//            //获取左边尾部点
//            point_t_l = point_l_1 = osg::Vec2(v_l_.x()+pointU_1.x(), v_l_.y()+pointU_1.y());
//            //获取右边尾部点
//            point_t_r = point_r_1 = osg::Vec2(v_r_.x()+pointU_1.x(), v_r_.y()+pointU_1.y());
//        } else { //否则获取上一次的记录
//            point_l_1 = points_C_l[points_C_l.size()-1];
//            point_r_1 = points_C_r[points_C_r.size()-1];
//        }
//
//        std::vector<osg::Vec2> v_lr = Math::calculateVector(v_U_1_2, a, 1);
//        //这里的向量需要反过来
//        //获取左边向量
//        osg::Vec2 v_l = v_lr[1];
//        //获取右边向量
//        osg::Vec2 v_r = v_lr[0];
//        //定义角平分线向量
//        osg::Vec2 v_angularBisector = Math::calculateAngularBisector(osg::Vec2(-v_U_1_2.x(), -v_U_1_2.y()), v_U_2_3);
//        //求交点
//        //计算左边第二个控制点
//        osg::Vec2 point_l_2 = Math::calculateIntersection(v_l, v_angularBisector, point_l_1, pointU_2);
//        osg::Vec2 point_r_2 = Math::calculateIntersection(v_r, v_angularBisector, point_r_1, pointU_2);
//
//        //添加后面的拐角控制点
//        points_C_l.push_back(osg::Vec2((point_l_1.x()+point_l_2.x())/2, (point_l_1.y()+point_l_2.y())/2));
//        points_C_l.push_back(point_l_2);
//        points_C_r.push_back(osg::Vec2((point_r_1.x()+point_r_2.x())/2, (point_r_1.y()+point_r_2.y())/2));
//        points_C_r.push_back(point_r_2);
//    }
//    //进入计算头部
//    //计算一下头部的长度
//    osg::Vec2 pointU_E2 = ctrlPts[ctrlPts.size()-2];//倒数第二个用户点
//    osg::Vec2 pointU_E1 = ctrlPts[ctrlPts.size()-1];//最后一个用户点
//    float head_d = sqrtf((pointU_E2.x()-pointU_E1.x())*(pointU_E2.x()-pointU_E1.x()) + (pointU_E2.y()-pointU_E1.y())*(pointU_E2.y()-pointU_E1.y()));
//    //定义头部的左右两结束点
//    osg::Vec2 point_h_l, point_h_r;
//    //三角形左右两点数组
//    std::vector<osg::Vec2> point_lr_t;
//    //定义曲线最后一个控制点，也就是头部结束点和最后一个拐角点的中点
//    osg::Vec2 point_C_l_e, point_C_r_e;
//    //定义三角形的左右两个点
//    osg::Vec2 point_triangle_l, point_triangle_r;
//
//    //获取当前的最后的控制点，也就是之前计算的拐角点
//    osg::Vec2 point_C_l_eq = points_C_l[points_C_l.size()-1];
//    osg::Vec2 point_C_r_eq = points_C_r[points_C_r.size()-1];
//    //申明三角形的两边向量
//    osg::Vec2 v_l_t, v_r_t;
//    //三角的高度都不够
//    if (head_d <= w) {
//        point_lr_t = Math::calculateVector(osg::Vec2(pointU_E1.x()-pointU_E2.x(), pointU_E1.y()-pointU_E2.y()), osg::PI_2, w/2);
//        //获取三角形左右两个向量
//        v_l_t = point_lr_t[0];
//        v_r_t = point_lr_t[1];
//        point_h_l = osg::Vec2(v_l_t.x()/_ratio+pointU_E2.x(), v_l_t.y()/_ratio+pointU_E2.y());
//        point_h_r = osg::Vec2(v_r_t.x()/_ratio+pointU_E2.x(), v_r_t.y()/_ratio+pointU_E2.y());
//        //计算三角形的左右两点
//        point_triangle_l = osg::Vec2(point_h_l.x()*2-point_h_r.x(), point_h_l.y()*2-point_h_r.y());
//        point_triangle_r = osg::Vec2(point_h_r.x()*2-point_h_l.x(), point_h_r.y()*2-point_h_l.y());
//
//        //计算最后的控制点
//        point_C_l_e = osg::Vec2((point_C_l_eq.x()+point_h_l.x())/2, (point_C_l_eq.y()+point_h_l.y())/2);
//        point_C_r_e = osg::Vec2((point_C_r_eq.x()+point_h_r.x())/2, (point_C_r_eq.y()+point_h_r.y())/2);
//
//        //添加最后的控制点（中心点）
//        points_C_l.push_back(point_C_l_e);
//        points_C_r.push_back(point_C_r_e);
//    } else { //足够三角的高度
//        //由于够了三角的高度，所以首先去掉三角的高度
//        //计算向量
//        osg::Vec2 v_E2_E1(pointU_E1.x()-pointU_E2.x(), pointU_E1.y()-pointU_E2.y());
//        //取模
//        float v_E2_E1_d = sqrtf(v_E2_E1.x()*v_E2_E1.x()+v_E2_E1.y()*v_E2_E1.y());
//        //首先需要计算三角形的底部中心点
//        osg::Vec2 point_c(pointU_E1.x()-v_E2_E1.x()*w/v_E2_E1_d, pointU_E1.y()-v_E2_E1.y()*w/v_E2_E1_d);
//
//        //计算出在三角形上底边上头部结束点
//        point_lr_t = Math::calculateVector(osg::Vec2(pointU_E1.x()-point_c.x(), pointU_E1.y()-point_c.y()), osg::PI_2, w/2);
//        //获取三角形左右两个向量
//        v_l_t = point_lr_t[0];
//        v_r_t = point_lr_t[1];
//
//        point_h_l = osg::Vec2(v_l_t.x()/_ratio+point_c.x(), v_l_t.y()/_ratio+point_c.y());
//        point_h_r = osg::Vec2(v_r_t.x()/_ratio+point_c.x(), v_r_t.y()/_ratio+point_c.y());
//
//        //计算三角形的左右两点
//        point_triangle_l = osg::Vec2(point_h_l.x()*2-point_h_r.x(), point_h_l.y()*2-point_h_r.y());
//        point_triangle_r = osg::Vec2(point_h_r.x()*2-point_h_l.x(), point_h_r.y()*2-point_h_l.y());
//
//        //计算最后的控制点
//        point_C_l_e = osg::Vec2((point_C_l_eq.x()+point_h_l.x())/2,(point_C_l_eq.y()+point_h_l.y())/2);
//        point_C_r_e = osg::Vec2((point_C_r_eq.x()+point_h_r.x())/2,(point_C_r_eq.y()+point_h_r.y())/2);
//        //添加最后的控制点（中心点）
//        points_C_l.push_back(point_C_l_e);
//        points_C_r.push_back(point_C_r_e);
//    }
//    //使用控制点计算差值
//    //计算贝塞尔的控制点
//    std::vector<osg::Vec2> points_BC_l = Math::createBezier2(points_C_l);
//    std::vector<osg::Vec2> points_BC_r = Math::createBezier2(points_C_r);
//    //组合左右点集和三角形三个点
//    std::vector<osg::Vec2> pointsR{point_t_l};
//    //首先连接左边的差值曲线
//    pointsR.insert(pointsR.end(), points_BC_l.begin(), points_BC_l.end());
//    //添加左边头部结束点
//    pointsR.push_back(point_h_l);
//    //添加三角形左边点
//    pointsR.push_back(point_triangle_l);
//    //添加三角形顶点
//    pointsR.push_back(pointU_E1);
//    //添加三角形右边点
//    pointsR.push_back(point_triangle_r);
//    //添加右边头部结束点
//    pointsR.push_back(point_h_r);
//    //合并右边的所有点
//    for (int k = points_BC_r.size()-1; k >= 0; k--) {
//        pointsR.push_back(points_BC_r[k]);
//    }
//    //添加右边尾部起始点
//    pointsR.push_back(point_t_r);
//    return pointsR;
//}
