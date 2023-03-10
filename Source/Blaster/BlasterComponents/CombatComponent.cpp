// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Blaster/CombatState.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
			
		}
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	} 
}
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
									 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	

	
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		SetHUDCrosshairs(DeltaTime);
		
		InterpFOV(DeltaTime);
	}
	
	
}
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character==nullptr || Character->Controller == nullptr)return;
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller):Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()):HUD;
		if (HUD)
		{

			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter =nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//Crosshair spread being calculated
			FVector2D WalkSpeedRange(0.f,Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f,1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z=0.5;
			
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,VelocityMultiplierRange,Velocity.Size());
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairAirVelocityFactor=FMath::FInterpTo(CrosshairAirVelocityFactor,2.25f,DeltaTime,2.25);
			}
			else
			{
				CrosshairAirVelocityFactor=FMath::FInterpTo(CrosshairAirVelocityFactor,0.f,DeltaTime,30.f);
			}
			if (bAiming)
			{
				CrosshairAimFactor=FMath::FInterpTo(CrosshairAimFactor,0.58f,DeltaTime,30.f);
			}
			else
			{
				CrosshairAimFactor=FMath::FInterpTo(CrosshairAimFactor,0.f,DeltaTime,30.f);
			}
			CrosshairShootingFactor= FMath::FInterpTo(CrosshairShootingFactor,0,DeltaTime,40.f);
			
			HUDPackage.CrossHairSpread=
				0.5f + CrosshairVelocityFactor+
					CrosshairAirVelocityFactor-
						CrosshairAimFactor+
							CrosshairShootingFactor;
			HUD->SetHUDPackage(HUDPackage);

		}
	}
}




void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon==nullptr)return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV,EquippedWeapon->GetZoomedFOV(),DeltaTime,EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV,DefaultFOV,DeltaTime,ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
		
	
}
void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(FireTimer,this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
		
	
}
void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr ) return;
	bCanFire=true;
	if (bFirePressed && EquippedWeapon-> bAutomatic)
	{
		Fire();
	}
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	
		return !EquippedWeapon->IsEmpty() && bCanFire && CombatState==ECombatState::ECS_Unoccupied;
	
	
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller==nullptr?Cast<ABlasterPlayerController>(Character->Controller):Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle,StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher,StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol,StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG,StartingSMGlAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun,StartingShotgunlAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle,StartingSniperlAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher,StartingGrenadeLauncherAmmo);
}



void UCombatComponent::Fire()
{
	/*if (bCanFire)
	{
		bCanFire=false;
		ServerFire(HitTarget);
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.80f;
		}
		StartFireTimer();
	}*/
	if (CanFire())
	{
		//bCanFire=false;
		ServerFire(HitTarget);
		if (EquippedWeapon)
		{
			bCanFire=false;
			CrosshairShootingFactor = 0.80f;
		}
		StartFireTimer();
	}

}

 



void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character==nullptr || EquippedWeapon == nullptr)return;
	
		
	
	bAiming = bIsAiming; // Aim first then send server info
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType()==EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	
}
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		if (EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,EquippedWeapon->EquipSound,Character->GetActorLocation());
		}
	}
	
}
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFirePressed = bPressed;
	if (bFirePressed)
	{
		
		Fire();
	}
}
void UCombatComponent::GetViewport(FVector2D ViewportSize)
{
}
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
	                                                               CrosshairLocation, CrosshairWorldPosition,
	                                                               CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		StartPosition = CrosshairWorldPosition;
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - StartPosition).Size();
			StartPosition+=CrosshairWorldDirection * (DistanceToCharacter+100.f);
			DrawDebugSphere(GetWorld(),StartPosition,16.f,12,FColor::Red,false);
		}

		
		EndPosition = StartPosition + CrosshairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, StartPosition, EndPosition, ECC_Visibility);
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
	if (!TraceHitResult.bBlockingHit)
	{
		TraceHitResult.ImpactPoint = EndPosition;
	//	HitTarget = EndPosition;
	}
	
}
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MultiCastFire(TraceHitTarget);
}
void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr)return;

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo,COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)return;

	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	EquippedWeapon = WeaponToEquip;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller==nullptr?Cast<ABlasterPlayerController>(Character->Controller):Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,EquippedWeapon->EquipSound,Character->GetActorLocation());
	}
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo>0 && CombatState!=ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}

void UCombatComponent::FinishReloading()
{
	if (Character==nullptr)return;
	if (Character->HasAuthority())
	{
		CombatState=ECombatState::ECS_Unoccupied;

		UpdateAmmoValues();
	}
	if (bFirePressed)
	{
		Fire();
	}
}

void UCombatComponent::HandleReload()
{
	

	Character->PlayReloadMontage();

}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon==nullptr)return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag,AmountCarried);
		return FMath::Clamp(RoomInMag,0,Least);
	}
	return 0;
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character==nullptr || EquippedWeapon == nullptr) return;
	
	//Character->PlayReloadMontage();
	CombatState=ECombatState::ECS_Reloading;
		HandleReload();
	
}
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;

	case ECombatState::ECS_Unoccupied:
		if (bFirePressed)
		{
			Fire();
		}
		break;
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character==nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller==nullptr?Cast<ABlasterPlayerController>(Character->Controller):Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}





