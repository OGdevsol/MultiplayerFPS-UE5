// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "Animation/AnimNotifies/AnimNotify_PlaySound.h"
#include "Blaster/Blaster.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/LowLevelTestAdapter.h"
#include "Blaster/CombatState.h"
#include "CombatState.generated.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"


ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	// false because it is attached to the CameraBoom with PawnControlRotation set to true
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
	//Designated to be replicating. Components dont neet to be registered to be replicated.
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	TurningInPlace = ETurningInPlace::ETIP_NOTTURNING;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
	//Optimized and widely used values for updating replications between server and clients for the game
}
/*void ABlasterCharacter::OffsetSocketForPlayer()
{
	
	FVector CameraOffsetForPlayerAiming(0.f,75.f,75.f);
	FVector CameraOffsetForPlayerNotAiming(0,0,0);
	if (IsAiming())
	{
		
		ABlasterCharacter* Character = this;
		CameraOffsetForPlayerAiming = FMath::VInterpTo(CameraOffsetForPlayerNotAiming,CameraOffsetForPlayerAiming,2,500);
		Character->CameraBoom->SocketOffset=CameraOffsetForPlayerAiming;
	}
	else
	{
		
		ABlasterCharacter* Character = this;
		CameraOffsetForPlayerNotAiming = FMath::VInterpTo(CameraOffsetForPlayerAiming,CameraOffsetForPlayerNotAiming,2,500);
		Character->CameraBoom->SocketOffset=CameraOffsetForPlayerNotAiming;
		
	}
	
}*/


void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	// The class that has the replicated variable and the variable itself that is to be replicated ==== Starts off as null. Is to be set in the weapon class
	DOREPLIFETIME(ABlasterCharacter , Health);
	DOREPLIFETIME(ABlasterCharacter , bDisableGameplay);
}


void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}
void ABlasterCharacter::Elim()
{
	if (Combat&&Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTImer,this,&ABlasterCharacter::ElimTimerFinisher,ElimDelay);
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed=true;
	PlayElimMontage();
	//	Start dissolve effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance,this);
		GetMesh()->SetMaterial(0,DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"),200.f);

	}
	StartDissolve();

	//Disable Character Movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay=true;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	//Disable mandatory collisions
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (ElimbotEffect)
	{
		FVector ElimbotSpawnPoint(GetActorLocation().X,GetActorLocation().Y,GetActorLocation().Z+300);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ElimbotEffect,ElimbotSpawnPoint,GetActorRotation());
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this,ElimBotSound,GetActorLocation());
	}
	if (IsLocallyControlled() &&Combat
		&&Combat->bAiming
		&&Combat->EquippedWeapon
		&&Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
}

void ABlasterCharacter::ElimTimerFinisher()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestForRespawning(this,Controller);
	}
	
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController== nullptr ? Cast<ABlasterPlayerController>(Controller) :BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController -> SetHUDHealth(Health,MaxHealth);
	}
}

void ABlasterCharacter::PollInitialize()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
		}
	}
}



void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
	
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	RotateInPlace(DeltaTime);
	HideCam();
	PollInitialize();
}
void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NOTTURNING;
		return;
	}
	
	if (GetLocalRole()>ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);

	}
	else
	{
		TimeSinceLastMovementReplication+=DeltaTime;
		if (TimeSinceLastMovementReplication>0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat==nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim"): FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
		//AnimInstance->Montage_Play(ReloadMontage);
	}
	
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat==nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		    case EWeaponType::EWT_AssaultRifle:
			 SectionName = FName("Rifle");
			 break;

		    case EWeaponType::EWT_RocketLauncher:
			 SectionName = FName("Rifle");
			 break;

			case EWeaponType::EWT_Pistol:
			 SectionName = FName("Rifle");
			 break;

		    case EWeaponType::EWT_SMG:
			 SectionName = FName("Rifle");
			 break;
			
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Rifle");
			break;

		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;

		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{

		
		AnimInstance->Montage_Play(ElimMontage);
	}
}




void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat==nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* Damagetype,
	AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health-Damage,0.f,MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();
	if (Health<=0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();

		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController==nullptr?Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
		  BlasterGameMode->PlayerEliminated(this,BlasterPlayerController,AttackerController);
		}
	}
	
}


