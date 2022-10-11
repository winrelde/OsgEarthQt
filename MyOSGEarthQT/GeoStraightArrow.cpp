#include <Windows.h>
#include "GeoStraightArrow.h"
#include <algorithm>
#include "PlottingMath.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

GeoStraightArrow::GeoStraightArrow(osgEarth::MapNode *mapNode, osg::Group *drawGroup)
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

void GeoStraightArrow::beginDraw(const osg::Vec3d &lla)
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
        _drawParts = calculateMorePoints(_controlPoints);

//    if (_polygonEdit.valid()) {
//        _polygonEdit->removeChildren(0, _polygonEdit->getNumChildren());
//        _polygonEdit = NULL;
//    }

    if (!_featureNode.valid()) {
         Feature* feature = new Feature(new osgEarth::Symbology::Polygon, getMapNode()->getMapSRS(), _polygonStyle);
        _featureNode = new FeatureNode(feature);
//        _drawGroup->addChild(_featureNode);
        drawCommand(_featureNode);
    }

    Geometry* geom = _featureNode->getFeature()->getGeometry();
    geom->clear();
    for (auto& n : _drawParts) {
        geom->push_back(osg::Vec3(n.x(), n.y(), 0));
    }
    _featureNode->init();

//    if (!_polygonEdit.valid()) {
//        _polygonEdit = new FeatureEditor(_featureNode);
//        drawCommand(_polygonEdit);
//    }
}

void GeoStraightArrow::moveDraw(const osg::Vec3d &lla)
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
            drawPts = calculateMorePoints(ctrlPts);

        Geometry* geom = _featureNode->getFeature()->getGeometry();
        geom->clear();
        for (auto& n : drawPts) {
            geom->push_back(osg::Vec3(n.x(), n.y(), 0));
        }
        _featureNode->init();
    }
}

void GeoStraightArrow::endDraw(const osg::Vec3d &lla)
{

}

void GeoStraightArrow::resetDraw()
{
    _controlPoints.clear();
    _featureNode = NULL;
}

