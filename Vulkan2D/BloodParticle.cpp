#include "BloodParticle.h"

BloodParticle::BloodParticle(int _x, int _y, float _xv, float _yv, float _w, float _h, Graphics * _graphicsPointer){
	boundingBox.x = _x;
	boundingBox.y = _y;
	boundingBox.xv = _xv;
	boundingBox.yv = _yv;
	boundingBox.w = _w;
	boundingBox.h = _h;
}

BloodParticle::~BloodParticle() {
}

BoundingBox BloodParticle::getBoundingBox() {
	return boundingBox;
}

BoundingBox* BloodParticle::getBoundingBoxPointer()
{
	return &boundingBox;
}

void BloodParticle::updatePosition() {
	boundingBox.y += 0.7;
	boundingBox.x += boundingBox.xv;
	boundingBox.y += boundingBox.yv;
}

void BloodParticle::draw() {
	graphicsPointer->drawRect(boundingBox.x, boundingBox.y, boundingBox.w, boundingBox.h, 0.7, 0.05, 0.05, 1);
}
