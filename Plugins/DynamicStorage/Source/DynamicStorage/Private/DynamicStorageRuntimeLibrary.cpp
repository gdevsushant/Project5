#include "DynamicStorageRuntimeLibrary.h"
#include "DynamicStorageEditorDynamicDataStructure.h"
#include "NativeGameplayTags.h"

TMap<FGameplayTag, FDynamicValue> UDynamicStorageRuntimeLibrary::Storage; // Define global storage for dynamic values

// Fake c++ hook, never called, just to satisfy the compiler
void UDynamicStorageRuntimeLibrary::SetDynamicValue(const FGameplayTag Tag, const int32& InValue) { checkNoEntry(); }
void UDynamicStorageRuntimeLibrary::GetDynamicValue(const FGameplayTag Tag, int32& OutValue) { checkNoEntry(); }

// C++ hook, copy blueprint node input pin datatype & value to output pin, never called directly, just to satisfy the compiler
void UDynamicStorageRuntimeLibrary::PassThroughValue(const int32& InValue, int32& OutValue) { OutValue = InValue; }

void UDynamicStorageRuntimeLibrary::Internal_SetGenericValue(const FGameplayTag Tag, FProperty* InProp, const void* ValuePtr)
{
	if (!InProp || !ValuePtr || !Tag.IsValid()) return;

	FDynamicValue NewVar;
	NewVar.TypeName = InProp->GetFName();

	if (FStructProperty* StructProp = CastField<FStructProperty>(InProp)) // Validates data to structure
	{
		NewVar.Category = EDynamicDataCategory::Struct;
		NewVar.StructValue = StructProp->Struct;
		NewVar.Data.SetNumZeroed(StructProp->Struct->GetStructureSize());
		StructProp->Struct->InitializeStruct(NewVar.Data.GetData());
		StructProp->Struct->CopyScriptStruct(NewVar.Data.GetData(), ValuePtr);
	}
	else if (FObjectProperty* ObjectProp = CastField<FObjectProperty>(InProp)) // Validates data to UObject
	{
		NewVar.Category = EDynamicDataCategory::Object;
		NewVar.ObjectValue = ObjectProp->GetObjectPropertyValue(ValuePtr);
		NewVar.ClassValue = ObjectProp->PropertyClass;
	}
	else // Validates data to primitive type by default
	{
		NewVar.Category = EDynamicDataCategory::Primitive;
		NewVar.Data.SetNumZeroed(InProp->GetElementSize());
		InProp->InitializeValue(NewVar.Data.GetData());
		InProp->CopyCompleteValue(NewVar.Data.GetData(), ValuePtr);
	}

	if (FDynamicValue* Existing = GetStorage().Find(Tag)) Existing->Clear(); // Validate existing data, clear from global storage and break logic
	GetStorage().Add(Tag, MoveTemp(NewVar)); // Cache new data in global storage
}

void UDynamicStorageRuntimeLibrary::Internal_GetGenericValue(const FGameplayTag Tag, FProperty* OutProp, void* OutValuePtr)
{
	if (!OutProp || !OutValuePtr || !Tag.IsValid()) return;
	FDynamicValue* Found = GetStorage().Find(Tag);

	if (!Found)
	{
		OutProp->InitializeValue(OutValuePtr); // Cache data to actual property
		return;
	}

	if (Found->Category == EDynamicDataCategory::Struct)
	{
		FStructProperty* OutStructProp = CastField<FStructProperty>(OutProp);
		if (OutStructProp && OutStructProp->Struct == Found->StructValue)
		{
			Found->StructValue->CopyScriptStruct(OutValuePtr, Found->Data.GetData()); // Update the output pin with the value from global storage
		}
	}
	else if (Found->Category == EDynamicDataCategory::Object)
	{
		FObjectProperty* OutObjProp = CastField<FObjectProperty>(OutProp);
		if (OutObjProp && Found->ObjectValue != nullptr)
		{
			OutObjProp->SetObjectPropertyValue(OutValuePtr, Found->ObjectValue.Get()); // Update the output pin with the value from global storage
		}
	}
	else
	{
		if (Found->Data.Num() == OutProp->GetElementSize())
		{
			OutProp->CopyCompleteValue(OutValuePtr, Found->Data.GetData()); // Copy the value from global storage to the output pin
		}
	}
}

void UDynamicStorageRuntimeLibrary::Internal_SetGenericValue(const FGameplayTag Tag, UScriptStruct* Struct, const void* ValuePtr)
{
	if (!Struct || !ValuePtr || !Tag.IsValid()) return;
	FDynamicValue NewVar;
	NewVar.Category = EDynamicDataCategory::Struct;
	NewVar.StructValue = Struct;
	NewVar.Data.SetNumZeroed(Struct->GetStructureSize()); // Allocate memory for the struct data
	Struct->InitializeStruct(NewVar.Data.GetData()); // Initialize the struct memory to ensure any default values or constructors are properly handled
	Struct->CopyScriptStruct(NewVar.Data.GetData(), ValuePtr); // Copy the struct data from the input pointer to our internal storage

	if (FDynamicValue* Existing = GetStorage().Find(Tag)) Existing->Clear(); // Validate existing data, clear from global storage
	GetStorage().Add(Tag, MoveTemp(NewVar)); // Cache new data in global storage
}

