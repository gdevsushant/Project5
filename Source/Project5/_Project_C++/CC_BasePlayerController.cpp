#include "_Project_H/CC_BasePlayerController.h"
#include "_Project_H/CC_InputComponent.h"

ACC_BasePlayerController::ACC_BasePlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	// Create a default input component for this player controller
	InputComp = ObjectInitializer.CreateDefaultSubobject<UCC_InputComponent>(this, TEXT("Input Component"));
}

void ACC_BasePlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
}
