//=============================================================================
// TC_Weapon_FlameThrowerSuper: Super Flame Thrower Class.
//
// Super Flame Thrower lays down the ground with napalm 
// Right now it spawns three lines which form a cone to spawn the napalm
//
// Author: Abhinav Choudhary, 2013
//=============================================================================
class TC_Weapon_FlameThrowerSuper extends TC_Weapon_SuperWeaponArchtype;

//=============================================================================
// Variables : TC_Weapon_FlameThrowerSuper class
//=============================================================================

//The ground fire projectile to spawn
var (SuperFlameThrower) TC_Weapon_BaseWeaponProjectileArchetype GroundFireProjectile;

//This is the actual size of the flame. We subdivide our 3 lines using this distance 
var (SuperFlameThrower) float									GroundFlameDistance;

//This is the frequency at which the ground flame is spawned
var (SuperFlameThrower) float									GroundFlameFrequency;

//The width of the cone
var (SuperFlameThrower) float									GroundFlameConeWidth;

//The percent at which the flame starts spawning. It is between 0 and 1
var (SuperFlameThrower) float									GroundFlameStartPercent;

//This counter is used by the ground flame to make sure that overlapping flames of different counter get deleted
var						int										SpawnCounter;

var						int										TotalSpawned;
var						float                                   GroundFlameFrequencyTimer;

//=============================================================================
// Functions : TC_Weapon_FlameThrowerSuper class
//=============================================================================

simulated function Tick( float DeltaTime )
{
	super.Tick( DeltaTime );
	GroundFlameFrequencyTimer-=DeltaTime;
}

//Sets up the ground fire
//This divides the line into subdivisions based on the flame lentgh
simulated function SetupGroundFire( vector StartTrace, vector EndTrace, int CurrentSpawncounter )
{
	local Projectile	    SpawnedProjectile;
	local vector            CurrentFlamePos, TraceDirection;
	local float				NumOfFlames, StartNumOfFlames;
	local float				i;
	
	TraceDirection = EndTrace - StartTrace;		
	NumOfFlames = vSize(TraceDirection) / GroundFlameDistance;
	StartNumOfFlames = GroundFlameStartPercent * NumOfFlames;
	NumOfFlames = ( 1 - GroundFlameStartPercent ) * NumOfFlames;
	TraceDirection = Normal(TraceDirection);

	for( i = StartNumOfFlames; i<NumOfFlames; i=i+1 )
	{
		CurrentFlamePos = StartTrace + ( TraceDirection * i * GroundFlameDistance );		

		//Set the flame to spawn close to the ground so it feels like napalm being sprayed
		CurrentFlamePos.Z = StartTrace.Z - 20;
		SpawnedProjectile = Spawn( GroundFireProjectile.Class,,,CurrentFlamePos, rotator(TraceDirection),GroundFireProjectile,true );
		
		//Sets the correct spawn counter as index.
		TC_Weapon_GroundFlameProjectile( SpawnedProjectile ).SetSpawnIndex( CurrentSpawncounter );
		
		//We don't want the projectile to move at all
		SpawnedProjectile.Speed = 0;

		TotalSpawned++;
		//`log("TotalSpawned "$TotalSpawned );
	}
}

