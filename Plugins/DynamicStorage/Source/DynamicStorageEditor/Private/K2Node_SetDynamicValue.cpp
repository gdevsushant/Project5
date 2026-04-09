#include "K2Node_SetDynamicValue.h"
#include "DynamicStorageRuntimeLibrary.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "KismetCompiler.h"
#include "K2Node_CallFunction.h"
#include "ToolMenuSection.h"
#include "ToolMenus.h"
#include "Engine/UserDefinedEnum.h"
#include "Engine/UserDefinedStruct.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "DynamicStorageProjectSetting.h"

void UK2Node_SetDynamicValue::AllocateDefaultPins()
{
	CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute); // input execution pin
	CreatePin(EEdGraphPinDirection::EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then); // output execution pin
	UEdGraphPin* InPin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, FName("In")); // input value pin (default wildcard)
	InPin->PinFriendlyName = FText::FromString("In"); // Set the display name of the input pin
	UEdGraphPin* KeyInPin = CreatePin(EEdGraphPinDirection::EGPD_Input, UEdGraphSchema_K2::PC_Struct, FName("Tag")); // input pin for channel(tag)
	KeyInPin->PinType.PinSubCategoryObject = TBaseStructure<FGameplayTag>::Get(); // Set the struct type of the pin to FGameplayTag
	KeyInPin->PinFriendlyName = FText::FromString(TEXT("Tag")); // Set the display name of the input pin
	Super::AllocateDefaultPins(); // Call parent function in case it does something important (not strictly necessary in this case since the parent function doesn't do anything, but good practice)
}

FText UK2Node_SetDynamicValue::GetNodeTitle(ENodeTitleType::Type TitleType) const { return FText::FromString(TEXT("Set Storage")); }
FText UK2Node_SetDynamicValue::GetTooltipText() const { return FText::FromString(TEXT("Set to any datatype without any engine limitations.")); }
FText UK2Node_SetDynamicValue::GetMenuCategory() const { return FText::FromString(TEXT("Custom Nodes")); }
FLinearColor UK2Node_SetDynamicValue::GetNodeTitleColor() const { return FLinearColor::Red; }

void UK2Node_SetDynamicValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& Registrar) const
{
	UClass* ActionKey = GetClass(); // Get class of this node to use as a key for registration
	if (Registrar.IsOpenForRegistration(ActionKey)) // Check if the registrar is open for registration for this class (prevents registering multiple times)
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass()); // Create a spawner to spawn a node of this class when action is selected in the right click menu
		Registrar.AddBlueprintAction(ActionKey, Spawner); // Spawn the node when the action is selected in right click menu
	}
}

