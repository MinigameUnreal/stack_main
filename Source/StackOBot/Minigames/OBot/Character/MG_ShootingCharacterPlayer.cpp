// Fill out your copyright notice in the Description page of Project Settings.


#include "MG_ShootingCharacterPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../../../ShootingGames/Weapons/WeaponBase.h"	
#include "../../../ShootingGames/FPSHUD.h"	
#include "Net/UnrealNetwork.h"
#include "Engine/DamageEvents.h"

AMG_ShootingCharacterPlayer::AMG_ShootingCharacterPlayer()
{
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->TargetArmLength = 400.f;
	CameraArm->bUsePawnControlRotation = true;

	CameraArm->bEnableCameraLag = true;
	CameraArm->bEnableCameraRotationLag = true;
	CameraArm->CameraLagSpeed = 20.0f;
	CameraArm->CameraRotationLagSpeed = 20.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Jetpack = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Jetpack"));
	Jetpack->SetupAttachment(GetMesh(), FName("BackpackSocket"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> JetpackMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/StackOBot/Characters/Backpack/Mesh/SKM_Backpack.SKM_Backpack'"));
	if (JetpackMeshRef.Object)
	{
		Jetpack->SetSkeletalMesh(JetpackMeshRef.Object);
	}

	// Test HpBar
	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Character/Input/IA_Attack.IA_Attack'"));
	if (nullptr != InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	bCanAttack = true;

}

void AMG_ShootingCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		EnableInput(PlayerController);
		HUD = Cast<AFPSHUD>(PlayerController->GetHUD());
	}
	
}

void AMG_ShootingCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDCrossHair(DeltaTime);
}


void AMG_ShootingCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		UInputMappingContext* NewMappingContext = InputMappingContext;
		if (NewMappingContext)
		{
			Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMG_ShootingCharacterPlayer::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMG_ShootingCharacterPlayer::Look);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AMG_ShootingCharacterPlayer::OnCrouch);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AMG_ShootingCharacterPlayer::OffCrouch);
	// Test
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AMG_ShootingCharacterPlayer::Attack);

}

void AMG_ShootingCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMG_ShootingCharacterPlayer, HasWeapon);
	DOREPLIFETIME(AMG_ShootingCharacterPlayer, CurrentWeaponBase);
	// Test HpBar
	DOREPLIFETIME(AMG_ShootingCharacterPlayer, bCanAttack);
}

void AMG_ShootingCharacterPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AMG_ShootingCharacterPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(-LookAxisVector.Y);
}

void AMG_ShootingCharacterPlayer::OnCrouch(const FInputActionValue& Value)
{
	if (!GetCharacterMovement()->IsFalling())
	{
		Crouch(false);
	}
}

void AMG_ShootingCharacterPlayer::OffCrouch(const FInputActionValue& Value)
{
	UnCrouch(false);
}


void AMG_ShootingCharacterPlayer::SetHUDCrossHair(float DeltaTime)
{
	if (HUD)
	{
		FHUDPackage HUDPackage;
		//무기를 끼고 있으면 
		if (IsValid(CurrentWeaponBase))
		{

			HUDPackage.CrosshairsCenter = CurrentWeaponBase->CrosshairsCenter;
			HUDPackage.CrosshairsLeft = CurrentWeaponBase->CrosshairsLeft;
			HUDPackage.CrosshairsRight = CurrentWeaponBase->CrosshairsRight;
			HUDPackage.CrosshairsTop = CurrentWeaponBase->CrosshairsTop;
			HUDPackage.CrosshairsBottom = CurrentWeaponBase->CrosshairsBottom;
		}
		else
		{
			HUDPackage.CrosshairsCenter = nullptr;
			HUDPackage.CrosshairsLeft = nullptr;
			HUDPackage.CrosshairsRight = nullptr;
			HUDPackage.CrosshairsBottom = nullptr;
			HUDPackage.CrosshairsTop = nullptr;
		}
		//Calculate Crosshair spread
		//[0,600] -> [0,1]
		FVector2D WalkSpeedRange(0.f, GetCharacterMovement()->MaxWalkSpeed);
		FVector2D VelocityMultiplierRange(0.f, 1.f);//charactermovement를 0-1로 정규화
		FVector Velocity = GetVelocity();
		Velocity.Z = 0.f;

		CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
		//속도를 기반으로 퍼지는걸 가져오는 함수 
		HUDPackage.CrosshairSpread = CrosshairVelocityFactor;
		HUD->SetHUDPackage(HUDPackage);
	}

}

// Test
void AMG_ShootingCharacterPlayer::Attack()
{
	if (bCanAttack)
	{
		ServerAttack();
	}
}

void AMG_ShootingCharacterPlayer::AttackHitCheck()
{
	if (HasAuthority())
	{
		FHitResult HitResult;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), true, this);

		const float AttackRange = 10.0f;
		const float AttackRadius = 20.0f;
		const FVector Start = GetMesh()->GetSocketLocation(TEXT("AttackSocket")) + GetActorForwardVector();
		const FVector End = Start + GetActorForwardVector() * AttackRange;

		bool HitDetected = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel2, FCollisionShape::MakeSphere(AttackRadius), Params);

		if (HitDetected)
		{
			FDamageEvent DamageEvent;
			HitResult.GetActor()->TakeDamage(20.0f, DamageEvent, GetController(), this);
		}

#if ENABLE_DRAW_DEBUG

		FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
		float CapsuleHalfHeight = AttackRange * 0.5f;
		FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;

		DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 5.0f);

#endif
	}
}

bool AMG_ShootingCharacterPlayer::ServerAttack_Validate()
{
	return true;
}

void AMG_ShootingCharacterPlayer::ServerAttack_Implementation()
{
	MulticastAttack();
}

void AMG_ShootingCharacterPlayer::MulticastAttack_Implementation()
{
	if (HasAuthority())
	{
		bCanAttack = false;
		OnRep_CanAttack();

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]
			{
				bCanAttack = true;
				OnRep_CanAttack();
			}
		), AttackTime, false, -1.0f);
	}
	// Animation은 Server와 Client 모두 보여야함
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(ActionMontage);
}

void AMG_ShootingCharacterPlayer::OnRep_CanAttack()
{
	if (!bCanAttack)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
}

