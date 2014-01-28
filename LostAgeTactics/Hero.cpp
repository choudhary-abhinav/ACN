#include "Hero.h"
#include "HeroDatabase.h"
#include "..\Engine\ACNRenderer\ACNgl.h"
#include "..\Engine\ACNAnimation\ModelFactory.h"
#include "ModelBlueprint.h"
#include "..\Engine\ACNCore\eventsystem.h"
#include "TacticsGame.h"
#include "..\Engine\ACNRenderer\ACNGeometry.h"
#include "HeroStates.h"
#include "AStarPathFinder.h"
#include "fontfactory.h"
#include "AbilityBlueprint.h"

namespace ACN
{
	//////////////////////////////////////////////////////////////////////////
	///Const Balance Definitions
	//////////////////////////////////////////////////////////////////////////	
	const float STR_HP_MODIFIER = 10.0f;
	const float INT_HP_MODIFIER = 0.0f;
	const float ARM_HP_MODIFIER = 20.0f;

	const float STR_MP_MODIFIER = 0.0f;
	const float INT_MP_MODIFIER = 10.0f;
	const float ARM_MP_MODIFIER = 0.0f;

	const float STR_HP_REGEN_MODIFIER = 3.0f;
	const float INT_HP_REGEN_MODIFIER = 0.0f;
	const float ARM_HP_REGEN_MODIFIER = 6.0f;

	const float STR_MP_REGEN_MODIFIER = 0.0f;
	const float INT_MP_REGEN_MODIFIER = 3.0f;
	const float ARM_MP_REGEN_MODIFIER = 0.0f;

	// Ax2 + Bx + C
	const float EXPERIENCE_MODIFIER_A = 10.0f;
	const float EXPERIENCE_MODIFIER_B = 20.0f;
	const float EXPERIENCE_MODIFIER_C = 100.0f;

