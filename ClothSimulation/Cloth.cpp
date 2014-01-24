#include "Cloth.h"
#include "gameutil.h"
#include "CollisionObjContainer.h"
#include "ForceAccumulator.h"

namespace ACN
{

	ClothVertex::ClothVertex() 
		: isFixed( false )
		, isFixedOnCollision( false )
		, sheerStiffness( 1.0f )
		, strechStiffness( 1.0f )
		, bendStiffness( 1.0f )
		, massMultiplier( 1.0f )
	{

	}

	Cloth::Cloth(void)
		: dampingFactor( 0.05f )
		, edgeMassMult( 0.8f ) //Between 0 and 1
	{
	}
	
	Cloth::~Cloth(void)
	{
	}

	void Cloth::UpdateSimulation( float deltaTime )
	{		
		AccumulateForces();
		UpdateVerletMotion( deltaTime );
		SatisfyConstraints();
	}
	
	void Cloth::UpdateDisplay()
	{
		
	}

	void Cloth::CreateCloth( vec3 topLeft, vec3 rightDirection /*= vec3( 1,0,0)*/, vec3 bottomDirection /*= vec3( 0,-1,0)*/, 
							float widthLength /*= 10.0f*/, float heightLength /*= 10.0f*/, size_t noOfPoints /*= 25*/, 
							float strechStiff /*= 1.0f*/, float sheerStiff /*= 1.0f*/, float bendStiff /*=1.0f*/, 
							ClothMode defMode /*= C_DEFAULT */ )
	{
		topLeftPosition = topLeft;
		
		rightDirection.normalize();
		rightDir = rightDirection;

		bottomDirection.normalize();
		botDir = bottomDirection;

		width = widthLength;
		height = heightLength;
		subDivisions = noOfPoints;

		strechStiffness = strechStiff;
		sheerStiffness = sheerStiff;
		bendStiffness = bendStiff;

		clothMode = defMode;

		dampingFactor = 0.01f;

		GenerateVertices();
		
		CreateLinks();
		
		SetupModeData();
		
		GenerateQuadData();
	}

	void Cloth::GenerateVertices()
	{
		float rightIncrement =   width/(float)(subDivisions-1);
		float bottomIncrement = height/(float)(subDivisions-1);

		CalculateConstraintLengths( bottomIncrement, rightIncrement );
		
		//Row major vertex generation
		for ( unsigned int y = 0; y<subDivisions; ++y )
		{
			for ( unsigned int x = 0; x<subDivisions; ++x )
			{
				ClothVertex tempVertex;
				tempVertex.currentPosition = topLeftPosition + rightDir * ( x * rightIncrement ) + botDir*( y*bottomIncrement );
				tempVertex.previousPosition = tempVertex.currentPosition;

				tempVertex.texX = float(x)/(float)(subDivisions-1);
				tempVertex.texY = float(y)/(float)(subDivisions-1);

				tempVertex.sheerStiffness = sheerStiffness;
				tempVertex.bendStiffness = bendStiffness;
				tempVertex.sheerStiffness = sheerStiffness;
				
				vecOfClothVertex.push_back( tempVertex );
			}
		}
	}

	void Cloth::CreateLinks()
	{
		for ( unsigned int y = 0; y<subDivisions; ++y )
		{
			for ( unsigned int x = 0; x<subDivisions; ++x )
			{
				ClothVertex* currVertex = GetClothVertex( x, y );
			
				//Calculating stretch links
				//Top
				currVertex->strechConstraintLink[0] = GetClothVertex( x, y-1 );
				//Down			   								 
				currVertex->strechConstraintLink[1] = GetClothVertex( x, y+1 );
				//Left			   								 
				currVertex->strechConstraintLink[2] = GetClothVertex( x-1, y );
				//Right			   								 
				currVertex->strechConstraintLink[3] = GetClothVertex( x+1, y );

				//Calculating bend links
				//Top
				currVertex->bendConstraintLink[0] = GetClothVertex( x, y-2 );
				//Down			   								 
				currVertex->bendConstraintLink[1] = GetClothVertex( x, y+2 );
				//Left			   								 
				currVertex->bendConstraintLink[2] = GetClothVertex( x-2, y );
				//Right			   								 
				currVertex->bendConstraintLink[3] = GetClothVertex( x+2, y );

				//Calculating sheer links
				//Top Left
				currVertex->sheerConstraintLink[0] = GetClothVertex( x-1, y-1 );
				//Top Right
				currVertex->sheerConstraintLink[1] = GetClothVertex( x+1, y-1 );
				//Bottom Left
				currVertex->sheerConstraintLink[2] = GetClothVertex( x-1, y+1 );
				//Bottom Right
				currVertex->sheerConstraintLink[3] = GetClothVertex( x+1, y+1 );
			}
		}
	}

