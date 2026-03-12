#include "K2Node_SetDynamicValue.h"
#include "Project5RuntimeLibrary.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "KismetCompiler.h"
#include "K2Node_CallFunction.h"
#include "ToolMenuSection.h"
#include "ToolMenus.h"
#include "Engine/UserDefinedEnum.h"
#include "Engine/UserDefinedStruct.h"
#include "Kismet2/BlueprintEditorUtils.h"

void UK2Node_SetDynamicValue::AllocateDefaultPins()
{
	CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EEdGraphPinDirection::EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* InPin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, FName("In"));
	InPin->PinFriendlyName = FText::FromString("In");
	UEdGraphPin* KeyInPin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Struct, FName("Tag"));
	KeyInPin->PinType.PinSubCategoryObject = TBaseStructure<FGameplayTag>::Get();
	KeyInPin->PinFriendlyName = FText::FromString(TEXT("Tag"));
	Super::AllocateDefaultPins();
}

FText UK2Node_SetDynamicValue::GetNodeTitle(ENodeTitleType::Type TitleType) const { return FText::FromString(TEXT("Set Storage")); }
FText UK2Node_SetDynamicValue::GetTooltipText() const { return FText::FromString(TEXT("Set to any datatype without any engine limitations.")); }
FText UK2Node_SetDynamicValue::GetMenuCategory() const { return FText::FromString(TEXT("Custom Nodes")); }
FLinearColor UK2Node_SetDynamicValue::GetNodeTitleColor() const { return FLinearColor::Red; }

void UK2Node_SetDynamicValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& Registrar) const
{
	UClass* ActionKey = GetClass();
	if (Registrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
		Registrar.AddBlueprintAction(ActionKey, Spawner);
	}
}

void UK2Node_SetDynamicValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Real && Pin->PinType.PinSubCategory == NAME_None) Pin->PinType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
	}

	UEdGraphPin* InPin = FindPin(FName("In"), EGPD_Input);
	UEdGraphPin* TagPin = FindPin(FName("Tag"), EGPD_Input);

	if (InPin->LinkedTo.Num() == 0) { CompilerContext.MessageLog.Error(TEXT("Node @@ In pin must have a wire connected."), this); BreakAllNodeLinks(); return; }
	if (TagPin->LinkedTo.Num() == 0 && TagPin->DefaultValue.IsEmpty()) { CompilerContext.MessageLog.Error(TEXT("Node @@ Tag pin must have a wire connected or a value set."), this); BreakAllNodeLinks(); return; }

	if (!InPin || !TagPin) { CompilerContext.MessageLog.Error(TEXT("Node @@ missing pins."), this); BreakAllNodeLinks(); return; }

	if (InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard) { CompilerContext.MessageLog.Warning(TEXT("Node @@ In pin type must be set."), this); BreakAllNodeLinks(); return; }

	UK2Node_CallFunction* SetValueNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	SetValueNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UProject5RuntimeLibrary, SetDynamicValue), UProject5RuntimeLibrary::StaticClass());
	SetValueNode->AllocateDefaultPins();

	UEdGraphPin* FuncExecPin = SetValueNode->GetExecPin();
	UEdGraphPin* FuncThenPin = SetValueNode->GetThenPin();
	UEdGraphPin* FuncInPin = SetValueNode->FindPin(FName("InValue"));
	UEdGraphPin* FuncTagPin = SetValueNode->FindPin(FName("Tag"));

	if (!FuncExecPin || !FuncThenPin || !FuncInPin || !FuncTagPin) { CompilerContext.MessageLog.Error(TEXT("Node @@ failed to find intermediate pins."), this); BreakAllNodeLinks(); return; }

	FEdGraphPinType SavedType = InPin->PinType;
	if (SavedType.PinCategory == UEdGraphSchema_K2::PC_Real && SavedType.PinSubCategory == NAME_None)
		SavedType.PinSubCategory = UEdGraphSchema_K2::PC_Float;

	FuncInPin->PinType = SavedType;
	if (InPin->LinkedTo.Num() == 0 && !InPin->DefaultValue.IsEmpty()) FuncInPin->DefaultValue = InPin->DefaultValue;

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *FuncExecPin);
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *FuncThenPin);
	CompilerContext.MovePinLinksToIntermediate(*InPin, *FuncInPin);
	CompilerContext.MovePinLinksToIntermediate(*TagPin, *FuncTagPin);
	FuncInPin->PinType = SavedType;

	BreakAllNodeLinks();
}

