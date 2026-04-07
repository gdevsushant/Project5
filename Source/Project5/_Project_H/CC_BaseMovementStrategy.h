/*
	Movement Strategy Base Object

	Orchestrates logic for various character movement types
	Supports per instance configs
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CC_BaseMovementStrategy.generated.h"

class ACharacter;
UCLASS(Blueprintable)
class PROJECT5_API UCC_BaseMovementStrategy : public UObject
{
	GENERATED_BODY()

public:
	// Blueprint/C++ hook invoked by movement component
	UFUNCTION(BlueprintNativeEvent, Category = "Movement Strategy")
	void Move(ACharacter* Character);

	virtual void Move_Implementation(ACharacter* Character);
};
