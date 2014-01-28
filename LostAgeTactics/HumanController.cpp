#include "HumanController.h"
#include "TacticsGame.h"
#include "AbilityBlueprint.h"
#include "AStarPathFinder.h"

namespace ACN
{
	HumanController::HumanController()
	{
		m_isGivingUnitOrder = false;
		m_canMoveUnit = false;
		m_currentTurnMode = TURN_NOT_STARTED;
		m_gaveUnitOrder = false;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	HumanController::~HumanController()
	{
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HumanController::StartTurn()
	{
		m_isControllersTurn = true;
		m_heroTurnNumber = 0;

		if (m_heroTurnNumber>=(int)m_vecOfHeroes.size())
		{
			m_isControllersTurn = false;
			TacticsGame::GetInstance().EndTurn();
			return;
		}

		ResetOldTurnVariables();
		m_currentTurnMode = TURN_NEEDS_ACTION;
		m_currentHeroWithTurn = m_vecOfHeroes[ m_heroTurnNumber ];
		BeginTurnForHero( m_vecOfHeroes[ m_heroTurnNumber ] );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HumanController::EndCurrentTurn()
	{
		if ( m_currentTurnMode == TURN_NEEDS_ACTION )
		{
			ResetOldTurnVariables();
			m_heroTurnNumber++;
			if (m_heroTurnNumber>=(int)m_vecOfHeroes.size())
			{
				m_isControllersTurn = false;
				TacticsGame::GetInstance().EndTurn();
				return;
			}
			m_currentHeroWithTurn = m_vecOfHeroes[ m_heroTurnNumber ];
			BeginTurnForHero( m_vecOfHeroes[ m_heroTurnNumber ] );
		}
		else
		{
			BeginTurnForHero( m_vecOfHeroes[ m_heroTurnNumber ] );
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HumanController::BeginTurnForHero( Hero* ptrToHero )
	{
		if ( m_currentTurnMode == TURN_NEEDS_ACTION )
		{
			ptrToHero->m_textMsg = "Make Me Move!";
			ptrToHero->SetTimer( 2.0f );
			Order order;
			order.m_orderName = G_ORDER_BEGIN;
			ptrToHero->AddNewGlobalOrderToFront(order);
			m_currentTurnMode = TURN_NEEDS_MOVE_UNIT;
		}

		else if ( m_currentTurnMode == TURN_NEEDS_MOVE_UNIT )
		{
			Order order;			
			order.m_orderName = G_ORDER_BEGIN;
			ptrToHero->AddNewGlobalOrderToFront(order);
			ptrToHero->m_textMsg = "Give Me Order";
			ptrToHero->SetTimer( 2.0f );

			m_currentTurnMode = TURN_NEEDS_GIVE_ACTION; 
		}

		else if ( m_currentTurnMode == TURN_NEEDS_GIVE_ACTION )
		{
			ptrToHero->SetTimer( 0.0f );
			Order order;
			order.m_orderName = G_ORDER_END;
			ptrToHero->AddNewGlobalOrderToFront(order);
	
			order.m_orderName = G_ORDER_BEGIN;
			ptrToHero->AddNewGlobalOrderToFront(order);
			m_currentTurnMode = TURN_NEEDS_ACTION; 
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HumanController::ResetOldTurnVariables()
	{
		m_canMoveUnit = false;
		m_isGivingUnitOrder = false;
		m_gaveUnitOrder = false;		
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HumanController::UpdateSimulation( float deltaTime )
	{
		Controller::UpdateSimulation(deltaTime);

		if ( m_gaveUnitOrder )
		{			
			 if ( IsCurrentSelectedTileValidPos() && m_currentTurnMode == TURN_NEEDS_GIVE_ACTION )
			 {
				  m_currentHeroWithTurn-> m_bShouldDisplayAttack = true;
				  m_currentHeroWithTurn->m_targetTileCoords = m_ptrToGameMap->GetSquareSelected();
				  m_currentHeroWithTurn->m_targetAbilityName = m_currentOrderName;
			 }
			 else
			 {
				 m_currentHeroWithTurn-> m_bShouldDisplayAttack = false;
			 }
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HumanController::OnMouseButton( int whichButton, int state, int mouseX, int mouseY )
	{
		mouseX;
		mouseY;
		switch ( whichButton )
		{
		case GLUT_LEFT_BUTTON:
			if ( state == GLUT_DOWN )
			{
				LeftMoustButtonDown();
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HumanController::LeftMoustButtonDown()
	{
		if ( m_currentTurnMode == TURN_NEEDS_MOVE_UNIT && ! m_canMoveUnit)
		{
			if( m_ptrToGameMap->IsSquareSelected() )
			{
				GameState* ptrToMainGameState = &(TacticsGame::GetInstance().GetMainGameState());
				std::vector< int > vecOfPossiblePos = ptrToMainGameState->GetVecOfPossiblePosForEntityID( m_currentHeroWithTurn->GetID() );

				if ( m_ptrToGameMap->GetSquareSelected() == m_currentHeroWithTurn->m_currentTileCoords )
				{
					m_canMoveUnit = true;
				}

				for ( unsigned int i = 0; i < vecOfPossiblePos.size(); ++i )
				{
					if ( m_ptrToGameMap->GetSquareSelected() == m_ptrToGameMap->GetMapTileAtIndex( vecOfPossiblePos[i] ).m_tileCoords )
					{
						m_canMoveUnit = true;
						break;
					}
					
				}
				
				if ( m_canMoveUnit )
				{
					Order order;
					order.m_orderName = G_ORDER_END;
					m_currentHeroWithTurn->AddNewGlobalOrderToFront(order);

					order.m_orderName = G_ORDER_MOVE;
					order.m_orderTargetCoord = m_ptrToGameMap->GetSquareSelected();
					m_currentHeroWithTurn->AddNewGlobalOrderToFront(order);
				}
			}			
		}
		
		else if ( m_gaveUnitOrder )
		{
			if ( IsCurrentSelectedTileValidPos() && m_currentTurnMode == TURN_NEEDS_GIVE_ACTION )
			{
				m_currentHeroWithTurn-> m_bShouldDisplayAttack = false;
				Order order;
				order.m_orderName = G_ORDER_END;
				m_currentHeroWithTurn->AddNewGlobalOrderToFront(order);
				order.m_orderName = m_currentOrderName;
				order.m_orderTargetCoord = m_ptrToGameMap->GetSquareSelected();
				m_currentHeroWithTurn->AddNewGlobalOrderToFront(order);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HumanController::OnKeyDown( unsigned char keyCode, int mouseX, int mouseY )
	{
		mouseX;
		mouseY;

		if ( !m_isGivingUnitOrder && m_currentTurnMode == TURN_NEEDS_GIVE_ACTION )
		{
			if ( keyCode >= '0' && keyCode <='9')
			{
				if ( keyCode == '0')
				{
					m_currentHeroWithTurn-> m_bShouldDisplayAttack = false;
					Order order;
					order.m_orderName = G_ORDER_END;
					m_currentHeroWithTurn->AddNewGlobalOrderToFront(order);
					order.m_orderName = G_ORDER_HOLD;
					m_currentHeroWithTurn->AddNewGlobalOrderToFront(order);
					m_isGivingUnitOrder = true;
				}
				else
					SetupCurrentOrder( keyCode );
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void HumanController::SetupCurrentOrder( unsigned char keyCode )
	{
		int num = keyCode - '0';
		num-=1;
		auto& vecOfAbilities = m_currentHeroWithTurn->GetVecOfAvaiableAbilities();
		if ( num < (int)vecOfAbilities.size() )
		{
			m_currentOrderName = vecOfAbilities[num];
			m_currentHeroWithTurn->m_textMsg = m_currentOrderName;
			m_currentHeroWithTurn->SetTimer( 1.0f );
			m_gaveUnitOrder = true;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	bool HumanController::IsCurrentSelectedTileValidPos()
	{
		AbilityInfo currentAbility = AbilityBluePrint::CreateOrGetBluePrint( m_currentOrderName )->GetAbilityInfo();

		if( m_ptrToGameMap->IsSquareSelected() )
		{
			float manDis = (float)GetManhattanDistance( m_ptrToGameMap->GetSquareSelected(), m_currentHeroWithTurn->GetCurrentTileCoords() );
			if ( manDis <= currentAbility.m_maxRange && manDis>=currentAbility.m_minRange )
			{
				return true;
			}
		}
		return false;
	}
}
