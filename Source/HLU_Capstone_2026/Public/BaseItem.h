#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rootable.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "Project_Types.h"
#include "BaseItem.generated.h"

// 아이템 습득 시 UI에 아이템 데이터를 전달하는 델리게이트
// 인벤토리 완성 후 여기에 바인딩하여 아이템 목록에 추가
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPickedUpSignature, const FPilgrimItemData&, ItemData);

UCLASS()
class HLU_CAPSTONE_2026_API ABaseItem : public AActor, public IRootable
{
    GENERATED_BODY()

public:
    ABaseItem();

protected:
    virtual void BeginPlay() override;

protected:
    // 습득 판정 범위 - ECC_GameTraceChannel1(플레이어 채널)과만 Overlap
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* PickupRange;

    // 줍기 전부터 켜져 있는 위치 알림 이펙트
    // SetAutoActivate(true) - 게임 시작 시 자동 재생
    // 습득 시 Deactivate() 호출로 꺼짐
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* PickupEffectComponent;

public:
    // UI에서 바인딩 가능하도록 public 설정
    // 인벤토리 완성 후 여기에 바인딩하여 획득 아이템 목록 관리
    UPROPERTY(BlueprintAssignable, Category = "Item_Events")
    FOnItemPickedUpSignature OnItemPickedUp;

public:
    // 아이템 데이터 - BP 디테일 패널 Item_Info 카테고리에서 설정
    // ItemCode, ItemName, ItemDescription, ItemType, ItemAmount 포함
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    FPilgrimItemData ItemData;

    // 습득 방식 - Project Settings > GameplayTags에서 등록 후 사용
    // Item.Pickup.Auto  : 범위 진입 즉시 자동 습득
    // Item.Pickup.Input : F키를 눌러야 습득
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    FGameplayTag PickupTag;

    // 줍기 전 상시 재생할 나이아가라 에셋
    // PickupEffectComponent에 자동으로 연결됨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    class UNiagaraSystem* IdleEffect;

    // 습득 후 이펙트가 꺼지고 액터가 삭제되기까지의 딜레이(초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info", meta = (ClampMin = "0.0"))
    float DestroyDelay = 0.2f;

    // 플레이어가 현재 범위 안에 있는지 - TryPickupByInput에서 체크
    UPROPERTY(BlueprintReadOnly, Category = "Item_Info")
    bool bPlayerInRange = false;

public:
    // BP_Player의 IA_Interact 입력에서 호출
    // bPlayerInRange가 true일 때만 실제로 습득 처리
    UFUNCTION(BlueprintCallable, Category = "Item")
    void TryPickupByInput(AActor* Picker);

public:
    virtual void OnPickedUp_Implementation(AActor* Picker) override;
    virtual void OnUsed_Implementation(AActor* User) override;

protected:
    UFUNCTION()
    void OnPickupRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPickupRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 실제 습득 처리 - 내부에서만 호출
    // OnPickupRangeBeginOverlap(Auto), TryPickupByInput(Input) 두 경로에서 호출됨
    void ExecutePickup(AActor* Picker);

    // DestroyDelay 후 액터 삭제 (타이머 콜백)
    void DestroyAfterEffect();

    // 자식 BP에서 구현 - F키 프롬프트 UI 표시
    // UI 완성 후 실제 위젯 연결 예정
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void ShowPickupHint();

    // 자식 BP에서 구현 - F키 프롬프트 UI 숨기기
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void HidePickupHint();

    // 중복 습득 방지 - ExecutePickup 진입 시 true로 변경
    bool bIsPickedUp = false;

    FTimerHandle DestroyTimerHandle;
};