void ABlasterCharacter::MoveForward(float Value)
//Make sure controller isn't null and value isn't zero. Character class has an inherited variable "Controller" type AController
{
	if(bDisableGameplay)return;
	if (Controller != nullptr || Value != 0.f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if(bDisableGameplay)return;

	if (Controller != nullptr || Value != 0.f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if(bDisableGameplay)return;

	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else // If reached, means we are calling from client
		{
			ServerEquipButtonPressed(); // Called without implementation, this is only for defining the function
		}
	}
}


void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
		//For hiding pickup widget on server as repnotify is not called on servers
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return  Combat->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatStatee() const
{
	if (Combat == nullptr) return  ECombatState::ECS_Max;
	return  Combat->CombatState;
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if(bDisableGameplay)return;

	
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if(bDisableGameplay)return;

	if (Combat)
	{
		Combat->Reload();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if(bDisableGameplay)return;

	if (Combat)
	{
		Combat->SetAiming(true);
		//OffsetSocketForPlayer();
		UE_LOG(LogTemp,Warning,TEXT("AIMING"))
	}
	
	
}

void ABlasterCharacter::AimButtonReleased()
{
	if(bDisableGameplay)return;

	if (Combat)
	{
		Combat->SetAiming(false);
	//	OffsetSocketForPlayer();
		UE_LOG(LogTemp,Warning,TEXT("NotAiming"))
	}
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_PITCH = GetBaseAimRotation().Pitch;
	//Will malfunction on clients if not dealth with according to bitwise operations as angle values are compressed and then sent, when decompressed, value will be changed from negative to a bitwise compatible positive value
	if (AO_PITCH > 90.f && !IsLocallyControlled())
	{
		//map pitch from 270-360 to -90-0 >>>>>> the former value is what we get when angles are compressed and then sent over the network and decompressed>>>>>result, malfunction in pitch values of clients for users
		FVector2d InRange(270.f, 360.f);
		FVector2d OutRange(-90.f, 0.f);
		AO_PITCH = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_PITCH);
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	if (Speed == 0.f && !bIsInAir) // == Standing still
	{
		bRotateRootBone=true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_YAW = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NOTTURNING)
		{
			Interp_AO_YAW = AO_YAW;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // Running or jumping
	{
		bRotateRootBone=false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_YAW = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NOTTURNING;
	}
	CalculateAO_Pitch();
	/*if (HasAuthority() && !IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AO_PITCH: %f"),AO_PITCH);
	}*/
}
float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return  Velocity.Size();
}


void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone=false;
	float Speed = CalculateSpeed();
	if (Speed>0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NOTTURNING;
        return ;
	}
	
	//CalculateAO_Pitch();
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation,ProxyRotationLastFrame).Yaw;
	UE_LOG(LogTemp,Warning,TEXT("ProxyYaw: %f "), ProxyYaw);
	if (FMath::Abs(ProxyYaw)>TurnThreshold)
	{
		if (ProxyYaw>TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_RIGHT;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_LEFT;

		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NOTTURNING;

		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NOTTURNING;
}


void ABlasterCharacter::Jump()
{
	if(bDisableGameplay)return;
	if (bIsCrouched)
	{
		UnCrouch();
		
		
	}
	Super::Jump(); 
	
	
}

void ABlasterCharacter::FireButtonPressed()
{
	if(bDisableGameplay)return;

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if(bDisableGameplay)return;

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}




void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	// UE_LOG(LogTemp, Warning, TEXT("AO_YAW: %f"),AO_YAW);
	if (AO_YAW > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_RIGHT;
	}
	else if (AO_YAW < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_LEFT;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NOTTURNING)
	{
		Interp_AO_YAW = FMath::FInterpTo(Interp_AO_YAW, 0, DeltaTime, 4.f);
		AO_YAW = Interp_AO_YAW;
		if (FMath::Abs(AO_YAW) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NOTTURNING;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

/*void ABlasterCharacter::MultiCastHit_Implementation()
{
	PlayHitReactMontage();
}*/

void ABlasterCharacter::HideCam()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size()<CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon &&  Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon &&  Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}


void ABlasterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
	
	
}


void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve,DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon) //On EndSphereCollision, this will be null so this statement will not run
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	//On end sphere collision, LastWeapon will have the value of the last weapon we overlapped with so it wont be null and this will be called
	{
		LastWeapon->ShowPickupWidget(false);
	}
}
