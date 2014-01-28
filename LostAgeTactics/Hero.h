/********************************************************************
	author:			Abhinav Choudhary
	filename: 		Hero.h
	description:	The main class for each hero. 
*********************************************************************/
#pragma once
#include "BaseWorldItem.h"
#include "HeroClassBlueprint.h"
#include "CharacterBlueprint.h"
#include "MapTile.h"
#include "ModelFactory.h"
#include "GameMap.h"
#include "Clock.h"
#include "StateMachineUser.h"

namespace ACN
{
	const std::string G_ORDER_BEGIN = "Begin Turn";
	const std::string G_ORDER_END	= "End Turn";
	const std::string G_ORDER_MOVE	= "Move";
	const std::string G_ORDER_TRYBEST	= "Try Best";
	const std::string G_ORDER_HOLD	= "Hold";
	
	//Simple order. Using a string instead of enum
	struct Order
	{
		TileCoords m_orderTargetCoord;
		std::string m_orderName;
	};

	class Hero: public StateMachineUser< Hero >
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		///Initializers & Constructor/Destructor
		//////////////////////////////////////////////////////////////////////////
		
		Hero();				
		Hero( const CharacterInfo& charInfo, float baseLvl = 0 );
		virtual ~Hero();
		void GenerateHeroData( const CharacterInfo& charInfo, float baseLvl );

		//////////////////////////////////////////////////////////////////////////
		///Frame Updates
		//////////////////////////////////////////////////////////////////////////
		
		virtual void UpdateInput();
		virtual void UpdateSimulation( float deltaTime );
		virtual void UpdateDisplay();
		
		//////////////////////////////////////////////////////////////////////////
		///Accessors
		//////////////////////////////////////////////////////////////////////////
		
		inline int GetID();
		
		inline ACN::TileCoords& GetCurrentTileCoords() { return m_currentTileCoords; }
		inline Vec3 GetWorldPos() const;
		inline int GetTileIndex() { return m_ptrToGameMap->GetIndex( m_currentTileCoords.x, m_currentTileCoords.y ); }

		inline float GetCurrentLvl() const;
		inline Color GetTeamColor() const { return m_teamColor; }
		inline bool WantsToDie() const { return m_currentHP<=0; }
		inline int GetControllerNum() { return m_characterInfo.m_controllerNum; }
		inline int GetMovement() { return m_heroClassInfo.m_movement; }
		inline Order GetTopMostGlobalOrder();
		inline bool HasGlobalOrder() { return m_queOfGlobalOrders.size()>0; }

		inline GameMap* GetPtrToGameMap() { return m_ptrToGameMap; }			
		inline float GetTimer() { return m_myTimer; }	
		inline ACN::VecOfAbilities& GetVecOfAvaiableAbilities() { return m_vecOfAvaiableAbilities; }
	
		//////////////////////////////////////////////////////////////////////////
		///Mutators
		//////////////////////////////////////////////////////////////////////////

		inline void SetWorldPos(Vec3 val);
		void SetCurrentTileCoords(const ACN::TileCoords& val) { m_currentTileCoords = val; }

		inline void SetCurrentLvl(float val);
		inline void SetTeamColor(Color val) { m_teamColor = val; }			
		
		inline void AddNewGlobalOrderToFront( Order newOrder );
		inline void AddNewGlobalOrderToBack( Order newOrder );
		inline void PopTopMostOrder() { m_queOfGlobalOrders.pop_front();}
		inline void ClearAllGlobalOrders();		
		
		inline void SetPtrToGameMap( GameMap* val) { m_ptrToGameMap = val; }
		void SetTimer(const float& val) { m_myTimer = val; }

		//////////////////////////////////////////////////////////////////////////
		///Events
		//////////////////////////////////////////////////////////////////////////
		
		void BeginTurn();
		void EndTurn();
		
		void PerformGlobalAction( NamedProperties& params );
		void TestGlobalAction( NamedProperties& params );

		void RegenAndCheckLvlUp();
	private:
		//////////////////////////////////////////////////////////////////////////
		///Data
		//////////////////////////////////////////////////////////////////////////

		//Hero Clock
		Clock m_heroClock;

		//Turn related
		bool m_isHeroTurn;

		//Health related
		float m_maxHP;
		float m_currentHP;

		float m_maxMP;
		float m_currentMP;

		float m_mpRegenPerTurn;
		float m_hpRegenPerTurn;

