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
	ACC_BaseCharacter::SetMovementInterface();
}

void ACC_BaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACC_BaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UObject* ACC_BaseCharacter::GetComponentInterface(TSubclassOf<UInterface> InterfaceClass)
{
	if (InterfaceClass) {

		TArray<UActorComponent*> Components;
		Components = GetComponentsByInterface(InterfaceClass);

		if (Components.Num() > 0) {

			return Cast<UObject>(Components[0]);
		}

		return nullptr;
	}

	return nullptr;
}

void ACC_BaseCharacter::Move(UCC_BaseMovementStrategy* MovementStrategy)
{
	if (MovementInterface) {

		MovementInterface->Execute_RequestMove(MovementInterface.GetObject(), MovementStrategy);
	}
}

void ACC_BaseCharacter::SetMovementInterface()
{
	UActorComponent* Comp = Cast<UActorComponent>(GetComponentInterface(UCC_MovementInterface::StaticClass()));

	if (Comp) {

		MovementInterface = TScriptInterface<ICC_MovementInterface>(Comp);
	}
}
