/********************************************************************
	author:			Abhinav Choudhary
	filename: 		AStarPathFinder.h
	description:	Path finder using A* modified to work exclusively for 
					LostAgeTactics. MultiThreaded. When paths are needed by multiple heroes,
					each one adds path finding as a required job todo.
					Each job is done in different thread.
*********************************************************************/
#pragma once
#include "GameMap.h"
#include "GameState.h"

namespace ACN
{
	struct OpenNode
	{
		float fValue;
		int index;
		bool operator<( const OpenNode& rhs )const { return fValue > rhs.fValue; } 
	};	

	typedef std::priority_queue<OpenNode> MyHeap;

	class AStarPathFinder : public Singleton< AStarPathFinder>
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		///Initializers & Constructors/Destructor
		//////////////////////////////////////////////////////////////////////////
		
		AStarPathFinder();
		~AStarPathFinder();
		
		//////////////////////////////////////////////////////////////////////////
		///Accessors
		//////////////////////////////////////////////////////////////////////////
		
		void CalculateAllPossiblePaths( GameState* ptrToGameState, std::map< int , HeroMovementData>& stateMapOfTileIndexToHeroMovement );
		void CalculatePossiblePaths( NamedProperties& params );
		TileCoords GetBestPossibleTargetPos( TileCoords& currentPos, TileCoords& bestPos, std::map< int , HeroMovementData>& stateMapOfTileIndexToHeroMovement, std::vector< int > possibleMovPos, int controllerNumber, int movement );
		
	private:
		//////////////////////////////////////////////////////////////////////////
		///Data
		//////////////////////////////////////////////////////////////////////////
		
		GameMap* m_ptrToGameMap;
		int m_pathNumber;
		bool m_pathFound;

		//Mutex for multi threading.
		ACNFastMutex m_pathNumberMutex;

		TileCoords baseCoord;
		TileCoords targetCoord;
		
		//////////////////////////////////////////////////////////////////////////
		///Private Functions
		//////////////////////////////////////////////////////////////////////////
		
		void IncrementAndGetPathNumber( int& pathNumberToGet ) { LockGuardFast guard( m_pathNumberMutex ); m_pathNumber++; pathNumberToGet = m_pathNumber; }
		void AddToOpenList( MyHeap& currentOpenNodeBinaryHeap, MapTile& tiletoAdd, float f, float g, float h, int parentIndex, int pathNumber );
		
		inline bool FoundAllPossiblePaths( MyHeap& currentOpenNodeBinaryHeap );
		void FindAllPossiblePaths( const std::map< int , HeroMovementData>& mapOfHeroMovement, std::vector< int >& vecOfClosedListIndicies, MyHeap& currentOpenNodeBinaryHeap, int pathNumber, int maxMovement, int controllerNum );
		bool FoundPath( MyHeap& currentOpenNodeBinaryHeap );
		void FindPath( std::map< int , HeroMovementData>& stateMapOfTileIndexToHeroMovement, MyHeap& openNodeBinaryHeap, int pathNumber, int maxMovement, int controllerNumber );
	};	

	//////////////////////////////////////////////////////////////////////////
	///Template and inline definitions
	//////////////////////////////////////////////////////////////////////////

	inline int GetManhattanDistance( const TileCoords& firstCoord,  const TileCoords& secondCoord )
	{
		return abs( firstCoord.x - secondCoord.x ) + abs( firstCoord.y - secondCoord.y );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	inline bool AStarPathFinder::FoundAllPossiblePaths( MyHeap& currentOpenNodeBinaryHeap )
	{
		unsigned int heapSize = currentOpenNodeBinaryHeap.size();
		return heapSize==0;
	}
}

