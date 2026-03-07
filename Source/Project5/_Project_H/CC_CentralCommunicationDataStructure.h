#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "InputActionValue.h"
#include "CC_CentralCommunicationDataStructure.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FUniversalCommunicationMessage
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> Sender = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag Tag = FGameplayTag::EmptyTag;

	UPROPERTY(BlueprintReadWrite)
	float FloatValue = 0;
};