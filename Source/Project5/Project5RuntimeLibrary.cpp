#include "Project5RuntimeLibrary.h"

// Thunk stubs
void UProject5RuntimeLibrary::SetDynamicValue(const FGameplayTag Tag, const int32& InValue) { checkNoEntry(); }
void UProject5RuntimeLibrary::GetDynamicValue(const FGameplayTag Tag, int32& OutValue) { checkNoEntry(); }
void UProject5RuntimeLibrary::PassThroughValue(const int32& InValue, int32& OutValue) { OutValue = InValue; }

void UProject5RuntimeLibrary::Internal_SetGenericValue(const FGameplayTag Tag, FProperty* InProp, const void* ValuePtr)
{
	if (!InProp || !ValuePtr || !Tag.IsValid()) return;

	FDynamicValue NewVar;
	if (FStructProperty* StructProp = CastField<FStructProperty>(InProp))
	{
		NewVar.Category = EDynamicDataCategory::Struct;
		NewVar.StructValue = StructProp->Struct;
		NewVar.Data.SetNumZeroed(StructProp->Struct->GetStructureSize());
		StructProp->Struct->InitializeStruct(NewVar.Data.GetData());
		StructProp->Struct->CopyScriptStruct(NewVar.Data.GetData(), ValuePtr);
	}
	else if (FObjectProperty* ObjectProp = CastField<FObjectProperty>(InProp))
	{
		NewVar.Category = EDynamicDataCategory::Object;
		NewVar.ObjectValue = ObjectProp->GetObjectPropertyValue(ValuePtr);
		NewVar.ClassValue = ObjectProp->PropertyClass;
	}
	else
	{
		NewVar.Category = EDynamicDataCategory::Primitive;
		NewVar.TypeName = InProp->GetFName();
		NewVar.Data.SetNumUninitialized(InProp->GetElementSize());
		InProp->CopySingleValue(NewVar.Data.GetData(), ValuePtr);
	}

	if (FDynamicValue* Existing = GetStorage().Find(Tag)) Existing->Clear();
	GetStorage().Add(Tag, MoveTemp(NewVar));
}

void UProject5RuntimeLibrary::Internal_GetGenericValue(const FGameplayTag Tag, FProperty* OutProp, void* OutValuePtr)
{
	if (!OutProp || !OutValuePtr || !Tag.IsValid()) return;
	FDynamicValue* Found = GetStorage().Find(Tag);
	if (!Found) return;

	if (Found->Category == EDynamicDataCategory::Struct)
	{
		FStructProperty* OutStructProp = CastField<FStructProperty>(OutProp);
		if (OutStructProp && OutStructProp->Struct == Found->StructValue)
		{
			Found->StructValue->CopyScriptStruct(OutValuePtr, Found->Data.GetData());
		}
	}
	else if (Found->Category == EDynamicDataCategory::Object)
	{
		FObjectProperty* OutObjProp = CastField<FObjectProperty>(OutProp);
		if (OutObjProp && Found->ObjectValue != nullptr)
		{
			OutObjProp->SetObjectPropertyValue(OutValuePtr, Found->ObjectValue.Get());
		}
	}
	else
	{
		if (Found->Data.Num() == OutProp->GetElementSize())
		{
			OutProp->CopySingleValue(OutValuePtr, Found->Data.GetData());
		}
	}
}

void UProject5RuntimeLibrary::Internal_SetGenericValue(const FGameplayTag Tag, UScriptStruct* Struct, const void* ValuePtr)
{
	if (!Struct || !ValuePtr || !Tag.IsValid()) return;
	FDynamicValue NewVar;
	NewVar.Category = EDynamicDataCategory::Struct;
	NewVar.StructValue = Struct;
	NewVar.Data.SetNumZeroed(Struct->GetStructureSize());
	Struct->InitializeStruct(NewVar.Data.GetData());
	Struct->CopyScriptStruct(NewVar.Data.GetData(), ValuePtr);

	if (FDynamicValue* Existing = GetStorage().Find(Tag)) Existing->Clear();
	GetStorage().Add(Tag, MoveTemp(NewVar));
}

