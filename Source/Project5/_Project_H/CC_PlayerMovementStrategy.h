#pragma once

#include "CoreMinimal.h"
#include "_Project_H/CC_BaseMovementStrategy.h"
#include "CC_PlayerMovementStrategy.generated.h"

UCLASS(Blueprintable)
class PROJECT5_API UCC_PlayerMovementStrategy : public UCC_BaseMovementStrategy
{
	GENERATED_BODY()

public:
	virtual void Move_Implementation(ACharacter* Character) override;

public:
	FVector Direction;
};
