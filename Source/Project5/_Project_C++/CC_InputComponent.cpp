#include "_Project_H/CC_InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "_Project_H/CC_TaggedInputActionsDataAsset.h"
#include "_Project_H/CC_TaggedInputActionsStructure.h"
#include "_Project_H/CC_BaseCharacter.h"
#include "_Project_H/CC_CentralCommunicationDataStructure.h"
#include "_Project_H/CC_CentralCommunicationInterface.h"
#include "_Project_H/CC_CentralCommunicationSubsystem.h"
#include "Project5RuntimeLibrary.h"
#include "Project5EditorDynamicDataStructure.h"
#include "NativeGameplayTags.h"

UCC_InputComponent::UCC_InputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCC_InputComponent::BeginPlay()
{
	Super::BeginPlay();	

	ICC_InputSystemInterface::Execute_AddInputMappingContext(this, DefaultInputMappingContext, 0);

	if (DefaultInputDataAsset) {
	
		ICC_InputSystemInterface::Execute_SetInputDataAsset(this, DefaultInputDataAsset);
	}
}

void UCC_InputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCC_InputComponent::InputMappingContext(UInputMappingContext* InputMappingContext, int32 Priority)
{
	if (InputMappingContext) {

		if (APlayerController* PlayerController = GetPlayerController()) {

			if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer()) {

				if (UEnhancedInputLocalPlayerSubsystem* PlayerSubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer)) {

					PlayerSubSystem->AddMappingContext(InputMappingContext, Priority);
				}
			}
		}
	}
}

void UCC_InputComponent::BindAction(APlayerController* Requester)
{
	if (APlayerController* PlayerController = GetPlayerController()) {

		if (PlayerController->InputComponent) {

			UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);

			if (EnhancedInputComponent) {

				if (InputAsset) {

					if (InputAsset->TaggedInputActions.Num() > 0) {

						for (const auto& Action : InputAsset->TaggedInputActions) {

							if (Action.InputAction && Action.InputTag.IsValid()) {

								EnhancedInputComponent->BindAction(Action.InputAction, Action.TriggerEvent, this, &UCC_InputComponent::OnInputRecievedMethod, Action.InputTag, Requester);
								EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Completed, this, &UCC_InputComponent::OnInputCompletedMethod, Requester, Action.InputTag);
								EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Canceled, this, &UCC_InputComponent::OnInputCompletedMethod, Requester, Action.InputTag);
								continue;
							}

							continue;
						}
					}
				}
			}
		}
	}
}

void UCC_InputComponent::OnInputRecievedMethod(const FInputActionValue& Value, FGameplayTag InputTag, APlayerController* Requester)
{
	if (InputTag.IsValid() && Requester) {

		//UE_LOG(LogActor, Warning, TEXT("Input is started or ongoing..."));
		FUniversalCommunicationMessage InputMessage;
		InputMessage.Sender = this;
		InputMessage.Tag = InputTag;
		InputMessage.InputValue = Value;

		UProject5RuntimeLibrary::SetValue<FUniversalCommunicationMessage>(InputTag, InputMessage);
		return;
	}

	return;
}

void UCC_InputComponent::OnInputCompletedMethod(APlayerController* Requester, FGameplayTag InputTag)
{
	//UE_LOG(LogActor, Warning, TEXT("Input is completed or canceled..."));
}

void UCC_InputComponent::InputDataAsset(UCC_TaggedInputActionsDataAsset* DataAsset)
{
	if (DataAsset) {

		InputAsset = DataAsset;

		if (APlayerController* PlayerController = GetPlayerController()) {

			if (PlayerController->InputComponent) {

				BindAction(PlayerController);
			}
		}
	}
}

APlayerController* UCC_InputComponent::GetPlayerController()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetOwner())) {

		return PlayerController;
	}

	return nullptr;
}

ACharacter* UCC_InputComponent::GetCharacter()
{
	if (ACharacter* Character = (UCC_InputComponent::GetPlayerController()->GetCharacter())) {

		return Character;
	}

	return nullptr;
}

void UCC_InputComponent::SetInputDataAsset_Implementation(UCC_TaggedInputActionsDataAsset* DataAsset)
{
	UCC_InputComponent::InputDataAsset(DataAsset);
}

void UCC_InputComponent::AddInputMappingContext_Implementation(UInputMappingContext* InputMappingContext, int32 Priority)
{
	UCC_InputComponent::InputMappingContext(InputMappingContext, Priority);
}

void UCC_InputComponent::BroadcastCentralMessage(FUniversalCommunicationMessage& Message)
{
	UCC_CentralCommunicationSubsystem::BroadcastCentralCommunicationMessage(this, Message.Tag, Message);
}

void UCC_InputComponent::RegisterCentralMessageListener(FGameplayTag Channel)
{
	UCC_CentralCommunicationSubsystem::RegisterListener(this, this, Channel)->CentralMessageDelegate.AddUniqueDynamic(this, &UCC_InputComponent::OnCentralMessageReceived);
	return;
}

void UCC_InputComponent::OnCentralMessageReceived(FUniversalCommunicationMessage Message)
{
	UE_LOG(LogActor, Log, TEXT("Central Message Received"));
	UE_LOG(LogActor, Log, TEXT("Current Tag:- %s"), *Message.Tag.ToString());
}
