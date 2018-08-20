#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <algorithm>

#ifdef WIN32
#undef min
#undef max
#endif /* WIN32 */

// Just a note about inlining:
// All functions here are declared as inline... Normally the compiler should 
// ignore this, because the function is too big, but maybe it makes sense in
// some cases!

namespace Parallelogram {

// the origin of x and y must be in the bottom-left of the pararallelogram!
template<const int Width, const int Height, const int TriangleWidth>
inline bool IntersectsWithPoint(int x, int y)
{
	static const double Left = Height / (double) TriangleWidth;

	if(x < Width * 0.2)
		return(y + Left * x > Height);
	else if(x > Width * 0.8)
		return(y + Left * (x - (Width - TriangleWidth)) < Height);

	return(true);
}

template<const int PWidth, const int PHeight, const int TriangleWidth>
inline bool IntersectWithRectanglePart(int rx, int ry, int rwidth, int rheight,
									   int px, int py)
{
	// Please keep in mind that this item somehow intersects width the 
	// sourounding rectangle of this instruction-item...
	// If this is not assured the function's result is incorrect!

	int cl = std::min(px, rx); // circumscribed rectangle's left-value
	int cr = std::max(px + PWidth, rx + rwidth); // ... right-value

	// just to remind you: the intersection rectangle's width can be
	// calculated like this: - ((cr - cl) - (item.width + width))
	// (which is the additive inverse of sourounding rect - sum of 
	// both rect's widths)

	// If the intersection rectangle's width is greater than the width of the 
	// triangle that is part of the sourounding rect but not of this item
	// (due to the non-rectangular form of this item) the rectangle provided
	// as argument of this function intersects with this item
	if(- ((cr - cl) - (PWidth + rwidth)) > TriangleWidth)
		return(true);


	// Else we have to check whether one of the rectangle's corner points is
	// within the item
	rx -= px;
	ry -= py;
	return(IntersectsWithPoint<PWidth, PHeight, TriangleWidth>
		       (rx, ry) || 
		   IntersectsWithPoint<PWidth, PHeight, TriangleWidth>
		       (rx + rwidth, ry) ||
		   IntersectsWithPoint<PWidth, PHeight, TriangleWidth>
		       (rx + rwidth, ry + rheight) || 
		   IntersectsWithPoint<PWidth, PHeight, TriangleWidth>
		       (rx, ry + rheight));
}

// This function does not assure that there is a intersection of the
// sourounding rectangles... so it does a bit more, but hardly the same
template<const int PWidth, const int PHeight, const int TriangleWidth>
inline bool IntersectWithRectangle(int rx, int ry, int rwidth, int rheight,
								   int px, int py)
{
	int cl = std::min(px, rx); // circumscribed rectangle's left-value
	int cb = std::min(py, ry); // ... and the bottom-value
	int ct = std::max(py + PHeight, ry + rheight); // ... top-value
	int cr = std::max(px + PWidth, rx + rwidth); // ... right-value

	if(!((rwidth + PWidth > cr - cl && rheight + PHeight > ct - cb)))
		return(false);
	
	if(- ((cr - cl) - (PWidth + rwidth)) > TriangleWidth)
		return(true);
	
	rx -= px;
	ry -= py;
	return(IntersectsWithPoint<PWidth, PHeight, TriangleWidth>
		       (rx, ry) || 
		   IntersectsWithPoint<PWidth, PHeight, TriangleWidth>
		       (rx + rwidth, ry) ||
		   IntersectsWithPoint<PWidth, PHeight, TriangleWidth>
		       (rx + rwidth, ry + rheight) || 
		   IntersectsWithPoint<PWidth, PHeight, TriangleWidth>
		       (rx, ry + rheight));
}

}

