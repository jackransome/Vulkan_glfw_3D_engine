#include "Platform.h"

Platform::Platform(int _x, int _y, int _width, int _height, Graphics* _graphicsPointer)
{
	boundingBox.x = _x;
	boundingBox.y = _y;
	boundingBox.w = _width;
	boundingBox.h = _height;
	graphicsPointer = _graphicsPointer;
}

Platform::~Platform()
{
}

BoundingBox Platform::getBoundingBox()
{
	return boundingBox;
}

BoundingBox* Platform::getBoundingBoxPointer()
{
	return &boundingBox;
}

void Platform::draw()
{
	graphicsPointer->drawRect(boundingBox.x, boundingBox.y, boundingBox.w, boundingBox.h, 0.2, 0.2, 0.2, 1);
}