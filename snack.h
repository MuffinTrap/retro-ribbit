#pragma once


#include <glm/glm.hpp>
#include <mgdl-wii.h>

class Snack{
	public:
	Snack();
	Snack(gdl::SpriteSet* sprites, glm::vec2 startPosition);

	void Draw(gdl::FFont* font);
	void Update(float deltaTime);
	void ResetToRandom();
	float GetCatchRadius();
	float maxSpeed;
	float maxForce;
	glm::vec2 position;

	private:
	void SeekTarget(float deltaTime);
	glm::vec2 direction;
	glm::vec2 velocity;
	glm::vec2 acceleration;
	glm::vec2 target;
	glm::vec2 steer;
	gdl::SpriteSet* sprites;

	float frameTime;
	float animationTimer;
	int animationFrame;
	float radius;
};
