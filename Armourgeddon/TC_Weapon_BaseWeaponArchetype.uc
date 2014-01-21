//=============================================================================
// TC_Weapon_BaseWeaponArchetype: Basic Weapon Archetype.
//
// Handles Firing logic for all kinds of ranged weapons needed for the game.
// Pretty generalized and specialized for all cases. Also handles special logic to
// add the ability of have a main hand and offhand weapon.
//
// Author: Abhinav Choudhary, 2013
//=============================================================================
class TC_Weapon_BaseWeaponArchetype extends UTWeapon
ClassGroup(Archetypes,Weapon)
HideCategories(Attachment,Collision,Debug,Mobile,Movement,Weapon,Object,Physics, Animations, Sounds, UTWeapon, FirstPerson, Inventory, Display, Advanced)
placeable;
    
//=============================================================================
// Variables : TC_Weapon_BaseWeaponArchetype class
//=============================================================================

//////////////////////////////////////////////////////////////////
// Weapon Attributes
//////////////////////////////////////////////////////////////////

//Weapons will only have a max of two fire modes.
const MaxFireModes = 2;

//This is the main struct that holds all of the dynamic weapon info for
//the weapons. 
struct WeaponInfo
{
    //Weapon's name
    var()         string              WeaponName;
	//A Description that shows up in the HUD
	var()         string              WeaponDescription;

	//Name of the archetype for third person attachment reference. Should be the exact archetype name.
    //We use the correct archetype name to dynamically get the archetype and the corresponding mesh.
    var()         string              WeaponArchetypeName; 
  
    //Mesh Information 
    var(Mesh)     SkeletalMesh        FirstPersonWeaponMesh;
    var(Mesh)     SkeletalMesh        ThirdPersonWeaponMesh;

	//The socket that the weapon is attached to.
	var(Mesh)     name                WeaponSocket;

	//Make sure both of them are the same
    var(Logic)    EWeaponFireType     WeaponFireType[MaxFireModes];
    
    //The projectile it fires incase we are using Projectile Fire.
    var(Logic)    TC_Weapon_BaseWeaponProjectileArchetype   WeaponProjectiles[MaxFireModes]; //For use with Projectile
    
    //Shot cost per weapon fire. Set to 0 for our game
    var(Logic)    int                 ShotCost[MaxFireModes] <ClampMin=0.0>;
    
    //Momentum for instant hits
    var(Logic)    int                 InstantHitMomentum[MaxFireModes] <ClampMin=0.0>;
 
	//Muzzle flash template for the weapon
    var(Visuals)  ParticleSystem      MuzzleFlashes[MaxFireModes];
    
    //The bullet tracer flight template. This is used for Instant as well as Projectile Fire
    var(Visuals)  ParticleSystem      TracerRounds[MaxFireModes];
	
    //The Target point set on the tracer particle system if a tracer is used
    var(Visuals)  name                TraceEndPointName;      

	var(UI)       int                 ImageIndex;
};
 
var(WeaponAttributes)   int                     BaseInstantHitDamage[MaxFireModes];
var(WeaponAttributes)   int                     BaseWeaponRange[MaxFireModes];

//This is basically in seconds * 100
var(WeaponAttributes)   int                     BaseFireInterval[MaxFireModes]; 

//Set it to a high value for our game
var(WeaponAttributes)   int                     BaseMaxAmmoCount;
 
//Weapon Attributes which will be set in
//object archetypes within the editor
var(WeaponAttributes)   editinline WeaponInfo   MyWeaponInfo;

//Base Spread the weapon starts at
var(WeaponAttributes)   float                   InitialSpread;

//Max Spread the weapon can get to while the weapon is firing
var(WeaponAttributes)   float                   MaxSpread;

//Spread goes to max in this many seconds
var(WeaponAttributes)   float                   SpreadIncrementTime;
var                     float                   SpreadIncrementRate;

//Spread goes to min in this many seconds
var(WeaponAttributes)   float                   SpreadDecrementTime;
var                     float                   SpreadDecrementRate;

//Is this a main hand weapon or an offhand weapon
var(WeaponAttributes)   bool                    bIsMainhand;

/**AimTraceExtent, AreaTraceSize, AreaTraceSubDivisionSize are used incase 
an instant fire is required with an AOE requirement.*/