	void Cloth::CalculateConstraintLengths( float bottomIncrement, float rightIncrement )
	{
		//Calculating stretch lengths
		//Top
		strechLength[0]= ( botDir * ( -bottomIncrement ) ).length();
		//Down			   								 
		strechLength[1]= ( botDir * ( bottomIncrement ) ).length();
		//Left			   								 
		strechLength[2]= ( rightDir * ( -rightIncrement ) ).length();
		//Right			   								 
		strechLength[3]= ( rightDir * ( rightIncrement ) ).length();

		//Calculating bend lengths
		//Top
		bendLength[0] = strechLength[0] * 2.0f;
		//Down
		bendLength[1] = strechLength[1] * 2.0f;
		//Left
		bendLength[2] = strechLength[2] * 2.0f;
		//Right
		bendLength[3] = strechLength[3] * 2.0f;

		//Calculating sheer lengths
		//Top Left
		sheerLength[0] = ( botDir * ( -bottomIncrement ) + rightDir * ( -rightIncrement ) ).length();
		//Top Right
		sheerLength[1] = ( botDir * ( -bottomIncrement ) + rightDir * ( rightIncrement ) ).length();
		//Bottom Left
		sheerLength[2] = ( botDir * ( bottomIncrement ) + rightDir * ( -rightIncrement ) ).length();
		//Bottom Right
		sheerLength[3] = ( botDir * ( bottomIncrement ) + rightDir * ( rightIncrement ) ).length();
	}

	ClothVertex* Cloth::GetClothVertex( int x, int y )
	{
		if ( x<0 || x>=(int)subDivisions || y<0 || y>=(int)subDivisions )
		{
			return nullptr;
		}
		else
		{
			return &( vecOfClothVertex[ (y*(int)subDivisions) + x ] );
		}
	}

	void Cloth::GenerateQuadData()
	{
		for ( unsigned int y = 0; y<subDivisions-1; ++y )
		{
			for ( unsigned int x = 0; x<subDivisions-1; ++x )
			{
				//Top left
				vecOfClothQuadData.push_back( GetClothVertex( x, y ) );
				//Top right				
				vecOfClothQuadData.push_back( GetClothVertex( x+1, y ) );
				//Bottom Left
				vecOfClothQuadData.push_back( GetClothVertex( x, y+1 ) );
				//Bottom Right
				vecOfClothQuadData.push_back( GetClothVertex( x+1, y+1 ) );
			}
		}
	}
	
	std::vector< ClothVertex* >* Cloth::GetQuadDataPTR()
	{
		return &vecOfClothQuadData;
	}

	void Cloth::SetClothTextureMode( unsigned int texMode )
	{
		textureMode = texMode;
	}

	void Cloth::AccumulateForces()
	{
		for( unsigned int i = 0; i< vecOfClothVertex.size(); ++i )
		{
			if ( !vecOfClothVertex[i].isFixed )
			{
				vec3 windForce = ForceAccumulator::GetInstance().GetWindForce( vecOfClothVertex[i].currentPosition );
				windForce.x /= vecOfClothVertex[i].massMultiplier;
				windForce.y /= vecOfClothVertex[i].massMultiplier;
				windForce.z /= vecOfClothVertex[i].massMultiplier;

				vecOfClothVertex[i].currentAcceleration = ForceAccumulator::GetInstance().GetGravity() + windForce;
			}			
		}
	}

	void Cloth::UpdateVerletMotion( float deltaTime )
	{
		for( unsigned int i = 0; i< vecOfClothVertex.size(); ++i )
		{
			ClothVertex &vert = vecOfClothVertex[i];

			if ( !vert.isFixed)
			{
				vec3 vVelocity = vert.currentPosition - vert.previousPosition;
				vert.previousPosition = vert.currentPosition;
				vert.currentPosition += vVelocity * ( 1.0f - dampingFactor * vert.massMultiplier ) + vert.currentAcceleration * deltaTime * deltaTime;			
			}
		}
	}

