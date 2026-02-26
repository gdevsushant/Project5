#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	void MovePawn(FVector Direction, float ScaleValue, bool bForce);

	// Interfaces functions
	virtual void RequestMovePawn_Implementation(FVector Direction, float ScaleValue, bool bForce) override;
};
