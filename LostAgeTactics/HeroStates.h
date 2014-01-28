/********************************************************************
	author:			Abhinav Choudhary
	filename: 		HeroStates.h
	description:	A simple state system used by a hero
*********************************************************************/
#pragma once
#include "Hero.h"

namespace ACN
{
	#define HERO_STATE_GLOBAL &HeroStateGlobal::GetInstance()
	#define HERO_STATE_IDLE &HeroStateIdle::GetInstance()
	#define HERO_STATE_MOVE &HeroStateMove::GetInstance()
	#define HERO_STATE_TRYBEST &HeroStateTryBest::GetInstance()
	#define HERO_USE_ABILITY &HeroStateUseAbility::GetInstance()

	//Simple node used in priority que to decide the best ability to be used
	struct AbilityNode
	{
		float score;
		TileCoords targetCoords;
		std::string abilityName;
		bool operator<( const AbilityNode& rhs )const { return score < rhs.score; } 
	};	

	typedef std::priority_queue<AbilityNode> MyAbilityHeap;
	
	class HeroStateGlobal : public State< Hero >, public Singleton<HeroStateGlobal>
	{	
	public:
		void Enter(Hero* hero);
		void Execute(Hero* hero);
		void Exit(Hero* hero);
	};

	class HeroStateIdle : public State< Hero >, public Singleton<HeroStateIdle>
	{
	public:
		void Enter(Hero* hero);
		void Execute(Hero* hero);
		void Exit(Hero* hero);
	};	

	class HeroStateMove : public State< Hero >, public Singleton<HeroStateMove>
	{
	public:
		void Enter(Hero* hero);
		void Execute(Hero* hero);
		void Exit(Hero* hero);
	};

	class HeroStateTryBest : public State< Hero >, public Singleton<HeroStateTryBest>
	{
	public:
		void Enter(Hero* hero);
		void Execute(Hero* hero);
		void Exit(Hero* hero);
	};

	class HeroStateUseAbility : public State< Hero >, public Singleton<HeroStateUseAbility>
	{
	public:
		void Enter(Hero* hero);
		void Execute(Hero* hero);
		void Exit(Hero* hero);
	};
}

