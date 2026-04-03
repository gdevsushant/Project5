#include "_Project_H/CC_CentralCommunicationSubsystem.h"
#include "_Project_H/CC_GameplayMessageListenerData.h"
#include "NativeGameplayTags.h"
#include "_Project_H/CC_CentralCommunicationDataStructure.h"
#include "_Project_H/CC_GameplayMessageListenerList.h"

void UCC_CentralCommunicationSubsystem::BroadcastCentralCommunicationMessage(const UObject* WorldContextObject, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	if (!World) return;
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();
	if (!Subsystem) return;
	if (!Channel.IsValid()) return;
	if (Subsystem->ListenersMap.Num() == 0) return;

	if (UCC_GameplayMessageListenerList** MessageListPtrPtr = Subsystem->ListenersMap.Find(Channel)) {
		UCC_GameplayMessageListenerList* MessageListPtr = *MessageListPtrPtr;
		if (!MessageListPtr) return;

		for (int32 i = MessageListPtr->Listeners.Num() - 1; i >= 0; --i) {
			if (!MessageListPtr->Listeners.IsValidIndex(i)) continue;
			if (!MessageListPtr->Listeners[i].ListenerObject.Get()) {
				MessageListPtr->Listeners.RemoveAt(i);
			}
		}

		MessageListPtr->CentralMessageDelegate.Broadcast();
	}
}

UCC_GameplayMessageListenerList* UCC_CentralCommunicationSubsystem::RegisterListener(const UObject* WorldContextObject, UObject* Listener, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	if (!World) return nullptr;
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();
	if (!Subsystem) return nullptr;
	if (!Channel.IsValid()) return nullptr;
	if (!Listener) return nullptr;

	FMessageData Data;
	Data.ListenerObject = Listener;
	Data.HandleId = Subsystem->HandleIdCounter;

	if (UCC_GameplayMessageListenerList** MessageListPtrPtr = Subsystem->ListenersMap.Find(Channel)) {
		if (UCC_GameplayMessageListenerList* MessageListPtr = *MessageListPtrPtr) {
			MessageListPtr->Listeners.Add(Data);
			Subsystem->HandleIdCounter++;
			return MessageListPtr;
		}
		return nullptr;
	}
	else {
		UCC_GameplayMessageListenerList* List = NewObject<UCC_GameplayMessageListenerList>(Subsystem);
		List->Listeners.Add(Data);
		Subsystem->ListenersMap.Add(Channel, List);
		Subsystem->HandleIdCounter++;
		return List;
	}
}

void UCC_CentralCommunicationSubsystem::UnRegisterListener(const UObject* WorldContextObject, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	if (!World) return;
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();
	if (!Subsystem) return;
	if (!Channel.IsValid()) return;

	Subsystem->ListenersMap.Remove(Channel);
}

bool UCC_CentralCommunicationSubsystem::HasListenerRegistered(const UObject* WorldContextObject, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	if (!World) return false;
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();
	if (!Subsystem) return false;
	if (!Channel.IsValid()) return false;

	if (UCC_GameplayMessageListenerList** FoundList = Subsystem->ListenersMap.Find(Channel)) {
		return (FoundList && *FoundList);
	}

	return false;
}