//Max Trace Area for Instant fire. This is used incase AreaTraceSize is 0
var(WeaponAttributes)   vector                  AimTraceExtent;

//Make this non zero to fire multiple traces to give an aoe effect
var(WeaponAttributes)   float                   AreaTraceSize;

//The size thats used to sub divide the AreaTraceSize. 
//Ideal should be close to the area trace size so that less ray casts are done
var(WeaponAttributes)   float                   AreaTraceSubDivisionSize;

//Debug mode for weapons using instant fire. This spawns the tracer to give a better idea how many raycasts are being done
var(WeaponAttributes)   bool                    bWeaponDebugModeOn;

//Used for some weapon firing logic, to check if the weapon is still firing
var                     bool                    bIsWeaponFiring;

//Do you want the weapon to fire during wind up.
var(WeaponAttributes)   bool                    bShouldWeaponFireDuringWindUp;

//Super Weapon related
var(WeaponAttributes)   TC_Weapon_BaseWeaponArchetype SuperWeapon;

//////////////////////////////////////////////////////////////////
// Trevin Ints
//////////////////////////////////////////////////////////////////

//Range is 0 to 5 inclusive
var(TrevinInts)         int						FireInt;
var(TrevinInts)         int						StrrInt;
var(TrevinInts)         int						RangeInt;
var(TrevinInts)         int						AOEInt;

//////////////////////////////////////////////////////////////////
// Weapon Animations
//////////////////////////////////////////////////////////////////

//Weapons animset
var (TCAnimations)      AnimSet                 WeaponAnimset;

//Spin up animation name and time. Works as a mini cool down. Never set Time to 0.0, instead Set it to a very small number
var (TCAnimations)      name                    WeaponSpinUpAnim; 

//Spin up animation name and time. Works as a mini cool down. Never set Time to 0.0, instead Set it to a very small number
var (TCAnimations)      float                   WeaponSpinUpTime <ClampMin=0.001>;

//Spin down animation name and time. Works as a mini cool down. Never set Time to 0.0, instead Set it to a very small number
var (TCAnimations)      name                    WeaponSpinDownAnim;

//Spin down animation name and time. Works as a mini cool down. Never set Time to 0.0, instead Set it to a very small number
var (TCAnimations)      float                   WeaponSpinDownTime <ClampMin=0.001>;

//Weapon fire animation
var (TCAnimations)      name                    WeaponFiringAnim; 

// Is it a looping fire animation
var (TCAnimations)      bool                    bIsWeaponFireLooping; 

//////////////////////////////////////////////////////////////////
// Weapon Sounds
//////////////////////////////////////////////////////////////////

//Weapon Spin up and spin down Sound
var (TCSound)			SoundCue				WeaponSpinUpSnd;
var (TCSound)			SoundCue				WeaponSpinDownSnd;

//Weapon fire sound
var (TCSound)           SoundCue                WeaponFireSound;

//Sound cue is looping or not
var (TCSound)           bool                    WeaponFireSoundLooping;

//This is used to initialize a looping sound incase the sound is looping.
var                     AudioComponent		    WeaponFireSoundComponent;

//TODO. REMOVE THIS. This is for the inventory, it helps with the upgrade system
var                     int                     CurrentWeaponNumber;

//This is like a debug counter to keep in check the amount of traces
var                     int                     TraceCounter;

//This limits the number of traces that can be done in a frame for area based trace attacks
var (WeaponAttributes)  int                     TraceLimit;

//=============================================================================
// Functions : TC_Weapon_BaseWeaponArchetype class
//=============================================================================

//Setting up default props
simulated function PostBeginPlay()
{
	local int i;

	for( i = 0; i < MaxFireModes; i++ )
	{
		WeaponFireTypes[ i ] = MyWeaponInfo.WeaponFireType[ i ];
	}

	Spread[0] = InitialSpread;
	Spread[1] = InitialSpread;
	SpreadIncrementRate = (MaxSpread - InitialSpread)/FMax(0.0001, SpreadIncrementTime);
	SpreadDecrementRate = (MaxSpread - InitialSpread)/FMax(0.0001, SpreadDecrementTime);
	
	super.PostBeginPlay();
}

