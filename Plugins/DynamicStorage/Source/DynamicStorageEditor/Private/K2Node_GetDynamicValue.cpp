#include "K2Node_GetDynamicValue.h"
#include "NativeGameplayTags.h"
#include "DynamicStorageRuntimeLibrary.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "KismetCompiler.h"
#include "K2Node_CallFunction.h"
#include "DynamicStorageProjectSetting.h"
#include "Kismet2/BlueprintEditorUtils.h"

void UK2Node_GetDynamicValue::AllocateDefaultPins()
{
	CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute); // input execution pin
	CreatePin(EEdGraphPinDirection::EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then); // output execution pin
	UEdGraphPin* KeyInPin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Struct, FName("Tag")); // input pin for channel(tag)
	KeyInPin->PinType.PinSubCategoryObject = TBaseStructure<FGameplayTag>::Get(); // Set the struct type of the pin to FGameplayTag
	KeyInPin->PinFriendlyName = FText::FromString(TEXT("Tag")); // Set the display name of the input pin
	UEdGraphPin* OutPin = CreatePin(EEdGraphPinDirection::EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, FName("Out")); // output pin for the value(default wildcard)
	OutPin->PinFriendlyName = FText::FromString("OutValue"); // Set the display name of output pin
	Super::AllocateDefaultPins(); // Call parent function in case it does something important (not strictly necessary in this case since the parent function doesn't do anything, but good practice)
}

FText UK2Node_GetDynamicValue::GetNodeTitle(ENodeTitleType::Type TitleType) const { return FText::FromString(TEXT("Get Storage")); }
FText UK2Node_GetDynamicValue::GetTooltipText() const { return FText::FromString(TEXT("Get any type of value")); }
FText UK2Node_GetDynamicValue::GetMenuCategory() const { return FText::FromString(TEXT("Custom Nodes")); }
FLinearColor UK2Node_GetDynamicValue::GetNodeTitleColor() const { return FLinearColor::Green; }

void UK2Node_GetDynamicValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& Registrar) const
{
	UClass* ActionKey = GetClass(); // Get class of this node to use as a key for registration
	if (Registrar.IsOpenForRegistration(ActionKey)) // Check if the registrar is open for registration for this class (prevents registering multiple times)
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass()); // Create a spawner to spawn a node of this class when action is selected in the right click menu
		Registrar.AddBlueprintAction(ActionKey, Spawner); // Spawn the node when the action is selected in right click menu
	}
}

