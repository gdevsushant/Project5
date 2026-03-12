#include "K2Node_GetDynamicValue.h"
#include "NativeGameplayTags.h"
#include "Project5RuntimeLibrary.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "KismetCompiler.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"

void UK2Node_GetDynamicValue::AllocateDefaultPins()
{
	CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EEdGraphPinDirection::EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* KeyInPin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Struct, FName("Tag"));
	KeyInPin->PinType.PinSubCategoryObject = TBaseStructure<FGameplayTag>::Get();
	KeyInPin->PinFriendlyName = FText::FromString(TEXT("Tag"));
	UEdGraphPin* OutPin = CreatePin(EEdGraphPinDirection::EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, FName("Out"));
	OutPin->PinFriendlyName = FText::FromString("OutValue");
	Super::AllocateDefaultPins();
}

FText UK2Node_GetDynamicValue::GetNodeTitle(ENodeTitleType::Type TitleType) const { return FText::FromString(TEXT("Get Storage")); }
FText UK2Node_GetDynamicValue::GetTooltipText() const { return FText::FromString(TEXT("Get any type of value")); }
FText UK2Node_GetDynamicValue::GetMenuCategory() const { return FText::FromString(TEXT("Custom Nodes")); }
FLinearColor UK2Node_GetDynamicValue::GetNodeTitleColor() const { return FLinearColor::Green; }

void UK2Node_GetDynamicValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& Registrar) const
{
	UClass* ActionKey = GetClass();
	if (Registrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
		Registrar.AddBlueprintAction(ActionKey, Spawner);
	}
}

void UK2Node_GetDynamicValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Real && Pin->PinType.PinSubCategory == NAME_None) Pin->PinType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
	}

	UEdGraphPin* TagPin = FindPin(FName("Tag"), EEdGraphPinDirection::EGPD_Input);
	UEdGraphPin* OutPin = FindPin(FName("Out"), EEdGraphPinDirection::EGPD_Output);

	if (!TagPin || !OutPin) { CompilerContext.MessageLog.Error(TEXT("Node @@ missing pins."), this); BreakAllNodeLinks(); return; }
	if (OutPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard) { CompilerContext.MessageLog.Warning(TEXT("Node @@ Out pin has no type set."), this); BreakAllNodeLinks(); return; }
	if (TagPin->LinkedTo.Num() <= 0 && TagPin->DefaultValue.IsEmpty()) { CompilerContext.MessageLog.Warning(TEXT("Node @@ Tag pin must be connected or have a value."), this); BreakAllNodeLinks(); return; }

	UK2Node_CallFunction* GetValueNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	GetValueNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UProject5RuntimeLibrary, GetDynamicValue), UProject5RuntimeLibrary::StaticClass());
	GetValueNode->AllocateDefaultPins();

	UEdGraphPin* FuncExecPin = GetValueNode->GetExecPin();
	UEdGraphPin* FuncThenPin = GetValueNode->GetThenPin();
	UEdGraphPin* FuncOutPin = GetValueNode->FindPin(FName("OutValue"));
	UEdGraphPin* FuncTagPin = GetValueNode->FindPin(FName("Tag"));

	if (!FuncExecPin || !FuncThenPin || !FuncOutPin || !FuncTagPin) { CompilerContext.MessageLog.Error(TEXT("Node @@ failed to find intermediate pins."), this); BreakAllNodeLinks(); return; }

	FEdGraphPinType SavedType = OutPin->PinType;
	if (SavedType.PinCategory == UEdGraphSchema_K2::PC_Real && SavedType.PinSubCategory == NAME_None) SavedType.PinSubCategory = UEdGraphSchema_K2::PC_Float;

	FString SavedTagDefault = TagPin->DefaultValue;
	FString SavedTagTextDefault = TagPin->DefaultTextValue.ToString();

	FuncOutPin->PinType = SavedType;

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *FuncExecPin);
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *FuncThenPin);
	CompilerContext.MovePinLinksToIntermediate(*OutPin, *FuncOutPin);
	CompilerContext.MovePinLinksToIntermediate(*TagPin, *FuncTagPin);

	FuncOutPin->PinType = SavedType;
	if (TagPin->LinkedTo.Num() == 0 && !SavedTagDefault.IsEmpty()) FuncTagPin->DefaultValue = SavedTagDefault;

	BreakAllNodeLinks();
}

