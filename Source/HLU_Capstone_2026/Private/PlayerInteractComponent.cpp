// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerInteractComponent.h"
#include "PlayerBase.h"
#include "InteractReceiver.h"
#include "Rootable.h"
#include "CheckpointInteractable.h"

// Sets default values for this component's properties
UPlayerInteractComponent::UPlayerInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerInteractComponent::RegisterInteractable(AActor* Interactable)
{
	if (Interactable)
	{
		NearbyInteractables.AddUnique(Interactable);
	}
}

void UPlayerInteractComponent::UnregisterInteractable(AActor* Interactable)
{
	NearbyInteractables.Remove(Interactable);
}

void UPlayerInteractComponent::RegisterItem(AActor* Item)
{
	if (Item)
	{
		NearbyItems.AddUnique(Item);
	}
}

void UPlayerInteractComponent::UnregisterItem(AActor* Item)
{
	NearbyItems.Remove(Item);
}

void UPlayerInteractComponent::HandleInteractInput(APlayerBase* Player, FVector OriginCamSocketOffset)
{
    // 1. InteractableBase 우선 처리
    TArray<AActor*> InteractablesCopy = NearbyInteractables;
    if (InteractablesCopy.Num() > 0)
    {
        AActor* Target = InteractablesCopy[0];
        if (Target && Target->Implements<UInteractReceiver>())
        {
            // 카메라 연출을 위한 계산 (Player 정보가 필요하므로 매개변수로 받음)
            FVector Midpoint = (Player->GetActorLocation() + Target->GetActorLocation()) * 0.5f;
            FVector RelativeOffset = Midpoint - Player->GetActorLocation();
            RelativeOffset.Z = OriginCamSocketOffset.Z - 20.f;

            // 델리게이트 발동 (BP에서 이 신호를 받고 PlayInteractCameraZoomIn 실행)
            OnInteractStarted.Broadcast(RelativeOffset, Target);

            // 타겟 기능 실행
            IInteractReceiver::Execute_TryInteract(Target, Player);

            // 상호작용 플래그 전환
            bIsInteracting = true;

            return;
        }
    }

    // 2. 아이템 처리
    TArray<AActor*> ItemsCopy = NearbyItems;
    for (AActor* Item : ItemsCopy)
    {
        if (Item && Item->Implements<URootable>())
        {
            IRootable::Execute_TryPickup(Item, Player);
        }
    }
}

void UPlayerInteractComponent::CancelInteraction()
{   
    UE_LOG(LogTemp, Warning, TEXT("CancelInteraction Call"));
    // 상호작용 도중일때만 호출됨
    if (bIsInteracting)
    {   
        // 만약 체크포인트 포인터가 비어있지 않다면 체크포인트에게 상호작용 종료를 전달
        if (CurrentRestingCheckpoint && CurrentRestingCheckpoint->Implements<UCheckpointInteractable>())
        {   
            // 이때, 체크포인트와 상호작용 하지 않았더라도 체크포인트 포인터에서 이를 무시함
            ICheckpointInteractable::Execute_EndCheckpointRest(CurrentRestingCheckpoint);

            UE_LOG(LogTemp, Warning, TEXT("EndCheckpointRest Call"));
        }

        // 포인터 비우기
        CurrentRestingCheckpoint = nullptr;

        // 플래그 전환 
        bIsInteracting = false;

        // 줌아웃 델리게이트 발동
        OnInteractEnded.Broadcast();
    }
}

//댕글링 포인터 크래시 방지
void UPlayerInteractComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // 종료 시 배열 비우기 - 댕글링 포인터 방지
    NearbyItems.Empty();
    NearbyInteractables.Empty();

    // 포인터 초기화
    CurrentRestingCheckpoint = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("InteractComponent: EndPlay - Cleanup arrays"));
}
