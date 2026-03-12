#include "_Project_H/CC_CentralCommunicationSubsystem.h"
#include "_Project_H/CC_GameplayMessageListenerData.h"
#include "NativeGameplayTags.h"
#include "_Project_H/CC_CentralCommunicationDataStructure.h"
#include "_Project_H/CC_GameplayMessageListenerList.h"

void UCC_CentralCommunicationSubsystem::BroadcastCentralCommunicationMessage(const UObject* WorldContextObject, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();

	if (World) {

		if (Subsystem) {

			if (Channel.IsValid() && Subsystem->ListenersMap.Num() > 0) {

				if (UCC_GameplayMessageListenerList** MessageListPtrPtr = Subsystem->ListenersMap.Find(Channel)) {

					if (UCC_GameplayMessageListenerList* MessageListPtr = *MessageListPtrPtr) {

						for (int32 i = MessageListPtr->Listeners.Num() - 1; i >= 0; i--) {

							if (MessageListPtr->Listeners.IsValidIndex(i)) {

								if (!MessageListPtr->Listeners[i].ListenerObject.Get()) {

									MessageListPtr->Listeners.RemoveAt(i);
									continue;
								}

								continue;
							}

							continue;
						}

						MessageListPtr->CentralMessageDelegate.Broadcast();
						goto FUNCTION_END;
					}

					goto FUNCTION_END;
				}

				goto FUNCTION_END;
			}

			goto FUNCTION_END;
		}

		goto FUNCTION_END;
	}

	goto FUNCTION_END;

	FUNCTION_END:
		return;
}

UCC_GameplayMessageListenerList* UCC_CentralCommunicationSubsystem::RegisterListener(const UObject* WorldContextObject, UObject* Listener, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();

	if (!World) return nullptr;
	if (!Subsystem) return nullptr;
	if (!Channel.IsValid()) return nullptr;
	if (!Listener) return nullptr;

	UCC_GameplayMessageListenerList* List = NewObject<UCC_GameplayMessageListenerList>(Subsystem);
	FMessageData Data;
	Data.ListenerObject = Listener;
	Data.HandleId = Subsystem->HandleIdCounter;
	List->Listeners.Add(Data);


	if (UCC_GameplayMessageListenerList** MessageListPtrPtr = Subsystem->ListenersMap.Find(Channel)) {

		if (UCC_GameplayMessageListenerList* MessageListPtr = *MessageListPtrPtr) {

			MessageListPtr->Listeners.Add(Data);
			Subsystem->HandleIdCounter++;
			return MessageListPtr;
		}

		return nullptr;
	}

	else {

		Subsystem->ListenersMap.Add(Channel, List);
		Subsystem->HandleIdCounter++;
		return List;
	}
}

void UCC_CentralCommunicationSubsystem::UnRegisterListener(const UObject* WorldContextObject, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();

	if (World) {

		if (Subsystem) {

			if (Channel.IsValid()) {

				if (UCC_GameplayMessageListenerList** MessageListPtrPtr = Subsystem->ListenersMap.Find(Channel)) {

					if (UCC_GameplayMessageListenerList* MessageListPtr = *MessageListPtrPtr) {

						Subsystem->ListenersMap.Remove(Channel);
					}

					return;
				}

				return;
			}

			return;
		}

		return;
	}

	return;
}

bool UCC_CentralCommunicationSubsystem::HasListenerRegistered(const UObject* WorldContextObject, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();

	if (World) {

		if (Subsystem) {

			if (Channel.IsValid()) {

				if (UCC_GameplayMessageListenerList** FoundList = Subsystem->ListenersMap.Find(Channel)) {

					if (Subsystem->ListenersMap.Num() > 0) {

						if (FoundList && *FoundList) {

							return true;
						}

						return false;
					}

					return false;
				}

				return false;
			}

			return false;
		}

		return false;
	}

	return false;
}
