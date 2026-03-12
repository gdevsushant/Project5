#pragma once

#include "CoreMinimal.h"
#include "CC_GameplayMessageListenerData.generated.h"

USTRUCT()
struct FMessageData
{
	GENERATED_BODY()

public:

	TWeakObjectPtr<UObject> ListenerObject;
	int32 HandleId;
};