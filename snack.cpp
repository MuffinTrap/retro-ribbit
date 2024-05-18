#include "snack.h"

Snack::Snack()
{

}

Snack::Snack(gdl::Image* image, glm::vec2 startPosition)
{
	this->position = startPosition;
	this->image = image;
}


void Snack::Draw()
{
	image->Put(position.x, position.y, gdl::Color::White, gdl::Centered, gdl::Centered, 1.0f, 0.0f);
}
