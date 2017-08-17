#pragma once
#include "Math.h"
#include <vector>
#include <GLM\glm.hpp>
#include "BoundingBox.h"
//class which has all methods needed for collision detection used in the game, the standard paramater for an object that is going to be tested is BoundingBox found in the GameEngine
namespace GameEngine {
	class CollisionDetection
	{
	public:
		CollisionDetection();
		~CollisionDetection();
		bool CheckRectangleIntersect(glm::vec4* _rect1, glm::vec4* _rect2);
		void correctPosition(glm::vec4* _rect1, glm::vec4* _rect2);
		void correctPosition(BoundingBox* bb1, BoundingBox* bb2);
		bool isCheckRequired(BoundingBox* bb1, BoundingBox* bb2);
		glm::vec2 getLineIntersect(glm::vec2 _p1, glm::vec2 _p2, glm::vec2 _p3, glm::vec2 _p4);
		bool lineRectCollision(glm::vec2 _p1, glm::vec2 _p2, BoundingBox* _boundingBox);
		glm::vec2 getLineRectCollision(glm::vec2 _p1, glm::vec2 _p2, BoundingBox* _boundingBox);
		float getDistance(glm::vec2 p1, glm::vec2 p2);
		bool isBetween(float _1, float _2, float _3);
		bool doLinesColide(glm::vec2 _p1, glm::vec2 _p2, glm::vec2 _p3, glm::vec2 _p4);
		float getAngle(glm::vec2 _p1, glm::vec2 _p2);
		float getDistBetween(float _1, float _2);
	private:
		bool test1(glm::vec4* _rect1, glm::vec4* _rect2);
		bool test2(glm::vec4* _rect1, glm::vec4* _rect2);
		bool test3(glm::vec4* _rect1, glm::vec4* _rect2);
		bool test4(glm::vec4* _rect1, glm::vec4* _rect2);
		bool test5(glm::vec4* _rect1, glm::vec4* _rect2);
		bool test6(glm::vec4* _rect1, glm::vec4* _rect2);
		bool test7(glm::vec4* _rect1, glm::vec4* _rect2);
		bool test8(glm::vec4* _rect1, glm::vec4* _rect2);
		int getSide(glm::vec4* _rect1, glm::vec4* _rect2);
	};
}