#include "Player.h"

Player::Player(int _x, int _y, Input* _inputPointer, Graphics* _graphicsPointer)
{
	boundingBox.x = _x;
	boundingBox.y = _y;
	inputPointer = _inputPointer;
	graphicsPointer = _graphicsPointer;
	walkCycle.init(_graphicsPointer, glm::vec2(20, 225), 13, 25, 7, 8, 7);
	velocity = 8;
}

Player::~Player()
{
}

BoundingBox Player::getBoundingBox()
{
	return boundingBox;
}

void Player::handleInput()
{
	if (inputPointer->keys.a) {
		if (!inputPointer->keys.d) {
			boundingBox.xv = -velocity;
		}
		else {
			boundingBox.xv = 0;
		}
	}
	else if (inputPointer->keys.d) {
		boundingBox.xv = velocity;
	}
	else {
		boundingBox.xv = 0;
	}
	if (inputPointer->keys.w) {
		if (!inputPointer->keys.s) {
			boundingBox.yv = -velocity;
		}
		else {
			boundingBox.yv = 0;
		}
	}
	else if (inputPointer->keys.s) {
		boundingBox.yv = velocity;
	}
	else {
		boundingBox.yv = 0;
	}
}

void Player::updatePosition()
{
	boundingBox.x += boundingBox.xv;
	boundingBox.y += boundingBox.yv;
}

void Player::draw()
{
	if (boundingBox.xv > 0) {
		walkCycle.run();
		walkCycle.draw(boundingBox.x, boundingBox.y);
	} else 	if (boundingBox.xv < 0) {
		walkCycle.run();
		walkCycle.drawFlipped(boundingBox.x, boundingBox.y);
	}
}