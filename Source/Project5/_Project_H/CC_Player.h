#pragma once

#include "CoreMinimal.h"
#include "_Project_H/CC_BaseCharacter.h"
#include "CC_Player.generated.h"

UCLASS()
class PROJECT5_API ACC_Player : public ACC_BaseCharacter
{
	GENERATED_BODY()

	ACC_Player();

protected:
	virtual void BeginPlay() override;
};