void UK2Node_GetDynamicValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph); // Call parent function in case it does something important (not strictly necessary in this case since the parent function doesn't do anything, but good practice)

	for (UEdGraphPin* Pin : Pins) // Loop through pins and set any wildcard pins to the default type of the node (float in this case) to prevent issues with pin connections during compilation. 
		// This also allows the node to be used without needing to set a type for the output pin, since it will just default to float.
	{
		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Real && Pin->PinType.PinSubCategory == NAME_None) Pin->PinType.PinSubCategory = UEdGraphSchema_K2::PC_Float; // Set any wildcard real pins to float by default
	}

	UEdGraphPin* TagPin = FindPin(FName("Tag"), EEdGraphPinDirection::EGPD_Input); // Find the input pin for the tag
	UEdGraphPin* OutPin = FindPin(FName("Out"), EEdGraphPinDirection::EGPD_Output); // Find the output pin for the value

	if (!TagPin || !OutPin) { CompilerContext.MessageLog.Error(TEXT("Node @@ missing pins."), this); BreakAllNodeLinks(); return; } // Validates the pins and show error in blueprints
	if (OutPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard) { CompilerContext.MessageLog.Warning(TEXT("Node @@ Out pin has no type set."), this); BreakAllNodeLinks(); return; } // Validates the pins and show warning if output pin is still wildcard (not set to a specific type)
	if (TagPin->LinkedTo.Num() <= 0 && TagPin->DefaultValue.IsEmpty()) { CompilerContext.MessageLog.Warning(TEXT("Node @@ Tag pin must be connected or have a value."), this); BreakAllNodeLinks(); return; } // Validates the pins and show warning if tag pin is not connected and has no default value

	// Spawn a call function node to call the GetDynamicValue() in runtimelibrary, which will do the actual work of retrieving the value from global storage
	// Included in shipping build
	UK2Node_CallFunction* GetValueNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph); 

	// Set the function to call to GetDynamicValue from runtimelibrary and allocate pins for the function call node
	GetValueNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UDynamicStorageRuntimeLibrary, GetDynamicValue), UDynamicStorageRuntimeLibrary::StaticClass());
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

	// Move the connections from blueprint nodes to function call node
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

	// Add a section to context menu to change the type of the output pin
	FToolMenuSection& Section = Menu->AddSection(FName("ChangePinType"), FText::FromString(TEXT("Change Pin Type")));

	// Use lambda functions to change the pin type when a menu entry is selected. Need to use a mutable lambda and const_cast to modify the node from within the lambda since this function is const.
	UK2Node_GetDynamicValue* MutableThis = const_cast<UK2Node_GetDynamicValue*>(this);

	// Options for different pin types
	Section.AddMenuEntry(FName("None"), FText::FromString(TEXT("None")), FText::FromString(TEXT("Reset pin to wildcard")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Wildcard, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Boolean"), FText::FromString(TEXT("Boolean")), FText::FromString(TEXT("Set pin to Boolean")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Integer"), FText::FromString(TEXT("Integer")), FText::FromString(TEXT("Set pin to Integer")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Integer64"), FText::FromString(TEXT("Integer64")), FText::FromString(TEXT("Set pin to Integer64")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Float"), FText::FromString(TEXT("Float")), FText::FromString(TEXT("Set pin to Float")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, nullptr); })));
	Section.AddMenuEntry(FName("Double"), FText::FromString(TEXT("Double")), FText::FromString(TEXT("Set pin to Double")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double, nullptr); })));
	Section.AddMenuEntry(FName("Name"), FText::FromString(TEXT("Name")), FText::FromString(TEXT("Set pin to Name")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("String"), FText::FromString(TEXT("String")), FText::FromString(TEXT("Set pin to String")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_String, NAME_None, nullptr); })));
	Section.AddMenuEntry(
		FName("FVector"),
		FText::FromString(TEXT("FVector")),
		FText::FromString(TEXT("Set pin to FVector")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([MutableThis]() {
			// Retrieve the actual struct object for FVector
			UScriptStruct* VectorStruct = TBaseStructure<FVector>::Get();

			// Pass the struct object so the pin knows what it is
			MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, VectorStruct);
			}))
	);

	Section.AddMenuEntry(
		FName("FRotator"),
		FText::FromString(TEXT("FRotator")),
		FText::FromString(TEXT("Set pin to FRotator")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([MutableThis]() {
			// Retrieve the actual struct object for FRotator
			UScriptStruct* RotatorStruct = TBaseStructure<FRotator>::Get();

			// Pass the struct object so the pin knows it has Pitch, Yaw, and Roll
			MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, RotatorStruct);
			}))
	);

	Section.AddMenuEntry(
		FName("FVector2D"),
		FText::FromString(TEXT("Vector 2D")),
		FText::FromString(TEXT("Set pin to FVector2D")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([MutableThis]() {
			// Retrieve the UScriptStruct for FVector2D
			UScriptStruct* Vector2DStruct = TBaseStructure<FVector2D>::Get();

			// Apply the change
			MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, Vector2DStruct);
			}))
	);

	Section.AddMenuEntry(
		FName("FQuat"),
		FText::FromString(TEXT("Quaternion")),
		FText::FromString(TEXT("Set pin to FQuat")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([MutableThis]() {
			// Retrieve the UScriptStruct for FQuat
			UScriptStruct* QuatStruct = TBaseStructure<FQuat>::Get();

			// Apply the change to the wildcard pin
			MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, QuatStruct);
			}))
	);

	Section.AddMenuEntry(
		FName("FTransform"),
		FText::FromString(TEXT("Transform")),
		FText::FromString(TEXT("Set pin to FTransform")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([MutableThis]() {
			// Retrieve the UScriptStruct for FTransform
			UScriptStruct* TransformStruct = TBaseStructure<FTransform>::Get();

			// Apply the change
			MutableThis->ChangeOutputPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, TransformStruct);
			}))
	);

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
	GetGraph()->NotifyGraphChanged(); // Notify the graph that the node has changed so that it can update the connections and show the new pin type

	// Find the blueprint that this node belongs to so that we can mark it as modified to ensure the changes are saved.
	// This is necessary because changing the pin type and breaking links does not automatically mark the blueprint as modified, 
	// which can lead to changes not being saved if the user forgets to manually save after changing the pin type.
	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(this);
	if (!Blueprint) return;
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint); // Mark the blueprint as modified to ensure changes are saved
}

