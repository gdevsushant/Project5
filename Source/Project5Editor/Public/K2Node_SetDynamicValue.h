#pragma once
#include "CoreMinimal.h"
#include "K2Node.h"
#include "GameplayTagContainer.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "K2Node_SetDynamicValue.generated.h"

UCLASS()
class PROJECT5EDITOR_API UK2Node_SetDynamicValue : public UK2Node
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual bool IsNodePure() const override { return false; }
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& Registrar) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetMenuCategory() const override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
	virtual void PostReconstructNode() override;
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;

private:
	void ChangeWildcardPinType(FName PinCategory, FName PinSubCategory, UObject* Object);
	UPROPERTY() FName SelectedTypeName = NAME_None;
	UPROPERTY() TObjectPtr<UObject> SelectedObject = nullptr;
	UPROPERTY() FName SelectedCategory = NAME_None;
	UPROPERTY() FName SelectedSubCategory = NAME_None;
};