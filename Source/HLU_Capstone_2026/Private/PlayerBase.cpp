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

void APlayerBase::TryAttack()
{
    // 부모클래스 TryAttack을 실행하지 않고 완전히 재정의
    // 상태검사, 사망 / 넉백 / 피해무적상태(추후 추가 가능) 등등이라면 공격 불가
    if (bIsKnockBack)
    {
        UE_LOG(LogTemp, Warning, TEXT("C++ Player Attack Return: Other Reason"));
        return;
    }

    // 플레이어가 공격상태일 경우 리턴하지만, 만약 연계공격 타이밍에 입력이 들어온다면 연계 트리거를 활성화
    if (bIsAttacking)
    {   
        if (!bSaveAttack && !bIgnoreSaveAttack)
        {
            ComboCount++;       // 콤보 카운트 증가
            bSaveAttack = true; // 연계 트리거 활성화
            UE_LOG(LogTemp, Warning, TEXT("C++ Player Attack Return: Attack saved"));
            return;
        }

        // 이외의 경우 모두 리턴
        UE_LOG(LogTemp, Warning, TEXT("C++ Player Attack Return: Now attcking"));
        return;
    }
    else // 첫 번째 공격일 경우에 연계공격 트리거 및 콤보 초기화 이후 공격 로직 호출
    {
        bSaveAttack = false;
        ComboCount = 0;
        Attack();
    }
}

void APlayerBase::Attack_Implementation()
{   
    // 부모 Attack 함수 미실행
    //Super::Attack_Implementation();

    // 공격중/연계중 아니라면 첫 번째 공격 시작
    bIsAttacking = true;

    UE_LOG(LogTemp, Warning, TEXT("C++: Now Attack!(PlayerBase)"));
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

void APlayerBase::CheckCombo()
{   
    // 공격 연계가 가능하도록 전환
    bIgnoreSaveAttack = false;
}

void APlayerBase::ResetCombo()
{   
    // 공격 연계가 불가능하도록 전환
    bIgnoreSaveAttack = true;

    // TryAttack에서 bSaveAttack이 활성화 되었다면 다음 공격으로 자동으로 넘어감, 연계공격 트리거 비활성화로 반복 방지
    if (bSaveAttack)
    {   
        Attack();
        bSaveAttack = false;
        return;
    }

    // 연계공격 트리거가 false라면 공격, 콤보공격 관련 트리거 초기화
    bSaveAttack = false;
    ComboCount = 0;
    UE_LOG(LogTemp, Warning, TEXT("C++: Combo Reset(PlayerBase)"));
}

void APlayerBase::TryJump()
{   
    // 점프가 불가능한 상황에서는 점프 불가
    if (bIsKnockBack)
    {
        return;
    }
    
    Jump();
}

void APlayerBase::TryStopJumping()
{   
    // 점프 중단, 추후 별도 로직 추가 가능
    StopJumping();
}