/*
	Central communication hub

	Acts as primary message broadcaster across multiple systems
	Facilitates decoupled communication btw disparate game systems
	Facilitates targeted event subscription via specific message channel(Gameplay Tag)
	Facilitates targeted event unsubscription to prevent memory leaks and unintended behavior
	Facilitates targeted event existence check for specific message channel(Gameplay Tag)
*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NativeGameplayTags.h"
#include "_Project_H/CC_GameplayMessageListenerList.h"
#include "_Project_H/CC_GameplayMessageListenerData.h"
#include "_Project_H/CC_CentralCommunicationDataStructure.h"
#include "CC_CentralCommunicationSubsystem.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PROJECT5_API UCC_CentralCommunicationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:

	// Broadcast a message to active listener on specific channel
	UFUNCTION(BlueprintCallable, Category = "Central Communication System", meta = (WorldContext = "WorldContextObject"))
	static void BroadcastCentralCommunicationMessage(const UObject* WorldContextObject, FGameplayTag Channel);

	// Subscribe a listener to a specific channel
	UFUNCTION(BlueprintCallable, Category = "Central Communication System", meta = (WorldContext = "WorldContextObject"))
	static UCC_GameplayMessageListenerList* RegisterListener(const UObject* WorldContextObject, UObject* Listener, FGameplayTag Channel);

	// Unsubscribe a listener from a specific channel
	UFUNCTION(BlueprintCallable, Category = "Central Communication System", meta = (WorldContext = "WorldContextObject"))
	static void UnRegisterListener(const UObject* WorldContextObject, FGameplayTag Channel);

	// Check if a listener is subscribed for a specific channel?
	UFUNCTION(BlueprintCallable, Category = "Central Communication System", meta = (WorldContext = "WorldContextObject"))
	static bool HasListenerRegistered(const UObject* WorldContextObject, FGameplayTag Channel);

private:

	// Persistent storage of registered listener instances
	UPROPERTY()
	TMap<FGameplayTag, UCC_GameplayMessageListenerList*> ListenersMap;

	// Unique Identifier(UID) generator
	int32 HandleIdCounter;

};