//Setup a tick function so that spread increases and decreases accordingly
simulated function Tick( float DeltaTime )
{
	if(bIsWeaponFiring)
	{
		Spread[0] += SpreadIncrementRate * DeltaTime;
		Spread[1] += SpreadIncrementRate * DeltaTime;
	}
	else
	{
		Spread[0] -= SpreadDecrementRate * DeltaTime;
		Spread[1] -= SpreadDecrementRate * DeltaTime;
	}

	Spread[0] = FClamp( Spread[0], InitialSpread, MaxSpread);
	Spread[1] = FClamp( Spread[1], InitialSpread, MaxSpread);
}

//////////////////////////////////////////////////////////////////
// Weapon helper functions
// Modified from UT version to use our archetype values
//////////////////////////////////////////////////////////////////

function class<Projectile> GetProjectileClass()
{
    return ( MyWeaponInfo.WeaponFireType[ CurrentFireMode ] ==  EWFT_Projectile ) ? MyWeaponInfo.WeaponProjectiles[CurrentFireMode].Class : None;
}
 
simulated event float GetTraceRange()
{
    return ( CurrentFireMode < MaxFireModes ) ? BaseWeaponRange[ CurrentFireMode ] : 1000;
}
 
simulated function float GetFireInterval( byte FireModeNum )
{	
   return fmax(BaseFireInterval[ CurrentFireMode ]/100.0f,0.01);
}

simulated function float MaxRange()
{
    local int i;
 
    // return the range of the fire mode that fires farthest
    if ( MyWeaponInfo.WeaponFireType[ CurrentFireMode ] == EWFT_InstantHit )
        CachedMaxRange = BaseWeaponRange[ CurrentFireMode ];
 
    for (i = 0; i < MaxFireModes; i++)
    {
        if (MyWeaponInfo.WeaponProjectiles[i] != None)
            CachedMaxRange = FMax(CachedMaxRange, MyWeaponInfo.WeaponProjectiles[i].static.GetRange());
    }
    return CachedMaxRange;
}

//Modified Death of Weapon to nto detach as we dont want it to be done by the weapon
simulated function DetachWeapon();

//Play the correct third person weapon animation.
//Calls the correct attachment to play the animation.
simulated function PlayThirdPersonWeaponAnimation(name AnimName, optional float Duration, optional bool bLoop, optional bool bRestartIfAlreadyPlaying = true, optional float StartTime=0.0f, optional bool bPlayBackwards=false)
{
	local TC_Weapon_BaseWeaponAttachment CurrentWeaponAttachment;
	local TC_Pawn CurrentPawn;

	CurrentPawn = TC_Pawn(Instigator);
	
	if( CurrentFireMode == 0 )
	{
		CurrentWeaponAttachment = TC_Weapon_BaseWeaponAttachment(CurrentPawn.CurrentWeaponAttachment);
	}
	else
	{
		CurrentWeaponAttachment = TC_Weapon_BaseWeaponAttachment(CurrentPawn.AltWeaponAttachment);
	}

	CurrentWeaponAttachment.PlayThirdPersonWeaponAnimation( AnimName, Duration, bLoop, bRestartIfAlreadyPlaying, StartTime, bPlayBackwards );
}
 
//////////////////////////////////////////////////////////////////
// Weapon Localization
//////////////////////////////////////////////////////////////////

simulated function string GetWeaponName()
{
	return MyWeaponInfo.WeaponName;
}

simulated function string GetWeaponDescription()
{
	return MyWeaponInfo.WeaponDescription;
}

//////////////////////////////////////////////////////////////////
// Ammo related
//
// Modified from UT version as we are not using any kind of ammo in our game
//////////////////////////////////////////////////////////////////

function int AddAmmo( int Amount )
{
   return BaseMaxAmmoCount;
}
 
simulated function bool AmmoMaxed(int mode)
{
    return true;
}
 
simulated function bool HasAmmo( byte FireModeNum, optional int Amount )
{
    return true;
}
 
simulated function bool HasAnyAmmo()
{
    return true;
}
 
//Bot related. None of our bots use this weapon. 
simulated function float DesireAmmo(bool bDetour)
{
    return (1.f - float(AmmoCount)/BaseMaxAmmoCount);
}
 
simulated function Loaded(optional bool bUseWeaponMax)
{
    if (bUseWeaponMax)
        AmmoCount = BaseMaxAmmoCount;
    else
        AmmoCount = 10000;
}
 
//////////////////////////////////////////////////////////////////
// Attachment related functions
//
// Copied from UTWeapon
// Modified to use our object archetype data
//////////////////////////////////////////////////////////////////
 
