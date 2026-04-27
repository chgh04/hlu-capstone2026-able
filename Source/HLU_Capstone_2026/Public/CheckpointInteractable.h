
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CheckpointInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCheckpointInteractable : public UInterface
{
	GENERATED_BODY()
};

class HLU_CAPSTONE_2026_API ICheckpointInteractable
{
	GENERATED_BODY()

public:
	// 휴식을 취할 때 호출되는 함수
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void RestAtCheckpoint(float HealPercentage, AActor* CheckpointRef);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void EndCheckpointRest();
};
