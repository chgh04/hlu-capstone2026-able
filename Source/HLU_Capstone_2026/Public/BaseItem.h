#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rootable.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "BaseItem.generated.h"

/*
ABaseItem - 모든 아이템의 베이스 클래스

습득 방식 (Gameplay Tag로 구분)
- Item.Pickup.Auto  : 범위에 들어오는 순간 자동 습득 (
- Item.Pickup.Input : 범위 안에서 상호작용 키를 눌러야 습득 

나이아가라 이펙트
자식 BP의 디테일 패널에서 PickupEffect 항목에 원하는 Niagara System을 지정
이펙트 재생 시간은 EffectDuration으로 조절
*/

// 아이템 습득 시 UI에 알리는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemPickedUpSignature, const FString&, ItemName);


UCLASS()
class HLU_CAPSTONE_2026_API ABaseItem : public AActor, public IRootable
{
    GENERATED_BODY()

public:
    ABaseItem();

protected:
    virtual void BeginPlay() override;

    // 컴포넌트
protected:
    // 습득 판정 범위
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USphereComponent* PickupRange;

    // 임시 테스트용 메시
    // 에디터 디테일 패널 → ItemMesh → Static Mesh 에서 변경 가능
    // 경로: Engine/BasicShapes/Sphere 또는 Engine/BasicShapes/Cube
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* ItemMesh;

    // 습득 시 재생되는 나이아가라 이펙트 컴포넌트
    // 평소엔 비활성화 상태, 습득 시 Activate() 호출됨
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* PickupEffectComponent;

    // UI 델리게이트
public:
    UPROPERTY(BlueprintAssignable, Category = "Item_Events")
    FOnItemPickedUpSignature OnItemPickedUp;

    // 아이템 정보
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    FString ItemName = TEXT("Unknown Item");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    FString ItemDescription = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info", meta = (ClampMin = "1"))
    int32 ItemAmount = 1;

    // 습득 방식 설정 
public:
    // Item.Pickup.Auto 또는 Item.Pickup.Input 을 디테일 패널에서 지정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup_Setting")
    FGameplayTag PickupTag;

    // 나이아가라 이펙트 에셋
    // 디테일 패널 Pickup_Setting → PickupEffect 에서 원하는 이펙트 지정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup_Setting")
    class UNiagaraSystem* PickupEffect;

    // 이펙트 재생 후 액터 삭제까지의 시간 
    // 이 시간 안에 나이아가라 이펙트가 재생
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup_Setting", meta = (ClampMin = "0.0"))
    float EffectDuration = 1.0f;

    // 플레이어가 현재 범위 안에 있는지 
    UPROPERTY(BlueprintReadOnly, Category = "Pickup_Setting")
    bool bPlayerInRange = false;

    // IRootable 인터페이스 구현 
public:
    virtual void OnPickedUp_Implementation(AActor* Picker) override;
    virtual void OnUsed_Implementation(AActor* User) override;

    // 외부에서 호출하는 함수 
public:
    // 플레이어 BP의 상호작용 키 입력에서 이 함수를 호출
    UFUNCTION(BlueprintCallable, Category = "Item")
    void TryPickupByInput(AActor* Picker);

    // 내부 함수
protected:
    UFUNCTION()
    void OnPickupRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPickupRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 실제 습득 처리 (내부 전용)
    void ExecutePickup(AActor* Picker);

    // EffectDuration 후 액터 삭제 (타이머 콜백)
    void DestroyAfterEffect();

    // 자식 BP에서 구현 - "F키를 눌러 줍기" 같은 UI 힌트 표시
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void ShowPickupHint();

    // 자식 BP에서 구현 - 힌트 숨기기
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void HidePickupHint();

    bool bIsPickedUp = false;

    FTimerHandle DestroyTimerHandle;
};