std::vector<osg::Vec2> GeoStraightArrow::calculateTwoPoints(const std::vector<osg::Vec2>& ctrlPts)
{
	osg::Vec2 pointS, pointE;
    //取出第一和第二两个点

    pointS =  ctrlPts[0];
    pointE = ctrlPts[1];
    //计算箭头总长度，即两个控制点的距离

    float l = sqrtf((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
    float w = l / _ratio;
    //计算三角形的底边中心点坐标

    float x_ = pointS.x() + (pointE.x() - pointS.x())*(_ratio-1)/_ratio;
    float y_ = pointS.y() + (pointE.y() - pointS.y())*(_ratio-1)/_ratio;
    //计算与基本向量夹角90度的，长度为w/2的向量数组

    osg::Vec2 v_l, v_r;
    std::vector<osg::Vec2> v_lr = Math::calculateVector(osg::Vec2(pointE.x()-pointS.x(),pointE.y()-pointS.y()), osg::PI_2, w/2);
    v_l = v_lr[0];
    v_r = v_lr[1];

    //左1点

    osg::Vec2 point1(pointS.x()+v_l.x(),pointS.y()+v_l.y());
    //左2点

    osg::Vec2 point2(x_+point1.x()-pointS.x(),y_+point1.y()-pointS.y());
    // 左3点

    osg::Vec2 point3(2*point2.x()-x_, 2*point2.y()-y_);
    // 顶点

    osg::Vec2 point4(pointE.x(), pointE.y());
    // 右3点

    osg::Vec2 point7(pointS.x()+v_r.x(),pointS.y()+v_r.y());
    // 右2点

    osg::Vec2 point6(x_+point7.x()-pointS.x(), y_+point7.y()-pointS.y());
    // 右1点

    osg::Vec2 point5(2*point6.x()-x_, 2*point6.y()-y_);

    return std::vector<osg::Vec2>{point1, point2, point3, point4, point5, point6, point7};
}

std::vector<osg::Vec2> GeoStraightArrow::calculateMorePoints(const std::vector<osg::Vec2>& ctrlPts)
{
    //计算箭头总长度和直箭头的宽

    float l = 0, w = 0;
    for (unsigned int i = 0; i < ctrlPts.size()-1; i++) {
        //取出首尾两个点

        osg::Vec2 pointS = ctrlPts[i];
        osg::Vec2 pointE = ctrlPts[i+1];
        l += sqrtf((pointE.y()-pointS.y())*(pointE.y()-pointS.y())+(pointE.x()-pointS.x())*(pointE.x()-pointS.x()));
    }
    w = l/_ratio;
    float a = atanf(w/(2.0*l));
    //定义左右控制点集合

    std::vector<osg::Vec2> points_C_l;
    std::vector<osg::Vec2> points_C_r;
    // 定义尾部左右的起始点

    osg::Vec2 point_t_l;
    osg::Vec2 point_t_r;

	osg::Vec2 pointU_1, pointU_2, pointU_3;
    //计算中间的所有交点

    for (unsigned int j = 0; j < ctrlPts.size()-2; j++) {
        osg::Vec2 pointU_1 = ctrlPts[j];
        osg::Vec2 pointU_2 = ctrlPts[j+1];
        osg::Vec2 pointU_3 = ctrlPts[j+2];

        // 计算向量

        osg::Vec2 v_U_1_2(pointU_2.x()-pointU_1.x(), pointU_2.y()-pointU_1.y());
        osg::Vec2 v_U_2_3(pointU_3.x()-pointU_2.x(), pointU_3.y()-pointU_2.y());

        osg::Vec2 v_l_1_2, v_r_1_2;
        std::vector<osg::Vec2> v_lr_1_2 =Math::calculateVector(v_U_1_2, osg::PI_2, w / 2);
        v_l_1_2 = v_lr_1_2[0];
        v_r_1_2 = v_lr_1_2[1];

        osg::Vec2 v_l_2_3, v_r_2_3;
        std::vector<osg::Vec2> v_lr_2_3 = Math::calculateVector(v_U_2_3, osg::PI_2, w / 2);
        v_l_2_3 = v_lr_2_3[0];
        v_r_2_3 = v_lr_2_3[1];

        //获取左右

        osg::Vec2 point_l_1(pointU_1.x()+v_l_1_2.x(), pointU_1.y()+v_l_1_2.y());
        osg::Vec2 point_r_1(pointU_1.x()+v_r_1_2.x(), pointU_1.y()+v_r_1_2.y());
        osg::Vec2 point_l_2(pointU_2.x()+v_l_2_3.x(), pointU_2.y()+v_l_2_3.y());
        osg::Vec2 point_r_2(pointU_2.x()+v_r_2_3.x(), pointU_2.y()+v_r_2_3.y());
        //向量v_U_1_2和向量v-point_l_1和point_r_1是平行的
        //如果向量a=(x1，y1)，b=(x2，y2)，则a//b等价于x1y2－x2y1=0
        //得到(x-point_l_1.x)*v_U_1_2.y=v_U_1_2.x*(y-point_l_1.y)
        //得到(point_l_2.x-x)*v_U_2_3.y=v_U_2_3.x*(point_l_2.y-y)
        //可以求出坐边的交点(x,y)，即控制点

        osg::Vec2 point_C_l = Math::calculateIntersection(v_U_1_2,v_U_2_3,point_l_1,point_l_2);
        osg::Vec2 point_C_r = Math::calculateIntersection(v_U_1_2,v_U_2_3,point_r_1,point_r_2);
        //定义中间的控制点

        osg::Vec2 point_C_l_c;
        osg::Vec2 point_C_r_c;
        if (j == 0) {
            //记录下箭头尾部的左右两个端点

            point_t_l = point_l_1;
            point_t_r = point_r_1;
            //计算第一个曲线控制点

            point_C_l_c = osg::Vec2((point_t_l.x()+point_C_l.x())/2,(point_t_l.y()+point_C_l.y())/2);
            point_C_r_c = osg::Vec2((point_t_r.x()+point_C_r.x())/2,(point_t_r.y()+point_C_r.y())/2);
            //添加两个拐角控制点中间的中间控制点

            points_C_l.push_back(point_C_l_c);
            points_C_r.push_back(point_C_r_c);
        } else {
            //获取前一个拐角控制点

            osg::Vec2 point_C_l_q = points_C_l[points_C_l.size() - 1];
            osg::Vec2 point_C_r_q = points_C_r[points_C_r.size() - 1];
            // 计算两个拐角之间的中心控制点

            point_C_l_c = osg::Vec2((point_C_l_q.x()+point_C_l.x())/2,(point_C_l_q.y()+point_C_l.y())/2);
            point_C_r_c = osg::Vec2((point_C_r_q.x()+point_C_r.x())/2,(point_C_r_q.y()+point_C_r.y())/2);
            //添加两个拐角控制点中间的中间控制点

            points_C_l.push_back(point_C_l_c);
            points_C_r.push_back(point_C_r_c);
        }
        //添加后面的拐角控制点

        points_C_l.push_back(point_C_l);
        points_C_r.push_back(point_C_r);
    }

    // 计算
    // 进入计算头部
    // 计算一下头部的长度

    osg::Vec2 pointU_E2 = ctrlPts[ctrlPts.size()-2]; //倒数第二个用户点
    osg::Vec2 pointU_E1 = ctrlPts[ctrlPts.size()-1]; //倒数第二个用户点
    osg::Vec2 v_U_E2_E1(pointU_E1.x()-pointU_E2.x(),pointU_E1.y()-pointU_E2.y());
    float head_d = sqrtf(v_U_E2_E1.x()*v_U_E2_E1.x() + v_U_E2_E1.y()*v_U_E2_E1.y());
    // 定义头部的左右两结束点

    osg::Vec2 point_h_l, point_h_r;
    //头部左右两向量数组

    std::vector<osg::Vec2> v_lr_h;
    osg::Vec2 v_l_h, v_r_h;
    //定义曲线最后一个控制点，也就是头部结束点和最后一个拐角点的中点

    osg::Vec2 point_C_l_e, point_C_r_e;
    // 定义三角形的左右两个点

    osg::Vec2 point_triangle_l, point_triangle_r;

	//osg::Vec2 point_C_l_eq, point_C_r_eq;
    //获取当前的最后的控制点，也就是之前计算的拐角点

    osg::Vec2 point_C_l_eq = points_C_l[points_C_l.size()-1];
    osg::Vec2 point_C_r_eq = points_C_r[points_C_r.size()-1];

    //三角的高度都不够

    if (head_d <= w) {
        v_lr_h = Math::calculateVector(v_U_E2_E1, osg::PI_2, w/2);
        v_l_h = v_lr_h[0];
        v_r_h = v_lr_h[1];
        //获取头部的左右两结束点

        point_h_l = osg::Vec2(pointU_E2.x()+v_l_h.x(), pointU_E2.y()+v_l_h.y());
        point_h_r = osg::Vec2((point_C_r_eq.x()+point_h_r.x())/2,(point_C_r_eq.y()+point_h_r.y())/2);

        //添加最后的控制点（中心点）

        points_C_l.push_back(point_C_l_e);
        points_C_r.push_back(point_C_r_e);

        // 计算三角形的左右两点

        point_triangle_l = osg::Vec2(2*point_h_l.x()-pointU_E2.x(), 2*point_h_l.y()-pointU_E2.y());
        point_triangle_r = osg::Vec2(2*point_h_r.x()-pointU_E2.x(), 2*point_h_r.y()-pointU_E2.y());
    } else { //足够三角的高度
        //由于够了三角的高度，所以首先去掉三角的高度
        //计算向量

        osg::Vec2 v_E2_E1(pointU_E1.x()-pointU_E2.x(), pointU_E1.y()-pointU_E2.y());
        //取模

        float v_E2_E1_d = sqrtf(v_E2_E1.x()*v_E2_E1.x()+v_E2_E1.y()*v_E2_E1.y());
        //首先需要计算三角形的底部中心点

        osg::Vec2 point_c(pointU_E1.x()-v_E2_E1.x()*w/v_E2_E1_d,pointU_E1.y()-v_E2_E1.y()*w/v_E2_E1_d);
        //计算出在三角形上底边上头部结束点

        v_lr_h = Math::calculateVector(v_U_E2_E1,osg::PI_2,w/2);
        v_l_h = v_lr_h[0];
        v_r_h = v_lr_h[1];
        //获取头部的左右两结束点

        point_h_l = osg::Vec2(point_c.x()+v_l_h.x(), point_c.y()+v_l_h.y());
        point_h_r = osg::Vec2(point_c.x()+v_r_h.x(), point_c.y()+v_r_h.y());
        //计算最后的控制点

        point_C_l_e = osg::Vec2((point_C_l_eq.x()+point_h_l.x())/2, (point_C_l_eq.y()+point_h_l.y())/2);
        point_C_r_e = osg::Vec2((point_C_r_eq.x()+point_h_r.x())/2,(point_C_r_eq.y()+point_h_r.y())/2);
        //添加最后的控制点（中心点）

        points_C_l.push_back(point_C_l_e);
        points_C_r.push_back(point_C_r_e);
        // 计算三角形的左右点

        point_triangle_l = osg::Vec2(2*point_h_l.x()-point_c.x(), 2*point_h_l.y()-point_c.y());
        point_triangle_r = osg::Vec2(2*point_h_r.x()-point_c.x(), 2*point_h_r.y()-point_c.y());
    }
    //使用控制点计算差值
    //计算贝塞尔的控制点

    std::vector<osg::Vec2> points_BC_l = Math::createBezier2(points_C_l);
    std::vector<osg::Vec2> points_BC_r = Math::createBezier2(points_C_r);
    // 组合左右点集和三角形三个点

    std::vector<osg::Vec2> pointsR;
    pointsR.push_back(point_t_l);
    //首先连接左边的差值曲线

    pointsR.insert(pointsR.end(), points_BC_l.begin(), points_BC_l.end());
    //添加左边头部结束点

    pointsR.push_back(point_h_l);
    //添加三角形左边点

    pointsR.push_back(point_triangle_l);
    //添加三角形顶点

    pointsR.push_back(pointU_E1);
    // 添加三角形右边点

    pointsR.push_back(point_triangle_r);
    // 添加右边头部结束点

    pointsR.push_back(point_h_r);
    //合并右边的所有点（先把右边的点倒序）

    reverse(points_BC_r.begin(), points_BC_r.end());
    pointsR.insert(pointsR.end(), points_BC_r.begin(), points_BC_r.end());
    //添加右边尾部起始点

    pointsR.push_back(point_t_r);
    return pointsR;
}

void GeoStraightArrow::calculateParts()
{

}
