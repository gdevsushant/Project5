#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CC_BaseCharacter.generated.h"

UCLASS()
class PROJECT5_API ACC_BaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACC_BaseCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "InterfaceClass"), Category = "Base Character")
	UObject* GetComponentInterface(TSubclassOf<UInterface> InterfaceClass);
};
