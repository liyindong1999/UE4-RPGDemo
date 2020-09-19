// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"


void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDOverlayAsset)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
	}
	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);

	if (WEnenmyHealthBar)
	{
		EnenmyHealthBar = CreateWidget<UUserWidget>(this, WEnenmyHealthBar);
		if (EnenmyHealthBar)
		{
			EnenmyHealthBar->AddToViewport();
			EnenmyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		}
		FVector2D Alignment(0, 0);
		EnenmyHealthBar->SetAlignmentInViewport(Alignment);
	}

	if (WPauseMenu)
	{
		PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
			PauseMenu->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnenmyHealthBar)
	{
		FVector2D PositionInViewport;
		ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
		PositionInViewport.Y -= 100.f;

		FVector2D SizeInViewPort = FVector2D(200.f, 25.f);

		EnenmyHealthBar->SetPositionInViewport(PositionInViewport);
		EnenmyHealthBar->SetDesiredSizeInViewport(SizeInViewPort);
	}
}

void AMainPlayerController::DisplayPauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = true;
		PauseMenu->SetVisibility(ESlateVisibility::Visible);
		
		FInputModeGameAndUI InputModeGameAndUI;

		SetInputMode(InputModeGameAndUI);
		bShowMouseCursor = true;
	}
}

void AMainPlayerController::RemovePauseMenu_Implementation()
{
	if (PauseMenu)
	{
		bPauseMenuVisible = false;
		PauseMenu->SetVisibility(ESlateVisibility::Hidden);

		FInputModeGameOnly InputModeGameOnly;

		SetInputMode(InputModeGameOnly);
		bShowMouseCursor = false;
	}
}

void AMainPlayerController::TogglePauseMenu()
{
	if (bPauseMenuVisible)
	{
		RemovePauseMenu();
	}
	else
	{
		DisplayPauseMenu();
	}
}

void AMainPlayerController::DisplayEnenmyHealthBar()
{
	if (EnenmyHealthBar)
	{
		bEnemyHealthBarVisible = true;
		EnenmyHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
}

void AMainPlayerController::RemoveEnemyHealthBar()
{
	if (EnenmyHealthBar)
	{
		bEnemyHealthBarVisible = false;
		EnenmyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
}
