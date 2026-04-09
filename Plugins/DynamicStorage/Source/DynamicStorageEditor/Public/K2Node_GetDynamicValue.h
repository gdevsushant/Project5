#pragma once
#include "CoreMinimal.h"
#include "K2Node.h"
#include "NativeGameplayTags.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "K2Node_GetDynamicValue.generated.h"

struct FStorageDefinition;

UCLASS()
class DYNAMICSTORAGEEDITOR_API UK2Node_GetDynamicValue : public UK2Node
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override; // Func to create pins for the node
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override; // Func to set title of blueprint node
	virtual FText GetTooltipText() const override; // Func to set tooltip text for the node
	virtual bool IsNodePure() const override { return false; } // Whether the node needs execution pins or not
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& Registrar) const override; // Func to register the node in the blueprint action menu
	virtual FLinearColor GetNodeTitleColor() const override; // Func to set the color of the node title
	virtual FText GetMenuCategory() const override; // Func to set the category of the node in the blueprint action menu
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override; // Func to expand the node into more basic nodes during compilation
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override; // Func to add custom actions to the node's context menu
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override; // Func called when a pin's connection list changes
	virtual void PostReconstructNode() override; // Func called after the node has been reconstructed
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override; // Func to determine if a connection between pins is disallowed
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;

	FStorageDefinition* GetSettingStorageData(FGameplayTag Tag);

private:
	void ChangeOutputPinType(FName NewCategory, FName NewSubCategory, UObject* NewTypeObject); // Base func to change output pintype
	UPROPERTY() FName SelectedTypeName = NAME_None; // The datatype literal of active input pin
	UPROPERTY() FName SelectedCategory = NAME_None; // The category of the active input pin
	UPROPERTY() TObjectPtr<UObject> SelectedObject = nullptr; // The object type of the active input pin
	UPROPERTY() FName SelectedSubCategory = NAME_None; // The subcategory of the active input pin
};