#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "NativeGameplayTags.h"
#include "_Project_H/CC_TaggedInputActionsDataAsset.h"
#include "_Project_H/CC_InputSystemInterface.h"
#include "CC_InputComponent.generated.h"

class UInputMappingContext;
class UInputAction;
class UInputActionValue;
class UCC_TaggedInputActionsDataAsset;

UCLASS(Blueprintable , ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT5_API UCC_InputComponent : public UActorComponent, public ICC_InputSystemInterface
{
	GENERATED_BODY()

public:	

	UCC_InputComponent();

protected:
	
	virtual void BeginPlay() override;

public:	
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Mapping Context")
	UInputMappingContext* DefaultInputMappingContext = nullptr;

protected:

	void BindAction(APlayerController* Requester);
	APlayerController* GetPlayerController();
	void InputDataAsset(UCC_TaggedInputActionsDataAsset* DataAsset);
	void InputMappingContext(UInputMappingContext* InputMappingContext, int32 Priority);
	void ReturnInputTag(FGameplayTag InputTag);

public:

	void OnInputRecieved(const FInputActionValue& Value, FGameplayTag InputTag, APlayerController* Requester);

	virtual void SetInputDataAsset_Implementation(UCC_TaggedInputActionsDataAsset* DataAsset) override;
	virtual void AddInputMappingContext_Implementation(UInputMappingContext* InputMappingContext, int32 Priority) override;
	virtual FGameplayTag GetInputTag_Implementation() override;

protected:

	UCC_TaggedInputActionsDataAsset* CurrentInputDataAsset;
	FGameplayTag ReturnedInputTag = FGameplayTag::EmptyTag;
};
