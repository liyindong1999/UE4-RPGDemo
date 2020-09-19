// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "Enemy.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			Enemy = Cast<AEnemy>(Pawn);
		}
	}

	bCanMoveAfterAttack = false;
}

void UEnemyAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			Enemy = Cast<AEnemy>(Pawn);
		}
	}
	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity();
		FVector LaterSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LaterSpeed.Size();
	}
}