/*
	DynamicStorage Editor Runtime Library
	
	Provides static helper functions, data for global storage, checker functions
	
*/

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "DynamicStorageEditorDynamicDataStructure.generated.h"

UENUM(BlueprintType)
enum class EDynamicDataCategory : uint8 // Category(enum) of input data
{
	Primitive,
	Struct,
	Object,
	Enum
};

USTRUCT(BlueprintType)
struct FDynamicValue
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<uint8> Data;

	UPROPERTY()
	EDynamicDataCategory Category = EDynamicDataCategory::Primitive; // Category of input data

	UPROPERTY()
	TObjectPtr<UScriptStruct> StructValue = nullptr; // Validate the input data to struct and cache it

	UPROPERTY()
	TObjectPtr<UObject> ObjectValue = nullptr; // Validate the input data to object and cache it

	UPROPERTY()
	TObjectPtr<UClass> ClassValue = nullptr; // Validate the input data to class and cache it

	UPROPERTY()
	FName TypeName; // Alias for type of data

	bool IsValid() const // Is data valid?
	{
		return (Data.Num() > 0 || ObjectValue != nullptr);
	}

	void Clear() // Clear temporary containers
	{
		if (Category == EDynamicDataCategory::Struct && StructValue && Data.Num() > 0)
		{
			StructValue->DestroyStruct(Data.GetData());
		}
		Data.Empty();
		StructValue = nullptr;
		ObjectValue = nullptr;
		ClassValue = nullptr;
		Category = EDynamicDataCategory::Primitive;
		TypeName = NAME_None;
	}

	FDynamicValue() {} // Empty constructor
};