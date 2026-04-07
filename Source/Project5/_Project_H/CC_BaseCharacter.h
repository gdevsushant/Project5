/*
	Base Character class(Custom)
	Acts as the core template providing share data and common logic for all derived character types.
	Acts as an extensible placeholder to ensure long-term architectural stability with small logics and datas.
	Can be used for future logics
	Auto-instantiates a default movement component
*/

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
class UCC_MovementComponent;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement Component")
	UCC_MovementComponent* MovementComponent;
};
