#pragma once


#include <glm/glm.hpp>
#include <mgdl-wii.h>

class Snack{
	public:
	Snack();
	Snack(gdl::Image* image, glm::vec2 startPosition);

	void Draw();

	glm::vec2 position;
	gdl::Image* image;
};
