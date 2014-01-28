/********************************************************************
	author:			Abhinav Choudhary
	filename: 		GameState.h
	description:	Holds the Game's state information at a particular 
					instance
*********************************************************************/
#pragma once
#include "Hero.h"
#include "ThreadWrapper.h"
#include "EventSystem.h"

namespace ACN
{
	//Struct to have enough data to record a particular hero's movement
	struct HeroMovementData
	{
		//The units it can move
		int m_movement;
		
		//The controller the hero belongs to
		int m_controllerNum;

		//The unique entity id for the hero
		int m_entityID;
	};

	//Hero state data at a particular instant.
	struct HeroStateData
	{
		Hero* m_ptrtoHero;	

		//Vector of possible positions the hero can go to
		std::vector< int /*Map Indexes*/ > m_vecOfPossiblePositions;

		//Accessor
		std::vector< int > GetVecOfPossiblePositions();
		
		//Mutator
		void SetVecOfPossiblePositions(const std::vector< int >& val);

		//Mutex used for multi threading
		ACNFastMutex m_vecOfPossiblePositionsMutex;
	};

	class GameState: public BaseWorldItem
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		///Initializers & Constructor/Destructor
		//////////////////////////////////////////////////////////////////////////
		
		GameState();
		~GameState();
		
		//////////////////////////////////////////////////////////////////////////
		///Frame Updates
		//////////////////////////////////////////////////////////////////////////
		
		virtual void UpdateSimulation( float deltaTime );
		
		//////////////////////////////////////////////////////////////////////////
		///Accessors
		//////////////////////////////////////////////////////////////////////////
		
		std::vector< int > GetVecOfPossiblePosForEntityID( int entityID );
		TileCoords GetBestPossibleTargetPosForEntityID( int entityID );

		//////////////////////////////////////////////////////////////////////////
		///Mutators
		//////////////////////////////////////////////////////////////////////////
		
		void SetPossiblePaths( NamedProperties& params );

		void RegisterHeroToGameState( Hero* heroToRegister );
		void UnRegisterHeroToGameState( Hero* heroToUnRegister );
		
		//////////////////////////////////////////////////////////////////////////
		///Public Functions
		//////////////////////////////////////////////////////////////////////////
		void NeedsUpdate();

	private:		
		//////////////////////////////////////////////////////////////////////////
		///Data
		//////////////////////////////////////////////////////////////////////////
		
		//Mapping from unique entity id to hero state
		std::map< int /*Entity ID*/, HeroStateData > m_mapOfEntityToHeroState;

		//Mapping from tile index to hero movement
		std::map< int /*TileIndex*/, HeroMovementData> m_mapOfTileIndexToHeroMovement;

		//Has something changed since last time, an update is needed so all hero states can be updated
		bool m_needsUpdate;

		//Mutex used for multi threading
		ACNFastMutex m_mapOfEntityToHeroStateMutex;
		
		//////////////////////////////////////////////////////////////////////////
		///Functions
		//////////////////////////////////////////////////////////////////////////
	
		void CalculateMapOfIndextoControllerNum();
	};
}

