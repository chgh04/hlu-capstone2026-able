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

    // 습득 범위
    PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
    RootComponent = PickupRange;
    PickupRange->SetSphereRadius(80.f);
    PickupRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    PickupRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
    PickupRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 임시 테스트용 메시 (기본값: 구)
    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereMesh.Succeeded())
    {
        ItemMesh->SetStaticMesh(SphereMesh.Object);
    }
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 나이아가라 이펙트 컴포넌트 - 평소엔 비활성화
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

    // 플레이어가 범위 안에 들어왔음을 무조건 기록
    // 태그 체크는 여기서 하지 않음 - 태그 비교 실패 시 bPlayerInRange가 false로 남는 버그 방지
    bPlayerInRange = true;

    UE_LOG(LogTemp, Warning, TEXT("Item: Player entered range, bPlayerInRange = true"));

    // 자동 습득 방식이면 즉시 처리
    FGameplayTag AutoTag = FGameplayTag::RequestGameplayTag(FName("Item.Pickup.Auto"));
    if (PickupTag == AutoTag)
    {
        ExecutePickup(Player);
        return;
    }

    // 키 입력 방식이면 힌트만 표시하고 대기
    // bPlayerInRange가 true이므로 F키 누르면 TryPickupByInput에서 처리됨
    ShowPickupHint();
}

void ABaseItem::OnPickupRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerBase* Player = Cast<APlayerBase>(OtherActor);
    if (!Player) return;

    bPlayerInRange = false;
    UE_LOG(LogTemp, Warning, TEXT("Item: Player left range, bPlayerInRange = false"));

    HidePickupHint();
}

void ABaseItem::TryPickupByInput(AActor* Picker)
{
    UE_LOG(LogTemp, Warning, TEXT("Item: TryPickupByInput called, bPlayerInRange=%s, bIsPickedUp=%s"),
        bPlayerInRange ? TEXT("true") : TEXT("false"),
        bIsPickedUp ? TEXT("true") : TEXT("false"));

    // 범위 밖이거나 이미 습득했으면 무시
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

    // 충돌 제거 - 중복 습득 방지
    if (PickupRange)
    {
        PickupRange->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 메시 숨기기
    if (ItemMesh)
    {
        ItemMesh->SetVisibility(false);
    }

    // 나이아가라 이펙트 재생
    if (PickupEffectComponent)
    {
        PickupEffectComponent->Activate(true);
    }

    // EffectDuration 후 액터 삭제
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
