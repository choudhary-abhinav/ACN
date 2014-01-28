#include "GameState.h"
#include "AStarPathFinder.h"

namespace ACN
{
	void HeroStateData::SetVecOfPossiblePositions( const std::vector< int >& val )
	{
		LockGuardFast G(m_vecOfPossiblePositionsMutex);
		m_vecOfPossiblePositions = val;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	std::vector< int > HeroStateData::GetVecOfPossiblePositions()
	{
		LockGuardFast G(m_vecOfPossiblePositionsMutex);
		return m_vecOfPossiblePositions;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	GameState::GameState()
		: m_needsUpdate( true )
	{
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	GameState::~GameState()
	{
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameState::RegisterHeroToGameState( Hero* heroToRegister )
	{
		int heroID = heroToRegister->GetID();
		LockGuardFast G(m_mapOfEntityToHeroStateMutex);
		m_mapOfEntityToHeroState[ heroID ].m_ptrtoHero = heroToRegister;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameState::UnRegisterHeroToGameState( Hero* heroToUnRegister )
	{
		int heroID = heroToUnRegister->GetID();
		LockGuardFast G(m_mapOfEntityToHeroStateMutex);
		m_mapOfEntityToHeroState.erase(heroID);
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameState::UpdateSimulation( float deltaTime )
	{
		deltaTime;

		if ( m_needsUpdate )
		{
			m_needsUpdate = false;

			CalculateMapOfIndextoControllerNum();
			AStarPathFinder::GetInstance().CalculateAllPossiblePaths( this, m_mapOfTileIndexToHeroMovement );
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameState::CalculateMapOfIndextoControllerNum()
	{
		m_mapOfTileIndexToHeroMovement.clear();

		LockGuardFast G(m_mapOfEntityToHeroStateMutex);

		for ( auto i = m_mapOfEntityToHeroState.begin(); i!=m_mapOfEntityToHeroState.end(); ++i )
		{
			auto ptrToHero = i->second.m_ptrtoHero;
			int index = ptrToHero->GetTileIndex() ;
			m_mapOfTileIndexToHeroMovement[ index ].m_controllerNum = ptrToHero->GetControllerNum();
			m_mapOfTileIndexToHeroMovement[ index ].m_movement = ptrToHero->GetMovement();
			m_mapOfTileIndexToHeroMovement[ index ].m_entityID = ptrToHero->GetID();
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameState::SetPossiblePaths( NamedProperties& params )
	{
		int entityID;
		std::vector< int > vecOfPossiblePos;
		params.GetValue( "vecOfPossiblePos", vecOfPossiblePos );
		params.GetValue( "entityID", entityID );
		//LockGuardFast G(m_mapOfEntityToHeroStateMutex);
		m_mapOfEntityToHeroState[ entityID ].SetVecOfPossiblePositions( vecOfPossiblePos );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	std::vector< int > GameState::GetVecOfPossiblePosForEntityID( int entityID )
	{
		return m_mapOfEntityToHeroState[ entityID ].GetVecOfPossiblePositions();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameState::NeedsUpdate()
	{
		m_needsUpdate = true;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	ACN::TileCoords GameState::GetBestPossibleTargetPosForEntityID( int entityID )
	{
		std::vector< int > m_possiblePos = m_mapOfEntityToHeroState[ entityID ].GetVecOfPossiblePositions();

		//Find closest enemy entity to u
		int closestEntityID = entityID;
		Hero* currentHero = m_mapOfEntityToHeroState[ entityID ].m_ptrtoHero;
		Hero* mainHero = currentHero;
		
		TileCoords bestPos = currentHero->GetCurrentTileCoords();
		TileCoords currentPos = currentHero->GetCurrentTileCoords();
		
		if ( m_possiblePos.size() == 0 )
		{
			return bestPos;
		}
		
		//Get Manhattan Distance
		int manDis = 10000;

		//Find closest Hero
		for ( auto i = m_mapOfEntityToHeroState.begin(); i!=m_mapOfEntityToHeroState.end(); ++i )
		{
			HeroStateData& stateData = i->second;
			Hero* nextHero = stateData.m_ptrtoHero;
			if ( currentHero->GetControllerNum()!= nextHero->GetControllerNum() )
			{
				int manhatDis = GetManhattanDistance( currentHero->GetCurrentTileCoords(), nextHero->GetCurrentTileCoords() );
				if ( manhatDis < manDis )
				{
					manDis = manhatDis;
					closestEntityID = nextHero->GetID();
				}
			}
		}
		
		currentHero = m_mapOfEntityToHeroState[ closestEntityID ].m_ptrtoHero;
		bestPos = currentHero->GetCurrentTileCoords();
		TileCoords targetPos = currentHero->GetCurrentTileCoords();
		
		bestPos = AStarPathFinder::GetInstance().GetBestPossibleTargetPos( currentPos, targetPos, m_mapOfTileIndexToHeroMovement, m_possiblePos, mainHero->GetControllerNum(), mainHero->GetMovement() );

		return bestPos;
	}
}
