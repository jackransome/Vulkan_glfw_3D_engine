#pragma once
#include "BoundingBox.h"
#include "Graphics.h"
class BloodParticle {
public:
	BloodParticle(int _x, int _y, float _xv, float _yv, float _w, float _h, Graphics* _graphicsPointer);
	~BloodParticle();
	BoundingBox getBoundingBox();
	BoundingBox* getBoundingBoxPointer();
	void updatePosition();
	void draw();
private:
	Graphics* graphicsPointer;
	BoundingBox boundingBox;
};