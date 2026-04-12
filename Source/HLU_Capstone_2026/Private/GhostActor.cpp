
#include "GhostActor.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AGhostActor::AGhostActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 스프라이트 컴포넌트 생성 및 루트 컴포넌트 설정 
	GhostSpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("GhostSpriteComp"));
	RootComponent = GhostSpriteComp;

	// 물리연산충돌 완전히 꺼버리김
	GhostSpriteComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GhostSpriteComp->SetGenerateOverlapEvents(false);
}

// Called when the game starts or when spawned
void AGhostActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGhostActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float FadeSpeed = 2.0f;

	CurrentOpacity -= DeltaTime * FadeSpeed;

	if (GhostMat)
	{
		GhostMat->SetScalarParameterValue(FName("Opacity"), CurrentOpacity);
	}

	if (CurrentOpacity <= 0.0f)
	{
		Destroy();
	}
}

void AGhostActor::InitGhost(class UPaperSprite* InSprite, FVector InScale)
{
	if (InSprite)
	{
		GhostSpriteComp->SetSprite(InSprite);
		SetActorScale3D(InScale);
		GhostMat = GhostSpriteComp->CreateDynamicMaterialInstance(0);

		if (GhostMat)
		{
			GhostMat->SetScalarParameterValue(FName("Opacity"), CurrentOpacity);
		}
	}
}