void UK2Node_SetDynamicValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph); // Call parent function in case it does something important (not strictly necessary in this case since the parent function doesn't do anything, but good practice)

	for (UEdGraphPin* Pin : Pins) // Loop through pins and set any wildcard pins to the default type of the node (float in this case) to prevent issues with pin connections during compilation
	{
		if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Real && Pin->PinType.PinSubCategory == NAME_None) Pin->PinType.PinSubCategory = UEdGraphSchema_K2::PC_Float; // Set any wildcard real pins to float by default
	}

	UEdGraphPin* InPin = FindPin(FName("In"), EGPD_Input); // Find the input pin for the value
	UEdGraphPin* TagPin = FindPin(FName("Tag"), EGPD_Input); // Find the input pin for the tag

	if (InPin->LinkedTo.Num() == 0) { CompilerContext.MessageLog.Error(TEXT("Node @@ In pin must have a wire connected."), this); BreakAllNodeLinks(); return; } // Validates the pins and show error if input pin is not connected
	if (TagPin->LinkedTo.Num() == 0 && TagPin->DefaultValue.IsEmpty()) { CompilerContext.MessageLog.Error(TEXT("Node @@ Tag pin must have a wire connected or a value set."), this); BreakAllNodeLinks(); return; } // Validates the pins and show error if tag pin is not connected and has no default value

	if (!InPin || !TagPin) { CompilerContext.MessageLog.Error(TEXT("Node @@ missing pins."), this); BreakAllNodeLinks(); return; } // Validates the pins and show error in blueprints
	if (InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard) { CompilerContext.MessageLog.Warning(TEXT("Node @@ In pin type must be set."), this); BreakAllNodeLinks(); return; } // Validates the pins and show warning if input pin is still wildcard (not set to a specific type)

	// Spawn a call function node to call the SetDynamicValue() in runtimelibrary, which will do the actual work of storing the value in global storage
	// Included in shipping build
	UK2Node_CallFunction* SetValueNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	
	// Set the function to call to SetDynamicValue from runtimelibrary and allocate pins for the function call node
	SetValueNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UDynamicStorageRuntimeLibrary, SetDynamicValue), UDynamicStorageRuntimeLibrary::StaticClass());
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

	// Move the connections from blueprint nodes to function call node
	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *FuncExecPin);
	CompilerContext.MovePinLinksToIntermediate(*GetThenPin(), *FuncThenPin);
	CompilerContext.MovePinLinksToIntermediate(*InPin, *FuncInPin);
	CompilerContext.MovePinLinksToIntermediate(*TagPin, *FuncTagPin);
	FuncInPin->PinType = SavedType;

	// Refresh pin type on compile
	FGameplayTag Tag;
	FGameplayTag::StaticStruct()->ImportText(*TagPin->GetDefaultAsString(), &Tag, nullptr, EPropertyPortFlags::PPF_None, GError, FGameplayTag::StaticStruct()->GetName());
	if (!Tag.IsValid()) { UE_LOG(LogTemp, Log, TEXT("Invalid Tag to set")); BreakAllNodeLinks(); return; }
	SetSettingStorage(Tag, SelectedCategory, SelectedSubCategory, SelectedObject);

	BreakAllNodeLinks();
}

void UK2Node_SetDynamicValue::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	Super::GetNodeContextMenuActions(Menu, Context);
	if (!Context || !Context->Pin) return;
	if (Context->Pin->PinName != FName("In")) return;

	// Add a section to context menu to change the type of the input pin
	FToolMenuSection& Section = Menu->AddSection(FName("ChangePinType"), FText::FromString(TEXT("Change Pin Type")));
	
	// Use lambda functions to change the pin type when a menu entry is selected. Need to use a mutable lambda and const_cast to modify the node from within the lambda since this function is const.
	
	UK2Node_SetDynamicValue* MutableThis = const_cast<UK2Node_SetDynamicValue*>(this);

	// Options for different pin types
	Section.AddMenuEntry(FName("None"), FText::FromString(TEXT("None")), FText::FromString(TEXT("Set pin to None")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Wildcard, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Boolean"), FText::FromString(TEXT("Boolean")), FText::FromString(TEXT("Set pin to Boolean")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Boolean, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Integer"), FText::FromString(TEXT("Integer")), FText::FromString(TEXT("Set pin to Integer")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Int, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Integer64"), FText::FromString(TEXT("Integer64")), FText::FromString(TEXT("Set pin to Integer64")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Int64, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("Float"), FText::FromString(TEXT("Float")), FText::FromString(TEXT("Set pin to Float")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, nullptr); })));
	Section.AddMenuEntry(FName("Double"), FText::FromString(TEXT("Double")), FText::FromString(TEXT("Set pin to Double")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Double, nullptr); })));
	Section.AddMenuEntry(FName("Name"), FText::FromString(TEXT("Name")), FText::FromString(TEXT("Set pin to Name")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Name, NAME_None, nullptr); })));
	Section.AddMenuEntry(FName("String"), FText::FromString(TEXT("String")), FText::FromString(TEXT("Set pin to String")), FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([MutableThis]() { MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_String, NAME_None, nullptr); })));
	Section.AddMenuEntry(
		FName("FVector"),
		FText::FromString(TEXT("FVector")),
		FText::FromString(TEXT("Set pin to FVector")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([MutableThis]() {
			// Retrieve the actual struct object for FVector
			UScriptStruct* VectorStruct = TBaseStructure<FVector>::Get();

			// Pass the struct object so the pin knows what it is
			MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, VectorStruct);
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
			MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, RotatorStruct);
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
			MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, Vector2DStruct);
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
			MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, QuatStruct);
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
			MutableThis->ChangeWildcardPinType(UEdGraphSchema_K2::PC_Struct, NAME_None, TransformStruct);
			}))
	);
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
	GetGraph()->NotifyGraphChanged(); // Notify the graph that the node has changed so that it can update the connections and show the new pin type

	// Find the blueprint that this node belongs to so that we can mark it as modified to ensure the changes are saved.
	// This is necessary because changing the pin type and breaking links does not automatically mark the blueprint as modified, 
	// which can lead to changes not being saved if the user forgets to manually save after changing the pin type.
	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(this);
	if (!Blueprint) return;
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint); // Mark the blueprint as modified to ensure changes are saved
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
	UEdGraphPin* TagPin = FindPin(FName("Tag"));
	if (!InPin) return;
	if (!TagPin) return;
	if (Pin != InPin) return;

	// Update the input pin type based on connected output pin type
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

		FGameplayTag Tag;
		FGameplayTag::StaticStruct()->ImportText(*TagPin->GetDefaultAsString(), &Tag, nullptr, EPropertyPortFlags::PPF_None, GError, FGameplayTag::StaticStruct()->GetName());
		if (!Tag.IsValid()) { UE_LOG(LogTemp, Log, TEXT("Invalid Tag to set")); BreakAllNodeLinks(); return; }
		SetSettingStorage(Tag, SelectedCategory, SelectedSubCategory, SelectedObject);
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

