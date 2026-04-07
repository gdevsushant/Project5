#include "_Project_H/CC_InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
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

		ICC_InputSystemInterface::Execute_SetInputDataAsset(this, DefaultInputDataAsset); // Exectues function to active a custom chosen input data asset
	}
}

void UCC_InputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCC_InputComponent::InputMappingContext(UInputMappingContext* InputMappingContext, int32 Priority)
{
	if (!InputMappingContext)
		return;

	APlayerController* PlayerController = GetPlayerController();
	if (!PlayerController)
		return;

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
		return;

	UEnhancedInputLocalPlayerSubsystem* PlayerSubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!PlayerSubSystem)
		return;

	PlayerSubSystem->AddMappingContext(InputMappingContext, Priority);
}

void UCC_InputComponent::BindAction(APlayerController* Requester)
{
	APlayerController* PlayerController = GetPlayerController();
	if (!PlayerController || !PlayerController->InputComponent)
		return;

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!EnhancedInputComponent || !InputAsset || InputAsset->TaggedInputActions.Num() == 0)
		return;

	for (const auto& Action : InputAsset->TaggedInputActions) {
		if (!Action.InputAction || !Action.InputTag.IsValid())
			continue;

		// Binds input assets through data-driven actions through data asset
		// Defaults bound to 'Completed' & 'Canceled' to prevent redundant actions entry
		EnhancedInputComponent->BindAction(Action.InputAction, Action.TriggerEvent, this, &UCC_InputComponent::OnInputRecievedMethod, Action.InputTag, Requester);
		EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Completed, this, &UCC_InputComponent::OnInputCompletedMethod, Requester, Action.InputTag);
		EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Canceled, this, &UCC_InputComponent::OnInputCompletedMethod, Requester, Action.InputTag);
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

		if (const bool IsRegistered = UCC_CentralCommunicationSubsystem::HasListenerRegistered(this, InputTag) == false) { // Checks for existing binding

			//UCC_InputComponent::RegisterCentralMessageListener(InputTag);
			UE_LOG(LogActor, Log, TEXT("Registered the listener successfully"));
		}

		// Cache input value to global storage
		UProject5RuntimeLibrary::SetValue<FUniversalCommunicationMessage>(InputTag, InputMessage);

		// Dispatches input events to all active listeners
		UCC_InputComponent::BroadcastCentralMessage(InputTag);

		return;
	}

	return;
}

void UCC_InputComponent::OnInputCompletedMethod(const FInputActionValue& Value, APlayerController* Requester, FGameplayTag InputTag)
{
	//UE_LOG(LogActor, Warning, TEXT("Input is completed or canceled..."));
	FUniversalCommunicationMessage InputMessage;
	InputMessage.Sender = this;
	InputMessage.InputValue = FInputActionValue(); // Reset the input value

	// Cache input value to global storage when input is finished
	UProject5RuntimeLibrary::SetValue<FUniversalCommunicationMessage>(InputTag, InputMessage);

}

void UCC_InputComponent::InputDataAsset(UCC_TaggedInputActionsDataAsset* DataAsset)
{
	if (!DataAsset)
		return;

	InputAsset = DataAsset;

	APlayerController* PlayerController = GetPlayerController();
	if (!PlayerController || !PlayerController->InputComponent)
		return;

	// Bind actions when input data asset actives
	BindAction(PlayerController);
}

APlayerController* UCC_InputComponent::GetPlayerController() const
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetOwner())) {

		return PlayerController;
	}

	return nullptr;
}

ACharacter* UCC_InputComponent::GetCharacter() const
{
	APlayerController* PlayerController = GetPlayerController();
	if (!PlayerController)
		return nullptr;

	return PlayerController->GetCharacter();
}

void UCC_InputComponent::SetInputDataAsset_Implementation(UCC_TaggedInputActionsDataAsset* DataAsset)
{
	UCC_InputComponent::InputDataAsset(DataAsset);
}

void UCC_InputComponent::AddInputMappingContext_Implementation(UInputMappingContext* InputMappingContext, int32 Priority)
{
	UCC_InputComponent::InputMappingContext(InputMappingContext, Priority);
}

void UCC_InputComponent::BroadcastCentralMessage(FGameplayTag& Channel)
{
	UCC_CentralCommunicationSubsystem::BroadcastCentralCommunicationMessage(this, Channel);
}

void UCC_InputComponent::RegisterCentralMessageListener(FGameplayTag Channel)
{
	// Cache listener object
	auto* Listener = UCC_CentralCommunicationSubsystem::RegisterListener(this, this, Channel);
	if (!Listener) {
		UE_LOG(LogActor, Log, TEXT("Central Message Delegate is not valid..."));
		return;
	}

	// Subscribe a listening event to central communication messages(testing purpose)
	Listener->CentralMessageDelegate.AddUniqueDynamic(this, &UCC_InputComponent::OnCentralMessageReceived);
}

void UCC_InputComponent::OnCentralMessageReceived()
{
	UE_LOG(LogActor, Log, TEXT("Central Message Received"));
}

