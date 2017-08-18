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
		if (!inputPointer->keys.s) {
			boundingBox.yv = -velocity;
			lastDirection = up;
		}
		else {
			boundingBox.yv = 0;
		}
	}
	else if (inputPointer->keys.s) {
		boundingBox.yv = velocity;
		lastDirection = down;
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
		walkCycleHorizontal.run();
		walkCycleHorizontal.draw(boundingBox.x, boundingBox.y);
	} else 	if (boundingBox.xv < 0) {
		walkCycleHorizontal.run();
		walkCycleHorizontal.drawFlipped(boundingBox.x, boundingBox.y);
	}
	else {
		walkCycleHorizontal.setFrame(0);
		if (boundingBox.yv > 0) {
			walkCycleVerticleDown.run();
			walkCycleVerticleDown.draw(boundingBox.x, boundingBox.y);
			walkCycleHorizontal.setFrame(0);
			walkCycleVerticleUp.setFrame(0);
		}
		else if (boundingBox.yv < 0) {
			walkCycleHorizontal.setFrame(0);
			walkCycleVerticleDown.setFrame(0);
			walkCycleVerticleUp.run();
			walkCycleVerticleUp.draw(boundingBox.x, boundingBox.y);
		}
		else {
			walkCycleHorizontal.setFrame(0);
			walkCycleVerticleUp.setFrame(0);
			walkCycleVerticleDown.setFrame(0);
			if (lastDirection == right) {
				standingHorizontal.draw(boundingBox.x, boundingBox.y);
			}
			else if (lastDirection == left) {
				standingHorizontal.drawFlipped(boundingBox.x, boundingBox.y);
			}
			else if (lastDirection == up) {
				standingVerticleUp.draw(boundingBox.x, boundingBox.y);
			}
			else if (lastDirection == down) {
				standingVerticleDown.draw(boundingBox.x, boundingBox.y);
			}
		}
	}
}