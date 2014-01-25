//=============================================================================
// TC_Weapon_MineLauncherArchetype: Special Mine Launcher Weapon
//
// This weapon spawns a mine spawner projectile and sets an index to it.
// This keeps a track of the mines spawned and makes sure that only a certain number
// are active at a time.
//
// Author: Abhinav Choudhary, 2013
//=============================================================================
class TC_Weapon_MineLauncherArchetype extends TC_Weapon_BaseWeaponArchetype;

//=============================================================================
// Variables : TC_Weapon_MineLauncherArchetype class
//=============================================================================

//The counter increases each time you shoot
var                     int			    ShootCounter;

//The max number of mines active at a time.
var (MineLauncher)      int			    MaxMinesAtATime;

//This keeps track of the alive mines.
var                     array< int >    ArrayOfCurrentMineIndicies;

//=============================================================================
// Functions : TC_Weapon_MineLauncherArchetype class
//=============================================================================

//The mine porjectile spawned tells the weapon if it needs to die.
simulated function bool NeedsToDie( int index )
{
	local bool choice;

	choice = true;
	if ( ArrayOfCurrentMineIndicies.Find( index ) != INDEX_NONE )
		choice = false;

	return choice;
}

//This is called by the mine to tell the launcher that it died 
//(Either due to lifetime or due to explosion ).
simulated function ClearMineWithIndex( int index )
{
	ArrayOfCurrentMineIndicies.RemoveItem( index );
}

//This makes sure that a certain number of mines are active at a certain time.
simulated function AddNewMineIndex( int index )
{
	while( ArrayOfCurrentMineIndicies.Length > MaxMinesAtATime )
		ArrayOfCurrentMineIndicies.Remove( 0, 1 );
	ArrayOfCurrentMineIndicies.AddItem( index );
}

//Works as the regular projectile fire. Just sets the index.
//My version of projectile fire to use archetypes
//It spawns a projectile from the archetype and initializes it in the right direction
simulated function Projectile ProjectileFire()
{
	local vector                                    HitLocation;
	local vector		                            MuzzleLoc;
	local vector                                    AimingDirection;
	local Projectile	                            SpawnedProjectile;
	local TC_Weapon_BaseWeaponProjectileArchetype   CurrentProjectile;
	local TC_Weapon_MineProjectileArchetype         MineProjectile;
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
			
			//Set properties for the mine
			MineProjectile = TC_Weapon_MineProjectileArchetype( SpawnedProjectile );
			if( MineProjectile!=none)
			{
				ShootCounter++;
				MineProjectile.SetMineLauncher( Self );
				MineProjectile.SetMineIndex( ShootCounter );
				AddNewMineIndex( ShootCounter );
			}
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

//=============================================================================
// Default Properties : TC_Weapon_MineLauncherArchetype class
//=============================================================================

DefaultProperties
{
	ShootCounter = 0;
}
