#include "_Project_H/CC_MovementComponent.h"
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

void UCC_MovementComponent::MovePawn(FVector Direction, float ScaleValue, bool bForce)
{
	if (APawn* Owner = Cast<APawn>(GetOwner())) {

		Owner->AddMovementInput(Direction, ScaleValue, bForce);
	}
}

void UCC_MovementComponent::RequestMovePawn_Implementation(FVector Direction, float ScaleValue, bool bForce)
{
	UE_LOG(LogActor, Log, TEXT("This is overriding text"));
}