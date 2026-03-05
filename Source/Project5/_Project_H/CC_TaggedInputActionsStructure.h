#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "InputTriggers.h"
#include "CC_TaggedInputActionsStructure.generated.h"

class UInputAction;

USTRUCT(BlueprintType)

struct FTaggedInputAction
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly)
	const UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly)
	ETriggerEvent TriggerEvent = ETriggerEvent::None;
};