void UK2Node_GetDynamicValue::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);
	UEdGraphPin* OutPin = FindPin(FName("Out"));
	UEdGraphPin* TagPin = FindPin(FName("Tag"));
	if (!OutPin) return;
	if (!TagPin) return;
	if (Pin != OutPin) return;

	// Update the output pin type based on connected input pin type
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

		FGameplayTag Tag;
		FGameplayTag::StaticStruct()->ImportText(*TagPin->GetDefaultAsString(), &Tag, nullptr, EPropertyPortFlags::PPF_None, GError, FGameplayTag::StaticStruct()->GetName());

		if (!Tag.IsValid()) { UE_LOG(LogTemp, Log, TEXT("Tag pin in PinDefaultValueChanged() in GetStorage node is invalid")); OutPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard; return; }

		if (Pin->PinName == FName("Tag")) {

			FStorageDefinition* StorageDefinition = GetSettingStorageData(Tag);

			if (StorageDefinition) {

				UE_LOG(LogTemp, Log, TEXT("SubCategory = %s"), *StorageDefinition->PinSubCategory.ToString());
				OutPin->PinType.PinCategory = StorageDefinition->PinCategory;
				OutPin->PinType.PinSubCategory = StorageDefinition->PinSubCategory;
				OutPin->PinType.PinSubCategoryObject = StorageDefinition->PinSubObject;
			}
		}
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

void UK2Node_GetDynamicValue::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	UEdGraphPin* TagPin = FindPin(FName("Tag"));
	UEdGraphPin* OutPin = FindPin(FName("Out"));

	if (!TagPin) { UE_LOG(LogTemp, Log, TEXT("Tag pin in PinDefaultValueChanged() in GetStorage node is invalid")); return; }
	if (!OutPin) { UE_LOG(LogTemp, Log, TEXT("Tag pin in PinDefaultValueChanged() in GetStorage node is invalid")); return; }

	FGameplayTag Tag;
	FGameplayTag::StaticStruct()->ImportText(*TagPin->GetDefaultAsString(), &Tag, nullptr, EPropertyPortFlags::PPF_None, GError, FGameplayTag::StaticStruct()->GetName());

	if (!Tag.IsValid()) { UE_LOG(LogTemp, Log, TEXT("Tag pin in PinDefaultValueChanged() in GetStorage node is invalid")); OutPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard; return; }

	if (Pin->PinName == FName("Tag")) {

		FStorageDefinition* StorageDefinition = GetSettingStorageData(Tag);

		if (StorageDefinition) {

			UE_LOG(LogTemp, Log, TEXT("SubCategory = %s"), *StorageDefinition->PinSubCategory.ToString());
			OutPin->PinType.PinCategory = StorageDefinition->PinCategory;
			OutPin->PinType.PinSubCategory = StorageDefinition->PinSubCategory;
			OutPin->PinType.PinSubCategoryObject = StorageDefinition->PinSubObject;
		}
	}
}

FStorageDefinition* UK2Node_GetDynamicValue::GetSettingStorageData(FGameplayTag Tag)
{
	UDynamicStorageProjectSetting* Setting = GetMutableDefault<UDynamicStorageProjectSetting>();

	if (!Setting) { UE_LOG(LogTemp, Log, TEXT("Setting is invalid in getstorage node in GetSettingStorageData()")); return nullptr; }
	if (!Tag.IsValid()) { UE_LOG(LogTemp, Log, TEXT("Tag is invalid in getstorage node in GetSettingStorageData()")); return nullptr; }

	UE_LOG(LogTemp, Log, TEXT("Total no of elements in setting storage is :- %d"), Setting->StorageRegistry.Num());
	FStorageDefinition* Val =  Setting->StorageRegistry.Find(Tag);
	return Val;
}
