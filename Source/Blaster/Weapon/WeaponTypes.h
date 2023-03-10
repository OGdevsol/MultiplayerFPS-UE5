 #pragma once

#define TRACE_LENGTH 80000.f;
UENUM(BlueprintType)
enum class EWeaponType : uint8

 {
 	EWT_AssaultRifle UMETA (DisplayName = "AssaultRifle"),
	 EWT_RocketLauncher UMETA (DisplayName = "RocketLauncher"),
	 EWT_Pistol UMETA (DisplayName = "Pistol"),
	 EWT_SMG UMETA (DisplayName = "SMG"),
	 EWT_Shotgun UMETA (DisplayName = "Shotgun"),
	 EWT_SniperRifle UMETA (DisplayName = "SniperRifle"),
	 EWT_GrenadeLauncher UMETA (DisplayName = "GrenadeLauncher"),
	 EWT_MAX UMETA (DisplayName = "DefaultMAX")
 };