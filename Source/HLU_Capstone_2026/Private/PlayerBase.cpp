#include "PlayerBase.h"
#include "BlueprintGameplayTagLibrary.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

APlayerBase::APlayerBase()
{
    // 카메라 암 생성 및 설정
    CameraString = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraString->SetupAttachment(RootComponent); // 캐릭터의 루트 부착
    CameraString->TargetArmLength = 400.0f;       // 캐릭터와의 거리 설정(디테일창 변경 가능)
    CameraString->bUsePawnControlRotation = true; // 컨트롤러 회전에 따라 암도 회전

    // 카메라 생성
    MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    MainCamera->SetupAttachment(CameraString, USpringArmComponent::SocketName);
    MainCamera->bUsePawnControlRotation = false;
}

void APlayerBase::BeginPlay()
{
    Super::BeginPlay();
}

void APlayerBase::Attack_Implementation()
{
    Super::Attack_Implementation();

    UE_LOG(LogTemp, Warning, TEXT("Player is now attack"));
}

bool APlayerBase::CanAttackTarget(AActor* Target) const
{
    // 부모의 기본 체크(자기 자신 등)를 먼저 통과해야 함
    if (!Super::CanAttackTarget(Target)) return false;
    
    // 타겟의 태그 인터페이스를 불러오기
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Target);

    // 태그 인터페이스가 없다면 false 
    if (!TagInterface)
    {
        return false;
    }

    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Entity.Team.Enemy"));
    FGameplayTag DestructibleTag = FGameplayTag::RequestGameplayTag(FName("Entity.Type.Destructible"));

    // 타겟이 "Enemy" 태그를 가지고 있거나, "Destructible(파괴가능)" 태그가 있다면 공격 가능
    return TagInterface->HasMatchingGameplayTag(EnemyTag) || TagInterface->HasMatchingGameplayTag(DestructibleTag);
}
