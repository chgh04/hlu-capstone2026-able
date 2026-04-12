#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "InteractableBase.generated.h"

//FOnInteractSignature 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractSignature, AActor*, Interactor);

UCLASS()
class HLU_CAPSTONE_2026_API AInteractableBase : public AActor
{
    GENERATED_BODY()

public:
    AInteractableBase();

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* InteractRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPaperSpriteComponent* SpriteComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* InteractEffect;

public:
    // 상호작용 범위 반경 - 디테일 패널에서 조정 가능
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable")
    float InteractRadius = 150.f;

    // 현재 플레이어가 범위 안에 있는지
    UPROPERTY(BlueprintReadOnly, Category = "Interactable")
    bool bPlayerInRange = false;

public:
    // 자식 클래스에서 반드시 구현 - 실제 상호작용 로직
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
    void OnInteract(AActor* Interactor);

    // BP_Player의 IA_Interact에서 호출
    UFUNCTION(BlueprintCallable, Category = "Interactable")
    void TryInteract(AActor* Interactor);

protected:
    UFUNCTION()
    void OnInteractRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 자식 BP에서 구현 - F키 프롬프트 표시
    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void ShowInteractHint();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
    void HideInteractHint();
};