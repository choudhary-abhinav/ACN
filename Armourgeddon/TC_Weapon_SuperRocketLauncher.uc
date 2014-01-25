//=============================================================================
// TC_Weapon_SuperRocketLauncher: Super Rocket Launcher Archetype.
//
// This weapon is like the best weapon ever
// Spawns a bunch of rockets which home at different targets!
// Timeline : Charge -> -> Shoot/Shoot Loop -> -> End
//
// Author: Abhinav Choudhary, 2013
//=============================================================================

class TC_Weapon_SuperRocketLauncher extends TC_Weapon_SuperWeaponArchtype;

//=============================================================================
// Variables : TC_Weapon_SuperRocketLauncher class
//=============================================================================

//The number of rockets to spawn when shot
var(Super)      int         NumberOfRocketsSpawned;

//The max range infront of the player the bots will be considered valid for homing
var(Super)      float       RangeOfHoming;

//A struct to store the bots rating regarding homing.
struct BotRating
{
	var TC_BotPawn VictimBot;
	var float DotProduct;
};

//=============================================================================
// Functions : TC_Weapon_SuperRocketLauncher class
//=============================================================================

//A delegate to sort the bots according to rating
function int SortBot(BotRating A, BotRating B)
{
    return (A.DotProduct < B.DotProduct) ? -1 : 0;
}

//Adds a bort to a sorted list. Then reduces the lentgh if its more than the number of rockets
simulated function AddToSortedList( out Array<BotRating> AvailableBotPawns, BotRating CurrBot )
{
	AvailableBotPawns.AddItem( CurrBot );
	AvailableBotPawns.Sort( SortBot );
	if( AvailableBotPawns.Length > NumberOfRocketsSpawned )
		AvailableBotPawns.Length = NumberOfRocketsSpawned;
}

//Sets up bots worth aiming. It picks up bot closer to the players view direction.
//Also, it makes a sorted list with unique bots.
simulated function SetupAimableBots( out Array<TC_Bot> AvailableBots, vector MuzzleLoc, vector AimingDirection )
{
	local TC_BotPawn Victim;
	local Array<BotRating> AvailableBotPawns;
	local BotRating CurrBot;

	local int i;
	local Float thisDot;

	//Make the list
	foreach VisibleCollidingActors( class'TC_BotPawn', Victim, RangeOfHoming, MuzzleLoc )
	{
		thisDot = Normal( AimingDirection ) Dot	Normal(Victim.Location - MuzzleLoc);
		if( thisDot >=0 && Victim.Health > 0)
		{
			CurrBot.VictimBot = Victim;
			CurrBot.DotProduct = thisDot;
			AddToSortedList( AvailableBotPawns, CurrBot );
		}

	}

	for( i = 0; i<AvailableBotPawns.Length; ++i )
	{
		AvailableBots.AddItem( TC_Bot(AvailableBotPawns[i].VictimBot.Controller) );
	}
}

//This spawns the required number of rockets and sets them to home to their respective targets
simulated function Projectile ProjectileFire()
{
	local vector									HitLocation;
	local vector									MuzzleLoc;
	local vector									AimingDirection;
	local Projectile								SpawnedProjectile;
	local TC_Weapon_BaseWeaponProjectileArchetype   CurrentProjectile;
	local ParticleSystemComponent                   PSC;
	local TC_Bot                                    TargetBot;

	//Available bots within range to home
	local Array<TC_Bot>                             AvailableBots;
	local int i;
	local int iBot;

	CurrentProjectile =	MyWeaponInfo.WeaponProjectiles[ CurrentFireMode ];
	
	// tell remote clients that we fired, to trigger effects
	IncrementFlashCount();

	if( Role == ROLE_Authority )
	{
		// this is the location where the projectile is spawned.
		MuzzleLoc = GetEffectLocation();
		AimingDirection = GetAimingDirection( MuzzleLoc );

		//Setup the bots to aim at
		SetupAimableBots( AvailableBots, MuzzleLoc, AimingDirection );
		
		// Spawn reqd projectile		
		for( i = 0; i < NumberOfRocketsSpawned; ++i )
		{
			SpawnedProjectile = Spawn(CurrentProjectile.Class,,, MuzzleLoc, , CurrentProjectile);

			if( AvailableBots.Length > 0 )
			{
				TargetBot = AvailableBots[iBot];
				iBot++;
				if(iBot>AvailableBots.Length)
					iBot = 0;
			}
			
			if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
			{
				SpawnedProjectile.Init( -GetMuzzleDirection() );
				
				//Start homing for the target.
				if( TargetBot != none )
				{
					TC_Weapon_BaseWeaponProjectileArchetype(SpawnedProjectile).GiveTarget( TargetBot );
				}
			}
			
			if ( MyWeaponInfo.TracerRounds[Instigator.FiringMode] != none)
			{
				//Fire tracer rounds from our weapon muzzle. This assumes that our
				//weapons have a muzzle flash socket 
				
				HitLocation = MuzzleLoc + Vector(GetAdjustedAim( MuzzleLoc )) * 3000.0;
				PSC = WorldInfo.MyEmitterPool.SpawnEmitter( MyWeaponInfo.TracerRounds[Instigator.FiringMode],MuzzleLoc);;
				PSC.SetVectorParameter(MyWeaponInfo.TraceEndPointName,HitLocation);
				PSC.ActivateSystem();
			}
		}
		// Return it up the line
		return SpawnedProjectile;
	}

	return None;
}

//=============================================================================
// Default Properties : TC_Weapon_SuperRocketLauncher class
//=============================================================================

DefaultProperties
{
	NumberOfRocketsSpawned  = 10;
	RangeOfHoming           = 6000;
}
