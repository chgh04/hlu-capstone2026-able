#include "Engine/LevelStreamingDynamic.h"
#include "Engine/LevelStreaming.h"
#include "MapTransitionTrigger.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PilgrimGameInstance.h"
#include "MapEntrance.h"
#include "BlueprintGameplayTagLibrary.h"

AMapTransitionTrigger::AMapTransitionTrigger()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 트리거 박스 생성 - 플레이어 오버랩 감지용
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
    TriggerBox->SetBoxExtent(FVector(50.f, 50.f, 100.f));
    TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMapTransitionTrigger::BeginPlay()
{
    Super::BeginPlay();

    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMapTransitionTrigger::OnTriggerBeginOverlap);
}

void AMapTransitionTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // 전환 중이면 무시
    if (bIsTransitioning) return;

    // 태그로 플레이어 확인
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);
    if (!TagInterface || !TagInterface->HasMatchingGameplayTag(
        FGameplayTag::RequestGameplayTag(FName("Entity.Team.Player")))) return;

    ACharacter* Player = Cast<ACharacter>(OtherActor);
    if (!Player) return;

    // 전환 시작
    bIsTransitioning = true;
    TransitioningPlayer = Player;

    // 입력 차단
    if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
    {
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
    }

    // Tick 활성화 - 자동 이동 시작
    SetActorTickEnabled(true);

    // 자동 이동 시작 - AutoMoveDuration 동안 MoveDirection 방향으로 이동
    GetWorldTimerManager().SetTimer(
        AutoMoveTimerHandle,
        this,
        &AMapTransitionTrigger::StartFadeAndTransition,
        AutoMoveDuration,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("MapTransition: Transition started"));
}

void AMapTransitionTrigger::StartFadeAndTransition()
{
    if (!TransitioningPlayer) return;

    // 자동 이동 중지
    if (UCharacterMovementComponent* MoveComp = TransitioningPlayer->GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
    }

    // 암전 델리게이트 발동 - BP에서 UMG 페이드 아웃 연결
    OnFadeOut.Broadcast();

    // BlackoutDuration 후 텔레포트 실행
    GetWorldTimerManager().SetTimer(
        BlackoutTimerHandle,
        this,
        &AMapTransitionTrigger::ExecuteTransition,
        BlackoutDuration,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("MapTransition: FadeOut started"));
}

void AMapTransitionTrigger::ExecuteTransition()
{
    if (!TransitioningPlayer) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // GameInstance에 목적지 입구 태그 저장
    UPilgrimGameInstance* GI = Cast<UPilgrimGameInstance>(UGameplayStatics::GetGameInstance(World));
    if (GI)
    {
        GI->TargetEntranceTag = TargetEntranceTag;
        GI->CurrentSubLevelName = TargetLevelName;
    }

    // 서브레벨 언로드
    if (!CurrentLevelName.IsNone())
    {
        for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
        {
            if (!StreamingLevel) continue;
            FString LevelPackageName = StreamingLevel->GetWorldAssetPackageName();
            if (LevelPackageName.EndsWith(CurrentLevelName.ToString()))
            {
                StreamingLevel->SetShouldBeLoaded(false);
                StreamingLevel->SetShouldBeVisible(false);
                break;
            }
        }
    }

    // 서브레벨 로드
    if (!TargetLevelName.IsNone())
    {
        for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
        {
            if (!StreamingLevel) continue;
            FString LevelPackageName = StreamingLevel->GetWorldAssetPackageName();
            if (LevelPackageName.EndsWith(TargetLevelName.ToString()))
            {
                StreamingLevel->SetShouldBeLoaded(true);
                StreamingLevel->SetShouldBeVisible(true);
                break;
            }
        }
    }

    // 레벨 로드 완료 대기 후 텔레포트 - 0.2초마다 체크
    GetWorldTimerManager().SetTimer(
        WaitForLevelTimerHandle,
        this,
        &AMapTransitionTrigger::TryTeleportToEntrance,
        0.2f,
        true  // 반복 실행
    );

    UE_LOG(LogTemp, Warning, TEXT("MapTransition: Waiting for level to load..."));
}

void AMapTransitionTrigger::FinishTransition()
{
    // 입력 복구
    if (TransitioningPlayer)
    {
        if (APlayerController* PC = Cast<APlayerController>(TransitioningPlayer->GetController()))
        {
            PC->SetIgnoreMoveInput(false);
            PC->SetIgnoreLookInput(false);
        }
    }

    // 페이드 인 델리게이트 발동 - BP에서 UMG 페이드 인 연결
    OnFadeIn.Broadcast();

    // 전환 완료
    bIsTransitioning = false;
    TransitioningPlayer = nullptr;
    
    // Tick 비활성화
    SetActorTickEnabled(false);

    UE_LOG(LogTemp, Warning, TEXT("MapTransition: FadeIn started, transition complete"));
}

void AMapTransitionTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 전환 중일 때만 자동 이동 처리
    if (bIsTransitioning && TransitioningPlayer)
    {
        TransitioningPlayer->AddMovementInput(
            FVector(MoveDirection, 0.f, 0.f), 1.f
        );
    }
}

void AMapTransitionTrigger::TryTeleportToEntrance()
{
    if (!TransitioningPlayer) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // MapEntrance 찾기 - 레벨 로드 완료 전이면 못 찾음
    TArray<AActor*> AllEntrances;
    UGameplayStatics::GetAllActorsOfClass(World, AMapEntrance::StaticClass(), AllEntrances);
    for (AActor* Actor : AllEntrances)
    {
        AMapEntrance* Entrance = Cast<AMapEntrance>(Actor);
        if (Entrance && Entrance->EntranceTag == TargetEntranceTag)
        {
            // 입구 찾음 - 타이머 중지 후 텔레포트
            GetWorldTimerManager().ClearTimer(WaitForLevelTimerHandle);
            Entrance->TeleportPlayerToEntrance(TransitioningPlayer);
            FinishTransition();

            UE_LOG(LogTemp, Warning, TEXT("MapTransition: Teleported to entrance"));
            return;
        }
    }

    // 아직 못 찾으면 다음 틱에 재시도
    UE_LOG(LogTemp, Warning, TEXT("MapTransition: Level not ready yet, retrying..."));
}