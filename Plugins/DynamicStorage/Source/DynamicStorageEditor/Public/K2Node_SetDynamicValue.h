#pragma once
#include "CoreMinimal.h"
#include "K2Node.h"
#include "GameplayTagContainer.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "K2Node_SetDynamicValue.generated.h"

class UDynamicStorageProjectSetting;

UCLASS()
class DYNAMICSTORAGEEDITOR_API UK2Node_SetDynamicValue : public UK2Node
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override; // Func to create pins for node
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override; // Func for setting node title
	virtual FText GetTooltipText() const override; // Func for setting node tooltip
	virtual bool IsNodePure() const override { return false; } // Func for validate node needs execution pins
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& Registrar) const override; // Func for registering node in blueprint action menu
	virtual FLinearColor GetNodeTitleColor() const override; // Func for setting node title color
	virtual FText GetMenuCategory() const override; // Func for setting node category in blueprint action menu
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override; // Func for expanding node during compilation
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override; // Func for handling pin connection changes
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override; // Func for adding context menu actions
	virtual void PostReconstructNode() override; // Func for handling node reconstruction
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override; // Func for validating pin connections
	
	UDynamicStorageProjectSetting* GetProjectSetting();
	void SetSettingStorage(FGameplayTag Tag, FName Category, FName SubCategory, UObject* SubCategoryObj);

private:
	void ChangeWildcardPinType(FName PinCategory, FName PinSubCategory, UObject* Object);
	UPROPERTY() FName SelectedTypeName = NAME_None; // The datatype name of active pin
	UPROPERTY() TObjectPtr<UObject> SelectedObject = nullptr; // The object associated with the active pin, if applicable (e.g. enum or struct)
	UPROPERTY() FName SelectedCategory = NAME_None; // The category of the active pin (e.g. "struct" or "object")
	UPROPERTY() FName SelectedSubCategory = NAME_None; // The subcategory of the active pin (e.g. the specific struct or object type)
};