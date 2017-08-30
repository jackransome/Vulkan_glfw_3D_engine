#include "Player.h"

Player::Player(int _x, int _y, Input* _inputPointer, Graphics* _graphicsPointer)
{
	boundingBox.x = _x;
	boundingBox.y = _y;
	inputPointer = _inputPointer;
	graphicsPointer = _graphicsPointer;
	walkCycleHorizontal.init(_graphicsPointer, glm::vec2(20, 225), 13, 25, 6, 8, 5);
	walkCycleVerticleUp.init(_graphicsPointer, glm::vec2(20, 275), 13, 25, 6, 8, 5);
	walkCycleVerticleDown.init(_graphicsPointer, glm::vec2(20, 250), 13, 25, 6, 8, 5);
	standingHorizontal.init(_graphicsPointer, glm::vec2(124, 225), 13, 25, 6, 1, 5);
	standingVerticleUp.init(_graphicsPointer, glm::vec2(124, 275), 13, 25, 6, 1, 5);
	standingVerticleDown.init(_graphicsPointer, glm::vec2(124, 250), 13, 25, 6, 1, 5);
	velocity = 8;
	boundingBox.w = 50;
	boundingBox.h = 50;
}

Player::~Player()
{
}

BoundingBox Player::getBoundingBox()
{
	return boundingBox;
}

BoundingBox* Player::getBoundingBoxPointer()
{
	return &boundingBox;
}

void Player::handleInput()
{
	if (inputPointer->keys.a) {
		if (!inputPointer->keys.d) {
			boundingBox.xv = -velocity;
			lastDirection = left;
		}
		else {
			boundingBox.xv = 0;
		}
	}
	else if (inputPointer->keys.d) {
		boundingBox.xv = velocity;
		lastDirection = right;
	}
	else {
		boundingBox.xv = 0;
	}
	if (inputPointer->keys.w) {
		if (boundingBox.onGround) {
			boundingBox.yv = -3 * velocity;
			lastDirection = up;
		}
		else {
			//boundingBox.yv = 0;
		}
	}
	else if (inputPointer->keys.s) {
		//boundingBox.yv = velocity;
		lastDirection = down;
	}
	else {
		//boundingBox.yv = 0;
	}

	if (inputPointer->keys.f) {
		boundingBox.x = 0;
		boundingBox.y = 0;
	}
	boundingBox.yv += 0.5;
	boundingBox.onGround = false;
}

void Player::updatePosition()
{
	boundingBox.x += boundingBox.xv;
	boundingBox.y += boundingBox.yv;
}

void Player::draw()
{
	graphicsPointer->drawRect(boundingBox.x + 5, boundingBox.y + 5, boundingBox.w - 10, boundingBox.h - 10, 0.9, 0.1, 0.1, 1);
	graphicsPointer->drawRect(boundingBox.x, boundingBox.y, boundingBox.w, boundingBox.h, 0.0, 0.0, 0.0, 1);
}