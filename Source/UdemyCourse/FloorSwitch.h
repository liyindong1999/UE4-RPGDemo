// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorSwitch.generated.h"

UCLASS()
class UDEMYCOURSE_API AFloorSwitch : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloorSwitch();


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = FloorSwitch)
	class UBoxComponent* TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = FloorSwitch)
	class UStaticMeshComponent* FloorSwitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = FloorSwitch)
	UStaticMeshComponent* Door;

	//门初始化位置
	UPROPERTY(BlueprintReadWrite, Category = FloorSwitch)
	FVector InitialDoorLocation;

	//开关初始化位置
	UPROPERTY(BlueprintReadWrite, Category = FloorSwitch)
	FVector InitialSwitchLocation;

	FTimerHandle SwitchHandle;

	UPROPERTY(EditAnywhere, Category = FloorSwitch)
	float SwitchTime;

	bool bCharacterOnSwitch;

	void CloseDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	//BlueprintImplementableEvent可以使实现在蓝图不在C++中
	UFUNCTION(BlueprintImplementableEvent, Category = FloorSwitch)
	void RaiseDoor();

	UFUNCTION(BlueprintImplementableEvent, Category = FloorSwitch)
	void LowerDoor();

	UFUNCTION(BlueprintImplementableEvent, Category = FloorSwitch)
	void RaiseFloorSwitch();

	UFUNCTION(BlueprintImplementableEvent, Category = FloorSwitch)
	void LowerFloorSwitch();

	UFUNCTION(BlueprintCallable, Category = FloorSwitch)
	void UpdateDoorLocation(float Z);

	UFUNCTION(BlueprintCallable, Category = FloorSwitch)
	void UpdateFloorSwitchLocation(float Z);
};
