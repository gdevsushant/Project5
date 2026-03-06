#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NativeGameplayTags.h"
#include "CC_GlobalFunctionsLibrary.generated.h"

UCLASS(Blueprintable)
class PROJECT5_API UCC_GlobalFunctionsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Global Functions")
	static bool CompareTags(FGameplayTag Tag1, FGameplayTag Tag2);

	UFUNCTION(BlueprintCallable, Category = "Global Functions")
	static bool DoesGameplayTagContainerContainTag(FGameplayTagContainer GameplayTagContainer, FGameplayTag Tag);
};
