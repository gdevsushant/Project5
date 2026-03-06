#include "_Project_H/CC_InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "_Project_H/CC_TaggedInputActionsDataAsset.h"
#include "_Project_H/CC_TaggedInputActionsStructure.h"
#include "NativeGameplayTags.h"

UCC_InputComponent::UCC_InputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCC_InputComponent::BeginPlay()
{
	Super::BeginPlay();	
	ICC_InputSystemInterface::Execute_AddInputMappingContext(this, DefaultInputMappingContext, 0);
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

				if (CurrentInputDataAsset) {

					for (const FTaggedInputAction& Action : CurrentInputDataAsset->TaggedInputActions) {

						if (Action.InputAction && Action.InputTag.IsValid()) {

							EnhancedInputComponent->BindAction(Action.InputAction, Action.TriggerEvent, this, &UCC_InputComponent::OnInputRecieved, Action.InputTag, Requester);
							EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Completed, this, &UCC_InputComponent::OnInputCompleted, Requester, Action.InputTag);
							EnhancedInputComponent->BindAction(Action.InputAction, ETriggerEvent::Canceled, this, &UCC_InputComponent::OnInputCompleted, Requester, Action.InputTag);
						}
					}
				}
			}
		}
	}
}

void UCC_InputComponent::OnInputRecieved(const FInputActionValue& Value, FGameplayTag InputTag, APlayerController* Requester)
{
	if (InputTag.IsValid() && Requester) {

		UE_LOG(LogActor, Warning, TEXT("Input is started or ongoing..."));
		UCC_InputComponent::AddInputTagInContainer(InputTag);
	}
}

void UCC_InputComponent::OnInputCompleted(APlayerController* Requester, FGameplayTag InputTag)
{
	UE_LOG(LogActor, Warning, TEXT("Input is completed or canceled..."));
	RemoveInputTagFromContainer(InputTag);
}

void UCC_InputComponent::InputDataAsset(UCC_TaggedInputActionsDataAsset* DataAsset)
{
	if (DataAsset) {

		CurrentInputDataAsset = DataAsset;

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

void UCC_InputComponent::SetInputDataAsset_Implementation(UCC_TaggedInputActionsDataAsset* DataAsset)
{
	UCC_InputComponent::InputDataAsset(DataAsset);
}

void UCC_InputComponent::AddInputMappingContext_Implementation(UInputMappingContext* InputMappingContext, int32 Priority)
{
	UCC_InputComponent::InputMappingContext(InputMappingContext, Priority);
}

FGameplayTagContainer UCC_InputComponent::ReturnInputTag()
{
	if (ReturnedInputTagContainer.Num() > 0) {

		return ReturnedInputTagContainer;
	}

	return FGameplayTagContainer::EmptyContainer;
}

FGameplayTagContainer UCC_InputComponent::GetInputTagContainer_Implementation()
{
	return UCC_InputComponent::ReturnInputTag();
}

void UCC_InputComponent::AddInputTagInContainer(FGameplayTag InputTag)
{
	if (InputTag.IsValid()) {

		ReturnedInputTagContainer.AddTag(InputTag);
	}
}

void UCC_InputComponent::RemoveInputTagFromContainer(FGameplayTag InputTag)
{
	if (InputTag.IsValid()) {

		if (ReturnedInputTagContainer.HasTag(InputTag)) {

			ReturnedInputTagContainer.RemoveTag(InputTag);
		}
	}
}