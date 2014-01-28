/********************************************************************
	author:			Abhinav Choudhary
	filename: 		MapTile.h
	description:	Holds map tile info. Also contains path finding variables
*********************************************************************/
#pragma once
#include "vector2d.h"
#include "vector3d.h"
#include "TerrainBlueprint.h"

namespace ACN
{
	struct PathFindingVariables
	{
		//Path finding variables Modified by A star
		float f;
		float g;
		float h;

		int m_parentTileIndex;

		int m_currentClosedListCounter;
		int m_currentOpenListCounter;

		//Default constructor
		PathFindingVariables()
		{
			m_currentClosedListCounter = -999;
			m_currentOpenListCounter = -999;
		}
	};

	typedef Vector2D<int> TileCoords;
	
	struct MapTile
	{
		//Used by A star and important map data
		TileCoords m_tileCoords;

		//Pos for fast movement for players
		Vec3 m_pos;
		
		//Elevation for 
		int m_elevation;

		//Terrain Hash
		unsigned int m_terrainHash;		
		//TerrainInfo
		TerrainInfo m_terrainInfo;

		//Index inside map
		int m_index;

		//Mutex for multi threading purposes
		ACNFastMutex m_pathFindingVariablesMutes;

		//Path Finding Helpers
		std::map< ThreadID, PathFindingVariables > m_mapOfThreadIDToPathfindingVariables; 		

		//Special constructor
		MapTile( TileCoords& newTileCoords, int elevation, unsigned int terrainHash )			
		{			
			m_tileCoords = newTileCoords;
			m_pos.x = m_tileCoords.x + 0.5f;
			m_pos.y = m_tileCoords.y + 0.5f;
			m_pos.z = (float)elevation;
			m_elevation = elevation;
			m_terrainHash = terrainHash;
		}

		//Default constructor
		MapTile()
			: m_tileCoords()
			, m_pos()
			, m_elevation(0)
			, m_index(-1)
		{			
		}

		//Helper Functions
		PathFindingVariables GetPathFindingVariablesForCurrentThread()
		{			
			LockGuardFast guard( m_pathFindingVariablesMutes );
			return m_mapOfThreadIDToPathfindingVariables[ GetThisThreadID() ];
		}

		void SetPathFindingVariablesForCurrentThread( const PathFindingVariables& val )
		{
			LockGuardFast guard( m_pathFindingVariablesMutes );
			m_mapOfThreadIDToPathfindingVariables[ GetThisThreadID() ] = val;
		}
	};
}