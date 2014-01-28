#include "GameMap.h"
#include "..\Engine\ACNCore\HashGenerator.h"
#include "ModelBlueprint.h"
#include "ModelFactory.h"
#include "ACNgl.h"
#include "ProfilingSystem.h"
#include "camera.h"
#include "ACNGeometry.h"

namespace ACN
{
	const float G_ElevationMultiplier = 0.25f;

	GameMap::GameMap()
	{
		InitializeMaterial();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::InitializeMaterial()
	{
		m_tileMat.CreateMaterial( "data/shaders/lightingShaderGround.v.glsl", "data/shaders/lightingShaderGround.f.glsl" );

		m_isSquareSelected = false;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::CreateGameMap( MapBluePrint* MapBluePrint )
	{
		m_mapInfo = MapBluePrint->GetMapInfo();
		m_width = m_mapInfo.m_width;
		m_height = m_mapInfo.m_height;

		GenerateMap();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::CreateGameMap( const std::string& bluePrintName )
	{
		MapBluePrint* MapBluePrint = MapBluePrint::CreateOrGetBluePrint( bluePrintName );

		if ( !MapBluePrint->IsInitialized())
		{
			ShowErrorWithBreak( "Trying to create GameMap with uninitialized blueprint name");			
			return;
		}

		CreateGameMap( MapBluePrint );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::CreateGameMap( const XMLNode& bluePrintNode )
	{
		MapBluePrint* MapBluePrint = MapBluePrint::CreateOrGetBluePrintNode( bluePrintNode );
		CreateGameMap( MapBluePrint );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::GenerateMap()
	{
		//Clear Previous Map 
		ClearMapTerrainVBO();

		//Clear other stuff
		m_mapOfTileIndexToCharacterNames.clear();
		
		//Setup tiles for new map
		SetupMapTiles();

		GenerateMapTerrainVBOs();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::ClearMapTerrainVBO()
	{
		for ( auto iter = m_mapOfTerrainHashToVbos.begin(); iter!=m_mapOfTerrainHashToVbos.end(); ++iter )
		{
			VBOObject* vbo = iter->second;
			vbo->DeleteVBO();
			delete vbo;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::SetupMapTiles()
	{
		for ( int y = 0; y< m_height; ++y )
		{
			for ( int x = 0; x< m_width; ++x )
			{
				MapTile& currentMapTile = GetMapTile( x, y );
				currentMapTile.m_index = GetIndex( x, y );
				currentMapTile.m_tileCoords.x = x;
				currentMapTile.m_tileCoords.y = y;
				currentMapTile.m_pos.x = (float)x + 0.5f;
				currentMapTile.m_pos.z = (float)y + 0.5f;

				int layerID = m_mapInfo.m_ptrToTileLayer->at( currentMapTile.m_index );
				std::string terranName = m_mapInfo.m_ptrToMapOfLayerIdToTerrainName->at( layerID );
				
				currentMapTile.m_terrainInfo = TerrainBluePrint::CreateOrGetBluePrint( terranName )->GetTerrainInfo();
				currentMapTile.m_terrainHash = GenerateHashForString(terranName);

				int elevationID = m_mapInfo.m_ptrToElevationLayer->at( currentMapTile.m_index );
				currentMapTile.m_elevation = m_mapInfo.m_ptrToMapOfIdToElevation->at( elevationID );
				currentMapTile.m_pos.y = (float)currentMapTile.m_elevation * G_ElevationMultiplier;
				
				int playerStartID = m_mapInfo.m_ptrToPlayerStartLayer->at( currentMapTile.m_index );

				if ( playerStartID > 0 )
				{
					m_mapOfTileIndexToCharacterNames[ currentMapTile.m_index ] =  m_mapInfo.m_ptrToMapOfLayerIdToTerrainName->at( playerStartID );
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::GenerateMapTerrainVBOs()
	{
		for ( int y = 0; y< m_height; ++y )
		{
			for ( int x = 0; x< m_width; ++x )
			{
				AddToVBO( GetMapTile( x , y ) );
			}
		}

		for ( auto i = m_mapOfTerrainHashToVbos.begin(); i!= m_mapOfTerrainHashToVbos.end(); ++i )
		{
			VBOObject* vbo = i->second;
			vbo->CreateVBO();
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::AddToVBO( MapTile& mapTile )
	{		
		TileCoords& tileCoord = mapTile.m_tileCoords;
		VBOObject* currentVBO = GetWallVBOForMapTile( mapTile );
	
		//If wall Hash does not exist, add new vbo
		Vertex vertex[] = {	{ (float)tileCoord.x		, (float)mapTile.m_elevation * G_ElevationMultiplier , (float)tileCoord.y,  0.0f,   0.0f},
							{ (float)tileCoord.x + 1	, (float)mapTile.m_elevation * G_ElevationMultiplier , (float)tileCoord.y,  1.0f,   0.0f},
							{ (float)tileCoord.x		, (float)mapTile.m_elevation * G_ElevationMultiplier , (float)tileCoord.y+1,  0.0f,   1.0f},
							{ (float)tileCoord.x + 1	, (float)mapTile.m_elevation * G_ElevationMultiplier , (float)tileCoord.y+1,  1.0f,   1.0f} };
			
		currentVBO->AddQuad( vertex[0],vertex[1], vertex[2], vertex[3] );

		//Add borders for all neighbours		
		AddBordersToVBO( currentVBO, mapTile );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	VBOObject* GameMap::GetWallVBOForMapTile( MapTile& mapTile )
	{
		std::string& wallName = mapTile.m_terrainInfo.m_wallName;
		unsigned int wallHash = GenerateHashForString( wallName );
		auto found = m_mapOfTerrainHashToVbos.find( wallHash );
		
		if ( found != m_mapOfTerrainHashToVbos.end() )
		{
			return found->second;
		}

		VBOObject* vboToReturn; 
		vboToReturn = new VBOObject;
		vboToReturn->AttachMaterial( m_tileMat );
		m_mapOfTerrainHashToVbos[wallHash] = vboToReturn;
	
		AddWallToMaterialList( wallName, wallHash );
		return vboToReturn;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::AddWallToMaterialList( const std::string& wallName, unsigned int wallHash )
	{
		WallInfo wallInfo = WallBluePrint::CreateOrGetBluePrint( wallName )->GetWallInfo();
		m_tileMat.AddMaterialInfo( wallHash, *( wallInfo.m_ptrToTexturePaths ) );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::UpdateDisplay()
	{
		//Draw the tiles
		//ProfileSection R1("MapRender");
		{
			//ProfileSection R2("MapRender.Tiles");
			for ( auto i = m_mapOfTerrainHashToVbos.begin(); i!= m_mapOfTerrainHashToVbos.end(); ++i )
			{
				VBOObject* vbo = i->second;

				m_tileMat.UseMaterialForHash( i->first );
				vbo->BindVBO();
				vbo->DrawVBO();
			}
		}

		//Draw the necessary models
		for ( int y = 0; y< m_height; ++y )
		{
			for ( int x = 0; x< m_width; ++x )
			{
				MapTile& currentMapTile = GetMapTile( x, y );
				std::string& modelName = currentMapTile.m_terrainInfo.m_modelName;
				
				if ( modelName != "" )
				{
					const std::string& modelPathName = ModelBluePrint::CreateOrGetBluePrint(modelName)->GetPathName();
					
					ACNglPushMatrix();
					ACNglTranslate( currentMapTile.m_pos );
					auto model = StaticModel::CreateOrGetModel(modelPathName);
					model->Render();
					ACNglPopMatrix();
				}
			}
		}

		if ( m_isSquareSelected )
		{
			static ColoredQuad S_coloredQuad( Vec3( 0.0f, 0.0f, 0.0f), Vec3( 1.0f, 0.0f, 0.0f), Vec3( 0.0f, 0.0f, 1.0f), Vec3( 1.0f, 0.0f, 1.0f), Vec4( 1.0f, 1.0f, 1.0f, 0.7f ) );
			MapTile& mapTile = GetMapTile( m_squareSelected.x, m_squareSelected.y );
			
			ACNglPushMatrix();
			ACNglTranslate( mapTile.m_pos.x - 0.5f, mapTile.m_pos.y + 0.02f , mapTile.m_pos.z - 0.5f );
			S_coloredQuad.Draw();
			ACNglPopMatrix();
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::AddBordersToVBO( VBOObject* currentVBO, MapTile& mapTile )
	{
		//TODO : Optimize this. 
		//Border for north
		{
			MapTile& north = GetNeighbourMapTile( mapTile, DIR_NORTH );
			int baseElevation = north.m_elevation;

			if ( north.m_index == mapTile.m_index )
			{
				baseElevation = 0;
			}

			int maxElevation = mapTile.m_elevation;
			if ( baseElevation< maxElevation )
			{
				for( int i = baseElevation; i < maxElevation; ++i )
				{
					TileCoords& tileCoord = mapTile.m_tileCoords;
					Vertex vertex[] = {
						{(float)tileCoord.x			, ((float)i	+ 1.0f)	* G_ElevationMultiplier,   (float)tileCoord.y + 1.0f,  0.0f,   0.0f},
						{(float)tileCoord.x + 1.0f	, ((float)i + 1.0f)	* G_ElevationMultiplier,(float)tileCoord.y + 1.0f,  1.0f,   0.0f},
						{(float)tileCoord.x			, ((float)i	)		* G_ElevationMultiplier,  (float)tileCoord.y + 1.0f,  0.0f,   1.0f},
						{(float)tileCoord.x + 1.0f	, ((float)i	)		* G_ElevationMultiplier,  (float)tileCoord.y + 1.0f,  1.0f,   1.0f} };

						currentVBO->AddQuad( vertex[0], vertex[1], vertex[2], vertex[3] );
				}
			}
		}

		{
			MapTile& south = GetNeighbourMapTile( mapTile, DIR_SOUTH );

			int baseElevation = south.m_elevation;
			int maxElevation = mapTile.m_elevation;
			if ( south.m_index == mapTile.m_index )
			{
				baseElevation = 0;
			}

			if ( baseElevation< maxElevation )
			{				
				for( int i = baseElevation; i < maxElevation; ++i )
				{
					TileCoords& tileCoord = mapTile.m_tileCoords;
					Vertex vertex[] = {
						{(float)tileCoord.x	+ 1.0f	, ((float)i + 1.0f)* G_ElevationMultiplier	,  (float)tileCoord.y,  0.0f,   0.0f},
						{(float)tileCoord.x			, ((float)i + 1.0f)* G_ElevationMultiplier	,  (float)tileCoord.y,  1.0f,   0.0f},
						{(float)tileCoord.x	+ 1.0f	, ((float)i)		* G_ElevationMultiplier	,  (float)tileCoord.y,  0.0f,   1.0f},
						{(float)tileCoord.x 		, ((float)i)		* G_ElevationMultiplier	,  (float)tileCoord.y,  1.0f,   1.0f} };

						currentVBO->AddQuad( vertex[0], vertex[1], vertex[2], vertex[3] );
				}
			}
		}

		{
			MapTile& east = GetNeighbourMapTile( mapTile, DIR_EAST );

			int baseElevation = east.m_elevation;
			int maxElevation = mapTile.m_elevation;

			if ( east.m_index == mapTile.m_index )
			{
				baseElevation = 0;
			}

			if ( baseElevation< maxElevation  )
			{				
				for( int i = baseElevation; i < maxElevation; ++i )
				{
					TileCoords& tileCoord = mapTile.m_tileCoords;
					Vertex vertex[] = {
						{(float)tileCoord.x	+ 1.0f	, ((float)i + 1)* G_ElevationMultiplier		, 1 + (float)tileCoord.y,  0.0f,   0.0f},
						{(float)tileCoord.x + 1.0f	, ((float)i	+ 1)* G_ElevationMultiplier	,  (float)tileCoord.y,  1.0f,   0.0f},
						{(float)tileCoord.x	+ 1.0f	, ((float)i)* G_ElevationMultiplier,  1 + (float)tileCoord.y,  0.0f,   1.0f},
						{(float)tileCoord.x + 1.0f	, ((float)i)* G_ElevationMultiplier,  (float)tileCoord.y,  1.0f,   1.0f} };

						currentVBO->AddQuad( vertex[0], vertex[1], vertex[2], vertex[3] );
				}
			}
		}

		{
			MapTile& west = GetNeighbourMapTile( mapTile, DIR_WEST );
			int baseElevation = west.m_elevation;
			int maxElevation = mapTile.m_elevation;

			if ( west.m_index == mapTile.m_index )
			{
				baseElevation = 0;
			}

			if ( baseElevation< maxElevation  )
			{
				int baseElevation = west.m_elevation;
				int maxElevation = mapTile.m_elevation;

				if ( west.m_index == mapTile.m_index )
				{
					baseElevation = 0;
				}

				for( int i = baseElevation; i < maxElevation; ++i )
				{
					TileCoords& tileCoord = mapTile.m_tileCoords;
					Vertex vertex[] = {
						{(float)tileCoord.x	, ((float)i	+ 1)* G_ElevationMultiplier	,(float)tileCoord.y,  0.0f,   0.0f},
						{(float)tileCoord.x	, ((float)i	+ 1)* G_ElevationMultiplier	, 1 + (float)tileCoord.y,  1.0f,   0.0f},
						{(float)tileCoord.x	, ((float)i)* G_ElevationMultiplier,  (float)tileCoord.y,  0.0f,   1.0f},
						{(float)tileCoord.x	, ((float)i)* G_ElevationMultiplier, 1 + (float)tileCoord.y,  1.0f,   1.0f} };

						currentVBO->AddQuad( vertex[0], vertex[1], vertex[2], vertex[3] );
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	std::vector< HeroStartInfo > GameMap::GetHeroStartInfoForCurrentMap()
	{
		std::vector< HeroStartInfo > vecOfHeroStartInfo;
		
		for ( auto i = m_mapOfTileIndexToCharacterNames.begin(); i!= m_mapOfTileIndexToCharacterNames.end(); ++i )
		{
			HeroStartInfo tempHeroStartInfo;
			int index = i->first;
			std::string& charName = i->second;
			tempHeroStartInfo.m_worldPos = GetWorldPosAtIndex( index );
			tempHeroStartInfo.m_charInfo = CharacterBluePrint::CreateOrGetBluePrint( charName )->GetCharacterInfo();
			vecOfHeroStartInfo.push_back( tempHeroStartInfo );
		}

		return vecOfHeroStartInfo;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::UpdateSimulation( float deltaTime )
	{
		deltaTime;
		m_isSquareSelected = false;
		//Check for ray intersection and tell if it interescted with the thing
		DoRayIntersection();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void GameMap::DoRayIntersection()
	{
		float maxElevation = 20.0f;
		
		Vec3 cameraPos = Camera::GetInstance().position();
		Vec3 rayDir = Camera::GetInstance().GetMouseRayDir();

		for ( float i = maxElevation; i>= 0.0f; --i )
		{
			bool rayFound = DidRayHitTile( cameraPos, rayDir, i * G_ElevationMultiplier );
			
			if ( rayFound )
			{
				m_isSquareSelected = true;
				break;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	bool GameMap::DidRayHitTile( Vec3 cameraPos, Vec3 rayDir, float elevation )
	{
		Vec3 normal = Vec3( 0.0f, 1.0f, 0.0f );
		Vec3 planePos = Vec3( 0.0f, elevation, 0.0f );

		// Check l.n
		float normalDotRay = normal * rayDir;
		
		if ( normalDotRay == 0.0f )
		{
			return false;
		}

		//Basically line interestcs with the plane
		float distance = ( (planePos - cameraPos)*normal )/normalDotRay;
		Vec3 intersection = cameraPos + ( rayDir * distance );

		//Check if interesction lies within the map
		if ( (int)intersection.x >= 0 && (int)intersection.x < m_width && (int)intersection.z >= 0 && (int)intersection.z < m_height )
		{
			MapTile& mapTile = GetMapTile( (int)intersection.x, (int)intersection.z );

			if ( mapTile.m_pos.y == intersection.y )
			{
				m_squareSelected = mapTile.m_tileCoords;
				return true;
			}
		}

		return false;
	}
}