UDynamicStorageProjectSetting* UK2Node_SetDynamicValue::GetProjectSetting()
{
	UDynamicStorageProjectSetting* Setting = GetMutableDefault<UDynamicStorageProjectSetting>();
	if (Setting) {

		return Setting;
	}

	return nullptr;
}

void UK2Node_SetDynamicValue::SetSettingStorage(FGameplayTag Tag, FName Category, FName SubCategory, UObject* SubCategoryObj)
{
	if (!Tag.IsValid() || !Category.IsValid() || !SubCategory.IsValid()) { UE_LOG(LogTemp, Log, TEXT("Invalid tag")); return; }
	if (!Category.IsValid()) { UE_LOG(LogTemp, Log, TEXT("Invalid category")); return; }
	if (!SubCategory.IsValid()) { UE_LOG(LogTemp, Log, TEXT("Invalid subcategory")); return; }
	if (!SubCategoryObj) { UE_LOG(LogTemp, Log, TEXT("Invalid Object")); return; }

	UDynamicStorageProjectSetting* Setting = GetProjectSetting();
	if (!Setting) { UE_LOG(LogTemp, Log, TEXT("Invalid Setting")); return; }

	FStorageDefinition* Def = Setting->StorageRegistry.Find(Tag);
	if (Def) {

		Def->PinCategory = Category;
		Def->PinSubCategory = SubCategory;
		Def->PinSubObject = SubCategoryObj;

		Setting->SaveConfig();
		Setting->TryUpdateDefaultConfigFile();
		return;
	}

	FStorageDefinition Val;
	Val.PinCategory = Category;
	Val.PinSubCategory = SubCategory;
	Val.PinSubObject = SubCategoryObj;
	UE_LOG(LogTemp, Log, TEXT("ADDED TO SETTING STORAGE............."));
	Setting->StorageRegistry.Add(Tag, Val);
	Setting->SaveConfig();
	Setting->TryUpdateDefaultConfigFile();
	return;
}
