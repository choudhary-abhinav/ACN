#pragma once
#include "baseworld.h"
#include "singleton.h"
#include "inputlistener.h"
#include "vector3d.h"
#include "gameutil.h"

namespace ACN		
{	
	const unsigned int WIND_PARTITIONS = 10;
	const float WIND_PART_SIZE = ( ( float ) WORLD_DISPLACEMENT*2.0f ) / ( float )WIND_PARTITIONS;

	//A class which is used to caclulate all the worlds forces including gravity and wind
	class ForceAccumulator :
		public BaseWorldActor, public Singleton<ForceAccumulator>, public InputListener
	{
	public:
		ForceAccumulator(void);
		~ForceAccumulator(void);

		virtual void OnKeyUp( unsigned char keyCode, int mouseX, int mouseY );		

		virtual void OnKeyDown( unsigned char keyCode, int mouseX, int mouseY );

		virtual void UpdateSimulation( float deltaTime );

		vec3 GetGravity() {	return gravity;	}
		vec3 GetWindForce( vec3& atPosition );		
		vec3 GetSpotLightWindForce( vec3& atPosition );

	private:
		float time;
		bool isWindOn;

		//Spot light wind
		bool isSpotLightWindOn;

		float spotLightWindMax;
		float spotLightWindMin;
		float spotLightWindCurrent;

		vec3 spotDirection;
		vec3 spotPosition;

		float inverseAttenuationFactor;

		//Wind Exponents
		float spotCutoff;
		float spotExponent;

		vec3 gravity;

		vec3 windX[ WIND_PARTITIONS ];
		vec3 windY[ WIND_PARTITIONS ];
		vec3 windZ[ WIND_PARTITIONS ];

		vec3 moveOffset;

		float windRange;

		void GenerateRandomWind();
	};
}

