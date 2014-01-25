//=============================================================================
// TC_Weapon_SuperWeaponArchtype: Mainhand SuperWeapon Archetype.
//
// Mainhand super weapons have specific animations and behavior. 
// Their attacks are specified in the player's animation. So we use anim notifys 
// to do the attack at the right time. This class will write a new state system to handle 
// all the player related animation logic
//
// Timeline : Charge -> -> Shoot/Shoot Loop -> -> End
//
// Author: Abhinav Choudhary, 2013
//=============================================================================
class TC_Weapon_SuperWeaponArchtype extends TC_Weapon_BaseWeaponArchetype;

//=============================================================================
// Variables : TC_Weapon_SuperWeaponArchtype class
//=============================================================================

//The Player anim name that the pawn should do while doing this attack
var (SuperWeapon)               name        PawnSuperAnimName;

//Sounds related to the super. 
var (SuperWeaponSound)          SoundCue    FirstSound;
var (SuperWeaponSound)          SoundCue    SecondSound;

//=============================================================================
// Functions : TC_Weapon_SuperWeaponArchtype class
//=============================================================================

//Anim notify from pawn calls this function to start firing.
simulated function SuperWeaponFireReady()
{
	GotoState('WeaponFiring');
}

//Anim notify from pawn calls this function to end firing.
simulated function SuperWeaponFireEnd()
{
	GotoState('WeaponWindDown');
}

//////////////////////////////////////////////////////////////////
// WeaponSuperCharge State
// 
// This state is the state where the player pawn animation starts charging
//////////////////////////////////////////////////////////////////

simulated state WeaponSuperCharge
{
	simulated function bool IsFiring()
	{
		return true;
	}

	simulated function BeginState( Name PreviousStateName )
	{
		//We don't want to start firing while the pawn is jumping as this 
		//causes conflicts with the player anim. (The way the animtree was set)
		if( !TC_Pawn(Instigator).bIsJumping )
		{
			TC_Pawn(Instigator).DoRangedAttackAnim( PawnSuperAnimName );
			WeaponPlaySound( FirstSound );
			WeaponPlaySound( SecondSound );
		}
		else
		{
			TC_Pawn(Instigator).GiveSpecialCurrency( TC_Pawn(Instigator).PlayerConfigProfile.BaseMaxCurrency * 0.5 );
			GotoState('WeaponWindDown');
		}
	}
}

//////////////////////////////////////////////////////////////////
//  Firing state.
// 
//  Copied and modified from UTweapon.
//  This state only fires whenever SuperWeaponFireReady is called
//////////////////////////////////////////////////////////////////

simulated state WeaponFiring
{
	simulated function SuperWeaponFireReady()
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
				PlayThirdPersonWeaponAnimation(WeaponFiringAnim,GetFireInterval(CurrentFireMode),false); //GetFireInterval(0),true);
			}
			if( !WeaponFireSoundLooping )
			{
				WeaponPlaySound( WeaponFireSound );
			}
		FireAmmunition();
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
				PlayThirdPersonWeaponAnimation(WeaponFiringAnim,GetFireInterval(CurrentFireMode),false); //GetFireInterval(0),true);
			}
			if( !WeaponFireSoundLooping )
			{
				WeaponPlaySound( WeaponFireSound );
			}
		FireAmmunition();	
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
// Default Properties : TC_Weapon_SuperWeaponArchtype class
//=============================================================================

DefaultProperties
{
	FiringStatesArray(0)            =       "WeaponSuperCharge"
	FiringStatesArray(1)            =       "WeaponSuperCharge"
	PawnSuperAnimName               =       mainchar_anim_super_rocket;
}
