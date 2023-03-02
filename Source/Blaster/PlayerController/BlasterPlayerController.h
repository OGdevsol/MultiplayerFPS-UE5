// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/Weapon.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Engine.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats );
	void SetHUDWeaponAmmo(int32 Ammo );
	void SetHUDCarriedAmmo(int32 Ammo );
	void SetHUDMatchCountdown(float CountdownTime);
	void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&  OutLifetimeProps) const override;
	virtual float GetServerTime(); //Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible
	void OnMatchStateSet(FName State);
protected:
virtual void BeginPlay() override;
	void SetHUDTime();

	/*
	 * Sync time for server and clients
	 */

	UFUNCTION(Server,Reliable) // This function will request the current server time, will pass in the clien't time when the request was sent
	void ServerRequestServerTime(float TimeOfClientRequest);
	
    UFUNCTION(Server,Reliable) // This will report the current server time to the client in response to ServerRequestServerTime
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // Difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency =5.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaTime);
	void PollInIt();
private:
	UPROPERTY()
	ABlasterHUD* BlasterHUD;

	UPROPERTY()
	AWeapon* weapon;

	float MatchTime = 120.f;
	uint32 CountdownInt=0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay; 

	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32  HUDDefeats;
};
