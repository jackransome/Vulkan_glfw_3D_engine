#pragma once
#include "BoundingBox.h"
#include "SpriteSheet.h"
class Platform {
public:
	Platform(int _x, int _y, int _width, int _height, Graphics* _graphicsPointer);
	~Platform();
	BoundingBox getBoundingBox();
	BoundingBox* getBoundingBoxPointer();
	void draw();
private:
	Graphics* graphicsPointer;
	BoundingBox boundingBox;
};