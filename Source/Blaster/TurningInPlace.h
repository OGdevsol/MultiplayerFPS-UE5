#pragma once
UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	ETIP_LEFT UMETA(DisplayName="Turning Left"),
	ETIP_RIGHT UMETA(DisplayName="Turning Right"),
	ETIP_NOTTURNING UMETA(DisplayName="Not Turning "),
	ETIP_MAX UMETA(DisplayName="Default Max ")
};