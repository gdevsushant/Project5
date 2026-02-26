#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "_Project_H/CC_BaseMovementStrategy.h"
#include "CC_MovementInterface.generated.h"

UINTERFACE(MinimalAPI)
class UCC_MovementInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT5_API ICC_MovementInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Movement Interface")
	void RequestMove(UCC_BaseMovementStrategy* MovementStrategy);
	
	virtual void RequestMove_Implementation(UCC_BaseMovementStrategy* MovementStrategy) = 0;
};
