#include "_Project_H/CC_GlobalFunctionsLibrary.h"
#include "NativeGameplayTags.h"

bool UCC_GlobalFunctionsLibrary::CompareTags(FGameplayTag Tag1, FGameplayTag Tag2)
{
	if (Tag1 == Tag2) {
		return true;
	}

	return false;
}

bool UCC_GlobalFunctionsLibrary::DoesGameplayTagContainerContainTag(FGameplayTagContainer GameplayTagContainer, FGameplayTag Tag)
{
	if (GameplayTagContainer.IsValid()) {

		if (!GameplayTagContainer.IsEmpty()) {

			if (Tag.IsValid()) {

				if (GameplayTagContainer.HasTag(Tag)) {

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
