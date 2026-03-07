#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "_Project_H/CC_BaseMovementStrategy.h"
#include "_Project_H/CC_MovementInterface.h"
#include "_Project_H/CC_CentralCommunicationInterface.h"
#include "_Project_H/CC_CentralCommunicationDataStructure.h"
#include "NativeGameplayTags.h"
#include "CC_BaseCharacter.generated.h"

class UCC_CentralMessageValueDataAsset;

UCLASS()
class PROJECT5_API ACC_BaseCharacter : public ACharacter, public ICC_CentralCommunicationInterface
{
	GENERATED_BODY()

public:
	ACC_BaseCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "InterfaceClass"), Category = "Base Character")
	UObject* GetComponentInterface(TSubclassOf<UInterface> InterfaceClass);

	UFUNCTION(BlueprintCallable, Category = "Base Character")
	void Move(UCC_BaseMovementStrategy* MovementStrategy);

protected:
	void SetMovementInterface();

protected:
	TScriptInterface<ICC_MovementInterface> MovementInterface;
};
