/*
	Base Player Controller class
	
	Acts as a core template for player controllers
	Auto-instantiates a default input component
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CC_BasePlayerController.generated.h"

class UCC_InputComponent;

UCLASS()
class PROJECT5_API ACC_BasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACC_BasePlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	virtual void OnPossess(APawn* aPawn) override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Component")
	UCC_InputComponent* InputComp;
};
