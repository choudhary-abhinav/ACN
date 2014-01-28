/********************************************************************
	author:			Abhinav Choudhary
	filename: 		TacticsGame.h
	description:	The class responsible for initializing the game.
					It also owns all the entities.
*********************************************************************/
#pragma once
#include "singleton.h"
#include "inputlistener.h"
#include "MyXML.h"
#include "GameMap.h"
#include "Clock.h"
#include "BaseWorldItem.h"
#include "Controller.h"
#include "GameState.h"

namespace ACN
{
	class TacticsGame: public Singleton<TacticsGame>, public InputListener, public BaseWorldItem
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		///Initializers & Constructor/Destructor
		//////////////////////////////////////////////////////////////////////////

		TacticsGame();		
		void Initialize();

		//////////////////////////////////////////////////////////////////////////
		///Frame Updates
		//////////////////////////////////////////////////////////////////////////

		virtual void UpdateDisplay();
		virtual void UpdateSimulation( float deltaTime );
		virtual void UpdateInput();
		virtual void OnSpecialKeyDown( int keyCode, int mouseX, int mouseY );

		//////////////////////////////////////////////////////////////////////////
		///Accessors
		//////////////////////////////////////////////////////////////////////////

		ACN::GameState& GetMainGameState() { return m_mainGameState; }
		ACN::GameMap& GetGameMap() { return m_gameMap; }

		//////////////////////////////////////////////////////////////////////////
		///Mutators
		//////////////////////////////////////////////////////////////////////////

		void SetMainGameState(const ACN::GameState& val) { m_mainGameState = val; }
		void SetGameMap(const ACN::GameMap& val) { m_gameMap = val; }

		//////////////////////////////////////////////////////////////////////////
		///Public Functions
		//////////////////////////////////////////////////////////////////////////
		
		void EndTurn();
	
	private:
		//////////////////////////////////////////////////////////////////////////
		///Data
		//////////////////////////////////////////////////////////////////////////

		GameMap m_gameMap; 		
		
		Clock m_gameClock;

		//ID pointing to the current turn's controller 
		int m_currentTurnControllerID;

		//Did a new map just start
		bool m_startNewMap;

		//The base lvl for all the entities.
		float m_currentMapBaseLvl;

		//Vector of all the game controllers
		std::vector< Controller* > m_vecOfControllers;

		//The main state of the game
		GameState m_mainGameState;		
		
		//////////////////////////////////////////////////////////////////////////
		///Private Functions
		//////////////////////////////////////////////////////////////////////////
		
		void SetupNewGame();
		void SetDefaultTeamColors();
		
		void MakeCustomMap();
		
		template<typename T_BluePrintType >
		void PopulateBlueprints( XMLNode& blueprintPathsNode, T_BluePrintType );

		void AddBluePrints();
		void PopulateAllMapBlueprints( const XMLNode& mapsNode );
		
		void CreateHumanController();		
		void AddNewAIController();
	};	

	//////////////////////////////////////////////////////////////////////////
	///Template and inline definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T_BluePrintType >
	void TacticsGame::PopulateBlueprints( XMLNode& blueprintPathsNode, T_BluePrintType )
	{
		auto blueprintPathNode = blueprintPathsNode.first_child();

		while( !blueprintPathNode.empty() )
		{
			auto blueprintPath = GetXMLAttributeAsString( blueprintPathNode, "path","");
			auto rootNode = GetXMLRootNode( blueprintPath );
			auto blueprintNode = rootNode.first_child();

			while( !blueprintNode.empty() )
			{
				T_BluePrintType::CreateOrGetBluePrintNode( blueprintNode );
				blueprintNode=blueprintNode.next_sibling();
			}

			blueprintPathNode=blueprintPathNode.next_sibling();
		}
	}
}