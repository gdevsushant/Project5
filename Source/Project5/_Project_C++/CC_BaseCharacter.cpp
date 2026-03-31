#include "_Project_H/CC_BaseCharacter.h"
#include "_Project_H/CC_MovementInterface.h"
#include "_Project_H/CC_CentralCommunicationInterface.h"
#include "_Project_H/CC_CentralCommunicationDataStructure.h"
#include "_Project_H/CC_CentralCommunicationSubsystem.h"

ACC_BaseCharacter::ACC_BaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
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
