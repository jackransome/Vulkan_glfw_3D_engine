#include  "CollisionDetection.h"
#include <iostream>
struct collisionResult { collisionResult() : left(false), right(false), top(false), bottom(false) {} bool left; bool right; bool top; bool bottom; };
namespace GameEngine {
	CollisionDetection::CollisionDetection() {}
	CollisionDetection::~CollisionDetection() {}
	bool CollisionDetection::CheckRectangleIntersect(glm::vec4* _rect1, glm::vec4* _rect2)
	{
		if (_rect1->x + _rect1->z > _rect2->x && _rect1->x < _rect2->x + _rect2->z && _rect1->y + _rect1->w > _rect2->y && _rect1->y < _rect2->y + _rect2->w)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	bool CollisionDetection::test1(glm::vec4* _rect1, glm::vec4* _rect2) {
		if (_rect1->x + _rect1->z >= _rect2->x) {
			return true;
		}
		return false;
	}

	bool CollisionDetection::test2(glm::vec4* _rect1, glm::vec4* _rect2) {
		if (_rect1->x >= _rect2->x) {
			return true;
		}
		return false;
	}

	bool CollisionDetection::test3(glm::vec4* _rect1, glm::vec4* _rect2) {
		if (_rect1->x >= _rect2->x + _rect2->z) {
			return true;
		}
		return false;
	}

	bool CollisionDetection::test4(glm::vec4* _rect1, glm::vec4* _rect2) {
		if (_rect1->x + _rect1->z >= _rect2->x + _rect2->z) {
			return true;
		}
		return false;
	}

	bool CollisionDetection::test5(glm::vec4* _rect1, glm::vec4* _rect2) {
		if (_rect1->y >= _rect2->y) {
			return true;
		}
		return false;
	}

	bool CollisionDetection::test6(glm::vec4* _rect1, glm::vec4* _rect2) {
		if (_rect1->y + _rect1->w >= _rect2->y) {
			return true;
		}
		return false;
	}

	bool CollisionDetection::test7(glm::vec4* _rect1, glm::vec4* _rect2) {
		if (_rect1->y + _rect1->w >= _rect2->y + _rect2->w) {
			return true;
		}
		return false;
	}
	bool CollisionDetection::test8(glm::vec4* _rect1, glm::vec4* _rect2) {
		if (_rect1->y >= _rect2->y + _rect2->w) {
			return true;
		}
		return false;
	}

	int CollisionDetection::getSide(glm::vec4* _rect1, glm::vec4* _rect2) {
		int result = -1;
		int testvar = 0;
		if (test1(_rect1, _rect2))
		{
			testvar = testvar + 1;
		}
		if (test2(_rect1, _rect2)) {
			testvar = testvar + 2;
		}
		if (test3(_rect1, _rect2)) {
			testvar = testvar + 4;
		}
		if (test4(_rect1, _rect2)) {
			testvar = testvar + 8;
		}
		if (test5(_rect1, _rect2)) {
			testvar = testvar + 16;
		}
		if (test6(_rect1, _rect2)) {
			testvar = testvar + 32;
		}
		if (test7(_rect1, _rect2)) {
			testvar = testvar + 64;
		}
		if (test8(_rect1, _rect2)) {
			testvar = testvar + 128;
		}
		switch (testvar)
		{
			//left = 1, bottom = 2, right = 3, top = 4
			// 0,1,1,1,1,0,0,1=121
		case 121:
			result = 2;
			break;
			//0,0,1,0,1,0,0,1=73
		case 73:
			result = 4;
			break;
			//0,1,1,0,1,0,1,1
		case 107:
			result = 3;
			break;
			//0,1,1,0,0,0,0,1
		case 97:
			result = 1;
			break;
			//0,0,1,0,0,0,1,1
		case 35:
			result = 4;
			break;
			//0,0,1,1,0,0,0,1
		case 49:
			result = 1;
			break;
			//0,0,1,1,1,0,1,1 = 59
		case 59:
			result = 3;
			break;
			//0,1,1,1,0,0,1,1 = 115
		case 115:
			result = 2;
			break;
			//top left corner
			//0,0,1,0,0,0,0,1
		case 33:
			if ((_rect1->x + _rect1->z) - _rect2->x > (_rect1->y + _rect1->w) - _rect2->y) {
				result = 4;
			}
			else {
				result = 1;
			}
			break;

			//top right corner
			//0,0,1,0,1,0,1,1 = 43
		case 43:
			if (_rect2->x + _rect2->z - _rect1->x > (_rect1->y + _rect1->w) - _rect2->y) {
				result = 4;
			}
			else {
				result = 3;
			}
			break;

			//bottom left corner
			//0,1,1,1,0,0,0,1 = 113
		case 113:
			if (_rect1->x + _rect1->z - _rect2->x > _rect2->y + _rect2->w - _rect1->y) {
				result = 2;
			}
			else
			{
				result = 1;
			}
			break;

			//bottom right corner
			//0,1,1,1,1,0,1,1 = 123
		case 123:
			if (_rect2->x + _rect2->z - _rect1->x > _rect2->y + _rect2->w - _rect1->y) {
				result = 2;
			}
			else {
				result = 3;
			}
			break;
		}

		return result;
	}
	//this function corrects the position of a rectangle if it is colliding with another rectangle
	void CollisionDetection::correctPosition(glm::vec4* _rect1, glm::vec4* _rect2) {
		//left = 1, bottom = 2, right = 3, top = 4
		if (CheckRectangleIntersect(_rect1, _rect2))
		{
			switch (getSide(_rect1, _rect2))
			{
			case 1:
				_rect1->x = _rect2->x - _rect1->z;
				break;
			case 3:
				_rect1->x = (_rect2->x + _rect2->z);
				break;
			case 2:
				_rect1->y = (_rect2->y + _rect2->w);
				break;
			case 4:
				_rect1->y = (_rect2->y - _rect1->w);
				break;
			}
		}
	}
	void CollisionDetection::correctPosition(BoundingBox* bb1, BoundingBox* bb2) {
		collisionResult result;
		if (bb1->x + bb1->w > bb2->x && bb1->x < bb2->x + bb2->w) {
			if (bb1->y < bb2->y && bb1->y + bb1->h + bb1->yv > bb2->y) {
				result.bottom = true;
			}
			else if (bb1->y + bb1->h > bb2->y + bb2->h && bb1->y + bb1->yv < bb2->y + bb2->h) {
				result.top = true;
			}
		}
		if (bb1->y + bb1->h > bb2->y && bb1->y < bb2->y + bb2->h) {
			if (bb1->x < bb2->x && bb1->x + bb1->w + bb1->xv > bb2->x) {
				result.left = true;
			}
			else if (bb1->x + bb1->w > bb2->x + bb2->w && bb1->x + bb1->xv < bb2->x + bb2->w) {
				result.right = true;
			}
		}
		if (result.bottom) {
			if (result.left) {
				if (bb1->y + bb1->h + bb1->yv - bb2->y > (bb1->x + bb1->w + bb1->xv) - bb2->x) {
					bb1->y = bb2->y - bb1->h;
					if (bb1->yv > 0) { bb1->yv = 0; }
				}
				else {
					bb1->x = bb2->x - bb1->w;
					if (bb1->xv > 0) { bb1->xv = 0; }
				}
			}
			else if (result.right) {
				if (bb1->y + bb1->h + bb1->yv - bb2->y > bb1->x + bb1->w + bb1->xv - bb2->x) {
					bb1->y = bb2->y - bb1->h;
					if (bb1->yv > 0) { bb1->yv = 0; }
				}
				else {
					bb1->x = bb2->x + bb2->w;
					if (bb1->xv < 0) { bb1->xv = 0; }
				}
			}
			else {
				bb1->y = bb2->y - bb1->h;
				bb1->yv = 0;
			}
		}
		else if (result.top) {
			if (result.left) {
				if (bb2->y + bb2->h - (bb1->y + bb1->yv) > bb1->x + bb1->w + bb1->xv - bb2->x) {
					bb1->x = bb2->x - bb1->w;
					if (bb1->xv > 0) { bb1->xv = 0; }
				}
				else {
					bb1->y = bb2->y + bb2->h;
					if (bb1->yv < 0) { bb1->yv = 0; }
					bb1->onGround = true;
					//fullyOnGroundTest
					if (bb1->x > bb2->x && bb2->x + bb2->w > bb1->x + bb1->w) {
						bb1->fullyOnGround = true;
					}
				}
				
			}
			else if (result.right) {
				if (bb1->y + bb1->yv - bb2->y + bb2->h > bb1->x + bb1->w + bb1->xv - bb2->x) {
					bb1->x = bb2->x + bb2->w;
					bb1->xv = 0;
					if (bb1->xv < 0) { bb1->xv = 0; }
				}
				else {

					bb1->y = bb2->y + bb2->h;
					if (bb1->yv < 0) { bb1->yv = 0; }
					bb1->onGround = true;
					//fullyOnGroundTest
					if (bb1->x > bb2->x && bb2->x + bb2->w > bb1->x + bb1->w) {
						bb1->fullyOnGround = true;
					}
				}
			}
			else {
				bb1->y = bb2->y + bb2->h;
				bb1->yv = 0;
				bb1->onGround = true;
				if (bb1->x > bb2->x && bb2->x + bb2->w > bb1->x + bb1->w) {
					bb1->fullyOnGround = true;
				}
			}
		}
		else if (result.left) {
			bb1->x = bb2->x - bb1->w;
			bb1->xv = 0;
		}
		else if (result.right) {
			bb1->x = bb2->x + bb2->w;
			bb1->xv = 0;
		}
	}
	bool CollisionDetection::isCheckRequired(BoundingBox* bb1, BoundingBox* bb2) {
		bool check;
		if (bb1->xv > 0 && bb1->x < bb2->x) {
			check = true;
		} else if (bb1->xv < 0 && bb1->x + bb1->w > bb2->x + bb2->w) {
			check = true;
		} else if (bb1->yv < 0 && bb1->y + bb1->h > bb2->y + bb2->h) {
			check = true;
		} else 	if (bb1->yv > 0 && bb1->y < bb2->y) {
			check = true;
		} else { check = false; }
		return check;
	}
	glm::vec2 CollisionDetection::getLineIntersect(glm::vec2 _p1, glm::vec2 _p2, glm::vec2 _p3, glm::vec2 _p4) {
		if (((_p1.x - _p2.x)*(_p3.y - _p4.y) - (_p1.y - _p2.y)*(_p3.x - _p4.x)) == 0) {
			return glm::vec2(NULL, NULL);
		}
		else {
			return glm::vec2(((_p1.x*_p2.y - _p1.y*_p2.x)*(_p3.x - _p4.x) - (_p1.x - _p2.x)*(_p3.x*_p4.y - _p3.y*_p4.x)) / ((_p1.x - _p2.x)*(_p3.y - _p4.y) - (_p1.y - _p2.y)*(_p3.x - _p4.x)), ((_p1.x*_p2.y - _p1.y*_p2.x)*(_p3.y - _p4.y) - (_p1.y - _p2.y)*(_p3.x*_p4.y - _p3.y*_p4.x)) / ((_p1.x - _p2.x)*(_p3.y - _p4.y) - (_p1.y - _p2.y)*(_p3.x - _p4.x)));
		}
	}
	float CollisionDetection::getDistance(glm::vec2 _p1, glm::vec2 _p2)
	{
		return sqrt(pow(_p2.x - _p1.x, 2) + pow(_p2.y - _p1.y,2));
	}
	bool CollisionDetection::lineRectCollision(glm::vec2 _p1, glm::vec2 _p2, BoundingBox* _boundingBox) {
		glm::vec2 left = getLineIntersect(_p1, _p2, glm::vec2(_boundingBox->x, _boundingBox->y), glm::vec2(_boundingBox->x, _boundingBox->y + _boundingBox->h));
		glm::vec2 right = getLineIntersect(_p1, _p2, glm::vec2(_boundingBox->x + _boundingBox->w, _boundingBox->y), glm::vec2(_boundingBox->x + _boundingBox->w, _boundingBox->y + _boundingBox->h));
		glm::vec2 bottom = getLineIntersect(_p1, _p2, glm::vec2(_boundingBox->x, _boundingBox->y), glm::vec2(_boundingBox->x + _boundingBox->w, _boundingBox->y));
		glm::vec2 top = getLineIntersect(_p1, _p2, glm::vec2(_boundingBox->x, _boundingBox->y + _boundingBox->w), glm::vec2(_boundingBox->x + _boundingBox->w, _boundingBox->y + _boundingBox->w));
		if (isBetween(_boundingBox->y, _boundingBox->h + _boundingBox->y, left.y) && isBetween(_p1.x, _p2.x, left.x)) {
			return true;
		}
		else if (isBetween(_boundingBox->y, _boundingBox->h + _boundingBox->y, right.y) && isBetween(_p1.x, _p2.x, right.x)) {
			return true;
		}
		else if (isBetween(_boundingBox->x, _boundingBox->w + _boundingBox->x, top.x) && isBetween(_p1.y, _p2.y, top.y)) {
			return true;
		}
		else if (isBetween(_boundingBox->x, _boundingBox->w + _boundingBox->x, bottom.x) && isBetween(_p1.y, _p2.y, bottom.y)) {
			return true;
		}
		else { return false; }
	}
	glm::vec2 CollisionDetection::getLineRectCollision(glm::vec2 _p1, glm::vec2 _p2, BoundingBox* _boundingBox) {
		int distance = 1000000000;
		glm::vec2 final;
		final = glm::vec2(NULL, NULL);
		glm::vec2 left = getLineIntersect(_p1, _p2, glm::vec2(_boundingBox->x, _boundingBox->y), glm::vec2(_boundingBox->x, _boundingBox->y + _boundingBox->h));
		glm::vec2 right = getLineIntersect(_p1, _p2, glm::vec2(_boundingBox->x+_boundingBox->w, _boundingBox->y), glm::vec2(_boundingBox->x + _boundingBox->w, _boundingBox->y + _boundingBox->h));
		glm::vec2 bottom = getLineIntersect(_p1, _p2, glm::vec2(_boundingBox->x, _boundingBox->y), glm::vec2(_boundingBox->x + _boundingBox->w, _boundingBox->y));
		glm::vec2 top = getLineIntersect(_p1, _p2, glm::vec2(_boundingBox->x, _boundingBox->y + _boundingBox->w), glm::vec2(_boundingBox->x + _boundingBox->w, _boundingBox->y + _boundingBox->w));
		if (isBetween(_boundingBox->y, _boundingBox->h + _boundingBox->y, left.y) && isBetween(_p1.x, _p2.x, left.x)) {
			if (getDistance(left, _p1) < distance) {
				final = left;
				distance = getDistance(left, _p1);
			}
		}
		if (isBetween(_boundingBox->y, _boundingBox->h + _boundingBox->y, right.y) && isBetween(_p1.x, _p2.x, right.x)) {
			if (getDistance(right, _p2) < distance) {
				final = right;
				distance = getDistance(right, _p1);
			}
		}
		if (isBetween(_boundingBox->x, _boundingBox->w + _boundingBox->x, top.x) && isBetween(_p1.y, _p2.y, top.y)) {
			if (getDistance(top, _p2) < distance) {
				final = top;
				distance = getDistance(top, _p1);
			}
		}
		if (isBetween(_boundingBox->x, _boundingBox->w + _boundingBox->x, bottom.x) && isBetween(_p1.y, _p2.y, bottom.y)) {
			if (getDistance(bottom, _p1) < distance) {
				final = bottom;
				distance = getDistance(bottom, _p1);
			}
		}
		return final;
	}
	bool CollisionDetection::isBetween(float _1, float _2, float _3)
	{
		if (_3 == _2 || _3 == _1){return true;}else{
			if (_1 > _2){
			if (_3 > _2&&_3 < _1){	return true;}else{return false;}}
			else if (_3 < _2 && _3 > _1){return true;}else{return false;}
		}
	}

	bool CollisionDetection::doLinesColide(glm::vec2 _p1, glm::vec2 _p2, glm::vec2 _p3, glm::vec2 _p4)
	{
		glm::vec2 intersect = getLineIntersect(_p1, _p2, _p3, _p4);
		if (isBetween(_p1.y, _p2.y, intersect.y) && isBetween(_p3.y, _p4.y, intersect.y) && isBetween(_p1.x, _p2.x, intersect.x) && isBetween(_p3.x, _p4.x, intersect.x)) {
			return true;
		}
		else
		{
			return false;
		}
	}

	float CollisionDetection::getAngle(glm::vec2 _p1, glm::vec2 _p2)
	{
		if (_p1.x < _p2.x && _p1.y < _p2.y)
		{//top right
			return atan((_p2.x - _p1.x) / (_p2.y - _p1.y));
		}
		else 	if (_p1.x < _p2.x && _p1.y > _p2.y)
		{//bottom right
			return 3.1415 - atan((_p2.x - _p1.x) / (_p1.y - _p2.y));
		}
		else if (_p1.x > _p2.x && _p1.y > _p2.y)
		{// bottom left
			return atan((_p1.x - _p2.x) / (_p1.y - _p2.y)) + 3.1415;
		}
		else if (_p1.x > _p2.x && _p1.y < _p2.y)
		{// top left
			return 2 * 3.1415 - atan((_p1.x - _p2.x) / (_p2.y - _p1.y));
		}
	}
	float CollisionDetection::getDistBetween(float _1, float _2)
	{
		float test = abs(abs(_1) - abs(_2));
		return test;
	}
}
