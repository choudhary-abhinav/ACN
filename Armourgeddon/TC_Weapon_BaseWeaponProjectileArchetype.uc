//=============================================================================
// TC_Weapon_BaseWeaponProjectileArchetype: Basic Weapon Projectile Archetype.
//
// Most of our Projectile functionality is already 
// present in UTProjectile, so we derive from UTProj. 
// This class serves as a basic archetype which is used by the weapon class.
// Most of the categories for this archetype is hidden to make it easy for the 
// level designers to modify values
//
// Author: Abhinav Choudhary, 2013
//=============================================================================
class TC_Weapon_BaseWeaponProjectileArchetype extends UTProjectile
HideCategories(Movement, Display, Attachment, Physics, Advanced, Debug, Object);

//=============================================================================
// Variables : TC_Weapon_BaseWeaponProjectileArchetype class
//=============================================================================

//////////////////////////////////////////////////////////////////
// Projectile Attributes
//////////////////////////////////////////////////////////////////

// Flight particle system to use
var(ProjectileInfo) const ParticleSystem                MyFlightParticleSystem<DisplayName=ProjFlightTemplate>;
// Impact particle template to use
var(ProjectileInfo) const ParticleSystem                MyImpactTemplate<DisplayName=ProjExplosionTemplate>;
// Impact material instance time varying to use for decals. This assumes the linear scalar data is setup for fading away
var(ProjectileInfo) const MaterialInstanceTimeVarying   MyImpactDecalMaterialInstanceTimeVarying<DisplayName=ExplosionDecal>;

// Impact decal minimum Width
var(ProjectileInfo) const float                         ImpactDecalWidth;
// Impact decal maximum Height
var(ProjectileInfo) const float                         ImpactDecalHeight;

//-what-my-bullet-sounds-like-when-it's-coming-at-you 
var(ProjectileInfo)	SoundCue                            ProjAmbientSound<DisplayName=AmbientSound>;
// Sound made when projectile hits something.
var(ProjectileInfo)	SoundCue                            ProjExplosionSound<DisplayName=ExplosionSound>;

// The light spawned during Travel.
var(ProjectileInfo) class<UDKExplosionLight>            MyProjectileLightClass<DisplayName=ProjectileLightClass>;
// The light spawned on impact//The class of the light used when the projectile explodes.
var(ProjectileInfo) class<UDKExplosionLight>            MyProjExplosionLightClass<DisplayName=ProjExplosionLightClass>;

var(ProjectileInfo) float                               MyAccelRate<DisplayName=AccelRate>;

// Lifespan of projectile 
var(ProjectileInfo) float                               ProjectileLifespan;

// Set the physics type as Projectile or Falling only
var(ProjectileInfo) EPhysics                            ProjPhysics;

//Set the class for the dmg type
var	(ProjectileInfo) class<DamageType>                  MyProjDamageType;

//////////////////////////////////////////////////////////////////
// Homing related Attributes
//////////////////////////////////////////////////////////////////

var TC_BOT HomingTarget;

//The time before it starts homing
var (Homing)        float                               LockTime;
var                 bool                                bLocked;

//The frequency at which it changes direction
var (Homing)        float                               HomingCheckFrequency;
var                 float                               HomingCheckCount;

//The strentgh at which it changes direction. Between 0 and 1;
var (Homing)		float								MyHomingTrackingStrength;
var (Homing)		float								MinHomingTrackingStrength;
var (Homing)		float								DeltaHomingIncrease;

var                 float                               HomingAccel;

//The max angle it picks while changing into random direction
var (Homing)        float                               MaxAngleChangeWhileHoming;

//The time frequency it sets a random angle
var (Homing)        float                               RandomAngleTime;

//The time in which it smoothens out the angle change so it looks smooth
var (Homing)        float                               SmoothRandomAngleTime;

//These variables are used to give a sense of homing by changing the homing projectiles direction a bit while homing.
var                 float                               RandAngleCheckCount;
var                 float                               LastDeltaTime;
var                 Rotator                             DeltaRotation;
var                 float                               SmoothRandomTimeCounter;

//=============================================================================
// Functions : TC_Weapon_BaseWeaponProjectileArchetype class
//=============================================================================

//Set all the default properties to UT variables
simulated function PostBeginPlay()
{
	ProjFlightTemplate          =       MyFlightParticleSystem;
	ProjExplosionTemplate       =       MyImpactTemplate;
	ExplosionDecal              =       MyImpactDecalMaterialInstanceTimeVarying;
	DecalWidth                  =       ImpactDecalWidth;
	DecalHeight                 =       ImpactDecalHeight;
	AmbientSound	            =       ProjAmbientSound;
	ExplosionSound              =       ProjExplosionSound;
	ExplosionLightClass         =       MyProjExplosionLightClass;
	ProjectileLightClass        =       MyProjectileLightClass;
	
	LifeSpan                    =       ProjectileLifespan;
	HomingTrackingStrength      =       MinHomingTrackingStrength;
	MyDamageType                =       MyProjDamageType;
	
	SetPhysics(ProjPhysics);
	Super.PostBeginPlay();
}

simulated event CreateProjectileLight()
{
	if ( WorldInfo.bDropDetail )
		return;

	if( ProjectileLightClass != None )
	{
		ProjectileLight = new(self) ProjectileLightClass;
		AttachComponent(ProjectileLight);
	}
}

