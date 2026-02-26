#include "_Project_H/CC_BaseCharacter.h"

ACC_BaseCharacter::ACC_BaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACC_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
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

		if (Owner) {

			TArray<UActorComponent*> Components;
			Components = Owner->GetComponentsByInterface(InterfaceClass);

			if (Components.Num() > 0) {

				for (UActorComponent* Component : Components) {

					if (Component) {

						return Cast<UObject>(Component);
					}
				}
			}

			return nullptr;
		}

		return nullptr;
	}

	return nullptr;
}
