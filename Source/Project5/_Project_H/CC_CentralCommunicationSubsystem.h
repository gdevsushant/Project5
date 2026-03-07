#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NativeGameplayTags.h"
#include "_Project_H/CC_GameplayMessageListenerList.h"
#include "_Project_H/CC_GameplayMessageListenerData.h"
#include "_Project_H/CC_CentralCommunicationDataStructure.h"
#include "CC_CentralCommunicationSubsystem.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCentralMessage, FUniversalCommunicationMessage, Message);

UCLASS(Blueprintable, BlueprintType)
class PROJECT5_API UCC_CentralCommunicationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Central Communication System", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastCentralCommunicationMessage(const UObject* WorldContextObject, FGameplayTag Channel, FUniversalCommunicationMessage Message);

	UFUNCTION(BlueprintCallable, Category = "Central Communication System", meta = (WorldContext = "WorldContextObject"))
	static UCC_CentralCommunicationSubsystem* RegisterListener(const UObject* WorldContextObject, UObject* Listener, FGameplayTag Channel);

	UFUNCTION(BlueprintCallable, Category = "Central Communication System", meta = (WorldContext = "WorldContextObject"))
	static void UnRegisterListener(const UObject* WorldContextObject, FGameplayTag Channel);

	UPROPERTY(BlueprintAssignable, Category = "Central Communication System")
	FCentralMessage CentralMessageDelegate;

private:

	UPROPERTY()
	TMap<FGameplayTag, FMessageList> ListenersMap;

	int32 HandleIdCounter;

};