//Sets up a target for Homing
simulated Function GiveTarget( TC_BOT Target )
{
	HomingTarget = Target;	
	SetTimer( LockTime );
}

//Starts homing after timer is over
simulated event Timer()
{
//We want the missile to turn to it's target
	If(HomingTarget != None)
	{
		bLocked = True;
	}
}

//This function does a random rotation to give a good feel while homing
simulated function DoRandomRotation( float DeltaTime )
{
	Local Rotator MyRotator;
	Local float DeltaMultiplier;

	LastDeltaTime = DeltaTime;
	DeltaMultiplier = DeltaTime/SmoothRandomAngleTime;
	MyRotator.Pitch = DegToUnrRot*RandRange( -MaxAngleChangeWhileHoming, MaxAngleChangeWhileHoming);
	MyRotator.Yaw = DegToUnrRot*RandRange( -MaxAngleChangeWhileHoming, MaxAngleChangeWhileHoming);
	MyRotator.Roll = DegToUnrRot*RandRange( -MaxAngleChangeWhileHoming, MaxAngleChangeWhileHoming);
	
	DeltaRotation = MyRotator * DeltaMultiplier;
	SmoothRandomTimeCounter = SmoothRandomAngleTime;
	SetTimer(LastDeltaTime, false, 'DoDeltaRotation');	
}

//Does the rotation in small time steps to make the random rotation feel more smooth
simulated function DoDeltaRotation()
{
	SmoothRandomTimeCounter-=LastDeltaTime;
	Velocity = Velocity<<DeltaRotation;

	if( SmoothRandomTimeCounter>0 )
		SetTimer(LastDeltaTime, false, 'DoDeltaRotation');
}

//This function is responsible regarding the homing effect
simulated function DoHomingToTarget( float DeltaTime )
{
	Local Vector Heading, direction;

	if( HomingTarget == none )
	{
		//Perhaps the target died
		//We can either home to a new target or we 
		//can stop homing. Right now we stop homing.
		bLocked = false;
		return;
	}

	HomingCheckCount += DeltaTime;
	RandAngleCheckCount += DeltaTime;
	
	if( HomingCheckCount >= HomingCheckFrequency )
	{
		HomingCheckCount -= HomingCheckFrequency;
		Heading = Normal(HomingTarget.Pawn.Location - Location);
		Direction = Normal(Velocity) * ( 1 - HomingTrackingStrength ) + (HomingTrackingStrength * Heading);
		Velocity = Normal(Direction) * VSize(Velocity);
		
		SetRotation( rotator(Velocity) );

		HomingTrackingStrength += DeltaHomingIncrease;
		HomingTrackingStrength = FMin( HomingTrackingStrength, MyHomingTrackingStrength );
	}

	if(RandAngleCheckCount >= RandomAngleTime)
	{
		RandAngleCheckCount = 0;
		DoRandomRotation( DeltaTime );
	}
}

//Modified Tick to be responsible for Homing
simulated event Tick( float DeltaTime )
{
	if( bLocked )
	{
		DoHomingToTarget( DeltaTime );
	}
	super.Tick( DeltaTime );
}

//=============================================================================
// Default Properties : TC_Weapon_BaseWeaponProjectileArchetype class
//=============================================================================

defaultproperties
{
	MyFlightParticleSystem                      =       ParticleSystem'WP_RocketLauncher.Effects.P_WP_RocketLauncher_RocketTrail'
	MyImpactTemplate                            =       ParticleSystem'WP_RocketLauncher.Effects.P_WP_RocketLauncher_RocketExplosion'
	MyImpactDecalMaterialInstanceTimeVarying    =       MaterialInstanceTimeVarying'WP_RocketLauncher.Decals.MITV_WP_RocketLauncher_Impact_Decal01'
	DurationOfDecal                             =       8.0
	ImpactDecalWidth                            =		128.0
	ImpactDecalHeight                           =		128.0
	speed                                       =		1350.0
	MyAccelRate                                 =		1000
	MaxSpeed                                    =		1350.0
	Damage                                      =		100.0
	DamageRadius                                =		220.0
	MomentumTransfer                            =		85000
	MyProjDamageType                            =		class'TC_Weapon_DmgType_Mainhand'
	LifeSpan                                    =		8.0
	ProjAmbientSound                            =		SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_Travel_Cue'
	ProjExplosionSound                          =		SoundCue'A_Weapon_RocketLauncher.Cue.A_Weapon_RL_Impact_Cue'
	RotationRate                                =		( Roll=50000 )
	bCollideWorld                               =		true
	CheckRadius                                 =		42.0
	bCheckProjectileLight                       =		true
	MyProjectileLightClass                      =		class'UTGame.UTRocketLight'
	MyProjExplosionLightClass                   =		class'UTGame.UTRocketExplosionLight'
	ProjectileLifespan                          =		8.0
	bWaitForEffects                             =		true
	bAttachExplosionToVehicles                  =		false
	ProjPhysics                                 =		PHYS_Projectile
	
	//Homing Related
	MyHomingTrackingStrength                    =		0.5
	MinHomingTrackingStrength                   =		0.1
	DeltaHomingIncrease                         =		0.01
	HomingCheckFrequency                        =		0.05
	LockTime                                    =		0.21
	RandomAngleTime                             =		0.5
	MaxAngleChangeWhileHoming                   =		30 
	SmoothRandomAngleTime                       =		0.2
}