	void Cloth::SatisfyConstraints()
	{
		unsigned int constraintSolveTime = 1;

		for ( unsigned int i=0; i<constraintSolveTime; ++i )
		{
			SatisfyStrechConstraints();
			SaisfySheerConstraints();
			SatisfyBendConstraints();
			SatisfyCollisionConstraints();
		}
	}

	void Cloth::SatisfyStrechConstraints()
	{
		for( unsigned int i = 0; i< vecOfClothVertex.size(); ++i )
		{
			ClothVertex &vert = vecOfClothVertex[i];

			if ( !vert.isFixed)
			{
				for ( unsigned int index = 0; index < 4; index++ )
				{
					if ( vert.strechConstraintLink[ index ]!=nullptr )
					{
						vec3 vDelta = vert.strechConstraintLink[ index ]->currentPosition - vert.currentPosition;
						float fLength = vDelta.length();
						vDelta.normalize();
						
						vec3 vOffset = vDelta * (  strechLength[ index ] - fLength );
						vOffset *= vert.strechStiffness;
						vert.strechConstraintLink[ index ]->currentPosition += ( vOffset / 2.0f );
						vert.currentPosition -= (vOffset / 2.0f);
						
						if ( vert.strechConstraintLink[index]->isFixed )
						{
							vert.strechConstraintLink[ index ]->currentPosition -= (vOffset / 2.0f);
							vert.currentPosition -= (vOffset / 2.0f);
						}
					}
				}
			}
		}
		
	}

	void Cloth::SaisfySheerConstraints()
	{
		for( unsigned int i = 0; i< vecOfClothVertex.size(); ++i )
		{
			ClothVertex &vert = vecOfClothVertex[i];

			if ( !vert.isFixed)
			{
				for ( unsigned int index = 0; index < 4; index++ )
				{
					if ( vert.sheerConstraintLink[ index ]!=nullptr )
					{
						vec3 vDelta = vert.sheerConstraintLink[ index ]->currentPosition - vert.currentPosition;
						float fLength = vDelta.length();
						vDelta.normalize();
					
						vec3 vOffset = vDelta * (  sheerLength[ index ] - fLength );
						vOffset *= vert.sheerStiffness;
						vert.sheerConstraintLink[ index ]->currentPosition += vOffset / 2.0f;
						vert.currentPosition -= vOffset / 2.0f;

						if ( vert.sheerConstraintLink[index]->isFixed )
						{
							vert.sheerConstraintLink[ index ]->currentPosition -= vOffset / 2.0f;
							vert.currentPosition -= vOffset / 2.0f;
						}
					}
				}
			}
		}
	}

	void Cloth::SatisfyBendConstraints()
	{
		for( unsigned int i = 0; i< vecOfClothVertex.size(); ++i )
		{
			ClothVertex &vert = vecOfClothVertex[i];

			if ( !vert.isFixed)
			{
				for ( unsigned int index = 0; index < 4; index++ )
				{
					if ( vert.bendConstraintLink[ index ]!=nullptr )
					{
						vec3 vDelta = vert.bendConstraintLink[ index ]->currentPosition - vert.currentPosition;
						float fLength = vDelta.length();
						vDelta.normalize();
						vec3 vOffset = vDelta * ( bendLength[ index ] - fLength );
						vOffset *= vert.bendStiffness;
						vert.bendConstraintLink[ index ]->currentPosition += vOffset / 2.0f;
						vert.currentPosition -= vOffset / 2.0f;

						if ( vert.bendConstraintLink[index]->isFixed )
						{
							vert.bendConstraintLink[ index ]->currentPosition -= vOffset / 2.0f;
							vert.currentPosition -= vOffset / 2.0f;
						}
					}
				}
			}
		}
	}

