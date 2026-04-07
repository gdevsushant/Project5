/*
	Bridge btw developer and input component

	A public unified-interface to set up input system default data
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "_Project_H/CC_TaggedInputActionsDataAsset.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CC_InputSystemInterface.generated.h"

class UInputMappingContext;
class UCC_TaggedInputActionsDataAsset;

UINTERFACE(MinimalAPI, Blueprintable)
class UCC_InputSystemInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT5_API ICC_InputSystemInterface
{
	GENERATED_BODY()

public:

	// Blueprint/C++ hook to active custom-chosen input data asset
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input System Interface")
	void SetInputDataAsset(UCC_TaggedInputActionsDataAsset* DataAsset);
	
	// Blueprint/C++ hook to add input mapping context with specified priority
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input System Interface")
	void AddInputMappingContext(UInputMappingContext* InputMappingContext, int32 Priority);

	virtual void SetInputDataAsset_Implementation(UCC_TaggedInputActionsDataAsset* DataAsset) = 0;
	virtual void AddInputMappingContext_Implementation(UInputMappingContext* InputMappingContext, int32 Priority) = 0;
};