void UProject5RuntimeLibrary::Internal_GetGenericValue(const FGameplayTag Tag, UScriptStruct* Struct, void* OutValuePtr)
{
	FDynamicValue* Found = GetStorage().Find(Tag);
	if (Found && Found->Category == EDynamicDataCategory::Struct && Found->StructValue == Struct)
	{
		Struct->CopyScriptStruct(OutValuePtr, Found->Data.GetData());
	}
}

void UProject5RuntimeLibrary::Internal_SetPrimitiveValue(const FGameplayTag Tag, const void* ValuePtr, int32 Size, FName TypeName)
{
	if (!Tag.IsValid()) return;
	FDynamicValue NewVar;
	NewVar.Category = EDynamicDataCategory::Primitive;
	NewVar.TypeName = TypeName;
	NewVar.Data.SetNumUninitialized(Size);
	FMemory::Memcpy(NewVar.Data.GetData(), ValuePtr, Size);

	if (FDynamicValue* Existing = GetStorage().Find(Tag)) Existing->Clear();
	GetStorage().Add(Tag, MoveTemp(NewVar));
}

void UProject5RuntimeLibrary::Internal_GetPrimitiveValue(const FGameplayTag Tag, void* OutValuePtr, int32 Size)
{
	if (FDynamicValue* Found = GetStorage().Find(Tag))
	{
		if (Found->Category == EDynamicDataCategory::Primitive && Found->Data.Num() == Size)
		{
			FMemory::Memcpy(OutValuePtr, Found->Data.GetData(), Size);
		}
	}
}

DEFINE_FUNCTION(UProject5RuntimeLibrary::execSetDynamicValue)
{
	P_GET_STRUCT(FGameplayTag, Tag);
	Stack.StepCompiledIn<FProperty>(nullptr);
	FProperty* InProp = Stack.MostRecentProperty;
	void* InDataPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;
	P_NATIVE_BEGIN;
	Internal_SetGenericValue(Tag, InProp, InDataPtr);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UProject5RuntimeLibrary::execGetDynamicValue)
{
	P_GET_STRUCT(FGameplayTag, Tag);
	Stack.StepCompiledIn<FProperty>(nullptr);
	FProperty* OutProp = Stack.MostRecentProperty;
	void* OutDataPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;
	P_NATIVE_BEGIN;
	Internal_GetGenericValue(Tag, OutProp, OutDataPtr);
	P_NATIVE_END;
}

DEFINE_FUNCTION(UProject5RuntimeLibrary::execPassThroughValue)
{
	Stack.StepCompiledIn<FProperty>(nullptr);
	FProperty* InProp = Stack.MostRecentProperty;
	void* InDataPtr = Stack.MostRecentPropertyAddress;
	Stack.StepCompiledIn<FProperty>(nullptr);
	FProperty* OutProp = Stack.MostRecentProperty;
	void* OutDataPtr = Stack.MostRecentPropertyAddress;
	P_FINISH;
	if (InProp && OutProp && InDataPtr && OutDataPtr && InProp->SameType(OutProp))
	{
		InProp->CopySingleValue(OutDataPtr, InDataPtr);
	}
}

TMap<FGameplayTag, FDynamicValue>& UProject5RuntimeLibrary::GetStorage()
{
	static TMap<FGameplayTag, FDynamicValue> Storage;
	return Storage;
}

void UProject5RuntimeLibrary::RemoveDynamicValue(const FGameplayTag Tag)
{
	if (FDynamicValue* Found = GetStorage().Find(Tag))
	{
		Found->Clear();
		GetStorage().Remove(Tag);
	}
}

void UProject5RuntimeLibrary::ClearAllDynamicValues()
{
	for (auto& Pair : GetStorage()) Pair.Value.Clear();
	GetStorage().Empty();
}

bool UProject5RuntimeLibrary::HasDynamicValue(const FGameplayTag Tag)
{
	return Tag.IsValid() && GetStorage().Contains(Tag);
}