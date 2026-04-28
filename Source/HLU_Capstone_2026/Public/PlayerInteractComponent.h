// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractComponent.generated.h"

/*플레이어의 모든 상호작용 로직을 담당합니다.*/

// 상호작용 시작/종료 시 카메라 연출 등을 BP에 알리기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractStartedSignature, FVector, MidpointOffset, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractEndedSignature);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HLU_CAPSTONE_2026_API UPlayerInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerInteractComponent();

	// 델리게이트 인스턴스
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractStartedSignature OnInteractStarted;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractEndedSignature OnInteractEnded;

	// InteractableBase 핵심 상호작용 함수 
	void HandleInteractInput(class APlayerBase* Player, FVector OriginCamSocketOffset);
	void CancelInteraction();

	// 플레이어가 상호작용 범위 안에 들어왔을 때, 상호작용 가능한 아이템,상호작용 오브젝트 배열 관리
	void RegisterInteractable(AActor* Interactable);
	void UnregisterInteractable(AActor* Interactable);
	void RegisterItem(AActor* Item);
	void UnregisterItem(AActor* Item);

	// 상태 반환
	bool GetIsInteracting() const { return bIsInteracting; }
	void SetRestingCheckpoint(AActor* Checkpoint) { CurrentRestingCheckpoint = Checkpoint; }

private:
	// 주변의 상호작용 오브젝트
	UPROPERTY()
	TArray<AActor*> NearbyInteractables;

	// 주변의 아이템
	UPROPERTY()
	TArray<AActor*> NearbyItems;

	// 플레이어 상호작용 플래그 
	bool bIsInteracting = false;

	// 가장 최근에 휴식한 체크포인트 레퍼런스 
	UPROPERTY()
	AActor* CurrentRestingCheckpoint = nullptr;

	// 댕글링 포인터 크래시 방지 - NearbyItems/NearbyInteractables 배열에 이미 삭제된 액터 포인터가 남아있어서 생기는 크래시
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

		
};
