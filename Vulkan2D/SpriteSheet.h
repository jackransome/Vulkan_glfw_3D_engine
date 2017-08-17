#pragma once
#include "Graphics.h"
class SpriteSheet
{
public:
	SpriteSheet();
	~SpriteSheet();
	/// <summary>This method initialises a spritesheet. </summary>
	/// <param name="_graphicsPointer"> Pointer to a Graphics class. </param>
	/// <param name="_texturePosition"> Position of the upper left part of the loaded texture where the spritesheet starts. </param>
	/// <param name="_width"> The width of one frame of the spritesheet. </param>
	/// <param name="_height"> The height of one frame of the spritesheet. </param>
	/// <param name="_scale"> Scale to draw the sprite with. </param>
	/// <param name="_numberOfFrames"> Number of frames the spriteSheet has. </param>
	/// <param name="_frameChangeInterval"> How many frames before the run() function changes to the next frame. </param>
	void init(Graphics* _graphicsPointer, glm::vec2 _texturePosition, int _width, int _height, int _scale, int _numberOfFrames, int _frameChangeInterval);
	void run();
	void draw(float _x, float _y);
	void drawFlipped(float _x, float _y);
	void setFrame(int _frame);
	void reset();
private:
	Graphics* graphicsPointer;
	int frame = 0;
	int numberOfFrames;
	int frameChangeInterval;
	int frameChangeCounter = 0;
	float width;
	float height;
	float scale;
	glm::vec2 texturePosition;
};