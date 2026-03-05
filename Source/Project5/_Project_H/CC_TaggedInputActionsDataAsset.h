#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "_Project_H/CC_TaggedInputActionsStructure.h"
#include "CC_TaggedInputActionsDataAsset.generated.h"

UCLASS()
class PROJECT5_API UCC_TaggedInputActionsDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FTaggedInputAction> TaggedInputActions;
};