	void Cloth::SatisfyCollisionConstraints()
	{
		for( unsigned int i = 0; i< vecOfClothVertex.size(); ++i )
		{			
			ClothVertex &vert = vecOfClothVertex[i];
			if ( !vert.isFixed )
			{
				auto position = vert.currentPosition;

				//Returns the same position if no collision occured
				vert.currentPosition = CollisionObjContainer::GetInstance().CollisionResponse( vert.currentPosition );

				vert.currentPosition.x = clamp( vert.currentPosition.x, -( WORLD_DISPLACEMENT - 1.0f ), ( WORLD_DISPLACEMENT - 1.0f ) );
				vert.currentPosition.y = clamp( vert.currentPosition.y, -( WORLD_DISPLACEMENT - 1.0f ), ( WORLD_DISPLACEMENT - 1.0f ) );
				vert.currentPosition.z = clamp( vert.currentPosition.z, -( WORLD_DISPLACEMENT - 1.0f ), ( WORLD_DISPLACEMENT - 1.0f ) );

				if ( vert.isFixedOnCollision )
				{
					if ( position!= vert.currentPosition )
					{
						vert.isFixed = true;
					}
				}				
			}
		}
	}

	void Cloth::SetupModeData()
	{
		switch ( clothMode )
		{
		case C_DEFAULT:
			break;

		case C_COLLISION_TOP_CORNER:
			GetClothVertex( 0, 0)->isFixedOnCollision = true;
			GetClothVertex( subDivisions-1, 0)->isFixedOnCollision = true;
			break;

		case C_COLLISION_TOP:
			for ( size_t i = 0; i< subDivisions ; ++i )
			{
				GetClothVertex( i, 0)->isFixedOnCollision = true;
			}			
			break;

		case C_COLLISION_CORNERS:
			GetClothVertex( 0, 0)->isFixedOnCollision = true;
			GetClothVertex( subDivisions-1, 0)->isFixedOnCollision = true;

			GetClothVertex( 0, subDivisions-1)->isFixedOnCollision = true;
			GetClothVertex( subDivisions-1, subDivisions-1)->isFixedOnCollision = true;
			break;

		case C_COLLISION_TOP_BOTTOM:
			for ( size_t i = 0; i< subDivisions ; ++i )
			{
				GetClothVertex( i, 0)->isFixedOnCollision = true;
				GetClothVertex( i, subDivisions-1)->isFixedOnCollision = true;
			}	
			break;

		case C_FIXED_TOP_CORNER:
			GetClothVertex( 0, 0)->isFixed = true;
			GetClothVertex( subDivisions-1, 0)->isFixed = true;
			break;			

		case C_FIXED_TOP:
			for ( size_t i = 0; i< subDivisions ; ++i )
			{
				GetClothVertex( i, 0)->isFixed = true;
			}	
			break;

		case C_FIXED_CORNERS:
			GetClothVertex( 0, 0)->isFixed = true;
			GetClothVertex( subDivisions-1, 0)->isFixed = true;

			GetClothVertex( 0, subDivisions-1)->isFixed = true;
			GetClothVertex( subDivisions-1, subDivisions-1)->isFixed = true;
			break;
			

		case C_FIXED_TOP_BOTTOM:
			for ( size_t i = 0; i< subDivisions ; ++i )
			{
				GetClothVertex( i, 0)->isFixed = true;
				GetClothVertex( i, subDivisions-1)->isFixed = true;
			}	
			break;

		case C_FIX_ALL:
			for ( size_t i = 0; i< subDivisions ; ++i )
			{
				for ( size_t y = 0; y< subDivisions ; ++y )
				{
					GetClothVertex( i, y )->isFixed = true;					
				}
			}	
			break;

		case C_COLLISION_ALL:
			for ( size_t i = 0; i< subDivisions ; ++i )
			{
				for ( size_t y = 0; y< subDivisions ; ++y )
				{
					GetClothVertex( i, y)->isFixedOnCollision = true;					
				}
			}	
			break;

		}

		for ( size_t x = 0; x< subDivisions; ++x )
		{
			GetClothVertex( 0, x)->massMultiplier = edgeMassMult;
			GetClothVertex( subDivisions-1, x)->massMultiplier = edgeMassMult;
			GetClothVertex( x, 0)->massMultiplier = edgeMassMult;
			GetClothVertex( x, subDivisions-1)->massMultiplier = edgeMassMult;
		}
	}

	unsigned int Cloth::GetTextureMode()
	{
		return textureMode;
	}
	
}
