// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Components/CapsuleComponent.h"
#include "WorldBuilder.h"
#include "BlockCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class AWorldBuilder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS()
class BLOCKTASK_API ABlockCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlockCharacter();
	// WorldRef
	UPROPERTY()
	TObjectPtr<AWorldBuilder> WorldBuilderRef;
	// CharacterComponents
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerComponents")
	UCameraComponent* fps_cam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerComponents")
	UStaticMeshComponent* pickaxe_mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerComponents")
	float ray_distance = 750.0f;
	// Inputs
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess="true"))
	TObjectPtr<UInputMappingContext> IMC_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_MoveForward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_MoveSide;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Mine;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_SelectBlock1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_SelectBlock2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_SelectBlock3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_ScrollBlocks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> IA_Place;

	UPROPERTY(BlueprintAssignable)
	FOnInventoryChanged OnInventoryChanged;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(const FInputActionValue& Value);
	void MoveSide(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void StartMining(const FInputActionValue& Value);
	void HoldMining(const FInputActionValue& Value);
	void StopMining(const FInputActionValue& Value);


	bool bIsMining = false;
	FIntVector MiningTargetCell;
	float MiningProgress = 0.0f;
	float MiningDuration = 1.5f;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TMap<EBlockType, int32> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EBlockType SelectedBlockType = EBlockType::Gray;

	void SelectBlock1(const FInputActionValue& Value);
	void SelectBlock2(const FInputActionValue& Value);
	void SelectBlock3(const FInputActionValue& Value);
	void ScrollBlock(const FInputActionValue& Value);
	int HotbarIndex = 0;
	void PlaceBlock(const FInputActionValue& Value);
	
	

};
