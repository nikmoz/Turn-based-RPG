#pragma once
#include "Hero.h"
#include "Observer.h"
#include "Action.h"
#include <vector>
#include <thread>

using HeroPtr = std::shared_ptr<Hero>;

class Action;

class Scene:public Observer
{
public:
	
	Scene()=default;

	std::vector<HeroPtr> Characters;

	void SetupActions(std::string ActionPath);
	void AddCharacter(HeroPtr Character);
	void update(Keyboard::Keys Key)override;

	~Scene() = default;
private:
	
	int currentChar = 0;
	std::vector<Action*> Actions;
	Action* ChooseAction(Keyboard::Keys Key);
};



