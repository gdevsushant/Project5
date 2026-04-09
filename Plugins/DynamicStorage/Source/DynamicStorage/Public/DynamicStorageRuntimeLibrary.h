#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"
#include "UObject/StructOnScope.h"
#include "Templates/Models.h"
#include "NativeGameplayTags.h"  
#include "DynamicStorageRuntimeLibrary.generated.h"

struct FDynamicValue;

// Trait to detect if a type has a StaticStruct via TBaseStructure (Handles FVector, FRotator, etc...)
template<typename T>
struct TIsUnrealStruct {
	template<typename U> static char Check(decltype(TBaseStructure<U>::Get())*);
	template<typename U> static double Check(...);
	static constexpr bool Value = sizeof(Check<T>(nullptr)) == sizeof(char);
};

UCLASS()
class DYNAMICSTORAGE_API UDynamicStorageRuntimeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Blueprint hook, validate the correct datatype, cache in global storage
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Dynamic Value", meta = (CustomStructureParam = "InValue", DisplayName = "Set Dynamic Value"))
	static void SetDynamicValue(const FGameplayTag Tag, const int32& InValue);
	DECLARE_FUNCTION(execSetDynamicValue);

	// Blueprint hook, validate the correct datatype, retrieve from global storage
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Dynamic Value", meta = (CustomStructureParam = "OutValue", DisplayName = "Get Dynamic Value"))
	static void GetDynamicValue(const FGameplayTag Tag, int32& OutValue);
	DECLARE_FUNCTION(execGetDynamicValue);

	// Internal hook, copy blueprint node input pin datatype & value to output pin
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Custom Nodes", meta = (CustomStructureParam = "InValue,OutValue", BlueprintInternalUseOnly = "true"))
	static void PassThroughValue(const int32& InValue, int32& OutValue);
	DECLARE_FUNCTION(execPassThroughValue);

	// --- C++ ---

	/**C++ hook, validate data, cache in global storage. Handles UStructs (FVector, FRotator, and Custom USTRUCTs) */
	template<typename T>
	static typename TEnableIf<TModels<CStaticStructProvider, T>::Value || TIsUnrealStruct<T>::Value, void>::Type
		SetValue(const FGameplayTag Tag, const T& Value)
	{
		Internal_SetGenericValue(Tag, TBaseStructure<T>::Get(), &Value);
	}

	/** C++ hook, validate data, cache in global storage. Handles Enums safely */
	template<typename T>
	static typename TEnableIf<TIsEnum<T>::Value, void>::Type
		SetValue(const FGameplayTag Tag, const T& Value)
	{
		Internal_SetPrimitiveValue(Tag, &Value, sizeof(T), FName("Enum"));
	}

	/** C++ hook, validate data, cache in global storage. Handles Primitives (int, float, bool) */
	template<typename T>
	static typename TEnableIf<!TModels<CStaticStructProvider, T>::Value && !TIsUnrealStruct<T>::Value && !TIsEnum<T>::Value, void>::Type
		SetValue(const FGameplayTag Tag, const T& Value)
	{
		Internal_SetPrimitiveValue(Tag, &Value, sizeof(T), FName(TNameOf<T>::GetName()));
	}

	/** Getter members */

	// C++ hook, validate data, retrieve from global storage. Handles UStructs (FVector, FRotator, and Custom USTRUCTs)
	template<typename T>
	static T GetValue(const FGameplayTag Tag, const T& DefaultValue = T())
	{
		T Result = DefaultValue;
		if constexpr (TModels<CStaticStructProvider, T>::Value || TIsUnrealStruct<T>::Value)
		{
			Internal_GetGenericValue(Tag, TBaseStructure<T>::Get(), &Result);
		}
		else
		{
			Internal_GetPrimitiveValue(Tag, &Result, sizeof(T));
		}
		return Result;
	}

	// C++ hook, validate data, purge data from global storage
	UFUNCTION(BlueprintCallable, Category = "Dynamic Value")
	static void RemoveDynamicValue(const FGameplayTag Tag);

	// C++ hook, validate data, perform a comprehensive purge of all data from global storage
	UFUNCTION(BlueprintCallable, Category = "Dynamic Value")
	static void ClearAllDynamicValues();

	// C++ hook, validate data existence in global storage
	UFUNCTION(BlueprintPure, Category = "Dynamic Value")
	static bool HasDynamicValue(const FGameplayTag Tag);

	// C++ hook, retrieve global storage
	static TMap<FGameplayTag, FDynamicValue>& GetStorage();

private:
	static void Internal_SetGenericValue(const FGameplayTag Tag, FProperty* Property, const void* ValuePtr); // General setter handler for all possible datatypes of unreal engine
	static void Internal_GetGenericValue(const FGameplayTag Tag, FProperty* Property, void* OutValuePtr); // General getter handler for all possible datatypes of unreal engine
	static void Internal_SetGenericValue(const FGameplayTag Tag, UScriptStruct* Struct, const void* ValuePtr); // Direct setter handler for UStructs, avoid type checking
	static void Internal_GetGenericValue(const FGameplayTag Tag, UScriptStruct* Struct, void* OutValuePtr); // Direct getter handler for UStructs, avoid type checking
	static void Internal_SetPrimitiveValue(const FGameplayTag Tag, const void* ValuePtr, int32 Size, FName TypeName); // Direct setter handler for primitive types, avoid type checking
	static void Internal_GetPrimitiveValue(const FGameplayTag Tag, void* OutValuePtr, int32 Size); // Direct getter handler for primitive types, avoid type checking

	static TMap<FGameplayTag, FDynamicValue> Storage;
};