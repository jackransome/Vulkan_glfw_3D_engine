#include "SpriteSheet.h"


SpriteSheet::SpriteSheet()
{
}
SpriteSheet::~SpriteSheet()
{
}  
void SpriteSheet::init(Graphics* _graphicsPointer, glm::vec2 _texturePosition, int _width, int _height, int _scale, int _numberOfFrames, int _frameChangeInterval)
{
	graphicsPointer = _graphicsPointer;
	texturePosition = _texturePosition;
	width = _width;
	height = _height;
	scale = _scale;
	numberOfFrames = _numberOfFrames;
	frameChangeInterval = _frameChangeInterval;
}
void SpriteSheet::run()
{
	frameChangeCounter++;
	if (frameChangeCounter == frameChangeInterval) {
		frameChangeCounter = 0;
		frame++;
		if (frame == numberOfFrames) {
			frame = 0;
		}
	}
}
void SpriteSheet::draw(float _x, float _y)
{
	glm::vec2 pos = glm::vec2(texturePosition.x + frame * width, texturePosition.y);
	graphicsPointer->drawFlatImage(_x, _y, width * scale, height * scale, texturePosition + glm::vec2(frame * width, 0), texturePosition + glm::vec2(width, height) + glm::vec2(frame * width, 0));
}
void SpriteSheet::drawFlipped(float _x, float _y)
{
	glm::vec2 pos = glm::vec2(texturePosition.x + frame * width, texturePosition.y);
	graphicsPointer->drawFlatImage(_x, _y, width * scale, height * scale, texturePosition + glm::vec2(frame * width, 0) + glm::vec2(width, 0), texturePosition + glm::vec2(0, height) + glm::vec2(frame * width, 0));
}
void SpriteSheet::reset()
{
	frame = 0;
	frameChangeCounter = 0;
}
void SpriteSheet::setFrame(int _frame) {
	frame = _frame;
}