#pragma once
#include "VBOFactory.h"
#include "baseworld.h"
#include "vector3d.h"
#include <vector>

namespace ACN
{

	struct ClothVertex
	{
		//Positions
		vec3 currentPosition;
		vec3 previousPosition;

		//Mass
		float massMultiplier;

		//Accelaration
		vec3 currentAcceleration;

		//Texture Coordinates
		float texX, texY;

		//Don't update position if fixed
		bool isFixed;

		//Should it get fixed after collision
		bool isFixedOnCollision;

		//Strech ConstraintLinks  top down left right
		ClothVertex* strechConstraintLink[4];		

		//Shear ConstraintLinks Diagnols Top Left, Top Right, Bottom Left, Bottom Right
		ClothVertex* sheerConstraintLink[4];		

		//Bend ConstraintLinks  2x Size top down left right
		ClothVertex* bendConstraintLink[4];		

		//ConstraintLinks stiffness
		float strechStiffness;
		float sheerStiffness;
		float bendStiffness;

		// Default Constructor
		ClothVertex();

	};

	//The class for a single cloth which has all the cloth related data
	class Cloth : public BaseWorldActor
	{
	public:

		//Type of cloth created
		enum ClothMode
		{
			C_DEFAULT,
			C_COLLISION_TOP_CORNER,
			C_COLLISION_TOP,
			C_COLLISION_CORNERS,
			C_COLLISION_TOP_BOTTOM,
			C_COLLISION_ALL,
			C_FIXED_TOP_CORNER,
			C_FIXED_TOP,
			C_FIXED_CORNERS,
			C_FIXED_TOP_BOTTOM,
			C_FIX_ALL,
			C_MAX
		};

		Cloth(void);
		~Cloth(void);		
		
		//Creates the cloth with given values
		void CreateCloth(	vec3 topLeft, vec3 rightDirection = vec3( 1,0,0), vec3 bottomDirection = vec3( 0,-1,0),
			float widthLength = 10.0f, float heightLength = 10.0f, size_t noOfPoints = 25,	
			float strechStiff = 1.0f, float sheerStiff = 1.0f, float bendStiff =1.0f, 
			ClothMode defaultMode = C_DEFAULT );

		//Base world actor virtual functions
		virtual void UpdateSimulation( float deltaTime );		
		virtual void UpdateDisplay();		
			
		//Sets the current cloth texture
		void SetClothTextureMode( unsigned int texMode );
		
		unsigned int GetTextureMode();
		
		//Returns pointer to vecOfClothQuadData
		//This allows to modify pvt data, so use with caution!
		std::vector< ClothVertex* >* GetQuadDataPTR();		
		
	private:
		std::vector< ClothVertex > vecOfClothVertex;
		std::vector< ClothVertex* > vecOfClothQuadData;

		//Damping factor
		float dampingFactor;
		float edgeMassMult;

		//Direction
		vec3 topLeftPosition;
		vec3 rightDir;
		vec3 botDir;

		//Width and height of the cloth
		float width;
		float height;

		//Num of subdivisions
		size_t subDivisions;
		
		//Mode
		ClothMode clothMode;

		//Lengths
		float strechLength[4];
		float sheerLength[4];
		float bendLength[4];

		//ConstraintLinks stiffness
		float strechStiffness;
		float sheerStiffness;
		float bendStiffness;

		//Texture mode
		unsigned int textureMode;
		
		//Private functions	
		void AccumulateForces();

		void UpdateVerletMotion( float deltaTime );

		//Satisys all constraints
		void SatisfyConstraints();

		void SatisfyStrechConstraints();
		void SaisfySheerConstraints();
		void SatisfyBendConstraints();
		void SatisfyCollisionConstraints();
		void SatisfyHardStrechConstraints();

		//Sets up cloth according to the specified cloth mode
		void SetupModeData();
		
		//Generate the verts in the vector
		void GenerateVertices();

		void CalculateConstraintLengths( float bottomIncrement, float rightIncrement );

		//Creates the appropiate neighbouring links
		void CreateLinks();

		ClothVertex* GetClothVertex( int x, int y );
		
		void GenerateQuadData();
	};
}

