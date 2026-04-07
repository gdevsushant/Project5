/*
	Input Component

	Handles peripheral input events

	Acts as a base component for dynamic inputs
	Iterate over input action data asset and bind actions to the player controller
	Cache input data to global storage for across multiple systems/components

	// How to use?
	1. Add this component to player controller(strongly recommended to keep in player controller)
	2. Create a data asset based on CC_TaggedInputActionsDataAsset and fill it with input actions and tags
	3. Set the default data asset, input mapping context variables
	4. Register a listener event using a gameplay tag.

	To get input data of specific input action, use Get Storage(Blueprints)/GetValue(C++) functions of CentralCommunicationSystem
	with same gameplay tag provided in data asset.

	The system give freedom to execute a input under dynamic trigger event type
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "NativeGameplayTags.h"
#include "_Project_H/CC_InputSystemInterface.h"
#include "CC_InputComponent.generated.h"

class UInputMappingContext;
class UInputAction;
class UInputActionValue;
class UCC_TaggedInputActionsDataAsset;
class APlayerController;
class UCC_BaseCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInputReceived, APlayerController*, Requester, FGameplayTag, InputTag, FInputActionValue, Value);

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
	APlayerController* GetPlayerController() const;
	ACharacter* GetCharacter() const;
	void InputDataAsset(UCC_TaggedInputActionsDataAsset* DataAsset);
	void InputMappingContext(UInputMappingContext* InputMappingContext, int32 Priority);
	void BroadcastCentralMessage(FGameplayTag& Channel);
	void RegisterCentralMessageListener(FGameplayTag Channel);
	void OnInputRecievedMethod(const FInputActionValue& Value, FGameplayTag InputTag, APlayerController* Requester);
	void OnInputCompletedMethod(const FInputActionValue& Value, APlayerController* Requester, FGameplayTag InputTag);

	UFUNCTION()
	void OnCentralMessageReceived();

	UCC_TaggedInputActionsDataAsset* InputAsset;

public:

	virtual void SetInputDataAsset_Implementation(UCC_TaggedInputActionsDataAsset* DataAsset) override;
	virtual void AddInputMappingContext_Implementation(UInputMappingContext* InputMappingContext, int32 Priority) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input System Component")
	UCC_TaggedInputActionsDataAsset* DefaultInputDataAsset;
};
