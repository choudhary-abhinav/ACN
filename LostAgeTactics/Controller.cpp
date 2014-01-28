#include "Controller.h"
#include "TacticsGame.h"

namespace ACN
{
	Controller::Controller()
		: m_isControllersTurn( false )
		, m_heroTurnNumber(0)
	{
		G_EventSystem.RegisterObjectForEvent( "NotifyControllerHeroTurnEnd", *this, &Controller::HeroTurnEnd );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Controller::AddHero( const CharacterInfo& charInfo, const Vec3& worldPos, float baseLvl )
	{
		Hero* newHero = new Hero( charInfo, baseLvl );
		newHero->SetTeamColor( m_TeamColor );
		newHero->SetWorldPos( worldPos );
		newHero->SetPtrToGameMap( m_ptrToGameMap );
		m_vecOfHeroes.push_back( newHero );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	float Controller::GetAverageLevel()
	{
		float count = 0;
		float totalLevel = 0; 

		std::for_each( m_vecOfHeroes.begin(), m_vecOfHeroes.end(),
			[&] ( Hero* entity )
		{
			totalLevel+= entity->GetCurrentLvl();
			count ++;
		} );
	
		if ( count == 0 )
		{
			return 0.0f;
		}

		int avgLvl = (int)( totalLevel/count );
		return (float) avgLvl;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Controller::UpdateDisplay()
	{
		std::for_each( m_vecOfHeroes.begin(), m_vecOfHeroes.end(),
			[&] ( Hero* entity )
		{
			entity->UpdateDisplay();
		} );
	}

	void Controller::UpdateSimulation( float deltaTime )
	{
		std::for_each( m_vecOfHeroes.begin(), m_vecOfHeroes.end(),
			[&] ( Hero* entity )
		{
			entity->UpdateSimulation( deltaTime );
		} );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Controller::UpdateInput()
	{
		std::for_each( m_vecOfHeroes.begin(), m_vecOfHeroes.end(),
			[&] ( Hero* entity )
		{
			entity->UpdateInput();
		} );

		for ( auto iter = m_vecOfHeroes.begin(); iter!=m_vecOfHeroes.end(); )
		{
			Hero* entity = *iter;

			if ( entity->WantsToDie())
			{			
				delete entity;
				iter = m_vecOfHeroes.erase( iter );
			}
			else
				++iter;
		}

		unsigned int vecSize = m_vecOfHeroes.size();

		for ( unsigned int i = 0; i<vecSize; ++i )
		{
			Hero* entity = m_vecOfHeroes[i];
			if ( entity->WantsToDie() )
			{
				delete entity;
				m_vecOfHeroes[i] = m_vecOfHeroes[vecSize-1];
				vecSize--;
				m_vecOfHeroes.pop_back();
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Controller::ResetController()
	{
		for ( auto iter = m_vecOfHeroes.begin(); iter!=m_vecOfHeroes.end(); )
		{
			Hero* entity = *iter;
			delete entity;
		}

		m_vecOfHeroes.clear();
		m_isControllersTurn = false;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Controller::StartTurn()
	{
		m_isControllersTurn = true;
		m_heroTurnNumber = 0;
		if (m_heroTurnNumber>=(int)m_vecOfHeroes.size())
		{
			m_isControllersTurn = false;
			TacticsGame::GetInstance().EndTurn();
			return;
		}

		BeginTurnForHero( m_vecOfHeroes[ m_heroTurnNumber ] );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Controller::EndCurrentTurn()
	{
		m_heroTurnNumber++;

		if (m_heroTurnNumber>=(int)m_vecOfHeroes.size())
		{
			m_isControllersTurn = false;
			TacticsGame::GetInstance().EndTurn();
			return;
		}

		BeginTurnForHero( m_vecOfHeroes[ m_heroTurnNumber ] );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Controller::HeroTurnEnd( NamedProperties& params )
	{
		int controllerNum;
		params.GetValue("ControllerNum", controllerNum );

		if ( m_isControllersTurn && controllerNum==m_controllerID )
		{
			EndCurrentTurn();
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Controller::BeginTurnForHero( Hero* ptrToHero )
	{
		Order order;
		order.m_orderName = G_ORDER_END;
		ptrToHero->AddNewGlobalOrderToFront(order);

		GameState* ptrToMainGameState = &(TacticsGame::GetInstance().GetMainGameState());
		std::vector< int > vecOfPossiblePos = ptrToMainGameState->GetVecOfPossiblePosForEntityID( ptrToHero->GetID() );
		
		order.m_orderName = G_ORDER_TRYBEST;
		ptrToHero->AddNewGlobalOrderToFront(order);

		TileCoords targetCoord = ptrToMainGameState->GetBestPossibleTargetPosForEntityID( ptrToHero->GetID() );
		
		order.m_orderTargetCoord = targetCoord;
		order.m_orderName = G_ORDER_MOVE;
		ptrToHero->AddNewGlobalOrderToFront(order);
	
		order.m_orderName = G_ORDER_BEGIN;
		ptrToHero->AddNewGlobalOrderToFront(order);
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Controller::SetTeamColor( Color val )
	{
		m_TeamColor = val;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	Controller::~Controller()
	{
		G_EventSystem.UnRegisterAllForObject( *this );
	}
}

