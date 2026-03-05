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

		//UE_LOG(LogActor, Warning, TEXT("Input system is working fine...."));
		//UE_LOG(LogActor, Warning, TEXT("GameplayTag:- %s"), *InputTag.ToString());
		//UE_LOG(LogActor, Warning, TEXT("Requester Name:- %s"), *Requester->GetName());

		ReturnInputTag(InputTag);
	}
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

void UCC_InputComponent::ReturnInputTag(FGameplayTag InputTag)
{
	if (InputTag.IsValid()) {

		ReturnedInputTag = InputTag;
		return;
	}

	ReturnedInputTag = FGameplayTag::EmptyTag;
}

FGameplayTag UCC_InputComponent::GetInputTag_Implementation()
{
	if (ReturnedInputTag.IsValid()) {

		return ReturnedInputTag;
	}

	return FGameplayTag::EmptyTag;
}