void UDynamicStorageRuntimeLibrary::Internal_GetGenericValue(const FGameplayTag Tag, UScriptStruct* Struct, void* OutValuePtr)
{
	FDynamicValue* Found = GetStorage().Find(Tag);
	if (Found && Found->Category == EDynamicDataCategory::Struct && Found->StructValue == Struct)
	{
		Struct->CopyScriptStruct(OutValuePtr, Found->Data.GetData()); // Update the output pin with the value from global storage
	}
}

void UDynamicStorageRuntimeLibrary::Internal_SetPrimitiveValue(const FGameplayTag Tag, const void* ValuePtr, int32 Size, FName TypeName)
{
	if (!Tag.IsValid()) return;
	FDynamicValue NewVar;
	NewVar.Category = EDynamicDataCategory::Primitive;
	NewVar.TypeName = TypeName;
	NewVar.Data.SetNumUninitialized(Size); // Allocate memory for the primitive data
	FMemory::Memcpy(NewVar.Data.GetData(), ValuePtr, Size); // Copy the primitive data from the input pointer to our internal storage

	if (FDynamicValue* Existing = GetStorage().Find(Tag)) Existing->Clear(); // Validate existing data, clear from global storage
	GetStorage().Add(Tag, MoveTemp(NewVar)); // Cache new data in global storage
}

void UDynamicStorageRuntimeLibrary::Internal_GetPrimitiveValue(const FGameplayTag Tag, void* OutValuePtr, int32 Size)
{
	if (FDynamicValue* Found = GetStorage().Find(Tag))
	{
		if (Found->Category == EDynamicDataCategory::Primitive && Found->Data.Num() == Size)
		{
			FMemory::Memcpy(OutValuePtr, Found->Data.GetData(), Size); // Copy the primitive data from global storage to the output pointer
		}
	}
}

DEFINE_FUNCTION(UDynamicStorageRuntimeLibrary::execSetDynamicValue)
{
	P_GET_STRUCT(FGameplayTag, Tag); // Retrieve the gameplay tag parameter from the stack
	Stack.StepCompiledIn<FProperty>(nullptr); // Retrieve the property from stack(didn't copied yet anywhere)
	FProperty* InProp = Stack.MostRecentProperty; // Get the property for validation and processing
	void* InDataPtr = Stack.MostRecentPropertyAddress; // Get the property pointer to the actual data for processing
	P_FINISH; // Close the stack
	P_NATIVE_BEGIN; // Start wrapper macro for handle exceptions, error
	Internal_SetGenericValue(Tag, InProp, InDataPtr); // Actual logic to retrieve data from global storage and return
	P_NATIVE_END; // End wrapper macro
}

DEFINE_FUNCTION(UDynamicStorageRuntimeLibrary::execGetDynamicValue)
{
	P_GET_STRUCT(FGameplayTag, Tag); // Retrieve the gameplay tag parameter from the stack
	Stack.StepCompiledIn<FProperty>(nullptr); // Retrieve the property from stack(didn't copied yet anywhere)
	FProperty* OutProp = Stack.MostRecentProperty; // Get the property for validation and processing
	void* OutDataPtr = Stack.MostRecentPropertyAddress; // Get the property pointer to the actual data for processing
	P_FINISH; // Close the stack
	P_NATIVE_BEGIN; // Start wrapper macro for handle exceptions, error
	Internal_GetGenericValue(Tag, OutProp, OutDataPtr); // Actual logic to retrieve data from global storage and return
	P_NATIVE_END; // End wrapper macro
}

DEFINE_FUNCTION(UDynamicStorageRuntimeLibrary::execPassThroughValue)
{
	Stack.StepCompiledIn<FProperty>(nullptr); // Retrieve the input property from stack(didn't copied yet anywhere)
	FProperty* InProp = Stack.MostRecentProperty; // Get the input property for validation and processing
	void* InDataPtr = Stack.MostRecentPropertyAddress; // Get the input property pointer to the actual data for processing
	Stack.StepCompiledIn<FProperty>(nullptr); // Retrieve the output property from stack(don't copy to anywhere yet)
	FProperty* OutProp = Stack.MostRecentProperty; // Get the output property for validation and processing
	void* OutDataPtr = Stack.MostRecentPropertyAddress; // Get the output property pointer to the actual data for processing
	P_FINISH;
	if (InProp && OutProp && InDataPtr && OutDataPtr && InProp->SameType(OutProp))
	{
		InProp->CopyCompleteValue(OutDataPtr, InDataPtr); // Copy the complete value from input to output
	}
}

TMap<FGameplayTag, FDynamicValue>& UDynamicStorageRuntimeLibrary::GetStorage() // Shared actual storage object
{
	return Storage;
}

void UDynamicStorageRuntimeLibrary::RemoveDynamicValue(const FGameplayTag Tag)
{
	if (FDynamicValue* Found = GetStorage().Find(Tag))
	{
		Found->Clear();
		GetStorage().Remove(Tag);
	}
}

void UDynamicStorageRuntimeLibrary::ClearAllDynamicValues()
{
	for (auto& Pair : GetStorage()) Pair.Value.Clear();
	GetStorage().Empty();
}

bool UDynamicStorageRuntimeLibrary::HasDynamicValue(const FGameplayTag Tag)
{
	return Tag.IsValid() && GetStorage().Contains(Tag);
}