//Copied from the base weapon Archetype. Modified to setup ground fire
simulated function DoAreaInstantFire( vector StartTrace, vector EndTrace )
{
	local Array<ImpactInfo>	            ImpactList;
	local Array<ImpactInfo>	            FinalImpactList;
	local int                           i;
	local vector                        LocalYAxis, LocalZAxis;
	local ImpactInfo                    RealImpact;
	local vector                        CurrentAimTraceExtent;

	//For Muzzleflash
	local ParticleSystemComponent       PSC;

	//For the subdivision loop
	local vector                        BottomLeftStart, BottomLeftEnd;
	local vector                        CurrentStartTrace, CurrentEndTrace;
	local vector                        EndLeft, EndRight;
	local float                         YIncrement, ZIncrement;
	local bool                          bBreakY, bBreakZ;

	//Get local axis in local space
	LocalYAxis = ( vect( 0, 1, 0 ) << Instigator.Rotation );
	LocalZAxis = ( vect( 0, 0, 1 ) << Instigator.Rotation );
		
	//Get the bottom left start and end, so you can make multiple lines
	BottomLeftStart = StartTrace - ( LocalYAxis * AreaTraceSize ) - ( LocalZAxis * AreaTraceSize );
	BottomLeftEnd = EndTrace - ( LocalYAxis * AreaTraceSize ) - ( LocalZAxis * AreaTraceSize );

	YIncrement = 0;
	ZIncrement = 0;
	bBreakY = false;
	bBreakZ = false;
	CurrentAimTraceExtent.X = AreaTraceSize;
	CurrentAimTraceExtent.Y = AreaTraceSize;
	CurrentAimTraceExtent.Z = AreaTraceSize;

	EndLeft = EndTrace - ( LocalYAxis * GroundFlameConeWidth );
	EndRight = EndTrace + ( LocalYAxis * GroundFlameConeWidth );

	//Setup ground fire
	if( GroundFlameFrequencyTimer <= 0 )
	{   
		GroundFlameFrequencyTimer = GroundFlameFrequency;
		SetupGroundFire( StartTrace, EndTrace, SpawnCounter );
		SetupGroundFire( StartTrace, EndLeft, SpawnCounter );
		SetupGroundFire( StartTrace, EndRight, SpawnCounter );
		SpawnCounter++;
	}

	//This loop fires a trace in every subdivision
	while( !bBreakZ )
	{
		if( ZIncrement > ( AreaTraceSize * 2.0 ) )
			bBreakZ = true;
		YIncrement = 0;
		bBreakY = false;

		ImpactList.Length = 0;

		while( !bBreakY )
		{
			if( YIncrement > ( AreaTraceSize * 2.0 ) )
				bBreakY = true;
			
			//This is the main part of the loop code
			CurrentStartTrace = BottomLeftStart + ( LocalYAxis * YIncrement ) + ( LocalZAxis * ZIncrement );
			CurrentEndTrace = BottomLeftEnd + ( LocalYAxis * YIncrement ) + ( LocalZAxis * ZIncrement );
			
			//Debug mode
			if ( bWeaponDebugModeOn && MyWeaponInfo.TracerRounds[Instigator.FiringMode] != none )
			{
				//Fire tracer rounds from our weapon muzzle. This assumes that our
				//weapons have a muzzle flash socket.	
				PSC = WorldInfo.MyEmitterPool.SpawnEmitter( MyWeaponInfo.TracerRounds[Instigator.FiringMode],CurrentStartTrace );
				PSC.SetVectorParameter( MyWeaponInfo.TraceEndPointName,CurrentEndTrace );
				PSC.ActivateSystem();
			}
			
			// Perform shot
			//This version sets is so that it used 0 extent traces
			//CalcWeaponFire( CurrentStartTrace, CurrentEndTrace, ImpactList );//, CurrentAimTraceExtent);
			
			//This does area based traces
			RealImpact = CalcWeaponFire( CurrentStartTrace, CurrentEndTrace, ImpactList, CurrentAimTraceExtent );
			
			//Adds the results to a final impact list.
			AddToFinalImpactList( ImpactList, FinalImpactList );

			YIncrement+=AreaTraceSubDivisionSize;
		}
		ZIncrement+=AreaTraceSubDivisionSize;
	}

	for ( i = 0; i < FinalImpactList.length; i++ )
	{
		ProcessInstantHit(CurrentFireMode, FinalImpactList[i]);
	}

	if ( MyWeaponInfo.TracerRounds[Instigator.FiringMode] != none)
	{
		//Fire tracer rounds from our weapon muzzle. This assumes that our
		//weapons have a muzzle flash socket named MuzzleFlashSocket.
		PSC = WorldInfo.MyEmitterPool.SpawnEmitter( MyWeaponInfo.TracerRounds[Instigator.FiringMode],StartTrace );
		PSC.SetVectorParameter( MyWeaponInfo.TraceEndPointName,EndTrace );
		PSC.ActivateSystem();
	}

	SetFlashLocation(RealImpact.HitLocation);
}

//This allows us to get the correct aiming direction
//This is highly specific to the way muzzle was setup for our game 
//and makes on the fly adjustments to get the right direction
//Basically a hack!!
simulated function vector GetAimingDirection( vector MuzzleLoc )
{
	//Making on the fly adjustment to get right direction
	local vector Dir;
	local rotator MyRot;

	MyRot.Yaw = -90*DegToUnrRot;
	Dir = GetMuzzleDirection();
	Dir = Dir << MyRot;
	return Dir;
}

simulated function SuperWeaponFireReady()
{	
	GotoState('WeaponWindUp');	
}

simulated function SuperWeaponFireEnd()
{	
	GotoState('WeaponWindDown');
}

//////////////////////////////////////////////////////////////////
// Windup State
//  
//  The gun goes to this state, plays the wind up animation then goes to fire state
//////////////////////////////////////////////////////////////////