		//Ptr To GameMap
		GameMap* m_ptrToGameMap;		

		//Stats related
		Stats m_currentLevelStats;
		CharacterInfo m_characterInfo;
		HeroClassInfo m_heroClassInfo;

		//Lvl related
		float m_currentLvl;
		float m_currentExp;
		float m_expLimitForNextLvl;

		//World pos related
		Vec3 m_currentWorldPos;

		//Movement related
		bool m_isMoving;
		float m_movementSpeed;
		Vec3 m_targetWorldPos;

		//Team Related
		Color m_teamColor;
		
		//Map related
		TileCoords m_currentTileCoords;
		TileCoords m_previousTileCoords;

		//Target related
		TileCoords m_targetTileCoords;

		std::string m_targetAbilityName;

		VecOfAbilities m_vecOfAvaiableAbilities;
		
		//Rendering Related
		StaticModel* m_ptrToHeroModel;
		
		//Every entity has a unique identifying number
		int heroID;

		//Order Related
		std::list< Order > m_queOfGlobalOrders;

		//This value is updated
		static int validIDCounter;
		
		//Timer Related
		float m_myTimer;

		//U.I. on top related
		std::string m_textMsg;
		bool m_bShouldDisplayAttack;

		//////////////////////////////////////////////////////////////////////////
		///Private Functions
		//////////////////////////////////////////////////////////////////////////
	
		inline void SetID(int val);
		inline void SetCurrentCoordFromWorldPos();
		inline void SetTargetPosFromTargetCoord();
		
		//Reg/Un reg to database
		void RegisterHero();
		void UnRegisterHero();
		
		void LvlUpHeroToLvl( float newLvl );
		
		//Rendering related
		void RenderAllPossiblePaths( const Vec4& color = Vec4( 0.5f, 1.0f, 0.5f, 0.5f ) );
		void RenderHeroHealthAndMana();
		void RenderHeroInformation();
		void RenderFontStuff( std::stringstream& ss, float xPos, float yPos, Vector3F& color );
		void DisplayTextMsg();
		void DisplayAttackInformation();
		void RenderHeroName();

		//Hero action related
		void MoveToTargetPosition( float deltaTime );
		float PerformActionOnHero( int attackType, int controllerNum, float damage, const std::string& element, float level );
		float TestActionOnHero( int attackType, int controllerNum, float damage, const std::string& element, float level );
		
		//////////////////////////////////////////////////////////////////////////
		///Friend Classes
		//////////////////////////////////////////////////////////////////////////
		
		friend class HeroStateGlobal;
		friend class HeroStateIdle;
		friend class HeroStateMove;
		friend class HeroStateTryBest;
		friend class HeroStateUseAbility;
		friend class HumanController;
	};

	//////////////////////////////////////////////////////////////////////////
	///Inline functions definitions
	//////////////////////////////////////////////////////////////////////////

	void Hero::SetID( int val )
	{
		heroID = val;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	int Hero::GetID()
	{
		return heroID;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	Vec3 Hero::GetWorldPos() const
	{
		return m_currentWorldPos;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::SetWorldPos( Vec3 val )
	{
		m_currentWorldPos = val;
		SetCurrentCoordFromWorldPos();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	float Hero::GetCurrentLvl() const
	{
		return m_currentLvl;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::SetCurrentLvl( float val )
	{
		m_currentLvl = val;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	inline Order Hero::GetTopMostGlobalOrder()
	{
		Order orderToReturn;

		if ( m_queOfGlobalOrders.size()>0)
		{
			orderToReturn = m_queOfGlobalOrders.front();
		}
		
		return orderToReturn;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::ClearAllGlobalOrders()
	{
		m_queOfGlobalOrders.clear();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	inline void Hero::AddNewGlobalOrderToFront( Order newOrder )
	{
		m_queOfGlobalOrders.push_front( newOrder );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	inline void Hero::AddNewGlobalOrderToBack( Order newOrder )
	{
		m_queOfGlobalOrders.push_back( newOrder );
	}

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::SetCurrentCoordFromWorldPos()
	{
		m_currentTileCoords( (int)m_currentWorldPos.x, (int)m_currentWorldPos.z );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::SetTargetPosFromTargetCoord()
	{
		MapTile& mapTile = m_ptrToGameMap->GetMapTile( m_targetTileCoords.x, m_targetTileCoords.y );
		m_targetWorldPos = mapTile.m_pos;
	}
}