simulated function AttachWeaponTo( SkeletalMeshComponent MeshCpnt, optional Name SocketName )
{
    local TC_Pawn TC_TP;
 
    TC_TP = TC_Pawn(Instigator);
    // Our game is third person so we set the mesh to be hidden   		
    SetHidden(True);
    if (TC_TP != None)
    {
        Mesh.SetLightEnvironment(TC_TP.LightEnvironment);
        TC_TP.ArmsMesh[0].SetHidden(true);
        TC_TP.ArmsMesh[1].SetHidden(true);   
    }
     
    SetWeaponOverlayFlags(TC_TP);
 
    // Spawn the 3rd Person Attachment
    if (Role == ROLE_Authority && TC_TP != None)
    {
		//We use the correct archetype name to dynamically get the archetype and the corresponding mesh.
    	TC_TP.CurrentWeaponAttachmentArchetypeName = MyWeaponInfo.WeaponArchetypeName;
        TC_TP.CurrentWeaponAttachmentClass = AttachmentClass;
 
        if (WorldInfo.NetMode == NM_ListenServer || WorldInfo.NetMode == NM_Standalone || (WorldInfo.NetMode == NM_Client && Instigator.IsLocallyControlled()))
            TC_TP.WeaponAttachmentChanged();
    }
 
    SetSkin(UTPawn(Instigator).ReplicatedBodyMaterial);
}
 
//We don't attach a muzzle flash here as we are in 3rd person.
simulated function AttachMuzzleFlash()
{
    return;
}

//Disable the crosshair
simulated function ActiveRenderOverlays( HUD H )
{
    //Don't render weapon crosshairs here
}
 
//////////////////////////////////////////////////////////////////
// Weapon Firing related
//
// Copied and modified from UTWeapon
//////////////////////////////////////////////////////////////////

simulated function StartFire( byte FireModeNum )
{
	bIsWeaponFiring = true;
	CurrentFireMode = FireModeNum;
	super.StartFire( FireModeNum );
}

simulated function EndFire( byte FireModeNum )
{
	bIsWeaponFiring = false;
	super.EndFire( FireModeNum );
}

//Modified this because I was having issues with pending fire setting wrong firemode
simulated function SetCurrentFireMode(byte FiringModeNum)
{
	return;
	//Set weapon's current fire mode
// 	CurrentFireMode = FiringModeNum;
// 	// set on instigator, to replicate it to remote clients
// 	if( Instigator != None )
// 	{
// 		Instigator.SetFiringMode(Self, FiringModeNum);
// 	}
}

//To make sure the projectile is fired from the right position
//FireMode 0 implies mainhand weapon while FireMode 1 implies offhand weapon
//As the archetype doesn't really have a mesh, we ask the attachment to give us the revelant effect location
simulated function vector GetEffectLocation()
{
	return CurrentFireMode == 0 ? TC_Pawn(Instigator).CurrentWeaponAttachment.GetEffectLocation() : TC_Pawn(Instigator).AltWeaponAttachment.GetEffectLocation();
}

//This is use to get the direction the actual weapon is pointing towards ( during animation )
simulated function vector GetMuzzleDirection()
{
	local vector MuzzleDirection;
	if( CurrentFireMode == 0 )
	{
		MuzzleDirection = TC_Weapon_BaseWeaponAttachment( TC_Pawn(Instigator).CurrentWeaponAttachment ).GetMuzzleRotation();
	}
	else
	{
		MuzzleDirection = TC_Weapon_BaseWeaponAttachment( TC_Pawn(Instigator).AltWeaponAttachment ).GetMuzzleRotation();
	}
	return MuzzleDirection;
}

