#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CC_BasePlayerController.generated.h"

UCLASS()
class PROJECT5_API ACC_BasePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void OnPossess(APawn* aPawn) override;
	virtual void SetupInputComponent() override;
};
