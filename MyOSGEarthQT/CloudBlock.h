﻿#pragma once
/* -*-c++-*- OpenSceneGraph Cookbook
 * Chapter 6 Recipe 9
 * Author: Wang Rui <wangray84 at gmail dot com>
*/

#ifndef H_COOKBOOK_CH6_CLOUDBLOCK
#define H_COOKBOOK_CH6_CLOUDBLOCK

#include <osg/Drawable>
#include <osg/Version>

class CloudBlock : public osg::Drawable
{
public:
	struct CloudCell
	{
		CloudCell() : _brightness(0.0f), _density(0.0f) {}

		bool operator==(const CloudCell& copy) const
		{
			return _pos == copy._pos && _brightness == copy._brightness && _density == copy._density;
		}

		osg::Vec3d _pos;
		float _brightness;
		float _density;
	};

	struct LessDepthSortFunctor
	{
		LessDepthSortFunctor(const osg::Matrix& matrix)
		{
			_frontVector.set(-matrix(0, 2), -matrix(1, 2), -matrix(2, 2), -matrix(3, 2));
		}

		bool operator()(const CloudCell& lhs, const CloudCell& rhs) const
		{
			return getDepth(lhs._pos) > getDepth(rhs._pos);
		}

		float getDepth(const osg::Vec3d& pos) const
		{
			return (float)pos[0] * _frontVector[0] +
				(float)pos[1] * _frontVector[1] +
				(float)pos[2] * _frontVector[2] + _frontVector[3];
		}

		osg::Vec4 _frontVector;
	};

public:
	CloudBlock();
	CloudBlock(const CloudBlock& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

	META_Object(osgModeling, CloudBlock);
	virtual osg::Geometry* asGeometry() { return 0; }
	virtual const osg::Geometry* asGeometry() const { return 0; }

	typedef std::vector<CloudCell> CloudCells;
	void setCloudCells(const CloudCells& cells) { _cells = cells; dirtyBound(); }
	CloudCells& getCloudCells() { return _cells; }
	const CloudCells& getCloudCells() const { return _cells; }

	virtual osg::BoundingBox computeBoundingBox() const;

	virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

protected:
	void renderCells(const osg::Matrix& modelview) const;

	mutable CloudCells _cells;
};
#endif
