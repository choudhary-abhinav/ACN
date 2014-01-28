#include "TacticsGame.h"
#include "Color.h"
#include "TimeFactory.h"
#include "TerrainBlueprint.h"
#include "LocalizedMessageBlueprint.h"
#include "gameutil.h"
#include "ModelBlueprint.h"
#include "CharacterBlueprint.h"
#include "HeroClassBlueprint.h"
#include "AbilityBlueprint.h"
#include "HumanController.h"
#include "AIController.h"

namespace ACN
{
	Color G_teamColor[7];

	TacticsGame::TacticsGame()
		: m_currentTurnControllerID(0)
	{	
		SetDefaultTeamColors();
		Initialize();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::Initialize()
	{
		TimeFactory::GetInstance().ResetElapsedTime();
		m_gameClock.SetParentFromNamedClock("MasterClock");
		m_gameClock.SetAsNamedClock("GameClock");
		
		AddBluePrints();
		MakeCustomMap();
		CreateHumanController();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::UpdateDisplay()
	{
		m_gameMap.UpdateDisplay();

		std::for_each( m_vecOfControllers.begin(), m_vecOfControllers.end(),
			[&] ( Controller* currentController )
		{
			currentController->UpdateDisplay();
		} );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::UpdateSimulation( float deltaTime )
	{
		m_gameMap.UpdateSimulation( deltaTime );
		std::for_each( m_vecOfControllers.begin(), m_vecOfControllers.end(),
			[&] ( Controller* currentController )
		{
			currentController->UpdateSimulation( deltaTime );
		} );

		m_mainGameState.UpdateSimulation( deltaTime );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::UpdateInput()
	{
		if ( m_startNewMap )
		{
			SetupNewGame();
			m_startNewMap = false;
		}

		std::for_each( m_vecOfControllers.begin(), m_vecOfControllers.end(),
			[&] ( Controller* currentController )
		{
			currentController->UpdateInput();
		} );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::AddBluePrints()
	{
		XMLNode rootNode = GetXMLRootNode("data/xml/BluePrints.xml");

		XMLNode terrainsNode = GetNthXMLChildElementWithName( rootNode, "Terrains", 0 );
		PopulateBlueprints( terrainsNode, TerrainBluePrint() );		
		
		XMLNode mapsNode = GetNthXMLChildElementWithName( rootNode, "Maps", 0 );
		PopulateAllMapBlueprints( mapsNode );
		
		XMLNode modelsNode = GetNthXMLChildElementWithName( rootNode, "Models", 0 );		
		PopulateBlueprints( modelsNode, ModelBluePrint() );

		XMLNode wallsNode = GetNthXMLChildElementWithName( rootNode, "Walls", 0 );
		PopulateBlueprints( wallsNode, WallBluePrint() );		

		XMLNode localizedMsgNode = GetNthXMLChildElementWithName( rootNode, "LocalizedMessages", 0 );
		PopulateBlueprints( localizedMsgNode, LocalizedMsgBluePrint() );		

		XMLNode charactersNode = GetNthXMLChildElementWithName( rootNode, "Characters", 0 );
		PopulateBlueprints( charactersNode, CharacterBluePrint() );

		XMLNode classesNode = GetNthXMLChildElementWithName( rootNode, "Classes", 0 );
		PopulateBlueprints( classesNode, HeroClassBluePrint() );

		XMLNode abilitiesNode = GetNthXMLChildElementWithName( rootNode, "Abilities", 0 );
		PopulateBlueprints( abilitiesNode, AbilityBluePrint() );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::MakeCustomMap()
	{
		std::vector<std::string> bluePrintNames = MapBluePrint::GetAllNames();
		int i = RandInRangeIntExclusive( 0, bluePrintNames.size() );

		m_gameMap.CreateGameMap( bluePrintNames[i] );
		m_startNewMap = true;
	}	
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::OnSpecialKeyDown( int keyCode, int mouseX, int mouseY )
	{
		mouseX;
		mouseY;
		keyCode;		
	}	
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::PopulateAllMapBlueprints( const XMLNode& mapsNode )
	{
		auto mapPathNode = mapsNode.first_child();

		while( !mapPathNode.empty() )
		{
			auto mapPath = GetXMLAttributeAsString( mapPathNode, "path","");
			auto rootNode = GetXMLRootNode( mapPath );
			auto currentMapNode = rootNode;

			while( !currentMapNode.empty() )
			{
				MapBluePrint::CreateOrGetBluePrintNode( currentMapNode );

				currentMapNode=currentMapNode.next_sibling();
			}

			mapPathNode=mapPathNode.next_sibling();
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::CreateHumanController()
	{
		Controller* newHumanController = new HumanController();
		newHumanController->SetControllerID(0);
		newHumanController->SetTeamColor( G_teamColor[0] );
		newHumanController->SetPtrToGameMap( &m_gameMap );
		m_vecOfControllers.push_back( newHumanController );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::SetupNewGame()
	{
		m_currentMapBaseLvl = m_vecOfControllers[0]->GetAverageLevel();
		m_currentTurnControllerID = 0;
		
		std::for_each( m_vecOfControllers.begin(), m_vecOfControllers.end(),
			[&] ( Controller* entity )
		{
			entity->ResetController();
		} );

		std::vector< HeroStartInfo > vecOfHeroStartInfo = m_gameMap.GetHeroStartInfoForCurrentMap();

		for ( unsigned int i = 0; i < vecOfHeroStartInfo.size(); ++i )
		{
			auto &currentHeroInfo = vecOfHeroStartInfo[i];
			if ( currentHeroInfo.m_charInfo.m_controllerNum >= (int)m_vecOfControllers.size() )
			{
				AddNewAIController();
			}

			 m_vecOfControllers[ currentHeroInfo.m_charInfo.m_controllerNum ]->AddHero( currentHeroInfo.m_charInfo, currentHeroInfo.m_worldPos, m_currentMapBaseLvl );
		}

		m_vecOfControllers[ 0 ]->StartTurn();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::AddNewAIController()
	{
		Controller* newAIController = new AIController();
		newAIController->SetControllerID( m_vecOfControllers.size() );
		newAIController->SetTeamColor( G_teamColor[ m_vecOfControllers.size() ] );
		newAIController->SetPtrToGameMap( &m_gameMap );
		m_vecOfControllers.push_back( newAIController );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::EndTurn()
	{
		m_currentTurnControllerID++;
		m_currentTurnControllerID = m_currentTurnControllerID>=(int)m_vecOfControllers.size()? 0:m_currentTurnControllerID;
		m_vecOfControllers[ m_currentTurnControllerID ]->StartTurn();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void TacticsGame::SetDefaultTeamColors()
	{
		G_teamColor[0] = Color( 0.0f, 1.0f, 0.0f, 1.0f);
		G_teamColor[1] = Color(  0.0f, 1.0f, 1.0f, 1.0f);
		G_teamColor[2] = Color(  1.0f, 0.0f, 1.0f, 1.0f);
		G_teamColor[3] = Color(  1.0f, 0.0f, 0.0f, 1.0f);
		G_teamColor[4] = Color(  1.0f, 1.0f, 0.0f, 1.0f);
		G_teamColor[5] = Color(  0.0f, 0.0f, 1.0f, 1.0f);
		G_teamColor[6] = Color(  1.0f, 1.0f, 1.0f, 1.0f);
	}
}

