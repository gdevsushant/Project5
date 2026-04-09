#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EdGraph/EdGraphPin.h"
#include "NativeGameplayTags.h"
#include "ProjectSetting.generated.h"

USTRUCT()
struct FStorageDefinition
{
	GENERATED_BODY()

	// The type of data stored under this tag (e.g., Float, Int, String, Struct)
	UPROPERTY()
	FName PinCategory = FName("real");

	UPROPERTY()
	FName PinSubCategory = FName("float");

	UPROPERTY()
	TObjectPtr<UObject> PinSubObject;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Project 5 Storage Settings"))
class PROJECT5_API UProjectSetting : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UProjectSetting();

	UPROPERTY(Config, EditAnywhere, Category = "Storage", meta = (ForceExpandCategories))
	TMap<FGameplayTag, FStorageDefinition> StorageRegistry;

#if WITH_EDITOR
	/** Useful for refreshing nodes when settings change */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};