#include "HeroStates.h"
#include "TacticsGame.h"
#include "AbilityBlueprint.h"
#include "AStarPathFinder.h"

namespace ACN
{			
	
	void HeroStateGlobal::Enter( Hero* hero )
	{
		hero;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateGlobal::Execute( Hero* hero )
	{
		if ( hero->HasGlobalOrder() )
		{
			Order order = hero->GetTopMostGlobalOrder();

			//Check if its end turn or begin turn
			if ( order.m_orderName == G_ORDER_END )
			{
				if ( hero->IsInState( HERO_STATE_IDLE ) )
				{
					hero->PopTopMostOrder();
					hero->EndTurn();					
				}
			}

			else if ( order.m_orderName == G_ORDER_BEGIN )
			{
				if ( hero->IsInState( HERO_STATE_IDLE ) )
				{
					hero->PopTopMostOrder();
					hero->BeginTurn();					
				}
			}
			
			//Check if it is moving order
			else if ( order.m_orderName == G_ORDER_MOVE )
			{
				if ( hero->IsInState( HERO_STATE_IDLE ) )
				{
					hero->PopTopMostOrder();
					hero->m_targetTileCoords = order.m_orderTargetCoord;					
					hero->GetFSM()->ChangeState( HERO_STATE_MOVE );
					
				}
			}

			else if ( order.m_orderName == G_ORDER_TRYBEST )
			{
				if ( hero->IsInState( HERO_STATE_IDLE ) )
				{
					hero->PopTopMostOrder();
					hero->m_targetTileCoords = order.m_orderTargetCoord;					
					hero->GetFSM()->ChangeState( HERO_STATE_TRYBEST );
					
				}
			}

			else if ( order.m_orderName == G_ORDER_HOLD )
			{
				if ( hero->IsInState( HERO_STATE_IDLE ) )
				{					
					hero->PopTopMostOrder();
				}
			}

			//Else it is an action order and perform the attack/heal action
			else
			{
				if ( hero->IsInState( HERO_STATE_IDLE ) )
				{
					hero->PopTopMostOrder();
					hero->m_targetTileCoords = order.m_orderTargetCoord;
					hero->m_targetAbilityName = order.m_orderName;					
					hero->GetFSM()->ChangeState( HERO_USE_ABILITY );				
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateGlobal::Exit( Hero* hero )
	{
		hero;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateIdle::Enter( Hero* hero )
	{
		hero;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateIdle::Execute( Hero* hero )
	{
		hero;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateIdle::Exit( Hero* hero )
	{
		hero;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateMove::Enter( Hero* hero )
	{
		hero->m_isMoving = true;
		hero->SetTargetPosFromTargetCoord();
		hero->SetCurrentTileCoords( hero->m_targetTileCoords );		
		
		GameState* ptrToMainGameState = &(TacticsGame::GetInstance().GetMainGameState());
		ptrToMainGameState->NeedsUpdate();

		hero->SetTimer( 3.0f );
		hero->m_textMsg = "Moving!";

		hero->RegenAndCheckLvlUp();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateMove::Execute( Hero* hero )
	{
		if ( !hero->m_isMoving )
		{			
			hero->GetFSM()->ChangeState( HERO_STATE_IDLE );
		}		
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateMove::Exit( Hero* hero )
	{
		hero;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateTryBest::Enter( Hero* hero )
	{
		MyAbilityHeap myHeap;

		AbilityNode holdNode;
		holdNode.abilityName = G_ORDER_HOLD;
		holdNode.score = 1.0f;
		holdNode.targetCoords = hero->m_currentTileCoords;
		myHeap.push( holdNode );

		VecOfAbilities& currentAbilities = hero->m_vecOfAvaiableAbilities;

		for ( unsigned int i = 0; i<currentAbilities.size(); ++i )
		{
			std::string ability = currentAbilities[i];
			AbilityInfo currentAbility = AbilityBluePrint::CreateOrGetBluePrint( ability )->GetAbilityInfo();

			//Make sure guy has enough hp and mp
			if ( currentAbility.m_hpCost < hero->m_currentHP && currentAbility.m_mpCost< hero->m_currentMP )
			{
				//Fire at all possible positions
				for( int x = hero->m_currentTileCoords.x - currentAbility.m_maxRange;x <= hero->m_currentTileCoords.x + currentAbility.m_maxRange; ++x  )
				{
					for( int y = hero->m_currentTileCoords.y - currentAbility.m_maxRange;y <= hero->m_currentTileCoords.y + currentAbility.m_maxRange; ++y  )
					{
						MapTile& mapTile = hero->m_ptrToGameMap->GetMapTile( x, y );
						float manDis = (float)GetManhattanDistance( hero->m_currentTileCoords, mapTile.m_tileCoords );
						float damage = currentAbility.m_dmgModifiers * hero->m_currentLevelStats;
						
						if ( manDis>=currentAbility.m_minRange&&manDis<=currentAbility.m_maxRange )
						{
							AbilityNode currentAbilityNode;
							currentAbilityNode.abilityName = ability;
							currentAbilityNode.targetCoords = mapTile.m_tileCoords;
							
							NamedProperties params;							
							float score = 0.0f;
							params.SetValue("range", currentAbility.m_areaOfEffect );
							params.SetValue("attackCoords", mapTile.m_tileCoords);
							params.SetValue("attackType", (int)currentAbility.m_targetType );
							params.SetValue("controllerNum", hero->GetControllerNum() );
							params.SetValue("score", score );
							params.SetValue("damage", damage );
							params.SetValue("element", currentAbility.m_baseElement );
							params.SetValue("level", hero->m_currentLvl );
							G_EventSystem.FireEvent( "TestGlobalAction", params );
							params.GetValue("score", score);
							currentAbilityNode.score = score;
							
							myHeap.push( currentAbilityNode );
						}
					}
				}
			}
		}


		//Test for the best of the best of the best and do it
		AbilityNode bestAbility = myHeap.top();
		Order order;
		order.m_orderName = bestAbility.abilityName;
		order.m_orderTargetCoord = bestAbility.targetCoords;
		hero->AddNewGlobalOrderToFront(order);
		hero->SetTimer( 0.5f );
		hero->m_textMsg = order.m_orderName;
		hero->m_bShouldDisplayAttack = true;
		hero->m_targetTileCoords = order.m_orderTargetCoord;
		hero->m_targetAbilityName= order.m_orderName;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateTryBest::Execute( Hero* hero )
	{	
		if ( hero->m_myTimer < 0.0f )
		{
			hero->GetFSM()->ChangeState( HERO_STATE_IDLE );
		}		
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateTryBest::Exit( Hero* hero )
	{
		hero->m_bShouldDisplayAttack = false;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateUseAbility::Enter( Hero* hero )
	{
		std::string ability = hero->m_targetAbilityName;
		AbilityInfo currentAbility = AbilityBluePrint::CreateOrGetBluePrint( ability )->GetAbilityInfo();

		//Make sure guy has enough hp and mp
		if ( currentAbility.m_hpCost < hero->m_currentHP && currentAbility.m_mpCost< hero->m_currentMP )
		{
			float damage = currentAbility.m_dmgModifiers * hero->m_currentLevelStats;
			NamedProperties params;
			float score = 0.0f;
			params.SetValue("range", currentAbility.m_areaOfEffect );
			params.SetValue("attackCoords", hero->m_targetTileCoords );
			params.SetValue("attackType", (int)currentAbility.m_targetType );
			params.SetValue("controllerNum", hero->GetControllerNum() );
			params.SetValue("score", score );
			params.SetValue("damage", damage );
			params.SetValue("element", currentAbility.m_baseElement );
			params.SetValue("level", hero->m_currentLvl );
			G_EventSystem.FireEvent( "PerformGlobalAction", params );
			params.GetValue("score", score);
			hero->m_currentExp += score;
		}

		hero->SetTimer( 0.5f );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateUseAbility::Execute( Hero* hero )
	{
		if ( hero->m_myTimer < 0.0f )
		{
			hero->GetFSM()->ChangeState( HERO_STATE_IDLE );
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HeroStateUseAbility::Exit( Hero* hero )
	{
			hero;
	}
}
