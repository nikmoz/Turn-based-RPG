#include "BattleScene.h"

#include "NameAction.h"
#include "SideAction.h"
#include "TargetAction.h"

using std::move;

BattleScene::BattleScene()
{
	TurnInputHandler_ = std::make_unique<class InputHandler>();
	TurnInputHandler_->Subscribe(std::shared_ptr<class Observer>(this));
}

void BattleScene::AddCharacter(TargetPtr&& Character) noexcept
{
	Characters.push_back(Character);
	Render_->AddCharacter(Character->Graphic);
};
/*TODO(Nick):Turn() 
	Problem 1:How to change targets without passing Characters to Hero Class(tried GameClass with static, but it didn't look nice)
	Problem 2:Attack and Defend consumes turn, while ChangeTarget doesn't
	Problem 3:Skill system allows for multiple target types and multiple turn actions, which leads to bigger problem than problem 1

	Solution 1(checking...):
	Create Action stack and countdown param in Action(Done, but with queue)
	Create LoadAction() that loads Action list for scene(probably move to global scene load later) (Check next ChooseAction() THEME)
	Create ChooseAction() that returns Action(Command pattern)(Done)
	Execute Action, which leads to finding out target and executing one of the functions in Hero(Done)
*/
/*TODO(Nick):ChooseAction()
	The way is see it:
	Battle starts->
	Some Game function loads characters->
	Scene setups characters Actions->(Check Root 2)
	User chooses skill->
	User chooses Targets->
	Action executes->
	Tasks being checked

	Root 1: In current state scene should know about all actions for each actor
		Solution 1:
			Create set of actions {NextAction,PrevAction,ExecuteAction} and store actions in HeroClass
			Argument:
				This will keep Scene from knowing all actions, and leads to only 1 actions loading, on Character init
				More over, it means, that scene doesn't have same Action patter in each battle
				Despite it leads to storing copies of one Action in heroes, but their size is so small it will not became a problem
		Solution 2:
			Set of {Attack,Defend,Skill}. Attack and Defend execute Attack and Defend methods in Hero(or just call execute() on Action)
			Skill calls LoadSkills for current Hero, creating new set of actions
			Argument:
				No copies of standard actions in standard encounters
				This will move all Action resolving staff into Hero class, and easy tom implement, 
				but I'l be stuck with same Attack,Defend, Skill (and future Party) pattern
		Solution 3:
			Keep it that way, and think of format, that can save all information needed
			Argument:
				Headers aren't this kind of problem you should be thinking of
				Scenes than can have different set of actions for each encounter(And writing standard set in each standard battle)

	Root 2: Where is loading actions supposed to happen?
		Solution 1: On Character init (seems nice, but restrictive)
			Argument:
				Keeps character data in character class
				Creates copies of standard actions(if second solutions isn't choosed)
				Don't sure how to create special scene-based actions
		Solution 2: On Scene init (seems really bad)
			Argument:
				Easy scene-based actions creation
				Scene should know about all characters skills
		Solution 3: On Calling Skill Action (seems interesting, but with ugly code)
			Argument:
				Easy scene-based  and plot-based actions creation
				Scene should know about all characters skills
				Loads skills only if needed

	Root 3: Separated Scene and render means InputHandler code duplication(Solved)
		Solution 1:
			Create static InputHandler class that notifies each sub on key input. Make Scene and Render subs.(Done)
		Solution 2:
			Create static InputHandler class that sends Action to each sub. Make Scene and Render subs.
*/

ActionPtr BattleScene::ChooseAction(const Keyboard::Keys Key)//TODO(Nick):Read about Factory method
{
	static auto I(0U);

	if (Key==Keyboard::Down)
	{
		if (I < Actions_.size() - 1)
		{
			I++;

			std::cout << "Current Action:" << I<< std::endl;
		}
		return nullptr;
	}

	if (Key == Keyboard::Up)
	{
		if (I > 0)
		{
			I--;

			std::cout << "Current Action:" << I << std::endl;
		}
		return nullptr;
	}

	if (Key == Keyboard::Enter)
	{
		const auto Tmp = I;
			I = 0;

			return Actions_.at(Tmp);
	}

	return nullptr;
};

void BattleScene::Update(const Keyboard::Keys Key)
{
	auto TurnAction = ChooseAction(Key);

	if (TurnAction == nullptr) { 
		return;
	}

	std::cout <<"Current Char:"<< Characters.at(CurrentChar_)->Name << std::endl;

	TurnAction->Execute(*Characters.at(CurrentChar_));
	if (!TurnAction->IsResolved)
	{
		auto Char = Characters.at(CurrentChar_);
		ActionQueue_.push(std::make_pair(TurnAction, Char));
	}

	CurrentChar_++;
	if (CurrentChar_ >= Characters.size())
	{
		CurrentChar_ = 0;
	}
};

void BattleScene::UpdateScene()
{
	if (CheckActionQueue())
	{
		TurnInputHandler_->HandleInput();
	}
};

void BattleScene::Redraw()
{
	Render_->RenderScene();
};

void BattleScene::Load()//TODO(Nick): Figure out normal loading variant
{
	Render_ = std::make_unique<class Render>(400,400);

	TargetPtr Uther(new Hero("Uther", "res/img/sprite_base_addon_2012_12_14.png", sf::IntRect(10, 10, 70, 70),
	                         HeroDefinitions::Hero));
	TargetPtr AUther(new Hero("Arthas", "res/img/sprite_base_addon_2012_12_14.png", sf::IntRect(10, 10, 70, 70),
	                          HeroDefinitions::Enemy));

	AddCharacter(move(Uther));
	AddCharacter(move(AUther));
	SetupCharactersPosition();

	this->Actions_.push_back(ActionPtr(new NameAction));
	this->Actions_.push_back(ActionPtr(new SideAction));
	this->Actions_.push_back(ActionPtr(new TargetAction));

}

bool BattleScene::CheckActionQueue()//NOTE(Nick): Cause some actions may have countdown, i probably should switch to vector instead of queue
{
	if (ActionQueue_.empty())
	{
		return true;
	}

	auto CurrentAction = ActionQueue_.front().first;
	const auto CurrentHero = ActionQueue_.front().second;

	CurrentAction->Execute(*CurrentHero);

	if (CurrentAction->IsResolved)
	{
		ActionQueue_.pop();
	}

	return false;
};

void BattleScene::SetupCharactersPosition() noexcept//NOTE(Nick):still not sure if this belongs here
{
	auto HeroCount(0);
	auto EnemyCount(0);
	const auto SpriteSpace(100);

	for (auto& Char : Characters)
	{
		if (Char->Side == HeroDefinitions::Hero) {
			HeroCount++;
			Char->Graphic->Sprite.setPosition(static_cast<float>(Render_->RenderWidth) / 2 - static_cast<float>(SpriteSpace * HeroCount),
			                                  static_cast<float>(Render_->RenderHeight) / 2);

		}
		else if (Char->Side == HeroDefinitions::Enemy) {
			EnemyCount++;
			Char->Graphic->Sprite.setPosition(static_cast<float>(Render_->RenderWidth) / 2 + static_cast<float>(SpriteSpace * EnemyCount),
			                                  static_cast<float>(Render_->RenderHeight) / 2);

		}
	}
};