#include "ForceAccumulator.h"

#include "camera.h"

namespace ACN
{

	ForceAccumulator::ForceAccumulator(void)
		: isWindOn(true)
		, time( 30.0f )
		, isSpotLightWindOn( false )
		, windRange( 10.0f )
		, gravity( 0, -9.8f, 0 )
		, spotLightWindMin( 2.0f )
		, spotLightWindMax( 40.0f )
		, spotLightWindCurrent( 25.0f )
		, inverseAttenuationFactor( 0.001f )
	{		
		spotCutoff = cos( degreesToRadians(20.0f) );
		spotExponent = 4.0f;		
		GenerateRandomWind();
	}

	ForceAccumulator::~ForceAccumulator(void)
	{
	}

	void ForceAccumulator::GenerateRandomWind()
	{
		for ( unsigned int i =0; i<WIND_PARTITIONS; ++i )
		{
			windX[ i ] = ( randInRange( -windRange, windRange ),randInRange( -windRange, windRange ), randInRange( -windRange, windRange ));
			windY[ i ] = ( randInRange( -windRange, windRange ),randInRange( -windRange, windRange ), randInRange( -windRange, windRange ));
			windZ[ i ] = ( randInRange( -windRange, windRange ),randInRange( -windRange, windRange ), randInRange( -windRange, windRange ));
			
			windY[i]*=0.75f;
			windZ[i]*=0.50f;
		}
	}

	void ForceAccumulator::UpdateSimulation( float deltaTime )
	{
		time+=deltaTime;
		if ( time > 3000.0f )
		{
			time = 30.0f;
		}

		spotLightWindCurrent = clamp( spotLightWindCurrent, spotLightWindMin, spotLightWindMax );

		spotDirection = -Camera::GetInstance().GetViewDirection();
		spotPosition = Camera::GetInstance().position();

		moveOffset = ( time, 2*time, 3*time );
	}

	vec3 ForceAccumulator::GetWindForce( vec3& atPosition )
	{
		vec3 windForce(0,0,0);		
		
		if ( isWindOn && (atPosition.y > (-WORLD_DISPLACEMENT + 4)) )
		{
			vec3 newPos = atPosition + moveOffset;
			//Getting value between 0 and 60;
			newPos.x = fmod( newPos.x, WORLD_DISPLACEMENT*2.0f );    
			newPos.y = fmod( newPos.y, WORLD_DISPLACEMENT*2.0f );
			newPos.z = fmod( newPos.z, WORLD_DISPLACEMENT*2.0f );

			//Calculating which arry to search in
			int xVal = (int)( newPos.x / WIND_PART_SIZE );
			int yVal = (int)( newPos.y / WIND_PART_SIZE );
			int zVal = (int)( newPos.z / WIND_PART_SIZE );

			float xFrac = fmod( newPos.x, WIND_PART_SIZE );
			float yFrac = fmod( newPos.y, WIND_PART_SIZE );
			float zFrac = fmod( newPos.z, WIND_PART_SIZE );

			// Setting it so that it incrases from 0 to 1 when it reaches 0.5f size, then goes from 1 to 0
			xFrac = 1 - ( (WIND_PART_SIZE*0.5f - xFrac)/(WIND_PART_SIZE*0.5f) );  
			yFrac = 1 - ( (WIND_PART_SIZE*0.5f - yFrac)/(WIND_PART_SIZE*0.5f) );  
			zFrac = 1 - ( (WIND_PART_SIZE*0.5f - zFrac)/(WIND_PART_SIZE*0.5f) );  

			xFrac = xFrac * xFrac;
			yFrac = yFrac * yFrac;
			zFrac = zFrac * zFrac;

			windForce = windX[ xVal ]*xFrac + windY[ yVal ]*yFrac + windZ[ zVal ]*zFrac;	
		}

		if ( isSpotLightWindOn )
		{
			vec3 spotLightWindForce = GetSpotLightWindForce( atPosition );
			windForce += spotLightWindForce;
		}

		return windForce;
	}

	vec3 ForceAccumulator::GetSpotLightWindForce( vec3& atPosition )
	{
		vec3 dirToPos = atPosition - spotPosition;
		float attenuation = 1/ ( dirToPos.length_squared() * inverseAttenuationFactor );
		dirToPos.normalize();

		float clampedCosine = dirToPos * spotDirection;

		if ( clampedCosine < spotCutoff )
		{
			return vec3( 0,0,0 );
		}

		attenuation = clamp( attenuation, 0.0f, 1.0f );
		attenuation = attenuation * pow( clampedCosine, spotExponent );
		
		vec3 spotWindforce = dirToPos * attenuation * spotLightWindCurrent;
		return spotWindforce;
	}
	
	void ForceAccumulator::OnKeyUp( unsigned char keyCode, int mouseX, int mouseY )
	{
		switch( keyCode )
		{
		case '1':
			isWindOn = !isWindOn;
			break;

		case '2':
			isSpotLightWindOn = !isSpotLightWindOn;
			break;
		case '0':
			gravity( 0, 0, 0);
			break;
		case '9':
			gravity( 0, -9.8f, 0);
			break;
		case '8':
			gravity( 0, 9.8f, 0);
			break;
		}
	}
	
	void ForceAccumulator::OnKeyDown( unsigned char keyCode, int mouseX, int mouseY )
	{
		static float increment = 0.05f;
		switch( keyCode )
		{
		case '3':
			spotLightWindCurrent += increment;
			break;

		case '4':
			spotLightWindCurrent -= increment;
			break;
		}
	}

}
