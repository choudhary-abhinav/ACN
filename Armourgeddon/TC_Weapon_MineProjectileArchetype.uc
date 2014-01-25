//=============================================================================
// TC_Weapon_MineProjectileArchetype: Basic Weapon Projectile Archetype.
//
// This projectile sticks on a ground, then activates and acts like a proximity mine
// It also checks every few ticks wether it needs to explode as number of mines 
// crossed the certain number.
//
// Author: Abhinav Choudhary, 2013
//=============================================================================
class TC_Weapon_MineProjectileArchetype extends TC_Weapon_BaseWeaponProjectileArchetype;

//=============================================================================
// Variables : TC_Weapon_MineProjectileArchetype class
//=============================================================================

//If the life time is greater than the BlastTimer, it explodes
var (Mine)          float       BlastTimer;

//The proximity mine test radius
var (Mine)          float       TestRadius;

//The sound to play once the mine activates
var (Mine)          SoundCue    ActivationSound;

//Current mine index.
var					int         MineIndex;

var					float       LifeTime;
var					bool		bWantsToDie;
var					bool		bAlreadyKilled;

//Variables used for detecting ground and attaching.
var					vector      NormalHit;
var					rotator     WallRotation;

//Reference to the min launcher
var TC_Weapon_MineLauncherArchetype MineLauncher;

//Has it touched the ground yet?
var bool isOnGround;

//=============================================================================
// Functions : TC_Weapon_MineProjectileArchetype class
//=============================================================================

//Set Ref to launcher
simulated function SetMineLauncher( TC_Weapon_MineLauncherArchetype Launcher )
{
	MineLauncher = Launcher;
}

simulated function SetMineIndex( int index )
{
	MineIndex = index;
}

//Tells the launcher that it died.
simulated function Explode(vector HitLocation, vector HitNormal)
{
	if( MineLauncher!=none )
		MineLauncher.ClearMineWithIndex( MineIndex );
	super.Explode( HitLocation + ( HitNormal * 10 ), HitNormal );
}

//Explodes if lifetime passed or too many mines present
//This can be optimized.
simulated event Tick(float DeltaTime)
{
	local TC_BotPawn Victim;

	if( isOnGround == true ) 
	{
		foreach CollidingActors( class'TC_BotPawn', Victim, 160.0 )
		{
			if( Victim!= Owner )
 			bWantsToDie = true;
		}
	}

	LifeTime += DeltaTime;

	if( !bWantsToDie && ( LifeTime > BlastTimer ) )
		bWantsToDie = true;	
	
	if( MineLauncher!=none)
	{
		if( !bWantsToDie && MineLauncher.NeedsToDie( MineIndex ) )
			bWantsToDie = true;
	}

	super.Tick( DeltaTime );
	
	if( bWantsToDie && !bAlreadyKilled )
	{
		bAlreadyKilled = true;
		Explode(Location, NormalHit );
		Destroy();
	}
}

//Add a bit of randomness to make it feel good
function Init(vector Direction)
{
	SetRotation(Rotator(Direction));

	Velocity = Speed * Direction;
	TossZ = TossZ + (FRand() * TossZ / 2.0) - (TossZ / 4.0);
	Velocity.Z += TossZ;
	Acceleration = AccelRate * Normal(Velocity);
}

//Die if you are damaged.
simulated event TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	super.TakeDamage(DamageAmount, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
	bWantsToDie = true;
}

simulated function ProcessTouch ( Actor Other, vector HitLocation, vector HitNormal )
{
	if ( Pawn( Other ) == none )
	{
		bWantsToDie = true;
	}
}

simulated event HitWall(vector HitNormal, Actor Wall, PrimitiveComponent WallComp)
{
	if ( Pawn(Wall) != none )
	{
		//Bounce this off stuff
		Velocity = 0.75*(( Velocity dot HitNormal ) * HitNormal * -2.0 + Velocity);   // Reflect off Wall w/damping
		Speed = VSize(Velocity);

		if (Velocity.Z > 400)
		{
			Velocity.Z = 0.5 * (400 + Velocity.Z);
		}
	}
	else
	{
		WallRotation.Pitch = -HitNormal.X * 16384;
		WallRotation.Roll = HitNormal.Y * 16384;
		WallRotation.Yaw = HitNormal.Z * 16384;

		SetRotation( WallRotation );
		
		//Touched ground
		isOnGround=true;
		SetPhysics( PHYS_None );
		
		SetCollisionType( COLLIDE_TouchAll );
		Velocity *= 0;
		Speed = 0;
		MomentumTransfer = 1.0;
		
		bBlockedByInstigator = true;

		NormalHit = HitNormal;
		ImpactedActor = Wall;

		if ( ActivationSound != None )
		{
			PlaySound(ActivationSound, true);
		}
	}
}

//=============================================================================
// Default Properties : TC_Weapon_MineProjectileArchetype class
//=============================================================================

defaultproperties
{
	bCanBeDamaged = true;	
	isOnGround = false;
	
	NormalHit = (X=0, Y=0, Z=1)

	Physics = PHYS_Falling	
	TossZ = +245.0
	CustomGravityScaling=0.7
	
	TestRadius = 160.0
   
    bCollideActors=true
    bBlockActors=true
    bBounce=true	
	bWideCheck=true
	RotationRate=(Roll=50000)
    bCollideWorld=true
	DrawScale=1.5
    CheckRadius=42.0
    LifeTime = 0.0;
	bWantsToDie = false;
	bAlreadyKilled = false;
}

