#include "snack.h"
#include <cstdlib>

Snack::Snack()
{

}

Snack::Snack(gdl::SpriteSet* sprites, glm::vec2 startPosition)
{
	this->position = startPosition;
	target = position;
	this->sprites = sprites;

	velocity = glm::vec2(1,0);
	direction = glm::vec2(1,0);
	acceleration = glm::vec2(1,0);
	steer = glm::vec2(1,0);
	maxSpeed = 100;
	maxForce = 167.0f;


	frameTime = 0.05;
	animationTimer = 0.0f;
	animationFrame = 0;
}


void Snack::Draw(gdl::FFont* font)
{
	sprites->Put(position.x, position.y, animationFrame, gdl::Color::White, gdl::Centered, gdl::Centered, 0.5f, 0.0f);

	/*
	// Debug draw target
	gdl::DrawBox(target.x - 5, target.y - 5, target.x + 5, target.y + 5, gdl::Color::Red);

	// Debug values
	font->Printf(10, 80, 1.0f, gdl::Color::Red, "ms(%f) mf(%f)", maxSpeed, maxForce);
	*/
}

void Snack::Update(float deltaTime)
{
	SeekTarget(deltaTime);

	velocity += acceleration * deltaTime;
	float vl = glm::length(velocity);
	if (vl > maxSpeed)
	{
		velocity = glm::normalize(velocity) * maxSpeed;
	}
	position += velocity * deltaTime;
	acceleration *= 0.0f;

	animationTimer += deltaTime;
	if (animationTimer > frameTime)
	{
		animationTimer -= frameTime;
		animationFrame = (animationFrame + 1) %2;
	}
}

static float GetRandFloat()
{
	return (float)rand()/(float)RAND_MAX;
}

void Snack::SeekTarget(float deltaTime)
{
	// Have we reached the target

	glm::vec2 desired = target - position;
	float distanceToTarget = glm::length(desired);
	if (distanceToTarget < 15.0f)
	{
		target = glm::vec2( 40 + GetRandFloat() * (gdl::ScreenXres - 40),
						   10 + GetRandFloat() * gdl::ScreenYres/5.0f);
	}
	if (glm::length(desired) > 0.0)
	{
		if (distanceToTarget < 50)
		{
			// Slow down before arriving
			desired = glm::normalize(desired) * (distanceToTarget/50)*maxSpeed;
		}
		else
		{
			desired = glm::normalize(desired) * maxSpeed;
		}
	}
	steer = desired - velocity;
	float sl = glm::length(steer);
	if (sl > 0.0f && sl > maxForce)
	{
		steer = glm::normalize(steer) * maxForce;
	}
	acceleration += steer;
}