void UK2Node_GetDynamicValue::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);
	if (!Context || !Context->Pin) return;
	if (Context->Pin->PinName != FName("Out")) return;

	FToolMenuSection& Section = Menu->AddSection(FName("ChangePinType"), FText::FromString(TEXT("Change Pin Type")));
	UK2Node_GetDynamicValue* MutableThis = const_cast<UK2Node_GetDynamicValue*>(this);

	Section.AddMenuEntry(FName("None"), FText::FromString(TEXT("None")), FText::FromString(TEXT("Reset pin to wildcard")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Wildcard, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Boolean"), FText::FromString(TEXT("Boolean")), FText::FromString(TEXT("Set pin to Boolean")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Integer"), FText::FromString(TEXT("Integer")), FText::FromString(TEXT("Set pin to Integer")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Integer64"), FText::FromString(TEXT("Integer64")), FText::FromString(TEXT("Set pin to Integer64")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Float"), FText::FromString(TEXT("Float")), FText::FromString(TEXT("Set pin to Float")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, nullptr); })));
	Section.AddMenuEntry(FName("Double"), FText::FromString(TEXT("Double")), FText::FromString(TEXT("Set pin to Double")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double, nullptr); })));
	Section.AddMenuEntry(FName("Name"), FText::FromString(TEXT("Name")), FText::FromString(TEXT("Set pin to Name")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("String"), FText::FromString(TEXT("String")), FText::FromString(TEXT("Set pin to String")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_String, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Struct"), FText::FromString(TEXT("Structure")), FText::FromString(TEXT("Set pin to any Struct")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Enum"), FText::FromString(TEXT("Enum")), FText::FromString(TEXT("Set pin to any Enum")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Byte, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Object"), FText::FromString(TEXT("Object")), FText::FromString(TEXT("Set pin to any Object")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Object, FName("object"), UObject::StaticClass()); })));
	Section.AddMenuEntry(FName("Class"), FText::FromString(TEXT("Class")), FText::FromString(TEXT("Set pin to any Class")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Class, FName("object"), UObject::StaticClass()); })));
}

void UK2Node_GetDynamicValue::ChangeOutputPinType(FName NewCategory, FName NewSubCategory, UObject* NewTypeObject)
{
	SelectedCategory = NewCategory;
	SelectedSubCategory = NewSubCategory;
	SelectedObject = NewTypeObject;
	SelectedTypeName = NewTypeObject ? NewTypeObject->GetFName() : NAME_None;

	UEdGraphPin* OutPin = FindPin(FName("Out"), EGPD_Output);
	if (!OutPin) return;

	OutPin->PinType.PinCategory = NewCategory;
	OutPin->PinType.PinSubCategory = NewSubCategory;
	OutPin->PinType.PinSubCategoryObject = NewTypeObject;
	OutPin->BreakAllPinLinks();
	GetGraph()->NotifyGraphChanged();

	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(this);
	if (!Blueprint) return;
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

void UK2Node_GetDynamicValue::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);
	UEdGraphPin* OutPin = FindPin(FName("Out"));
	if (!OutPin) return;
	if (Pin != OutPin) return;

	if (OutPin->LinkedTo.Num() > 0)
	{
		UEdGraphPin* ConnectedPin = OutPin->LinkedTo[0];
		OutPin->PinType = ConnectedPin->PinType;
		SelectedCategory = ConnectedPin->PinType.PinCategory;
		SelectedSubCategory = ConnectedPin->PinType.PinSubCategory;
		SelectedObject = ConnectedPin->PinType.PinSubCategoryObject.Get();
		if (SelectedCategory == UEdGraphSchema_K2::PC_Real && SelectedSubCategory == NAME_None)
			SelectedSubCategory = UEdGraphSchema_K2::PC_Float;
		OutPin->PinType.PinSubCategory = SelectedSubCategory;
	}
	else
	{
		if (SelectedCategory != NAME_None && SelectedCategory != UEdGraphSchema_K2::PC_Wildcard)
		{
			OutPin->PinType.PinCategory = SelectedCategory;
			OutPin->PinType.PinSubCategory = SelectedSubCategory;
			OutPin->PinType.PinSubCategoryObject = SelectedObject;
		}
		else
		{
			OutPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
			OutPin->PinType.PinSubCategory = NAME_None;
			OutPin->PinType.PinSubCategoryObject = nullptr;
		}
	}
	GetGraph()->NotifyGraphChanged();
}

void UK2Node_GetDynamicValue::PostReconstructNode()
{
	Super::PostReconstructNode();
	if (SelectedCategory == NAME_None) return;
	UEdGraphPin* OutPin = FindPin(FName("Out"), EGPD_Output);
	if (!OutPin) return;

	OutPin->PinType.PinCategory = SelectedCategory;
	OutPin->PinType.PinSubCategory = (SelectedCategory == UEdGraphSchema_K2::PC_Real && SelectedSubCategory == NAME_None) ? UEdGraphSchema_K2::PC_Float : SelectedSubCategory;
	OutPin->PinType.PinSubCategoryObject = SelectedObject;
}

bool UK2Node_GetDynamicValue::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const
{
	if (MyPin->PinName != FName("Out")) return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);

	FName MyCategory = MyPin->PinType.PinCategory;
	FName OtherCategory = OtherPin->PinType.PinCategory;

	if (MyCategory == UEdGraphSchema_K2::PC_Struct && OtherCategory == UEdGraphSchema_K2::PC_Struct) return false;
	if (MyCategory == UEdGraphSchema_K2::PC_Byte && OtherCategory == UEdGraphSchema_K2::PC_Byte) return false;
	if (MyCategory == UEdGraphSchema_K2::PC_Object && OtherCategory == UEdGraphSchema_K2::PC_Object) return false;
	if (MyCategory == UEdGraphSchema_K2::PC_Class && OtherCategory == UEdGraphSchema_K2::PC_Class) return false;

	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}