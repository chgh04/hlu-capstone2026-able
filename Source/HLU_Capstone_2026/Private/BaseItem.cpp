#include "BaseItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerBase.h"
#include "GameplayTagsModule.h"
#include "UObject/ConstructorHelpers.h"

ABaseItem::ABaseItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // 습득 범위 설정
    PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
    RootComponent = PickupRange;
    PickupRange->SetSphereRadius(80.f); 
    PickupRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    // 플레이어 전용 채널만 Overlap - EnemyBase의 DetectionRange와 동일한 방식
    PickupRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
    PickupRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 임시 테스트용 메시 : 구 

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f)); 

    // 기본 메시 구로 설정
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        ItemMesh->SetStaticMesh(SphereMesh.Object);
    }

    // 충돌 없이 보이기만 하도록 설정 (습득 판정은 PickupRange가 담당)
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 나이아가라 이펙트 컴포넌트
    // 평소엔 비활성화, 습득 시 Activate() 호출됨
    PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
    PickupEffectComponent->SetupAttachment(RootComponent);
    PickupEffectComponent->SetAutoActivate(false); 
}

void ABaseItem::BeginPlay()
{
    Super::BeginPlay();

    if (PickupRange)
    {
        PickupRange->OnComponentBeginOverlap.AddDynamic(this, &ABaseItem::OnPickupRangeBeginOverlap);
        PickupRange->OnComponentEndOverlap.AddDynamic(this, &ABaseItem::OnPickupRangeEndOverlap);
    }

    // 디테일 패널에서 지정한 PickupEffect 에셋을 컴포넌트에 연결
    if (PickupEffect && PickupEffectComponent)
    {
        PickupEffectComponent->SetAsset(PickupEffect);
    }
}

void ABaseItem::OnPickupRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerBase* Player = Cast<APlayerBase>(OtherActor);
    if (!Player) return;

    // 자동 습득 방식
    FGameplayTag AutoTag = FGameplayTag::RequestGameplayTag(FName("Item.Pickup.Auto"));
    if (PickupTag == AutoTag)
    {
        ExecutePickup(Player);
        return;
    }

    // 키 입력 방식 - 힌트 표시 후 대기
    FGameplayTag InputTag = FGameplayTag::RequestGameplayTag(FName("Item.Pickup.Input"));
    if (PickupTag == InputTag)
    {
        bPlayerInRange = true;
        ShowPickupHint();
    }
}

void ABaseItem::OnPickupRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerBase* Player = Cast<APlayerBase>(OtherActor);
    if (!Player) return;

    bPlayerInRange = false;
    HidePickupHint();
}

void ABaseItem::TryPickupByInput(AActor* Picker)
{
    if (!bPlayerInRange || bIsPickedUp) return;
    ExecutePickup(Picker);
}

void ABaseItem::ExecutePickup(AActor* Picker)
{
    if (bIsPickedUp) return;
    bIsPickedUp = true;

    UE_LOG(LogTemp, Warning, TEXT("Item: [%s] picked up!"), *ItemName);

    // UI 델리게이트 브로드캐스트
    OnItemPickedUp.Broadcast(ItemName);

    // 콜리전 제거 - 이펙트 재생 중 중복 습득 방지
    if (PickupRange)
    {
        PickupRange->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 메시 숨기기 - 아이템은 사라진 것처럼 보이게
    if (ItemMesh)
    {
        ItemMesh->SetVisibility(false);
    }

    // 나이아가라 이펙트 재생
    // SetAutoActivate(false)로 꺼놨다가 여기서 켭니다
    if (PickupEffectComponent)
    {
        PickupEffectComponent->Activate(true);
    }

    // EffectDuration(기본 1초) 후 액터 전체 삭제
    // 이 시간 동안 나이아가라 이펙트가 재생됩니다
    GetWorldTimerManager().SetTimer(
        DestroyTimerHandle,
        this,
        &ABaseItem::DestroyAfterEffect,
        EffectDuration,
        false
    );
}

void ABaseItem::DestroyAfterEffect()
{
    Destroy();
}

void ABaseItem::OnPickedUp_Implementation(AActor* Picker)
{
    ExecutePickup(Picker);
}

void ABaseItem::OnUsed_Implementation(AActor* User)
{
    UE_LOG(LogTemp, Warning, TEXT("Item: [%s] used by %s"), *ItemName, *GetNameSafe(User));
}
