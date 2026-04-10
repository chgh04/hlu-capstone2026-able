#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rootable.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "BaseItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPickedUpSignature, const FString&, ItemName);

UCLASS()
class HLU_CAPSTONE_2026_API ABaseItem : public AActor, public IRootable
{
    GENERATED_BODY()

public:
    ABaseItem();

protected:
    virtual void BeginPlay() override;

protected:
    // 습득 판정 범위
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USphereComponent* PickupRange;

    // 위치 표시용 나이아가라 이펙트 - 게임 시작부터 켜져 있음
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* PickupEffectComponent;

public:
    UPROPERTY(BlueprintAssignable, Category = "Item_Events")
    FOnItemPickedUpSignature OnItemPickedUp;

public:
    // 아이템 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    FString ItemName = TEXT("Unknown Item");

    // 아이템 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    FString ItemDescription = TEXT("");

    // 아이템 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info", meta = (ClampMin = "1"))
    int32 ItemAmount = 1;

    /*
    습득 방식을 Gameplay Tag로 구분합니다.
    Item.Pickup.Auto  : 범위 진입 즉시 자동 습득
    Item.Pickup.Input : F키를 눌러야 습득
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    FGameplayTag PickupTag;

    // 줍기 전 상시 재생할 나이아가라 에셋 (흰색 빛 등)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    class UNiagaraSystem* IdleEffect;

    // 습득 후 이펙트가 꺼지고 액터가 사라지기까지의 딜레이
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info", meta = (ClampMin = "0.0"))
    float DestroyDelay = 0.2f;

    // 플레이어가 현재 범위 안에 있는지
    UPROPERTY(BlueprintReadOnly, Category = "Item_Info")
    bool bPlayerInRange = false;

public:
    // BP_Player의 IA_Interact 입력에서 호출
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

    void ExecutePickup(AActor* Picker);
    void DestroyAfterEffect();

    // 자식 BP에서 구현 - F키 프롬프트 UI 표시
    // BP_Player는 건드리지 않으므로 아이템 BP에서 처리
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void ShowPickupHint();

    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void HidePickupHint();

    bool bIsPickedUp = false;
    FTimerHandle DestroyTimerHandle;
};