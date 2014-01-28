#include "AStarPathFinder.h"
#include "TacticsGame.h"
#include "JobManager.h"
#include "SpecialJobs.h"

namespace ACN
{
	AStarPathFinder::AStarPathFinder()
		: m_pathNumber()
	{
		m_ptrToGameMap = &(TacticsGame::GetInstance().GetGameMap());
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	AStarPathFinder::~AStarPathFinder()
	{

	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void AStarPathFinder::CalculateAllPossiblePaths( GameState* ptrToGameState, std::map< int , HeroMovementData>& stateMapOfTileIndexToHeroMovement )
	{
		for ( auto i = stateMapOfTileIndexToHeroMovement.begin(); i!=stateMapOfTileIndexToHeroMovement.end(); ++i )
		{
			NamedProperties newParams;
			newParams.SetValue( "mapOfHeroes", stateMapOfTileIndexToHeroMovement );
			newParams.SetValue( "currentHeroIndex", i->first );
			FunctionJob* newJob = new FunctionJob( newParams );
			newJob->RegisterExecuteFunc( *this, &AStarPathFinder::CalculatePossiblePaths );
			newJob->RegisterOnCompleteFunc( *ptrToGameState, &GameState::SetPossiblePaths );
			JobManager::GetInstance().AddJobToQueue(newJob);
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void AStarPathFinder::CalculatePossiblePaths( NamedProperties& params )
	{
		std::map< int , HeroMovementData> mapOfHeroMovement;
		int currentHeroIndex;
		params.GetValue( "mapOfHeroes", mapOfHeroMovement );
		params.GetValue( "currentHeroIndex", currentHeroIndex );
		HeroMovementData& currentHero = mapOfHeroMovement[ currentHeroIndex ];
		params.SetValue( "entityID", currentHero.m_entityID );

		std::vector< int > vecOfClosedListIndicies;
		MyHeap currentOpenNodeBinaryHeap;
		int pathNumber;
		int maxMovement = currentHero.m_movement;
		int controllerNum = currentHero.m_controllerNum;
		IncrementAndGetPathNumber( pathNumber );
		
		MapTile& baseTile = m_ptrToGameMap->GetMapTileAtIndex( currentHeroIndex );
		float g = 0.0f;
		float h = 0.0f;
		float f = g + h;

		AddToOpenList( currentOpenNodeBinaryHeap, baseTile, f, g, h, -1, pathNumber );

		//Check for breakConditions
		//Open List Empty		
		while ( !FoundAllPossiblePaths( currentOpenNodeBinaryHeap) )
		{
			FindAllPossiblePaths( mapOfHeroMovement, vecOfClosedListIndicies, currentOpenNodeBinaryHeap, pathNumber, maxMovement, controllerNum );
		}
		
		for ( auto i = mapOfHeroMovement.begin(); i!= mapOfHeroMovement.end(); ++i )
		{
			if ( i->second.m_controllerNum == controllerNum )
			{
				int index = i->first;

				for ( unsigned int z = 0; z<vecOfClosedListIndicies.size(); ++z )
				{
					if ( vecOfClosedListIndicies[z]==index )
					{
						vecOfClosedListIndicies[z]=vecOfClosedListIndicies.back();
						vecOfClosedListIndicies.pop_back();
						break;
					}
				}				
			}
		}

		params.SetValue("vecOfPossiblePos", vecOfClosedListIndicies );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void AStarPathFinder::AddToOpenList( MyHeap& currentOpenNodeBinaryHeap, MapTile& tiletoAdd, float f, float g, float h, int parentIndex, int pathNumber )
	{
		PathFindingVariables currTileVar = tiletoAdd.GetPathFindingVariablesForCurrentThread();
		
		//Check if it is already in closed list
		if ( currTileVar.m_currentClosedListCounter == pathNumber )
		{
			return;
		}

		//Check if it is already in open list
		if ( currTileVar.m_currentOpenListCounter == pathNumber )
		{
			//Check if the f to be added to the open list is greater than the one already present
			if ( currTileVar.f <= f )
			{
				//We do not want to add anything to the open list
				return;
			}
			//Incase the new f has less value, i.e. it is better, we r still going to add it to the list as adding is a log(n) operation while replacing is an O(N) operation
		}

		currTileVar.f = f;
		currTileVar.g = g;
		currTileVar.h = h;
		currTileVar.m_parentTileIndex = parentIndex;
		currTileVar.m_currentOpenListCounter = pathNumber;

		tiletoAdd.SetPathFindingVariablesForCurrentThread( currTileVar );

		OpenNode newNode;
		newNode.fValue = f;
		newNode.index = tiletoAdd.m_index;
		currentOpenNodeBinaryHeap.push( newNode );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void AStarPathFinder::FindAllPossiblePaths( const std::map< int , HeroMovementData>& mapOfHeroMovement, std::vector< int >& vecOfClosedListIndicies, MyHeap& currentOpenNodeBinaryHeap, int pathNumber, int maxMovement, int controllerNum )
	{
		//Remove From Open and add to closed
		OpenNode node = currentOpenNodeBinaryHeap.top();
		currentOpenNodeBinaryHeap.pop();

		MapTile& closedTile = m_ptrToGameMap->GetMapTileAtIndex( node.index );
		PathFindingVariables currTileVar = closedTile.GetPathFindingVariablesForCurrentThread();
		
		//Check if this node was already added to the closed list
		if ( currTileVar.m_currentClosedListCounter == pathNumber )
		{
			//Meaning it was already evaluated, so return
			return;
		}

		currTileVar.m_currentClosedListCounter = pathNumber;
		closedTile.SetPathFindingVariablesForCurrentThread( currTileVar );

		vecOfClosedListIndicies.push_back( closedTile.m_index );

		// Add all sides to open list
		for ( int direction = (int)DIR_NORTH; direction<= (int)DIR_WEST; ++direction )
		{
			MapTile& sideTile = m_ptrToGameMap->GetNeighbourMapTile( closedTile, (Directions)direction );
			float h = 0;
			float g = currTileVar.g + 1;

			if ( sideTile.m_terrainInfo.m_collision!=0 )
			{
				continue;
			}

			if ( sideTile.m_elevation > closedTile.m_elevation && ( (closedTile.m_elevation - sideTile.m_elevation) == -1 ) )
			{
				g+=1;
			}

			if ((closedTile.m_elevation - sideTile.m_elevation)>1)
			{
				continue;
			}

			float f = g + h;
			
			if ( g < maxMovement )
			{
				auto found = mapOfHeroMovement.find( sideTile.m_index);
				if ( found==mapOfHeroMovement.end() )
				{
					AddToOpenList( currentOpenNodeBinaryHeap, sideTile, f, g, h, closedTile.m_index, pathNumber );
				}				
				else
				{
					if ( found->second.m_controllerNum==controllerNum )
					{
						AddToOpenList( currentOpenNodeBinaryHeap, sideTile, f, g, h, closedTile.m_index, pathNumber );
					}
				}
			}
		}		
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	ACN::TileCoords AStarPathFinder::GetBestPossibleTargetPos( TileCoords& currentPos, TileCoords& bestPos, std::map< int , HeroMovementData>& stateMapOfTileIndexToHeroMovement, std::vector< int > possibleMovPos, int controllerNumber, int movement )
	{
		int pathNumber;
		IncrementAndGetPathNumber( pathNumber );
		MapTile& myCurrentTile = m_ptrToGameMap->GetMapTile( currentPos.x, currentPos.y );

		//possibleMovPos.push_back( myCurrentTile.m_index);
		MyHeap openNodeBinaryHeap;
		for ( unsigned int i = 0; i < possibleMovPos.size(); ++i )
		{
			MapTile& targetMapTile = m_ptrToGameMap->GetMapTileAtIndex( possibleMovPos[i] );
			PathFindingVariables currTargetTileVar = targetMapTile.GetPathFindingVariablesForCurrentThread();
			
			if ( currTargetTileVar.m_currentClosedListCounter == pathNumber )
			{
				//Already calculated g for this guy, so no more need
			}
			else
			{
				m_pathFound = false;
				baseCoord.x = (int)bestPos.x;
				baseCoord.y = (int)bestPos.y;
				targetCoord.x = (int)targetMapTile.m_tileCoords.x;
				targetCoord.y = (int)targetMapTile.m_tileCoords.y;

				MapTile& baseTile = m_ptrToGameMap->GetMapTile(baseCoord.x , baseCoord.y );
				float g = 0.0f;
				float h = (float)GetManhattanDistance( baseCoord, targetCoord );
				float f = g + h;				

				AddToOpenList( openNodeBinaryHeap, baseTile, f, g, h, -1, pathNumber );

				while ( !FoundPath( openNodeBinaryHeap) )
				{
					FindPath( stateMapOfTileIndexToHeroMovement, openNodeBinaryHeap, pathNumber, movement, controllerNumber );
				}
			}
		}

		TileCoords returnCoord = currentPos;		
		float minG;

		if (myCurrentTile.GetPathFindingVariablesForCurrentThread().m_currentClosedListCounter == pathNumber)
		{
			minG = myCurrentTile.GetPathFindingVariablesForCurrentThread().g;
		}
		else
		{
			minG = 10000.0f;
		}
		
		for ( unsigned int i = 0; i < possibleMovPos.size(); ++i )
		{
			MapTile& targetMapTile = m_ptrToGameMap->GetMapTileAtIndex( possibleMovPos[i] );
			PathFindingVariables currTargetTileVar = targetMapTile.GetPathFindingVariablesForCurrentThread();
			if ( currTargetTileVar.m_currentClosedListCounter == pathNumber )
			{
				if ( currTargetTileVar.g<=minG)
				{
					minG = currTargetTileVar.g;
					returnCoord = targetMapTile.m_tileCoords;
				}
			}
		}

		return returnCoord;
	}
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	bool AStarPathFinder::FoundPath( MyHeap& currentOpenNodeBinaryHeap )
	{
		unsigned int heapSize = currentOpenNodeBinaryHeap.size();
		if (heapSize==0)
		{
			return true;
		}
		if ( m_pathFound )
		{
			return true;
		}
		return false;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void AStarPathFinder::FindPath( std::map< int , HeroMovementData>& stateMapOfTileIndexToHeroMovement, MyHeap& currentOpenNodeBinaryHeap, int pathNumber, int maxMovement, int controllerNumber )
	{
		controllerNumber;
		maxMovement;
		stateMapOfTileIndexToHeroMovement;
		OpenNode node = currentOpenNodeBinaryHeap.top();
		currentOpenNodeBinaryHeap.pop();

		MapTile& closedTile = m_ptrToGameMap->GetMapTileAtIndex( node.index );
		PathFindingVariables currTileVar = closedTile.GetPathFindingVariablesForCurrentThread();

		//Check if this node was already added to the closed list
		if ( currTileVar.m_currentClosedListCounter == pathNumber )
		{
			//Meaning it was already evaluated, so return
			return;
		}

		currTileVar.m_currentClosedListCounter = pathNumber;
		closedTile.SetPathFindingVariablesForCurrentThread( currTileVar );
		
		for ( int direction = (int)DIR_NORTH; direction<= (int)DIR_WEST; ++direction )
		{
			MapTile& sideTile = m_ptrToGameMap->GetNeighbourMapTile( closedTile, (Directions)direction );
			float h = (float)GetManhattanDistance( sideTile.m_tileCoords, targetCoord );
			float g = currTileVar.g + 1;

			if ( sideTile.m_terrainInfo.m_collision!=0 )
			{
				continue;
			}

			if ( sideTile.m_elevation > closedTile.m_elevation && ( (closedTile.m_elevation - sideTile.m_elevation) == -1 ) )
			{
				g+=1;
			}

			float f = g + h;
			AddToOpenList( currentOpenNodeBinaryHeap, sideTile, f, g, h, closedTile.m_index, pathNumber );				
		}

		if ( closedTile.m_tileCoords == targetCoord )
		{
			//Meaning this is destination, so return
			m_pathFound = true;
			return;
		}
	}
}