simulated state WeaponWindUp
{
	simulated function bool RefireCheck()
	{
		// If weapon should keep on firing, then do not leave state and fire again.
		return true;
	}

	simulated function RefireCheckTimer()
	{		
			if( bShouldWeaponFireDuringWindUp ) 
			{
				if ( !bIsWeaponFireLooping )
			{
				PlayThirdPersonWeaponAnimation(WeaponFiringAnim,GetFireInterval(CurrentFireMode),false); //GetFireInterval(0),true);
			}
			if( !WeaponFireSoundLooping )
			{
				WeaponPlaySound( WeaponFireSound );
			}
				FireAmmunition();
			}
			return;		
	}

	simulated function WeaponSpinnedUp()
	{
		if ( RefireCheck() )
			GotoState('WeaponFiring');
	}

	simulated function bool IsFiring()
	{
		return true;
	}

	simulated function BeginState(name PreviousStateName)
	{			
		PlayThirdPersonWeaponAnimation( WeaponSpinUpAnim, WeaponSpinUptime, false );
	 	WeaponPlaySound(WeaponSpinUpSnd);
		
		if( bShouldWeaponFireDuringWindUp )
			TimeWeaponFiring( CurrentFireMode );
	 	SetTimer(WeaponSpinUpTime,false,'WeaponSpinnedUp');				
	}

	simulated function EndState(name NextStateName)
	{
		Super.EndState(NextStateName);
		ClearFlashLocation();
		ClearTimer('WeaponSpinnedUp');	// Catchall just in case.
	}
}

//////////////////////////////////////////////////////////////////
// Wind down State
// 
//  Copied and modified from Stinger
//  The gun goes to this state after it is done firing. plays the wind down animation before going to the shut down state
//////////////////////////////////////////////////////////////////
simulated state WeaponWindDown
{
	simulated function bool IsFiring()
	{
		return true;
	}

	simulated function Timer()
	{
		if ( bWeaponPutDown )
		{
			// if switched to another weapon, put down right away
			PutDownWeapon();
		}
		else
		{
			// Return to the active state
			GotoState('Active');
		}
	}

	simulated function EndState(Name NextStateName)
	{
		super.EndState(NextStateName);
	}

Begin:
	PlayThirdPersonWeaponAnimation( WeaponSpinDownAnim, WeaponSpinDownTime, false );
	WeaponPlaySound( WeaponSpinDownSnd );
	SetTimer( WeaponSpinDownTime ,false );
}

//////////////////////////////////////////////////////////////////
//  Firing state.
// 
//  Copied and modified from UTweapon.
//  Modified so that it goes to wind down once weapon finishes firing
//////////////////////////////////////////////////////////////////
simulated state WeaponFiring
{
	simulated function RefireCheckTimer()
	{
		if ( !bIsWeaponFireLooping )
			{
				PlayThirdPersonWeaponAnimation(WeaponFiringAnim,GetFireInterval(CurrentFireMode),false); //GetFireInterval(0),true);
			}
			if( !WeaponFireSoundLooping )
			{
				WeaponPlaySound( WeaponFireSound );
			}
			FireAmmunition();
			return;
	}

	simulated function BeginState(Name PreviousStateName)
	{		

		//super.BeginState(PreviousStateName);
	
		if( WeaponFireSoundLooping )
		{
			if(WeaponFireSoundComponent == none)
			{
				WeaponFireSoundComponent = CreateAudioComponent(WeaponFireSound, false, true);
			}
			if(WeaponFireSoundComponent != none)
				WeaponFireSoundComponent.FadeIn(0.2f,1.0f);
		}

		if ( bIsWeaponFireLooping )
		{
			PlayThirdPersonWeaponAnimation(WeaponFiringAnim,,true); //GetFireInterval(0),true);
		}
		if ( !bIsWeaponFireLooping )
			{
				PlayThirdPersonWeaponAnimation(WeaponFiringAnim,GetFireInterval(CurrentFireMode),false); //GetFireInterval(0),true);
			}
			if( !WeaponFireSoundLooping )
			{
				WeaponPlaySound( WeaponFireSound );
			}
		FireAmmunition();
		TimeWeaponFiring(CurrentFireMode);
	}

	simulated function EndState(Name NextStateName)
	{
			if(WeaponFireSoundComponent != none)
		{
				WeaponFireSoundComponent.FadeOut(0.2f,0.0f);
				WeaponFireSoundComponent=none;
		}
		super.EndState(NextStateName);
	}
	simulated function bool IsFiring()
	{
		return true;
	}	
}

//=============================================================================
// Default Properties : TC_Weapon_FlameThrowerSuper class
//=============================================================================

DefaultProperties
{
	GroundFlameDistance = 200;
	GroundFlameFrequency = 0.9;
	SpawnCounter = 0;
	PawnSuperAnimName = mainchar_anim_super_flame;

	GroundFlameConeWidth = 150;
	GroundFlameStartPercent = 0.3;
}
