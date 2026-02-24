#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Movement")
	void Move(FVector Direction, float ScaleValue, bool bForce);

	virtual void Move_Implementation(FVector Direction, float ScaleValue, bool bForce);
};
