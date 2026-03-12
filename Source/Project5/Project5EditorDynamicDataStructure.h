#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Project5EditorDynamicDataStructure.generated.h"

UENUM(BlueprintType)
enum class EDynamicDataCategory : uint8
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
	EDynamicDataCategory Category = EDynamicDataCategory::Primitive;

	UPROPERTY()
	TObjectPtr<UScriptStruct> StructValue = nullptr;

	UPROPERTY()
	TObjectPtr<UObject> ObjectValue = nullptr;

	UPROPERTY()
	TObjectPtr<UClass> ClassValue = nullptr;

	UPROPERTY()
	FName TypeName;

	bool IsValid() const
	{
		return (Data.Num() > 0 || ObjectValue != nullptr);
	}

	void Clear()
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

	FDynamicValue() {}
};