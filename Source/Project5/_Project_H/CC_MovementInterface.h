#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "_Project_H/CC_BaseMovementStrategy.h"
#include "CC_MovementInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UCC_MovementInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT5_API ICC_MovementInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Movement Interface", BlueprintCallable)
	void Move(UCC_BaseMovementStrategy* MovementStrategy);

	virtual void Move_Implementation(UCC_BaseMovementStrategy* MovementStrategy) = 0;
};
