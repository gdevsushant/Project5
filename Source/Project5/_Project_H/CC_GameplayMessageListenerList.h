#pragma once

#include "CoreMinimal.h"
#include "_Project_H/CC_GameplayMessageListenerData.h"
#include "CC_GameplayMessageListenerList.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCentralMessage);

UCLASS(BlueprintType)
class PROJECT5_API UCC_GameplayMessageListenerList : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<FMessageData> Listeners;

	UPROPERTY(BlueprintAssignable)
	FCentralMessage CentralMessageDelegate;
};