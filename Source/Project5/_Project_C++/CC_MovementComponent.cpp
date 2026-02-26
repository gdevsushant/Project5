#include "_Project_H/CC_MovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCC_MovementComponent::UCC_MovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCC_MovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCC_MovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCC_MovementComponent::Move(UCC_BaseMovementStrategy* MovementStrategy)
{
	if (MovementStrategy) {

		ACharacter* Character = Cast<ACharacter>(GetOwner());

		if (Character) {

			MovementStrategy->Move(Character);
		}
	}
}

void UCC_MovementComponent::RequestMove_Implementation(UCC_BaseMovementStrategy* MovementStrategy)
{
	UCC_MovementComponent::Move(MovementStrategy);
}
