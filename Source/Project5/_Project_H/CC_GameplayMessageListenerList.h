#pragma once

#include "CoreMinimal.h"
#include "_Project_H/CC_GameplayMessageListenerData.h"
#include "CC_GameplayMessageListenerList.generated.h"

USTRUCT(BlueprintType)
struct FMessageList
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<FMessageData> Listeners;
};