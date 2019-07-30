#pragma once

#include <string>

#include "Utiles/Singleton.h"


class Entity;

class Animator : public Singleton<Animator>
{
	friend class Singleton<Animator>;

	public:
		void animate(Entity* object, float step);
		void launchAnimation(Entity* object, const std::string& labelName, bool flaged = false);
		void stopAnimation(Entity* object, const std::string& labelName);
		bool isAnimationRunning(Entity* object, const std::string& animationName);

	private:
		Animator() = default;
		~Animator() = default;
};

