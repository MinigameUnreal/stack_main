// Fill out your copyright notice in the Description page of Project Settings.


#include "MG_EnemyLich.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Minigames/OBot/AI/UI/MG_WidgetComponent.h"
#include "Minigames/OBot/AI/UI/MG_MonsterHpBar.h"
#include "Minigames/OBot/AI/Skill/MG_LichPoison.h"



AMG_EnemyLich::AMG_EnemyLich()
{
	GetCapsuleComponent()->InitCapsuleSize(40.0f, 110.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));

	GetCharacterMovement()->JumpZVelocity = 700.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->SetWalkableFloorAngle(50.0f);
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	GetCharacterMovement()->GravityScale = 1.0f;
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -110.0), FRotator(0.0, -90.0, 0.0));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("EnemyMesh"));


	static ConstructorHelpers::FObjectFinder<USkeletalMesh> LichMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/UndeadPack/Lich/Mesh/SK_Lich_Full.SK_Lich_Full'"));
	if (LichMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(LichMeshRef.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> LichAnimInstanceClassRef(TEXT("/Game/Character/AI/Lich/MG_Lich.MG_Lich_C"));
	if (LichAnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(LichAnimInstanceClassRef.Class);
	}
	

	

	HpBar->SetRelativeLocation(FVector(0.0f, 0.0f, 240.0f));

	CurrentMonsterType = EMonsterType::Lich;
	MaxHp = 3000.0f;
	AttackDamage = 50.0f;
	// 스킬 범위 1000에서 1500 정도가 적당해보임

}

void AMG_EnemyLich::BeginPlay()
{
	Super::BeginPlay();

	CurrentHp = MaxHp;

}

void AMG_EnemyLich::PoisonSkill()
{
	UWorld* World = GetWorld();
	FVector LichLocation = GetActorLocation();

	if (nullptr == World)
	{
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(Poison), false, this);
	bool bResult = World->OverlapMultiByChannel(
		OverlapResults,
		LichLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(SkillDetectRange),
		CollisionQueryParams
	);

	APawn* ClosePawn = nullptr;
	float MinDistanceSquared = FLT_MAX;

	if (bResult)
	{
		for (auto OverlapResult : OverlapResults)
		{
			APawn* Pawn = Cast<APawn>(OverlapResult.GetActor());
			if (Pawn && Pawn->GetController()->IsPlayerController())
			{
				float DistanceSquared = FVector::DistSquared(Pawn->GetActorLocation(), LichLocation);

				if (DistanceSquared < MinDistanceSquared)
				{
					MinDistanceSquared = DistanceSquared;
					ClosePawn = Pawn;
				}
			}
		}
	}

	if (ClosePawn)
	{
		const FVector PlayerLocation = ClosePawn->GetActorLocation();
		const FRotator PlayerRotator = ClosePawn->GetActorRotation();
		
		FActorSpawnParameters Param;

		GetWorld()->SpawnActor(LichPoisonClass, &PlayerLocation, &PlayerRotator, Param);
		PoisonSkillAnimation();
	}

}

void AMG_EnemyLich::SpawnSkill()
{

}

void AMG_EnemyLich::PoisonSkillAnimation()
{
	AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->Montage_Play(PoisionMontage, 1.0f);
	CurrentPoisonCoolTime = 0.0f;
}

void AMG_EnemyLich::SpawnSkillAnimation()
{
	AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->Montage_Play(SpawnMontage, 1.0f);
	CurrentSpawnCoolTime = 0.0f;
}

void AMG_EnemyLich::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		CurrentPoisonCoolTime += DeltaTime;
		CurrentSpawnCoolTime += DeltaTime;

		if (CurrentPoisonCoolTime >= MaxPoisonCoolTime)
		{
			PoisonSkill();
		}
		if (CurrentSpawnCoolTime >= MaxSpawnCoolTime)
		{
			SpawnSkill();
		}
	}
	

}

void AMG_EnemyLich::SetDead()
{
	Super::SetDead();

	FTimerHandle DeadTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DeadTimerHandle, FTimerDelegate::CreateLambda(
		[&]()
		{
			Destroy();
		}
	), DeadEventDelayTime, false);
}

void AMG_EnemyLich::AttackByAI_Implementation()
{
	PlayAttackAnimation();
}

void AMG_EnemyLich::NotifyAttackActionEnd_Implementation()
{
	Super::NotifyAttackActionEnd();
	OnAttackFinished.ExecuteIfBound();
}
