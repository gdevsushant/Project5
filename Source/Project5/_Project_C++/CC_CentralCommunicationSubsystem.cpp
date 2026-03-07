#include "_Project_H/CC_CentralCommunicationSubsystem.h"
#include "_Project_H/CC_GameplayMessageListenerData.h"
#include "NativeGameplayTags.h"
#include "_Project_H/CC_CentralCommunicationDataStructure.h"
#include "_Project_H/CC_GameplayMessageListenerList.h"

void UCC_CentralCommunicationSubsystem::BroadcastCentralCommunicationMessage(const UObject* WorldContextObject, FGameplayTag Channel, FUniversalCommunicationMessage Message)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();

	if (World) {

		if (Subsystem) {

			if (Channel.IsValid() && Subsystem->ListenersMap.Num() > 0) {

				if (FMessageList* MessageListPtr = Subsystem->ListenersMap.Find(Channel)) {

					for (int32 i = MessageListPtr->Listeners.Num() - 1; i >= 0; i--) {

						if (MessageListPtr->Listeners.IsValidIndex(i)) {

							if (MessageListPtr->Listeners[i].ListenerObject.Get()) {

								if (Message.Sender.IsValid() && Message.Sender.Get() && Message.Tag.IsValid()) {

									UE_LOG(LogActor, Log, TEXT("Broadcasting message"));
									Subsystem->CentralMessageDelegate.Broadcast(Message);
								}

								continue;
							}

							else {

								MessageListPtr->Listeners.RemoveAt(i);
								continue;
							}
						}

						continue;
					}
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

UCC_CentralCommunicationSubsystem* UCC_CentralCommunicationSubsystem::RegisterListener(const UObject* WorldContextObject, UObject* Listener, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();

	if (World) {

		if (Subsystem) {

			FMessageList List;
			FMessageData Data;
			Data.ListenerObject = Listener;
			Data.HandleId = Subsystem->HandleIdCounter;
			List.Listeners.Add(Data);

			if (Subsystem->ListenersMap.Find(Channel)) {

				FMessageList* GetList = Subsystem->ListenersMap.Find(Channel);
				GetList->Listeners.Add(Data);
			}

			else {

				Subsystem->ListenersMap.Emplace(Channel, List);
			}

			Subsystem->HandleIdCounter++;
			return Subsystem;
		}

		return nullptr;
	}

	return nullptr;
}

void UCC_CentralCommunicationSubsystem::UnRegisterListener(const UObject* WorldContextObject, FGameplayTag Channel)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UCC_CentralCommunicationSubsystem* Subsystem = World->GetSubsystem<UCC_CentralCommunicationSubsystem>();

	if (World) {

		if (Subsystem) {

			if (Channel.IsValid()) {

				if (Subsystem->ListenersMap.Find(Channel)) {

					Subsystem->ListenersMap.Remove(Channel);
					//UE_LOG(LogActor, Log, TEXT("Length of listener map:- %d"), Subsystem->ListenersMap.Num());
				}
			}
		}

		return;
	}

	return;
}
