//=============================================================================
// TC_Weapon_BaseWeaponAttachment: Basic Weapon Attachment class.
//
// This attaches the right weapon mesh. This class is also 
// responsible for playing the right weapon animations 
// and muzzle effects. 
//
// Author: Abhinav Choudhary, 2013
//=============================================================================
class TC_Weapon_BaseWeaponAttachment  extends UTWeaponAttachment;
 
//=============================================================================
// Functions : TC_Weapon_BaseWeaponAttachment class
//=============================================================================

//This function attaches the right Mesh to the right socket using Weapon Archetype data 
simulated function AttachTo( UTPawn OwnerPawn )
{
    local TC_Pawn TC_P;
	local ActorComponent PreviousComponent;
    
	SetWeaponOverlayFlags( OwnerPawn );
    TC_P = TC_Pawn(OwnerPawn);
    
    if ( TC_P != none && OwnerPawn.Mesh != None )
    {
        // Attach Weapon mesh to player skelmesh
        if ( TC_P.CurrentWeaponAttachmentArchetype.MyWeaponInfo.ThirdPersonWeaponMesh != None && Mesh != none )
        {
            //Set the skeletal mesh of our attachment to the one stored
            //in our weapons archetype
            Mesh.SetSkeletalMesh( TC_P.CurrentWeaponAttachmentArchetype.MyWeaponInfo.ThirdPersonWeaponMesh );
			
            Mesh.AnimSets[0] = TC_P.CurrentWeaponAttachmentArchetype.WeaponAnimset;
			Mesh.AnimSets[1] = TC_P.CurrentWeaponAttachmentArchetype.WeaponAnimset;

		    OwnerMesh = OwnerPawn.Mesh;

			//Get the right socket to attach to from Archetype data
            AttachmentSocket = TC_P.CurrentWeaponAttachmentArchetype.MyWeaponInfo.WeaponSocket;
			
            // Weapon Mesh Shadow
            Mesh.SetShadowParent(OwnerPawn.Mesh);
            Mesh.SetLightEnvironment(OwnerPawn.LightEnvironment);
 
            if (OwnerPawn.ReplicatedBodyMaterial != None)
            {
                SetSkin(OwnerPawn.ReplicatedBodyMaterial);
            }
			 
			//If a mesh is already there, detach it
			PreviousComponent =  OwnerPawn.Mesh.FindComponentAttachedToBone( OwnerPawn.Mesh.GetSocketBoneName( AttachmentSocket ) );
			if ( PreviousComponent != none )
				OwnerPawn.Mesh.DetachComponent( ( PreviousComponent ) );
            
			//Attach the new mesh to the right socket
			OwnerPawn.Mesh.AttachComponentToSocket(Mesh, AttachmentSocket);			
        }        
    }
 
    if (MuzzleFlashSocket != '')
    {
        //If our archetype has a muzzle flash reference, lets go ahead and
        //store those references in our WeaponAttachment for use with the
        //already existing muzzle flash system that UTWeapon provides
        if (TC_P.CurrentWeaponAttachmentArchetype.MyWeaponInfo.MuzzleFlashes[0] != None || TC_P.CurrentWeaponAttachmentArchetype.MyWeaponInfo.MuzzleFlashes[1] != None)
        {
            MuzzleFlashPSCTemplate = TC_P.CurrentWeaponAttachmentArchetype.MyWeaponInfo.MuzzleFlashes[0];
            MuzzleFlashAltPSCTemplate = TC_P.CurrentWeaponAttachmentArchetype.MyWeaponInfo.MuzzleFlashes[1];
            MuzzleFlashPSC = new(self) class'UTParticleSystemComponent';
            MuzzleFlashPSC.bAutoActivate = false;
            MuzzleFlashPSC.SetOwnerNoSee(true);
            Mesh.AttachComponentToSocket(MuzzleFlashPSC, MuzzleFlashSocket);
        }
    }
 
    OwnerPawn.SetWeapAnimType( WeapAnimType );
 
    GotoState('CurrentlyAttached');
}
 
//We only use this for Muzzle Flashes. Everything else is done by the main
//weapon class.
simulated function ThirdPersonFireEffects(vector HitLocation)
{   
    if ( EffectIsRelevant(Location,false,MaxFireEffectDistance) )
    {
        // Light it up
        CauseMuzzleFlash();
    } 
   
}
 
//The main weapon class calls this function to play the right animation
simulated function PlayThirdPersonWeaponAnimation(name AnimName, optional float Duration, optional bool bLoop, optional bool bRestartIfAlreadyPlaying = true, optional float StartTime=0.0f, optional bool bPlayBackwards=false)
{
	Mesh.PlayAnim( AnimName, Duration, bLoop, bRestartIfAlreadyPlaying, StartTime, bPlayBackwards );
}

//This gets the weapon exact rotation.
simulated function vector GetMuzzleRotation()
{	
	local rotator SockRot;
	local vector SocLoc;

	if (MuzzleFlashSocket != 'None')
	{
		Mesh.GetSocketWorldLocationAndRotation( MuzzleFlashSocket, SocLoc, SockRot );
		return vector(SockRot);
	}
	else
	{
		return vector(Mesh.Rotation);
	}
}

//=============================================================================
// Default Properties : TC_Weapon_BaseWeaponAttachment class
//=============================================================================

defaultproperties
{
    MuzzleFlashSocket=MussleFlashSocket
	WeapAnimType=EWAT_DualPistols;
	FireAnim = WeaponFire;
	AltFireAnim = WeaponFire;
}