//This function is modified from the UTversion to get t
//he correct aiming direction.
//As our game is in third person and we have multiple weapons, 
//we get different aiming directions based on different cases.
//Case 1. 
//The reticle is not pointing at anything relevant or is pointing at something too close. 
//Then the gun aims at a line parrallel to the reticle direction.
//Case 2.
//If the reticle is pointing at something relevant( Wall or enemy ),
//the gun's aiming direction will be from muzzle to the enemy.
simulated function vector GetAimingDirection( vector MuzzleLoc )
{
	local vector			StartTrace, EndTrace;
	local Actor				HitActor;
	local vector			HitLocation, HitNormal, AimDir;
	local TraceHitInfo		HitInfo;

	// We get the start location from the pawn.
	StartTrace = GetPhysicalFireStartLoc();

	// The end trace is calculated using the direction.
	EndTrace = StartTrace + vector(GetAdjustedAim(StartTrace)) * MaxRange();

	// Perform trace to retrieve hit info
	HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, EndTrace, StartTrace, TRUE,, HitInfo, TRACEFLAG_Bullet);

	//Use AimTraceExtent if we need some auto aim help.
	//HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, EndTrace, StartTrace, TRUE, AimTraceExtent, HitInfo, TRACEFLAG_Bullet);

	if( HitActor == None || HitInfo.HitComponent == none || vSize( HitLocation - MuzzleLoc) < 300)
	{
		//Just aim in a parallel direction
		AimDir = Vector( GetAdjustedAim( MuzzleLoc ) );
	}
	else
	{
		AimDir = Normal( HitLocation - MuzzleLoc );		
	}

	return Aimdir;
}

//Mostly copied from Weapon.uc. Modified to have a bit of adjusted aim
simulated function vector InstantFireEndTrace(vector StartTrace)
{
	return StartTrace + GetAimingDirection(StartTrace) * GetTraceRange();
}

//This is modified to make ceratin relevant classes to the game being able to pass through. This causes a peircing effect.
simulated static function bool PassThroughDamage(Actor HitActor)
{
	return (!HitActor.bBlockActors && (HitActor.IsA('Trigger') || HitActor.IsA('TriggerVolume')))
		|| HitActor.IsA('InteractiveFoliageActor') ||HitActor.IsA('KActor') || HitActor.IsA('TC_Bot') ||HitActor.IsA('TC_BotPawn')||HitActor.IsA('StaticMeshActor');
}

//This is modified from the UT version a bit to use Archetype instant hit damage
simulated function ProcessInstantHit( byte FiringMode, ImpactInfo Impact, optional int NumHits )
{
    local bool bFixMomentum;
    local KActorFromStatic NewKActor;
    local StaticMeshComponent HitStaticMesh;
    local int TotalDamage;
 
    if ( Impact.HitActor != None )
    {
        if ( Impact.HitActor.bWorldGeometry )
        {
            HitStaticMesh = StaticMeshComponent(Impact.HitInfo.HitComponent);
            if ( (HitStaticMesh != None) && HitStaticMesh.CanBecomeDynamic() )
            {
                NewKActor = class'KActorFromStatic'.Static.MakeDynamic(HitStaticMesh);
                if ( NewKActor != None )
                {
                    Impact.HitActor = NewKActor;
                }
            }
        }
        if ( !Impact.HitActor.bStatic && (Impact.HitActor != Instigator) )
        {
            if ( Impact.HitActor.Role == ROLE_Authority && Impact.HitActor.bProjTarget
                && !WorldInfo.GRI.OnSameTeam(Instigator, Impact.HitActor)
                && Impact.HitActor.Instigator != Instigator
                && PhysicsVolume(Impact.HitActor) == None )
            {
                HitEnemy++;
                LastHitEnemyTime = WorldInfo.TimeSeconds;
            }
            if ( (UTPawn(Impact.HitActor) == None) && (MyWeaponInfo.InstantHitMomentum[FiringMode] == 0) )
            {
                MyWeaponInfo.InstantHitMomentum[FiringMode] = 1;
                bFixMomentum = true;
            }
 
            // default damage model is just hits * base damage
            NumHits = Max(NumHits, 1);
            TotalDamage = BaseInstantHitDamage[FiringMode] * NumHits;
 
            Impact.HitActor.TakeDamage( TotalDamage, Instigator.Controller,
                        Impact.HitLocation, MyWeaponInfo.InstantHitMomentum[FiringMode] * Impact.RayDir,
                        InstantHitDamageTypes[FiringMode], Impact.HitInfo, self );
 
            if (bFixMomentum)
                MyWeaponInfo.InstantHitMomentum[FiringMode] = 0;
        }
    }
}