void UK2Node_SetDynamicValue::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);
	if (!Context || !Context->Pin) return;
	if (Context->Pin->PinName != FName("In")) return;

	FToolMenuSection& Section = Menu->AddSection(FName("ChangePinType"), FText::FromString(TEXT("Change Pin Type")));
	UK2Node_SetDynamicValue* MutableThis = const_cast<UK2Node_SetDynamicValue*>(this);

	Section.AddMenuEntry(FName("None"), FText::FromString(TEXT("None")), FText::FromString(TEXT("Set pin to None")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Wildcard, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Boolean"), FText::FromString(TEXT("Boolean")), FText::FromString(TEXT("Set pin to Boolean")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Integer"), FText::FromString(TEXT("Integer")), FText::FromString(TEXT("Set pin to Integer")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Integer64"), FText::FromString(TEXT("Integer64")), FText::FromString(TEXT("Set pin to Integer64")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Float"), FText::FromString(TEXT("Float")), FText::FromString(TEXT("Set pin to Float")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, nullptr); })));
	Section.AddMenuEntry(FName("Double"), FText::FromString(TEXT("Double")), FText::FromString(TEXT("Set pin to Double")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double, nullptr); })));
	Section.AddMenuEntry(FName("Name"), FText::FromString(TEXT("Name")), FText::FromString(TEXT("Set pin to Name")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Struct"), FText::FromString(TEXT("Structure")), FText::FromString(TEXT("Structure")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Enum"), FText::FromString(TEXT("Enum")), FText::FromString(TEXT("Enum")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("String"), FText::FromString(TEXT("String")), FText::GetEmpty(), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_String, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Object"), FText::FromString(TEXT("Object")), FText::GetEmpty(), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Object, FName("object"), UObject::StaticClass()); })));
	Section.AddMenuEntry(FName("Class"), FText::FromString(TEXT("Class")), FText::GetEmpty(), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Class, FName("object"), UObject::StaticClass()); })));
}

void UK2Node_SetDynamicValue::ChangeWildcardPinType(FName PinCategory, FName PinSubCategory, UObject* Object)
{
	SelectedCategory = PinCategory;
	SelectedSubCategory = PinSubCategory;
	SelectedObject = Object;
	SelectedTypeName = Object ? Object->GetFName() : NAME_None;

	UEdGraphPin* ValuePin = FindPin(FName("In"));
	if (!ValuePin) return;

	ValuePin->PinType.PinCategory = PinCategory;
	ValuePin->PinType.PinSubCategory = PinSubCategory;
	ValuePin->PinType.PinSubCategoryObject = Object;
	ValuePin->BreakAllPinLinks();
	GetGraph()->NotifyGraphChanged();

	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(this);
	if (!Blueprint) return;
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

void UK2Node_SetDynamicValue::PostReconstructNode()
{
	Super::PostReconstructNode();
	UEdGraphPin* ValuePin = FindPin(FName("In"));
	if (!ValuePin) return;
	if (SelectedCategory == NAME_None) return;

	ValuePin->PinType.PinCategory = SelectedCategory;
	ValuePin->PinType.PinSubCategoryObject = SelectedObject;
	ValuePin->PinType.PinSubCategory = (SelectedCategory == UEdGraphSchema_K2::PC_Real && SelectedSubCategory == NAME_None) ? UEdGraphSchema_K2::PC_Float : SelectedSubCategory;
}

bool UK2Node_SetDynamicValue::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const
{
	if (MyPin->PinName != FName("In")) return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);

	FName MyCategory = MyPin->PinType.PinCategory;
	FName OtherCategory = OtherPin->PinType.PinCategory;

	if (MyCategory == UEdGraphSchema_K2::PC_Struct && OtherCategory == UEdGraphSchema_K2::PC_Struct) return false;
	if (MyCategory == UEdGraphSchema_K2::PC_Byte && OtherCategory == UEdGraphSchema_K2::PC_Byte) return false;
	if (MyCategory == UEdGraphSchema_K2::PC_Object && OtherCategory == UEdGraphSchema_K2::PC_Object) return false;
	if (MyCategory == UEdGraphSchema_K2::PC_Class && OtherCategory == UEdGraphSchema_K2::PC_Class) return false;

	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}

void UK2Node_SetDynamicValue::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);
	UEdGraphPin* InPin = FindPin(FName("In"));
	if (!InPin) return;
	if (Pin != InPin) return;

	if (InPin->LinkedTo.Num() > 0)
	{
		UEdGraphPin* ConnectedPin = InPin->LinkedTo[0];
		InPin->PinType = ConnectedPin->PinType;
		SelectedCategory = ConnectedPin->PinType.PinCategory;
		SelectedSubCategory = ConnectedPin->PinType.PinSubCategory;
		SelectedObject = ConnectedPin->PinType.PinSubCategoryObject.Get();
		if (SelectedCategory == UEdGraphSchema_K2::PC_Real && SelectedSubCategory == NAME_None)
			SelectedSubCategory = UEdGraphSchema_K2::PC_Float;
		InPin->PinType.PinSubCategory = SelectedSubCategory;
	}
	else if (!InPin->DefaultValue.IsEmpty() && InPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
	{
		SelectedCategory = InPin->PinType.PinCategory;
		SelectedSubCategory = InPin->PinType.PinSubCategory;
		SelectedObject = InPin->PinType.PinSubCategoryObject.Get();
	}
	else
	{
		if (SelectedCategory != NAME_None && SelectedCategory != UEdGraphSchema_K2::PC_Wildcard)
		{
			InPin->PinType.PinCategory = SelectedCategory;
			InPin->PinType.PinSubCategory = SelectedSubCategory;
			InPin->PinType.PinSubCategoryObject = SelectedObject;
		}
		else
		{
			InPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
			InPin->PinType.PinSubCategory = NAME_None;
			InPin->PinType.PinSubCategoryObject = nullptr;
		}
	}
	GetGraph()->NotifyGraphChanged();
}