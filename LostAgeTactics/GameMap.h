/********************************************************************
	author:			Abhinav Choudhary
	filename: 		GameMap.h
	description:	Responsible for generating the map from xml data.
					Also contains all map specific data/vbos.
*********************************************************************/
#pragma once
#include "MapTile.h"
#include "GameMaterial.h"
#include "VBOFactory.h"
#include "MapBlueprint.h"
#include "CharacterBlueprint.h"

namespace ACN
{
	enum Directions{
		DIR_NORTH,
		DIR_SOUTH,
		DIR_EAST,
		DIR_WEST,
		DIR_NORTHEAST,
		DIR_NORTHWEST,
		DIR_SOUTHEAST,
		DIR_SOUTHWEST
	};

	static const int X_WIDTH = 50;
	static const int Y_HEIGHT = 50;

	//Simple 3D info regarding hero's start
	struct HeroStartInfo
	{
		CharacterInfo m_charInfo;
		Vec3 m_worldPos;
	};

	class GameMap
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		///Initializers & Constructor/Destructor
		//////////////////////////////////////////////////////////////////////////
		
		GameMap();
		void CreateGameMap( const std::string& bluePrintName );
		void CreateGameMap( MapBluePrint* MapBluePrint );
		void CreateGameMap( const XMLNode& bluePrintNode );		

		//////////////////////////////////////////////////////////////////////////
		///Accessors
		//////////////////////////////////////////////////////////////////////////

		inline MapTile& GetMapTile( int x, int y );
		inline int GetIndex( const TileCoords& tileCoords );
		inline int GetIndex( int x, int y );
		inline MapTile& GetMapTileAtIndex( int index );
		inline MapTile& GetNeighbourMapTile( MapTile& closedTile, Directions direction );
		inline Vec3 GetWorldPosAtIndex( int index );
		
		ACN::TileCoords& GetSquareSelected() { return m_squareSelected; }
		
		//////////////////////////////////////////////////////////////////////////
		///Frame Updates
		//////////////////////////////////////////////////////////////////////////

		void UpdateSimulation( float deltaTime );
		void UpdateDisplay();

		//////////////////////////////////////////////////////////////////////////
		///Public Functions
		//////////////////////////////////////////////////////////////////////////

		std::vector< HeroStartInfo > GetHeroStartInfoForCurrentMap();
		bool IsSquareSelected() { return m_isSquareSelected; }
		
	private:
		//////////////////////////////////////////////////////////////////////////
		///Data
		//////////////////////////////////////////////////////////////////////////

		//The material used by the tiles
		GameMaterial m_tileMat;

		//Mapping of string hash based on terrain name to corresponding VBO
		std::map< unsigned int /*terrainHash*/, VBOObject* > m_mapOfTerrainHashToVbos;
		
		//The game's map tiles
		MapTile m_mapTiles[ X_WIDTH * Y_HEIGHT ];
		
		int m_width;
		int m_height;
		
		//Basic info regarding the map, generated from XML
		MapInfo m_mapInfo;

		//Mapping of corresponding tile index to character names		
		std::map< int , std::string > m_mapOfTileIndexToCharacterNames;
		
		bool m_isSquareSelected;
		TileCoords m_squareSelected;

		//////////////////////////////////////////////////////////////////////////
		///Private Functions
		//////////////////////////////////////////////////////////////////////////
				
		//Generates the map from xml data
		void GenerateMap();

		//Setup tiles for new map
		void SetupMapTiles();	

		//Initializes the shaders to be used by the map
		void InitializeMaterial();

		//Clears pre existing vbos incase they already exist
		void ClearMapTerrainVBO();

		//Generates the appropriate VBOs
		void GenerateMapTerrainVBOs();

		//Adds the particular tile to the list of VBOs
		void AddToVBO( MapTile& param1 );

		//Gets the appropriate wall for the particular maptile
		VBOObject* GetWallVBOForMapTile( MapTile& mapTile );

		//Adds a particular wall to the existing material list
		void AddWallToMaterialList( const std::string& wallName, unsigned int wallHash );
		
		//Generates appropriate borders, so the map looks complete
		void AddBordersToVBO( VBOObject* currentVBO, MapTile& mapTile );
		
		//Ray interesection checks for Unit selection
		void DoRayIntersection();
		bool DidRayHitTile( Vec3 cameraPos, Vec3 rayDir, float elevation );		
	};

	//////////////////////////////////////////////////////////////////////////
	//Inline Definitions
	//////////////////////////////////////////////////////////////////////////

	inline MapTile& GameMap::GetNeighbourMapTile( MapTile& closedTile, Directions direction )
	{
		TileCoords tileCoord = closedTile.m_tileCoords;

		switch ( direction )
		{
		case DIR_NORTH:
			return GetMapTile( tileCoord.x, tileCoord.y +1 );
			break;
		case DIR_SOUTH:
			return GetMapTile( tileCoord.x , tileCoord.y -1 );
			break;
		case DIR_EAST:
			return GetMapTile( tileCoord.x + 1, tileCoord.y );
			break;
		case DIR_WEST:
			return GetMapTile( tileCoord.x - 1, tileCoord.y );
			break;
		case DIR_NORTHEAST:
			return GetMapTile( tileCoord.x + 1, tileCoord.y +1 );
			break;
		case DIR_NORTHWEST:
			return GetMapTile( tileCoord.x - 1, tileCoord.y +1 );
			break;
		case DIR_SOUTHEAST:
			return GetMapTile( tileCoord.x + 1, tileCoord.y -1 );
			break;
		case DIR_SOUTHWEST:
			return GetMapTile( tileCoord.x - 1, tileCoord.y -1 );
			break;
		}
		return closedTile;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	inline MapTile& GameMap::GetMapTileAtIndex( int index )
	{
		return m_mapTiles[ index ];
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	inline int GameMap::GetIndex( int x, int y )
	{
		return ( ( y*m_width ) + x );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	inline int GameMap::GetIndex( const TileCoords& tileCoords )
	{
		return GetIndex( tileCoords.x, tileCoords.y );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	inline MapTile& GameMap::GetMapTile( int x, int y )
	{
		x = clamp( x, 0, m_width -1 );
		y = clamp( y, 0, m_height-1 );
		int index = y * m_width + x;
		return m_mapTiles[ index ];
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	inline Vec3 GameMap::GetWorldPosAtIndex( int index )
	{
		return m_mapTiles[ index ].m_pos;
	}
}