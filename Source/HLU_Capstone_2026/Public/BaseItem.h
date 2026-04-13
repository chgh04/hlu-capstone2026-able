#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rootable.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "Project_Types.h"
#include "BaseItem.generated.h"

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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* PickupRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* PickupEffectComponent;

public:
    UPROPERTY(BlueprintAssignable, Category = "Item_Events")
    FOnItemPickedUpSignature OnItemPickedUp;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    FPilgrimItemData ItemData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    FGameplayTag PickupTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info")
    class UNiagaraSystem* IdleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item_Info", meta = (ClampMin = "0.0"))
    float DestroyDelay = 0.2f;

    UPROPERTY(BlueprintReadOnly, Category = "Item_Info")
    bool bPlayerInRange = false;

public:
    // IRootable 인터페이스 구현
    virtual void OnPickedUp_Implementation(AActor* Picker) override;
    virtual void OnUsed_Implementation(AActor* User) override;

    // HandleInteractInput에서 호출 - bPlayerInRange 체크 후 습득 처리
    virtual void TryPickup_Implementation(AActor* Picker) override;

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

    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void ShowPickupHint();

    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void HidePickupHint();

    bool bIsPickedUp = false;

    FTimerHandle DestroyTimerHandle;
};