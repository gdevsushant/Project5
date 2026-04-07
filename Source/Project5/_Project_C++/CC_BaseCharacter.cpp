#include "_Project_H/CC_BaseCharacter.h"
#include "_Project_H/CC_MovementComponent.h"

ACC_BaseCharacter::ACC_BaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create a default movement component
	MovementComponent = CreateDefaultSubobject<UCC_MovementComponent>(TEXT("Movement Component"));
}

void ACC_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACC_BaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ACC_BaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACC_BaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