//This makes an exclusive list of impacts. 
//It make sures that every actor is only counted once
simulated function AddToFinalImpactList( array<ImpactInfo> ImpactList, optional out array<ImpactInfo> FinalImpactList )
{
	local int i, j;
	local bool bFound;

	bFound = false;

	for( i = 0; i<ImpactList.Length; ++i )
	{
		bFound = false;
		for( j = 0; j<FinalImpactList.Length; ++j )
		{
			if( FinalImpactList[j].HitActor == ImpactList[i].HitActor )
			{
				bFound = true;
				break;
			}
		}
		if( !bFound )
		{
			FinalImpactList[ FinalImpactList.Length ] = ImpactList[i];
		}
	}
}

//This function was written as using Extent in the trace function was
//not getting a proper peircing effect. Their were cases when the ground 
//would block the trace and player would not get any sort of hit.
//This function breaks the trace area extent into subdivisions and does 
//a number of extent based traces.
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


	TraceCounter = 0;

	//This loop fires a trace in every subdivision
	while( !bBreakZ )
	{
		if( ZIncrement >= ( AreaTraceSize * 2.0 ) )
			bBreakZ = true;
		YIncrement = 0;
		bBreakY = false;

		ImpactList.Length = 0;

		while( !bBreakY )
		{
			if( YIncrement >= ( AreaTraceSize * 2.0 ) )
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

			//`log("Current Debug Counter" @TraceCounter );

			YIncrement+=AreaTraceSubDivisionSize ;
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
	
	TraceCounter=-1;

	SetFlashLocation(RealImpact.HitLocation);
}

//My version of instant fire. Mostly copied from UTWeapon and modified
//The instant fire does a regular instant fire if the area trace size 
//is equal to zero. Otherwise it does an area instant fire
simulated function InstantFire()
{
	local vector					StartTrace, EndTrace;
	local Array<ImpactInfo>			ImpactList;
	local ImpactInfo				RealImpact, NearImpact;
	local int                       i;

	local ParticleSystemComponent   PSC;

	// Get the start location for CalcWeaponFire()
	StartTrace = GetEffectLocation();
	
	EndTrace = InstantFireEndTrace(StartTrace);	

	//Tell the player controller that the weapon was fired so it can display a mini cooldown on hud
	TC_PlayerController( TC_Pawn( Instigator ).Controller ).FiredWeapon( int(bIsMainhand) + 1 );

	//If this is equal to zero, do a regular instant hit
	if( AreaTraceSize <=0 )
	{
		// Perform shot
		RealImpact = CalcWeaponFire(StartTrace, EndTrace, ImpactList, AimTraceExtent);		

		for (i = 0; i < ImpactList.length; i++)
		{
			ProcessInstantHit(CurrentFireMode, ImpactList[i]);
		}

		if ( MyWeaponInfo.TracerRounds[Instigator.FiringMode] != none)
			{
				//Fire tracer rounds from our weapon muzzle. This assumes that our
				//weapons have a muzzle flash socket 	
				PSC = WorldInfo.MyEmitterPool.SpawnEmitter( MyWeaponInfo.TracerRounds[Instigator.FiringMode],StartTrace );
				PSC.SetVectorParameter( MyWeaponInfo.TraceEndPointName,EndTrace );
				PSC.ActivateSystem();
			}

		if (Role == ROLE_Authority)
		{
			// Set flash location to trigger client side effects.
			// if HitActor == None, then HitLocation represents the end of the trace (maxrange)		
			if ( NearImpact.HitActor != None )
			{
				SetFlashLocation(NearImpact.HitLocation);
			}
			else
			{
				SetFlashLocation(RealImpact.HitLocation);
			}
		}
	}
	else
	{
		// An area based instant fire
		DoAreaInstantFire( StartTrace, EndTrace );
	}
}

//My version of projectile fire to use archetypes
//It spawns a projectile from the archetype and initializes it in the right direction
simulated function Projectile ProjectileFire()
{
	local vector                                    HitLocation;
	local vector		                            MuzzleLoc;
	local vector                                    AimingDirection;
	local Projectile	                            SpawnedProjectile;
	local TC_Weapon_BaseWeaponProjectileArchetype   CurrentProjectile;
	local ParticleSystemComponent                   PSC;
	
	//Tell the player controller that the weapon was fired so it can display a mini cooldown on hud
	TC_PlayerController( TC_Pawn( Instigator ).Controller ).FiredWeapon( int(bIsMainhand) + 1 );

	CurrentProjectile =	MyWeaponInfo.WeaponProjectiles[ CurrentFireMode ];
	
	// Tell remote clients that we fired, to trigger effects
	IncrementFlashCount();

	if( Role == ROLE_Authority )
	{
		// This is the location where the projectile is spawned.
		MuzzleLoc = GetEffectLocation();

		// Get the corret aiming direction
		AimingDirection = GetAimingDirection( MuzzleLoc );

		// Spawn projectile
		SpawnedProjectile = Spawn( CurrentProjectile.Class,,, MuzzleLoc, , CurrentProjectile );

		if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Init( Normal(AimingDirection) );			
		}
		
		// Fire a tracer if we have a tracer setup
		if ( MyWeaponInfo.TracerRounds[Instigator.FiringMode]!= none )
		{
			//Fire tracer rounds from our weapon muzzle. This assumes that our
			//weapons have a muzzle flash socket
			HitLocation = MuzzleLoc + Normal(AimingDirection) * 3000.0; //*MaxRange(); 3000 gives the right look for the gattling gun
			PSC = WorldInfo.MyEmitterPool.SpawnEmitter( MyWeaponInfo.TracerRounds[Instigator.FiringMode],MuzzleLoc);;
			PSC.SetVectorParameter(MyWeaponInfo.TraceEndPointName,HitLocation);
			PSC.ActivateSystem();
		}

		// Return it up the line
		return SpawnedProjectile;
	}

	return None;
}

