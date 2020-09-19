// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework//SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "FirstSaveGame.h"
#include "ItemStorage.h"

// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	//设置碰撞胶囊体大小
	GetCapsuleComponent()->SetCapsuleSize(48.f, 105.f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);
	Camera->bUsePawnControlRotation = false;

	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	//控制器旋转时角色不旋转，只影响摄像机
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	//角色的移动方向为输入的方向
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.2f;

	MaxHealth = 100.f;
	Health = 100.f;
	MaxStamina = 100.f;
	Stamina = 100.f;
	Coins = 0;

	RunningSpeed = 65.f;
	SprintingSpeed = 95.f;

	bShiftKeyDown = false;
	bLMBDown = false;
	bESCDown = false;
	bAttack = false;

	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 5.f;
	MinSprintStamina = 20.f;

	InterpSpeed = 50.f;
	bInterpToEnemy = false;
	bHasCombatTarget = false;

	bMovingForward = false;
	bMovingRight = false;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();
	
	//UKismetSystemLibrary::DrawDebugSphere(this, GetActorLocation() + FVector(0, 0, 100.f), 25.f, 12, FLinearColor::Green, 5.f, .5f);

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	//LoadGameNoSwitch();
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	if (MovementStatus == EMovementStatus::EMS_Dead)
	{
		return;
	}

	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown)
		{
			if (bMovingForward || bMovingRight)
			{
				if (Stamina - DeltaStamina <= MinSprintStamina)
				{
					SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
					Stamina -= DeltaStamina;
				}
				else
				{
					Stamina -= DeltaStamina;
				}
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
		} 
		else
		{
			if (Stamina + DeltaStamina >= MaxStamina)
			{
				Stamina = MaxStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown)
		{
			if (bMovingForward || bMovingRight)
			{
				if (Stamina - DeltaStamina <= 0.f)
				{
					SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
					Stamina = 0.f;
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
				else
				{
					Stamina -= DeltaStamina;
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
			}
		}
		else
		{
			if (Stamina + DeltaStamina >= MinSprintStamina)
			{
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else
			{
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown)
		{
			Stamina = 0.f;
		}
		else
		{
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	case EStaminaStatus::ESS_ExhaustedRecovering:
		if (Stamina + DeltaStamina >= MinSprintStamina)
		{
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else
		{
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	case EStaminaStatus::ESS_MAX:
		break;
	default:
		break;
	}
	
	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);
		SetActorRotation(InterRotation);
	}

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent); 

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);

	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpRate);
}

void AMain::MoveForward(float value)
{
	bMovingForward = false;
	if (CanMove(value))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
		bMovingForward = true;
	}
}

void AMain::MoveRight(float value)
{
	bMovingRight = false;
	if (CanMove(value))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
		bMovingRight = true;
	}
}

void AMain::Turn(float value)
{
	if (CanMove(value))
	{
		AddControllerYawInput(value);
	}
}

void AMain::LookUp(float value)
{
	if (CanMove(value))
	{
		AddControllerPitchInput(value);
	}
}

bool AMain::CanMove(float value)
{
	if (MainPlayerController)
	{
		return (Controller != nullptr) && (value != 0.f) && !bAttack && (MovementStatus != EMovementStatus::EMS_Dead) && (!MainPlayerController->bPauseMenuVisible);
	}
	return false;
}

void AMain::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LookUpRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMain::DecrementHealth(float Amount)
{
	if (Health - Amount <= 0.f)
	{
		Health -= Amount;
		Die();
	}
	else
	{
		Health -= Amount;
	}
}

void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead )
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMain::Jump()
{
	if (MainPlayerController)
	{
		if (MainPlayerController->bPauseMenuVisible)
		{
			return;
		}
	}

	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		Super::Jump();
	}
}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMain::IncreamentCoins(int32 Amount)
{
	Coins += Amount;
}

void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotatinYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotatinYaw;
}

void AMain::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMain::ShowPickUpLocation()
{
	//for (int32 i = 0 ; i < PickUpLocation.Num(); i++)
	//{
		//UKismetSystemLibrary::DrawDebugSphere(this, PickUpLocation[i], 25.f, 12, FLinearColor::Green, 5.f, .5f);
	//}
	
	for (auto Location : PickUpLocation)
	{
		UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 12, FLinearColor::Green, 5.f, .5f);
	}
}

void AMain::LMBDown()
{
	bLMBDown = true;

	if (MainPlayerController)
	{
		if (MainPlayerController->bPauseMenuVisible)
		{
			return;
		}
	}

	if (MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (ActiveOverlappingItem)
		{
			AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
			if (Weapon)
			{
				Weapon->Equip(this);
				SetActiveOverlappingItem(nullptr);
			}
		}
		else if (EquipedWeapon)
		{
			Attack();
		}
	}
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::ESCDown()
{
	bESCDown = true;
	if (MainPlayerController)
	{
		MainPlayerController->TogglePauseMenu();
	}
}

void AMain::ESCUp()
{
	bESCDown = false;
}

void AMain::SetEquipedWeapon(AWeapon* WeaponToSet)
{
	if (EquipedWeapon)
	{
		EquipedWeapon->Destroy();
	}
	EquipedWeapon = WeaponToSet;
}

void AMain::Attack()
{
	if (!bAttack && (MovementStatus != EMovementStatus::EMS_Dead))
	{
		bAttack = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			int32 Section = FMath::RandRange(0, 1);
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 2.2f);
				AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.8f);
				AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
				break;
			default:
				break;
			}
		}
	}
}

