/*
	Movement Component for a player character
	
	Executes provided movement strategy
	Provides a public interface for executing movement strategy
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "_Project_H/CC_BaseMovementStrategy.h"
#include "_Project_H/CC_MovementInterface.h"
#include "CC_MovementComponent.generated.h"


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT5_API UCC_MovementComponent : public UActorComponent, public ICC_MovementInterface
{
	GENERATED_BODY()

public:	
	UCC_MovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Executes provided movement strategy
	void Move(UCC_BaseMovementStrategy* MovementStrategy);

	// Bridge btw strategy move() and public blueprint/c++ calling move()
	virtual void Move_Implementation(UCC_BaseMovementStrategy* MovementStrategy) override;
};
