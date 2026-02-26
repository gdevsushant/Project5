#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CC_BaseMovementStrategy.generated.h"

class ACharacter;
UCLASS(Blueprintable, BlueprintType)
class PROJECT5_API UCC_BaseMovementStrategy : public UObject
{
	GENERATED_BODY()

public:
	virtual void Move(ACharacter* Character) { };
};