//Copied from UT Weapon. Modified to allow fire to pass through static meshes
simulated function ImpactInfo CalcWeaponFire(vector StartTrace, vector EndTrace, optional out array<ImpactInfo> ImpactList, optional vector Extent)
{
	local vector			HitLocation, HitNormal, Dir;
	local Actor				HitActor;
	local TraceHitInfo		HitInfo;
	local ImpactInfo		CurrentImpact;
	local PortalTeleporter	Portal;
	local float				HitDist;
	local bool				bOldBlockActors, bOldCollideActors;

	// Perform trace to retrieve hit info
	HitActor = GetTraceOwner().Trace(HitLocation, HitNormal, EndTrace, StartTrace, TRUE, Extent, HitInfo, TRACEFLAG_Bullet);

	if( TraceCounter >=0 )
		TraceCounter++;

	

	// If we didn't hit anything, then set the HitLocation as being the EndTrace location
	if( HitActor == None )
	{
		HitLocation	= EndTrace;
	}

	// Convert Trace Information to ImpactInfo type.
	CurrentImpact.HitActor		= HitActor;
	CurrentImpact.HitLocation	= HitLocation;
	CurrentImpact.HitNormal		= HitNormal;
	CurrentImpact.RayDir		= Normal(EndTrace-StartTrace);
	CurrentImpact.StartTrace	= StartTrace;
	CurrentImpact.HitInfo		= HitInfo;

	if( TraceCounter > TraceLimit )
		return CurrentImpact;

	// Add this hit to the ImpactList
	ImpactList[ImpactList.Length] = CurrentImpact;

	// check to see if we've hit a trigger.
	// In this case, we want to add this actor to the list so we can give it damage, and then continue tracing through.
	if( HitActor != None )
	{
		if (PassThroughDamage(HitActor))
		{
			// disable collision temporarily for the actor we can pass-through
			HitActor.bProjTarget = false;
			bOldCollideActors = HitActor.bCollideActors;
			bOldBlockActors = HitActor.bBlockActors;
			if (HitActor.IsA('Pawn') || HitActor.IsA('StaticMeshActor'))
			{
				// For pawns, we need to disable bCollideActors as well
				HitActor.SetCollision(false, false);

				// recurse another trace
				CalcWeaponFire(HitLocation, EndTrace, ImpactList, Extent);
			}
			else
			{
				if( bOldBlockActors )
				{
					HitActor.SetCollision(bOldCollideActors, false);
				}
				// recurse another trace and override CurrentImpact
				CurrentImpact = CalcWeaponFire(HitLocation, EndTrace, ImpactList, Extent);
			}

			// and reenable collision for the trigger
			HitActor.bProjTarget = true;
			HitActor.SetCollision(bOldCollideActors, bOldBlockActors);
		}
		else
		{
			// if we hit a PortalTeleporter, recurse through
			Portal = PortalTeleporter(HitActor);
			if( Portal != None && Portal.SisterPortal != None )
			{
				Dir = EndTrace - StartTrace;
				HitDist = VSize(HitLocation - StartTrace);
				// calculate new start and end points on the other side of the portal
				StartTrace = Portal.TransformHitLocation(HitLocation);
				EndTrace = StartTrace + Portal.TransformVectorDir(Normal(Dir) * (VSize(Dir) - HitDist));
				//@note: intentionally ignoring return value so our hit of the portal is used for effects
				//@todo: need to figure out how to replicate that there should be effects on the other side as well
				CalcWeaponFire(StartTrace, EndTrace, ImpactList, Extent);
			}
		}
	}

	return CurrentImpact;
}

