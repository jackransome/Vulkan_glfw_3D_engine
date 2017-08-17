#pragma once
//a standard struct commonly used by objects in the game
//includes the dimensions, position, and velocity
//used by collision detection methods
struct BoundingBox { float x; float y; float w; float h; float xv; float yv; bool onGround; bool fullyOnGround; };