void AMain::AttackEnd()
{
	bAttack = false;
	SetInterpToEnemy(false);
	if (bLMBDown)
	{
		Attack();
	}
}

void AMain::PlaySwingSound()
{
	if (EquipedWeapon->SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, EquipedWeapon->SwingSound);
	}
}

float AMain::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;
		Die();
		if (DamageCauser)
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			Enemy->bHasVaildTarget = false;
		}
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AMain::UpadateCombatTarget()
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);
	if (OverlappingActors.Num() == 0)
	{
		if (MainPlayerController)
		{
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	AEnemy* CloestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if (CloestEnemy)
	{
		FVector Location = GetActorLocation();
		float MinDistance = (CloestEnemy->GetActorLocation() - Location).Size();

		for (auto Actor : OverlappingActors)
		{
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy)
			{
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				if (DistanceToActor < MinDistance)
				{
					MinDistance = DistanceToActor;
					CloestEnemy = Enemy;
				}
			}	
		}

		if (MainPlayerController)
		{
			MainPlayerController->DisplayEnenmyHealthBar();
		}
		SetCombatTarget(CloestEnemy);
		bHasCombatTarget = true;
	}	
}

void AMain::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();
		FName CurrentLevelName(*CurrentLevel);
		if (CurrentLevelName != LevelName)
		{
			UGameplayStatics::OpenLevel(World, LevelName);
		}
	}
}

void AMain::SaveGame()
{
	UFirstSaveGame* SaveGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	if (SaveGameInstance)
	{
		if (EquipedWeapon)
		{
			SaveGameInstance->CharacterStatus.WeaponName = EquipedWeapon->WeaponName;
		}

		SaveGameInstance->CharacterStatus.Health = Health;
		SaveGameInstance->CharacterStatus.MaxHealth = MaxHealth;
		SaveGameInstance->CharacterStatus.Stamina = Stamina;
		SaveGameInstance->CharacterStatus.MaxStamina = MaxStamina;
		SaveGameInstance->CharacterStatus.Coins = Coins;
		SaveGameInstance->CharacterStatus.Location = GetActorLocation();
		SaveGameInstance->CharacterStatus.Rotation = GetActorRotation();

		FString MapName = GetWorld()->GetMapName();
		MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
		SaveGameInstance->CharacterStatus.LevelName = MapName;

		UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
	}
}

void AMain::LoadGame(bool bSetPosition)
{
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));

	if (LoadGameInstance)
	{
		LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

		Health = LoadGameInstance->CharacterStatus.Health;
		MaxHealth = LoadGameInstance->CharacterStatus.MaxHealth;
		Stamina = LoadGameInstance->CharacterStatus.Stamina;
		MaxStamina = LoadGameInstance->CharacterStatus.MaxStamina;
		Coins = LoadGameInstance->CharacterStatus.Coins;

		if (LoadGameInstance->CharacterStatus.LevelName != TEXT(""))
		{
			FString MapName = GetWorld()->GetMapName();
			MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
			
			if (LoadGameInstance->CharacterStatus.LevelName != MapName)
			{
				FName LevelName(*LoadGameInstance->CharacterStatus.LevelName);
				SwitchLevel(LevelName);
			}
		}

		if (bSetPosition)
		{
			SetActorLocation(LoadGameInstance->CharacterStatus.Location);
			SetActorRotation(LoadGameInstance->CharacterStatus.Rotation);
		}

		if (WeaponStorage)
		{
			AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
			if (Weapons)
			{
				FString WeaponName = LoadGameInstance->CharacterStatus.WeaponName;
				if (Weapons->WeaponMap.Contains(WeaponName))
				{
					AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
					WeaponToEquip->Equip(this);
				}
			}
		}

		SetMovementStatus(EMovementStatus::EMS_Normal);
		GetMesh()->bPauseAnims = false;
		GetMesh()->bNoSkeletonUpdate = false;
	}
}

void AMain::LoadGameNoSwitch()
{
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));

	if (LoadGameInstance)
	{
		LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

		Health = LoadGameInstance->CharacterStatus.Health;
		MaxHealth = LoadGameInstance->CharacterStatus.MaxHealth;
		Stamina = LoadGameInstance->CharacterStatus.Stamina;
		MaxStamina = LoadGameInstance->CharacterStatus.MaxStamina;
		Coins = LoadGameInstance->CharacterStatus.Coins;

		if (WeaponStorage)
		{
			AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
			if (Weapons)
			{
				FString WeaponName = LoadGameInstance->CharacterStatus.WeaponName;
				if (Weapons->WeaponMap.Contains(WeaponName))
				{
					AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
					WeaponToEquip->Equip(this);
				}
			}
		}

		SetMovementStatus(EMovementStatus::EMS_Normal);
		GetMesh()->bPauseAnims = false;
		GetMesh()->bNoSkeletonUpdate = false;
	}
}