//////////////////////////////////////////////////////////////////
// Windup State
// 
// Copied from Stinger
// The gun goes to this state, plays the wind up animation then goes to fire state
//////////////////////////////////////////////////////////////////

simulated state WeaponWindUp
{
	simulated function bool RefireCheck()
	{
		// if switching to another weapon, abort firing and put down right away
		//if( bWeaponPutDown )
		//{
		//	GotoState('WeaponWindDown');
		//	return false;
		//}

		// If weapon should keep on firing, then do not leave state and fire again.
		//if( ShouldRefire() )
		//{		
			return true;
		//}

		// if out of ammo, then call weapon empty notification
		if( !HasAnyAmmo() )
		{
			GotoState('Active');
			WeaponEmpty();
			return false;
		}
		else
		{
			GotoState('WeaponWindDown');
			return false;
		}
	}

	simulated function RefireCheckTimer()
	{
		// if switching to another weapon, abort firing and put down right away
		//if( bWeaponPutDown )
		//{
		//	GotoState('WeaponWindDown');
		//	return;
		//}

		// If weapon should keep on firing, then do not leave state and fire again.
		//if( ShouldRefire() )
		//{
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
		//}

		// if out of ammo, then call weapon empty notification
		if( !HasAnyAmmo() )
		{
			GotoState('Active');
			WeaponEmpty();
	
		}
		else
		{
			GotoState('WeaponWindDown');
		}
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
	 	WeaponPlaySound( WeaponSpinUpSnd );
		
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
// Winddown State
// 
// Copied and modified from Stinger
// The gun goes to this state after it is done firing. plays the wind down animation before going to the shut down state
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
		//ClearPendingFire(CurrentFireMode);
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
		// if switching to another weapon, abort firing and put down right away
		if( bWeaponPutDown )
		{
			GotoState('WeaponWindDown');
			return;
		}

		// If weapon should keep on firing, then do not leave state and fire again.
		if( ShouldRefire() )
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

		// if out of ammo, then call weapon empty notification
		if( !HasAnyAmmo() )
		{
			GotoState('Active');
			WeaponEmpty();
		}
		else
		{
			GotoState('WeaponWindDown');
		}
	}

	simulated function BeginState(Name PreviousStateName)
	{		
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
			PlayThirdPersonWeaponAnimation( WeaponFiringAnim,GetFireInterval(CurrentFireMode),false ); //GetFireInterval(0),true);
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

//////////////////////////////////////////////////////////////////
// Super Weapon function declarations
// Super Weapons are child of this base class, so added some 
// helper functions which can be called from other classes 
// without typecasting
//////////////////////////////////////////////////////////////////

//Related to super weapon special fire
simulated function SuperWeaponFireReady();
simulated function SuperWeaponFireEnd();

//=============================================================================
// Default Properties : TC_Weapon_BaseWeaponArchetype class
//=============================================================================

defaultproperties
{
    AttachmentClass				=           class'TC_Weapon_BaseWeaponAttachment'
	MuzzleFlashSocket			=           MussleFlashSocket
	bIsWeaponFiring				=           false;
	SpreadIncrementTime			=           10.0
	
	//Decides what state to go to on start of fire
	FiringStatesArray(0)		=           "WeaponWindUp"
	FiringStatesArray(1)		=           "WeaponWindUp"
	WeaponSpinUpTime			=           0.001;
	WeaponSpinDowntime			=           0.001;
	bIsWeaponFireLooping		=           false;

	InstantHitDamageTypes(0)	=			class'TC_Weapon_DmgType_Mainhand'
	InstantHitDamageTypes(1)	=			class'TC_Weapon_DmgType_Mainhand'

	AreaTraceSize               =           0;
	AreaTraceSubDivisionSize    =           2;
	bWeaponDebugModeOn          =           false;

	TraceCounter                =           -1;
	TraceLimit                  =           50;
}