namespace Rhombus
{

// be aware that x and y must be relative numbers
template<const int Width, const int Height>
inline bool IntersectsWithPoint(int x, int y)
{
	static const double mTopLeft = ((double) Height / 2) / ((double) Width / 2);
	static const double mTopRight = - ((double) Height / 2) / ((double) Width / 2);
	static const double mBottomLeft = - ((double) Height / 2) / ((double) Width / 2);

	if(x > Width / 2) {
		x -= Width / 2; // we have to move it so that the graph-formulas are correct
		if(y > Height / 2)
			// return((y - Height / 2 - (mTopRight * (x - Width / 2) + Height / 2)) < 0);
			return((y - Height - mTopRight * x) < 0);
		else
			return((y - (mTopLeft * x)) > 0);// + Height / 2
	} else {
		if(y > Height / 2)
			return((y - mTopLeft * x) < Height / 2); // y - Height / 2 - mTopLeft * x < 0
		else
			return((y - (mBottomLeft * x + Height / 2)) > 0);
	}
}

template<const int FWidth, const int FHeight>
inline bool IntersectWithRectangle(int rx, int ry, int rwidth, int rheight,
								   int fx, int fy)
{
	int cl = std::min(fx, rx); // circumscribed rectangle's left-value
	int cb = std::min(fy, ry); // ... and the bottom-value
	int ct = std::max(fy + FHeight, ry + rheight); // ... top-value
	int cr = std::max(fx + FWidth, rx + rwidth); // ... right-value

	// if the intersection rect is greater than half the width or half the height
	// of the sourounding rect, there must be an intersection!
	if(- ((cr - cl) - (FWidth + rwidth)) > FWidth / 2 ||
		- ((ct - cb) - (FHeight + rheight)) > FHeight / 2)
		return(true);
	
	rx -= fx;
	ry -= fy;
	return(IntersectsWithPoint<FWidth, FHeight>(rx, ry) || 
		   IntersectsWithPoint<FWidth, FHeight>(rx + rwidth, ry) ||
		   IntersectsWithPoint<FWidth, FHeight>(rx + rwidth, ry + rheight) || 
		   IntersectsWithPoint<FWidth, FHeight>(rx, ry + rheight));
}

}

class Direction
{
public:
	enum DirectionNames { Left = 1, Right = 2, NoHorizontal = 4, Up = 8, Down = 16, NoVertical = 32,
		Horizontal = Left | Right | NoHorizontal, Vertical = Up | Down | NoVertical };

	Direction(const int startX, const int startY, const int endX, const int endY)
		: direction(0)
	{
		if(startX == endX)
			direction = NoHorizontal;
		else if(startX < endX)
			direction |= Right;
		else
			direction |= Left;

		if(startY < endY)
			direction |= Down;
		else
			direction |= Up;
	}

	Direction(int direction)
		: direction(direction)
	{
	}

	template<int Search>
	inline bool Is() const
		{ return((direction & Search) != 0); };

	bool IsRight() const
		{ return(Is<Right>()); }
	bool IsLeft() const
		{ return(Is<Left>()); };
	bool IsUp() const
		{ return(Is<Up>()); };
	bool IsDown() const
		{ return(Is<Down>()); };
	bool IsHorizontal() const
		{ return(Is<Horizontal>()); };
	bool IsVerical() const
		{ return(Is<Vertical>()); };

	template<int Filter>
	Direction GetOnly() const
		{ return(Direction(direction & Filter)); };
	Direction GetOnlyHorizontal() const
		{ return(GetOnly<Horizontal>()); };
	Direction GetOnlyVertical() const
		{ return(GetOnly<Vertical>()); };

	bool operator ==(const Direction &other) const
		{ return(direction == other.direction); };

	Direction operator -() const
	{
		int newDirection = direction;
		if(newDirection & Right)
			newDirection = (newDirection & ~Right) | Left;
		else if(newDirection & Left)
			newDirection = (newDirection & ~Left) | Right;
		if(newDirection & Up)
			newDirection = (newDirection & ~Up) | Down;
		else if(newDirection & Down)
			newDirection = (newDirection & ~Down) | Up;
		return(Direction(newDirection));
	}

	
private:
	int direction;
};

#endif /* GEOMETRY_H */
