// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBase.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

APlayerBase::APlayerBase()
{
    // 체력 컴포넌트 생성 및 부착
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // 공격 판정 박스 생성 - RootComponent에 부착
    AttackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackBox"));
    AttackBox->SetupAttachment(RootComponent);
    AttackBox->SetCollisionProfileName(TEXT("Trigger"));
    AttackBox->SetGenerateOverlapEvents(true);
}

void APlayerBase::ReceiveDamage_Implementation(const FDamageData& DamageData)
{
    UE_LOG(LogTemp, Warning, TEXT("Player Received Damage: %f"), DamageData.DamageAmount);
}

void APlayerBase::OnDeath_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Player is Dead!"));
}