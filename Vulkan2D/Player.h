#pragma once
#include "BoundingBox.h"
#include "Input.h"
#include "SpriteSheet.h"

class Player {
public:
	Player(int _x, int _y, Input* _inputPointer, Graphics* _graphicsPointer);
	~Player();
	BoundingBox getBoundingBox();
	void handleInput();
	void updatePosition();
	void draw();
private:
	SpriteSheet walkCycle;
	Input* inputPointer;
	Graphics* graphicsPointer;
	BoundingBox boundingBox;
	float velocity;
};