#pragma once
#include "BoundingBox.h"
#include "Input.h"
#include "SpriteSheet.h"
enum LastDirection {up, down, left, right};
class Player {
public:
	Player(int _x, int _y, Input* _inputPointer, Graphics* _graphicsPointer);
	~Player();
	BoundingBox getBoundingBox();
	BoundingBox* getBoundingBoxPointer();
	void handleInput();
	void updatePosition();
	void draw();
private:
	SpriteSheet walkCycleHorizontal;
	SpriteSheet walkCycleVerticleUp;
	SpriteSheet walkCycleVerticleDown;
	SpriteSheet standingHorizontal;
	SpriteSheet standingVerticleUp;
	SpriteSheet standingVerticleDown;
	Input* inputPointer;
	Graphics* graphicsPointer;
	BoundingBox boundingBox;
	float velocity;
	LastDirection lastDirection;
};