	Hero::Hero()
		: m_maxHP(0)
		, m_currentHP(0)
		, m_maxMP(0)
		, m_currentMP(0)
		, m_mpRegenPerTurn(0)
		, m_hpRegenPerTurn(0)
		, m_currentLevelStats(0)
		, m_currentLvl(0)
		, m_currentExp(0)
		, m_expLimitForNextLvl(100)
		, m_currentWorldPos(0)
		, m_teamColor()
		, m_currentTileCoords(0,0)
		, m_previousTileCoords(0,0)
		, m_isHeroTurn( false )
		, m_isMoving( false )
		, m_movementSpeed( 4.10f )
		, m_myTimer(3.0f)
		, m_bShouldDisplayAttack( false )
	{
		SetID(validIDCounter++);	
		RegisterHero();
		m_heroClock.SetParentFromNamedClock("GameClock");		
		SetupStateMachine( this, HERO_STATE_IDLE, HERO_STATE_GLOBAL );		
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	Hero::Hero( const CharacterInfo& charInfo, float baseLvl /*= 0 */ )
		: m_maxHP(0)
		, m_currentHP(0)
		, m_maxMP(0)
		, m_currentMP(0)
		, m_mpRegenPerTurn(0)
		, m_hpRegenPerTurn(0)
		, m_currentLevelStats(0)
		, m_currentLvl(0)
		, m_currentExp(0)
		, m_expLimitForNextLvl(100)
		, m_currentWorldPos(0)
		, m_teamColor()
		, m_currentTileCoords(0,0)
		, m_previousTileCoords(0,0)
		, m_isHeroTurn( false )
		, m_isMoving( false )
		, m_movementSpeed( 4.10f )
		, m_myTimer(3.0f)
		, m_bShouldDisplayAttack( false )
	{
		SetID(validIDCounter++);	
		RegisterHero();
		GenerateHeroData( charInfo, baseLvl );
		m_heroClock.SetParentFromNamedClock("GameClock");
		SetupStateMachine( this, HERO_STATE_IDLE, HERO_STATE_GLOBAL );

		G_EventSystem.RegisterObjectForEvent( "PerformGlobalAction", *this, &Hero::PerformGlobalAction );
		G_EventSystem.RegisterObjectForEvent( "TestGlobalAction", *this, &Hero::TestGlobalAction );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	Hero::~Hero()
	{
		UnRegisterHero();
		G_EventSystem.UnRegisterAllForObject( *this );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::GenerateHeroData( const CharacterInfo& charInfo, float baseLvl )
	{
		m_characterInfo = charInfo;
		m_currentLvl = m_characterInfo.m_baseLvl + baseLvl;
		m_heroClassInfo = HeroClassBluePrint::CreateOrGetBluePrint( m_characterInfo.m_className )->GetClassInfo();
		const std::string& modelPathName = ModelBluePrint::CreateOrGetBluePrint( m_characterInfo.m_modelName )->GetPathName();
		m_ptrToHeroModel = StaticModel::CreateOrGetModel( modelPathName );

		m_textMsg = "Hello World";
		LvlUpHeroToLvl( m_currentLvl );
		SetCurrentCoordFromWorldPos();
	}	
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::RegisterHero()
	{
		HeroDatabase::GetInstance().RegisterThisHero( this );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::UnRegisterHero()
	{
		HeroDatabase::GetInstance().UnRegisterThisHero( this );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::UpdateDisplay()
	{
		ACNglPushMatrix();
		ACNglTranslate( m_currentWorldPos );		
		{
			//	ProfileSection R2("Hero.Draw");
			m_ptrToHeroModel->Render();
		}
		ACNglPopMatrix();

		RenderHeroHealthAndMana();
		RenderHeroName();
		
		if ( m_isHeroTurn )
		{
			RenderAllPossiblePaths( Vec4( m_teamColor.r, m_teamColor.g, m_teamColor.b, 0.3f) );
			RenderHeroInformation();
		}
		
		if ( m_myTimer >0.0f )
		{
			DisplayTextMsg();
		}

		if ( m_bShouldDisplayAttack )
		{
			DisplayAttackInformation();
		}		
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::UpdateSimulation( float deltaTime )
	{
		m_myTimer-=deltaTime;
		StateMachineUser::UpdateSimulation( deltaTime );

		if ( m_isMoving )
		{
			MoveToTargetPosition(deltaTime);
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::UpdateInput()
	{
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::LvlUpHeroToLvl( float newLvl )
	{
		m_currentLvl = newLvl;
		m_currentLevelStats = m_heroClassInfo.m_baseStats + m_heroClassInfo.m_perLvlStats * m_currentLvl;

		m_maxHP = ( m_currentLevelStats[ STAT_STRENTGH ]		* STR_HP_MODIFIER )
				+ ( m_currentLevelStats[ STAT_INTELLIGENCE ]	* INT_HP_MODIFIER )
				+ ( m_currentLevelStats[ STAT_ARMOR ]			* ARM_HP_MODIFIER );

		m_maxMP = ( m_currentLevelStats[ STAT_STRENTGH ]		* STR_MP_MODIFIER )
				+ ( m_currentLevelStats[ STAT_INTELLIGENCE ]	* INT_MP_MODIFIER )
				+ ( m_currentLevelStats[ STAT_ARMOR ]			* ARM_MP_MODIFIER );

		m_hpRegenPerTurn	= ( m_currentLevelStats[ STAT_STRENTGH ]		* STR_HP_REGEN_MODIFIER )
							+ ( m_currentLevelStats[ STAT_INTELLIGENCE ]	* INT_HP_REGEN_MODIFIER )
							+ ( m_currentLevelStats[ STAT_ARMOR ]			* ARM_HP_REGEN_MODIFIER );

		m_mpRegenPerTurn	= ( m_currentLevelStats[ STAT_STRENTGH ]		* STR_MP_REGEN_MODIFIER )
							+ ( m_currentLevelStats[ STAT_INTELLIGENCE ]	* INT_MP_REGEN_MODIFIER )
							+ ( m_currentLevelStats[ STAT_ARMOR ]			* ARM_MP_REGEN_MODIFIER );
		
		m_currentHP = m_maxHP;
		m_currentMP = m_maxMP;
		m_currentExp = 0;

		m_expLimitForNextLvl	= ( EXPERIENCE_MODIFIER_A * m_currentLvl * m_currentLvl )
								+ ( EXPERIENCE_MODIFIER_B * m_currentLvl )
								+ ( EXPERIENCE_MODIFIER_C );

		//Make current vec of abilities
		m_vecOfAvaiableAbilities.clear();
	
		for ( auto i = m_heroClassInfo.m_ptrToMapOfLvlReqToVecOfAbilities->begin(); i!=m_heroClassInfo.m_ptrToMapOfLvlReqToVecOfAbilities->end(); ++i )
		{
			if ( i->first <= m_currentLvl )
			{
				m_vecOfAvaiableAbilities.insert( m_vecOfAvaiableAbilities.begin(), i->second.begin(), i->second.end() );
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::EndTurn()
	{
		if ( m_isHeroTurn )
		{
			m_isHeroTurn = false;
			NamedProperties params;
			params.SetValue("ControllerNum", GetControllerNum() );
			G_EventSystem.FireEvent("NotifyControllerHeroTurnEnd", params);
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::BeginTurn()
	{
		m_isHeroTurn = true;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::RenderAllPossiblePaths( const Vec4& color /*= Vec4( ) */ )
	{
		GameState* ptrToMainGameState = &(TacticsGame::GetInstance().GetMainGameState());
		std::vector< int > vecOfPossiblePos = ptrToMainGameState->GetVecOfPossiblePosForEntityID( heroID );

		static ColoredQuad S_coloredQuad( Vec3( 0.0f, 0.0f, 0.0f), Vec3( 1.0f, 0.0f, 0.0f), Vec3( 0.0f, 0.0f, 1.0f), Vec3( 1.0f, 0.0f, 1.0f), Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
		S_coloredQuad.ModifyQuadColor( color );
		
		for ( unsigned int i = 0; i<vecOfPossiblePos.size(); ++i )
		{
			MapTile& currentTile = m_ptrToGameMap->GetMapTileAtIndex( vecOfPossiblePos[i] );
			ACNglPushMatrix();
			ACNglTranslate( currentTile.m_pos.x - 0.5f, currentTile.m_pos.y + 0.01f , currentTile.m_pos.z - 0.5f );
			S_coloredQuad.Draw();
			ACNglPopMatrix();
		}
		
		if ( m_isMoving )
		{
			Vec4 redColor ( 1.0f, 0.0f, 0.0f, 0.5f );
			S_coloredQuad.ModifyQuadColor( redColor );
			ACNglPushMatrix();
			ACNglTranslate( m_targetWorldPos.x - 0.5f, m_targetWorldPos.y + 0.02f , m_targetWorldPos.z - 0.5f );
			S_coloredQuad.Draw();
			ACNglPopMatrix();
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::MoveToTargetPosition( float deltaTime )
	{
		Vec3 travelDistanceVector = m_targetWorldPos - m_currentWorldPos;
		float travelAmountThisFrame = deltaTime * m_movementSpeed;
		float totalTravelDistanceAmountSquared = travelDistanceVector.length_squared();
	
		if ( totalTravelDistanceAmountSquared < travelAmountThisFrame*travelAmountThisFrame )
		{
			// Very close to final position
			m_currentWorldPos = m_targetWorldPos;
			m_isMoving = false;
		}
	
		else
		{
			Vec3 movementDirection = travelDistanceVector.normalized();
			m_currentWorldPos += movementDirection * travelAmountThisFrame;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::RenderHeroHealthAndMana()
	{
		float healthWidth = 0.1f;
		static ColoredQuad S_healthQuad( Vec3( 0.0f, 1.0f, 0.0f), Vec3( 1.0f, 1.0f, 0.0f), Vec3( 0.0f, 0.0f, 0.0f), Vec3( 1.0f, 0.0f, 0.0f), Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
		Vec4 blackColor ( 0.0f, 0.0f, 0.0f, 0.5f );
		Vec4 blueColor ( 0.0f, 0.0f, 1.0f, 0.5f );
		S_healthQuad.ModifyQuadColor( blackColor );
	
		ACNglPushMatrix();
		ACNglTranslate( m_currentWorldPos.x - 0.5f, m_currentWorldPos.y + 1.52f , m_currentWorldPos.z );
		ACNglScale( 0.9f, healthWidth, 1.0f );
		//S_healthQuad.Draw();
		S_healthQuad.ModifyQuadColor( Vec4( m_teamColor.r, m_teamColor.g, m_teamColor.b, 0.5f) );		
		float healthRatio = m_currentHP/ m_maxHP;
		
		ACNglScale( healthRatio, 1.0f, 1.0f );
		S_healthQuad.Draw();
		ACNglPopMatrix();

		ACNglPushMatrix();
		ACNglTranslate( m_currentWorldPos.x - 0.5f, m_currentWorldPos.y + 1.42f , m_currentWorldPos.z );
		ACNglScale( 0.9f, healthWidth, 1.0f );
		S_healthQuad.ModifyQuadColor( blackColor );
		//S_healthQuad.Draw();
		S_healthQuad.ModifyQuadColor( blueColor );
		float mpRatio = m_currentMP/ m_maxMP;
		
		ACNglScale( mpRatio, 1.0f, 1.0f );
		S_healthQuad.Draw();
		ACNglPopMatrix();
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::PerformGlobalAction( NamedProperties& params )
	{
		//Check if you are in range of attack/ heal
		int range;
		TileCoords attackCoords;
		params.GetValue("range", range);
		params.GetValue("attackCoords", attackCoords);

		if ( GetManhattanDistance(attackCoords, m_currentTileCoords) <= range )
		{
			int attackType;
			int controllerNum;
			float damage;
			float level;

			float score;
			std::string element;

			params.GetValue("attackType", attackType );
			params.GetValue("controllerNum", controllerNum );
			params.GetValue("score", score );
			params.GetValue("damage", damage );
			params.GetValue("element", element );
			params.GetValue("level", level );
			float currentScore =  PerformActionOnHero( attackType, controllerNum, damage, element, level );
			
			score+=currentScore;
			params.SetValue("score", score );
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	float Hero::PerformActionOnHero( int attackType, int controllerNum, float damage, const std::string& element, float level )
	{
		float score = 0;
		float scoreRatio = 0.0f;
		float levelMultiplier;
		float elementMultiplier = 1.0f;

		if ( m_heroClassInfo.m_pairOfWeakAndStrongElement.first == element )
		{
			elementMultiplier-=0.5f;
		}

		if ( m_heroClassInfo.m_pairOfWeakAndStrongElement.second == element )
		{
			elementMultiplier+=0.5f;
		}

		levelMultiplier =  ( m_currentLvl - level )*0.25f;
		levelMultiplier = clamp( levelMultiplier, -1.0f, 1.0f );
		levelMultiplier += 1.0f;

		//Its a healing attack
		if ( attackType == 0 && controllerNum == GetControllerNum() )
		{
			float m_currentHPRatio = m_currentHP/m_maxHP;
			float resultHp = m_currentHP + damage;
			resultHp = clamp( resultHp, m_currentHP, m_maxHP );
			float resultHpRatio = resultHp/m_maxHP;

			//Score break is 0.75 for first 50, 0.25 for last 50
			if ( m_currentHPRatio <= 0.5f && resultHpRatio<= 0.5f )
			{
				scoreRatio+= ( resultHpRatio - m_currentHPRatio ) * 0.75f;
			}

			else if ( m_currentHPRatio >= 0.5f && resultHpRatio >= 0.5f )
			{
				scoreRatio+= ( resultHpRatio - m_currentHPRatio ) * 0.25f;
			}

			else 
			{
				scoreRatio+= ( 0.5f - m_currentHPRatio ) * 0.75f;
				scoreRatio+= ( resultHpRatio - 0.5f ) * 0.25f;
			}

			m_currentHP = resultHp;			
			score= scoreRatio*m_expLimitForNextLvl*levelMultiplier;

			if ( score > 0 )
			{
				m_textMsg = "Thanks!";
				SetTimer( 1.0f );
			}

			return score; 
		}

		//If it is a damage attack
		if ( attackType == 1 && controllerNum != GetControllerNum() )
		{
			float m_currentHPRatio = m_currentHP/m_maxHP;
			float resultHp = m_currentHP - ( damage * elementMultiplier);
			resultHp = clamp( resultHp, 0.0f, m_currentHP );
			float resultHpRatio = resultHp/m_maxHP;

			if ( resultHp == 0.0f )
			{
				scoreRatio +=0.25f;
			}

			//Score break is 0.5 for first 50, 0.25 for last 50
			if ( m_currentHPRatio <= 0.5f && resultHpRatio<= 0.5f )
			{
				scoreRatio+= ( m_currentHPRatio - resultHpRatio ) * 0.5f;
			}

			else if ( m_currentHPRatio >= 0.5f && resultHpRatio >= 0.5f )
			{
				scoreRatio+= ( m_currentHPRatio - resultHpRatio ) * 0.25f;
			}

			else 
			{
				scoreRatio+= ( 0.5f -  resultHpRatio) * 0.5f;
				scoreRatio+= ( m_currentHPRatio - 0.5f ) * 0.25f;
			}

			m_currentHP = resultHp;
			score = scoreRatio*m_expLimitForNextLvl*levelMultiplier;

			if ( score > 0 )
			{
				m_textMsg = "OUCH!";
				SetTimer( 1.0f );
			}

			return score;
		}
		return score;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::TestGlobalAction( NamedProperties& params )
	{
		//Check if you are in range of attack/ heal
		int range;
		TileCoords attackCoords;
		params.GetValue("range", range);
		params.GetValue("attackCoords", attackCoords);

		if ( GetManhattanDistance(attackCoords, m_currentTileCoords) <= range )
		{
			int attackType;
			int controllerNum;
			float damage;
			float level;
			float score;
			std::string element;

			params.GetValue("attackType", attackType );
			params.GetValue("controllerNum", controllerNum );
			params.GetValue("score", score );
			params.GetValue("damage", damage );
			params.GetValue("element", element );
			params.GetValue("level", level );
			float currentScore =  TestActionOnHero( attackType, controllerNum, damage, element, level );

			score+=currentScore;
			params.SetValue("score", score );
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	float Hero::TestActionOnHero( int attackType, int controllerNum, float damage, const std::string& element, float level )
	{
		float score = 0;
		float scoreRatio = 0.0f;
		float levelMultiplier;
		float elementMultiplier = 1.0f;

		if ( m_heroClassInfo.m_pairOfWeakAndStrongElement.first == element )
		{
			elementMultiplier-=0.5f;
		}
		if ( m_heroClassInfo.m_pairOfWeakAndStrongElement.second == element )
		{
			elementMultiplier+=0.5f;
		}

		levelMultiplier =  ( m_currentLvl - level )*0.25f;
		levelMultiplier = clamp( levelMultiplier, -1.0f, 1.0f );
		levelMultiplier += 1.0f;
		//Its a healing attack
		if ( attackType == 0 && controllerNum == GetControllerNum() )
		{
			float m_currentHPRatio = m_currentHP/m_maxHP;
			float resultHp = m_currentHP + damage;
			resultHp = clamp( resultHp, m_currentHP, m_maxHP );
			float resultHpRatio = resultHp/m_maxHP;

			//Score break is 0.75 for first 50, 0.25 for last 50
			if ( m_currentHPRatio <= 0.5f && resultHpRatio<= 0.5f )
			{
				scoreRatio+= ( resultHpRatio - m_currentHPRatio ) * 0.75f;
			}

			else if ( m_currentHPRatio >= 0.5f && resultHpRatio >= 0.5f )
			{
				scoreRatio+= ( resultHpRatio - m_currentHPRatio ) * 0.25f;
			}

			else 
			{
				scoreRatio+= ( 0.5f - m_currentHPRatio ) * 0.75f;
				scoreRatio+= ( resultHpRatio - 0.5f ) * 0.25f;
			}

			score = scoreRatio*m_expLimitForNextLvl*levelMultiplier;
			return score;		
		}

		//If it is a damage attack
		if ( attackType == 1 && controllerNum != GetControllerNum() )
		{
			float m_currentHPRatio = m_currentHP/m_maxHP;
			float resultHp = m_currentHP - ( damage * elementMultiplier);
			resultHp = clamp( resultHp, 0.0f, m_currentHP );
			float resultHpRatio = resultHp/m_maxHP;

			if ( resultHp == 0.0f )
			{
				scoreRatio +=0.25f;
			}

			//Score break is 0.5 for first 50, 0.25 for last 50
			if ( m_currentHPRatio <= 0.5f && resultHpRatio<= 0.5f )
			{
				scoreRatio+= ( m_currentHPRatio - resultHpRatio ) * 0.5f;
			}

			else if ( m_currentHPRatio >= 0.5f && resultHpRatio >= 0.5f )
			{
				scoreRatio+= ( m_currentHPRatio - resultHpRatio ) * 0.25f;
			}

			else 
			{
				scoreRatio+= ( 0.5f -  resultHpRatio) * 0.5f;
				scoreRatio+= ( m_currentHPRatio - 0.5f ) * 0.25f;
			}
			
			score = scoreRatio*m_expLimitForNextLvl*levelMultiplier;			
			return score;
		}
		return score;
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::RenderHeroInformation()
	{		
		static ColoredQuad S_interfaceQuad1( Vec3( 0.0f, 1.0f, 0.0f), Vec3( 1.0f, 1.0f, 0.0f), Vec3( 0.0f, 0.0f, 0.0f), Vec3( 1.0f, 0.0f, 0.0f), Vec4( 0.5f, 0.5f, 1.0f, 0.5f ) );
		static ColoredQuad S_interfaceQuad2( Vec3( 0.0f, 1.0f, 0.0f), Vec3( 1.0f, 1.0f, 0.0f), Vec3( 0.0f, 0.0f, 0.0f), Vec3( 1.0f, 0.0f, 0.0f), Vec4( 0.5f, 0.5f, 1.0f, 0.5f ) );
		
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		
		ACNglPushMatrix();
		ACNglLoadIdentity();
		ACNglSetMode2D();
		ACNgluOrtho( 0, 100 , 0, 100 );	
		ACNglPushMatrix();
		ACNglScale( 40.0f, 21.0f, 1.0f );
		S_interfaceQuad1.Draw();
		ACNglPopMatrix();

		ACNglPushMatrix();
		ACNglTranslate( 41.0f, 0.0f, 0.0f );
		ACNglScale( 59.0f, 21.0f, 1.0f );
		S_interfaceQuad1.Draw();
		ACNglPopMatrix();
		
		ACNglSetMode3D();
		ACNglPopMatrix();
		glDisable( GL_DEPTH_TEST );

		float xPos = 5.0f;
		float xPos2 = 35.0f;
		float yPos = 18.0f;

		std::stringstream ss;
		ss<<"Character Name : " << m_characterInfo.ptrToBluePrintInfo->name;
		
		Vector3F color( 1.0f, 1.0f, 1.0f );

		RenderFontStuff( ss, xPos, yPos, color );
		ss.str( std::string() );
		ss.clear();
		
		ss<<"Team Number : " << m_characterInfo.m_controllerNum;
		RenderFontStuff( ss, xPos2, yPos, color );
		ss.str( std::string() );
		ss.clear();

		yPos -=2.0f;
		ss<<"Class : " << m_characterInfo.m_className;		
		RenderFontStuff( ss, xPos, yPos, color );
		ss.str( std::string() );
		ss.clear();

		ss<<"Level : " << m_currentLvl;
		RenderFontStuff( ss, xPos2, yPos, color );
		ss.str( std::string() );
		ss.clear();

		yPos -=2.0f;
		ss<<"Current HP : " <<m_currentHP;		
		RenderFontStuff( ss, xPos, yPos, color );
		ss.str( std::string() );
		ss.clear();

		ss<<"Max HP : " << m_maxHP;
		RenderFontStuff( ss, xPos2, yPos, color );
		ss.str( std::string() );
		ss.clear();

		yPos -=2.0f;
		ss<<"Current MP : " <<m_currentMP;		
		RenderFontStuff( ss, xPos, yPos, color );
		ss.str( std::string() );
		ss.clear();

		ss<<"Max MP : " << m_maxMP;
		RenderFontStuff( ss, xPos2, yPos, color );
		ss.str( std::string() );
		ss.clear();

		yPos -=2.0f;
		ss<<"Current Exp : " <<m_currentExp;		
		RenderFontStuff( ss, xPos, yPos, color );
		ss.str( std::string() );
		ss.clear();

		ss<<"Exp For Next Lvl : " << m_expLimitForNextLvl;
		RenderFontStuff( ss, xPos2, yPos, color );
		ss.str( std::string() );
		ss.clear();

		yPos -=2.0f;
		ss<<"Strength : " <<m_currentLevelStats[STAT_STRENTGH];		
		RenderFontStuff( ss, xPos, yPos, color );
		ss.str( std::string() );
		ss.clear();

		ss<<"Intelligence : " <<m_currentLevelStats[STAT_INTELLIGENCE];
		RenderFontStuff( ss, xPos2, yPos, color );
		ss.str( std::string() );
		ss.clear();

		yPos -=2.0f;
		ss<<"Armor : " << m_currentLevelStats[STAT_ARMOR];
		RenderFontStuff( ss, xPos, yPos, color );
		ss.str( std::string() );
		ss.clear();

		yPos -=2.0f;
		ss<<"Weak Against : " <<m_heroClassInfo.m_pairOfWeakAndStrongElement.first;		
		RenderFontStuff( ss, xPos, yPos, color );
		ss.str( std::string() );
		ss.clear();

		ss<<"Strong Against : " <<m_heroClassInfo.m_pairOfWeakAndStrongElement.second;
		RenderFontStuff( ss, xPos2, yPos, color );
		ss.str( std::string() );
		ss.clear();

		//Rendering The Ability information now
		xPos = 65.0f;
		yPos = 18.0f;

		ss<<"0 - Hold : Ends the turn without performing an action";
		RenderFontStuff( ss, xPos, yPos, color );
		ss.str( std::string() );
		ss.clear();

		for ( unsigned int i = 0; i < m_vecOfAvaiableAbilities.size(); ++i )
		{
			yPos -= 2.0f;
			int xOffSet = 0;
			AbilityInfo currentAbility = AbilityBluePrint::CreateOrGetBluePrint( m_vecOfAvaiableAbilities[i] )->GetAbilityInfo();
			ss<<i+1<<" - "<<m_vecOfAvaiableAbilities[i];
			
			RenderFontStuff( ss, xPos, yPos, color );
		
			ss.str( std::string() );
			ss.clear();
		
			xOffSet += 10;
			Vector3F tempColor( 1.0f, 0.0f, 0.0f );
			
			if ( currentAbility.m_targetType == T_ALLY )
			{
				tempColor( 0.0f, 1.0f, 0.0f );
				ss<<"Type : Heal";
			}

			else
			{			
				ss<<"Type : Damage";
			}

			RenderFontStuff( ss, xPos + xOffSet, yPos, tempColor );
			ss.str( std::string() );
			ss.clear();
			xOffSet += 15;
			int damage = (int)( m_currentLevelStats*currentAbility.m_dmgModifiers );
			ss<<"Power : "<<damage;
		
			RenderFontStuff( ss, xPos + xOffSet, yPos, color );
			ss.str( std::string() );
			ss.clear();
			xOffSet += 12;
			int minRange = currentAbility.m_minRange;
			int maxRange = currentAbility.m_maxRange;
			ss<<"Range : "<<minRange<<" - "<<maxRange;
		
			RenderFontStuff( ss, xPos + xOffSet, yPos, color );
			xOffSet += 13;
			ss.str( std::string() );
			ss.clear();
			int hpCost = (int)currentAbility.m_hpCost;
			int mpCost = (int)currentAbility.m_mpCost;
			ss<<"Cost : "<<hpCost<<"hp/"<<mpCost<<"mp";
		
			RenderFontStuff( ss, xPos + xOffSet, yPos, color );
			xOffSet += 15;
			ss.str( std::string() );
			ss.clear();			
			int aoe = currentAbility.m_areaOfEffect;
			ss<<"AOE : "<<aoe;
		
			RenderFontStuff( ss, xPos + xOffSet, yPos, color );
			xOffSet += 8;
			ss.str( std::string() );
			ss.clear();
			ss<<"Element : "<<currentAbility.m_baseElement;
			
			RenderFontStuff( ss, xPos + xOffSet, yPos, color );
			xOffSet += 15;
			ss.str( std::string() );
			ss.clear();
		}
		glEnable( GL_DEPTH_TEST );		
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::RenderFontStuff( std::stringstream& ss, float xPos, float yPos, Vector3F& color )
	{
		FontFactory::GetInstance().DrawString2D( ss.str(), 2.0f, xPos - 0.1f, yPos +0.1f, Vector3F( 0, 0, 0 ) );
		FontFactory::GetInstance().DrawString2D( ss.str(), 2.0f, xPos + 0.2f, yPos -0.2f, Vector3F( 0, 0, 0 ) );
		FontFactory::GetInstance().DrawString2D( ss.str(), 2.0f, xPos, yPos, color);

	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::DisplayTextMsg()
	{
		FontFactory::GetInstance().DrawString( m_textMsg, 0.2f, m_currentWorldPos + Vector3F( 0.0f, 1.8f, 0.0f ), m_teamColor );
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::DisplayAttackInformation()
	{
		bool abilityFound = false;

		static float timeDis = 0.0f;
		timeDis += 0.1f;

		if ( timeDis > 4.0f)
			timeDis = 0.0f;

		for ( unsigned int i = 0; i < m_vecOfAvaiableAbilities.size(); ++i )
		{
			if ( m_targetAbilityName == m_vecOfAvaiableAbilities[i] )
			{
				abilityFound = true;
				break;
			}
		}

		if ( abilityFound )
		{
			AbilityInfo currentAbility = AbilityBluePrint::CreateOrGetBluePrint( m_targetAbilityName )->GetAbilityInfo();

			for( int x = m_targetTileCoords.x - currentAbility.m_areaOfEffect;x <= m_targetTileCoords.x + currentAbility.m_areaOfEffect; ++x  )
			{
				for( int y = m_targetTileCoords.y - currentAbility.m_areaOfEffect;y <= m_targetTileCoords.y + currentAbility.m_areaOfEffect; ++y  )
				{
					MapTile& mapTile = m_ptrToGameMap->GetMapTile( x, y );

					float manDis = (float)GetManhattanDistance( m_targetTileCoords, mapTile.m_tileCoords );
				
					if ( manDis<= currentAbility.m_areaOfEffect)
					{
						Vec4 color( 0.0f, 0.0f, 1.0f, 0.7f );
						Vec4 color2( 0.0f, 1.0f, 1.0f, 0.7f );
						static ColoredQuad S_coloredQuad( Vec3( 0.0f, 0.0f, 0.0f), Vec3( 1.0f, 0.0f, 0.0f), Vec3( 0.0f, 0.0f, 1.0f), Vec3( 1.0f, 0.0f, 1.0f), Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
						if ( timeDis-2.0f < 0.0f )
						{
							S_coloredQuad.ModifyQuadColor( color );
						}
						else
							S_coloredQuad.ModifyQuadColor( color2 );
						ACNglPushMatrix();
						ACNglTranslate( mapTile.m_pos.x - 0.5f, mapTile.m_pos.y + 0.03f , mapTile.m_pos.z - 0.5f );
						S_coloredQuad.Draw();
						ACNglPopMatrix();
					}
				}
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::RegenAndCheckLvlUp()
	{
		m_currentHP += m_hpRegenPerTurn;
		m_currentMP += m_mpRegenPerTurn;

		m_currentHP = clamp( m_currentHP, 0.0f, m_maxHP );
		m_currentMP = clamp( m_currentMP, 0.0f, m_maxMP );

		if ( m_currentExp > m_expLimitForNextLvl )
		{
			float lvl = m_currentLvl + 1.0f;
			LvlUpHeroToLvl( lvl );
		}
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	void Hero::RenderHeroName()
	{
		FontFactory::GetInstance().DrawString( m_characterInfo.ptrToBluePrintInfo->name, 0.2f, m_currentWorldPos + Vector3F( 0.0f, 2.2f, 0.0f ), m_teamColor );
	}

	int Hero::validIDCounter = 0;
}
