// Fill out your copyright notice in the Description page of Project Settings.


#include "BlockCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WorldBuilder.h"
#include "Kismet/GameplayStatics.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "GameFramework/PlayerController.h"


// Sets default values
ABlockCharacter::ABlockCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	fps_cam = CreateDefaultSubobject<UCameraComponent>(TEXT("FPS_Cam"));
	pickaxe_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickaxe"));
	fps_cam->SetupAttachment(GetCapsuleComponent());
	pickaxe_mesh->SetupAttachment(fps_cam);

	fps_cam->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}



// Called when the game starts or when spawned
void ABlockCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =  ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_Default, 0);
		}
	}
	WorldBuilderRef = Cast<AWorldBuilder>(UGameplayStatics::GetActorOfClass(GetWorld(), AWorldBuilder::StaticClass()));
	
}

// Called every frame
void ABlockCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABlockCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &ABlockCharacter::MoveForward);
		EnhancedInputComponent->BindAction(IA_MoveSide, ETriggerEvent::Triggered, this, &ABlockCharacter::MoveSide);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ABlockCharacter::Look);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ABlockCharacter::Jump);
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ABlockCharacter::StopJumping);
		EnhancedInputComponent->BindAction(IA_Mine, ETriggerEvent::Started, this, &ABlockCharacter::StartMining);
		EnhancedInputComponent->BindAction(IA_Mine, ETriggerEvent::Triggered, this, &ABlockCharacter::HoldMining);
		EnhancedInputComponent->BindAction(IA_Mine, ETriggerEvent::Completed, this, &ABlockCharacter::StopMining);
		EnhancedInputComponent->BindAction(IA_SelectBlock1, ETriggerEvent::Started, this, &ABlockCharacter::SelectBlock1);
		EnhancedInputComponent->BindAction(IA_SelectBlock2, ETriggerEvent::Started, this, &ABlockCharacter::SelectBlock2);
		EnhancedInputComponent->BindAction(IA_SelectBlock3, ETriggerEvent::Started, this, &ABlockCharacter::SelectBlock3);
		EnhancedInputComponent->BindAction(IA_ScrollBlocks, ETriggerEvent::Triggered, this, &ABlockCharacter::ScrollBlock);
		EnhancedInputComponent->BindAction(IA_Place, ETriggerEvent::Started, this, & ABlockCharacter::PlaceBlock);
		

	}
	
}

void ABlockCharacter::MoveForward(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();

	if (FMath::IsNearlyZero(Axis)) return;

	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(ForwardDirection, Axis);
}

void ABlockCharacter::MoveSide(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();

	if (FMath::IsNearlyZero(Axis)) return;

	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(RightDirection, Axis);
}

void ABlockCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(-LookAxis.Y);
}

void ABlockCharacter::StartMining(const FInputActionValue& Value)
{
	bIsMining = true;
	float BlockSize = 256.0f;
	FHitResult HitResult;
	FVector Start = fps_cam->GetComponentLocation();
	FVector End = Start + fps_cam->GetForwardVector() * ray_distance;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, Start, End, ECC_WorldStatic
	);

	if (!bHit) return;

	FVector BlockPosition = HitResult.Location - HitResult.Normal * (BlockSize * 0.5f);

	FIntVector Cell(
		FMath::FloorToInt(BlockPosition.X / BlockSize),
		FMath::FloorToInt(BlockPosition.Y / BlockSize),
		FMath::FloorToInt(BlockPosition.Z / BlockSize)
	);

	MiningTargetCell = Cell;

	EBlockType* FoundBlockType = WorldBuilderRef->BlockData.Find(MiningTargetCell);
	if (!FoundBlockType) return;

	EBlockType BlockType = *FoundBlockType;

	switch (BlockType)
	{
	case EBlockType::Empty:
		break;
	case EBlockType::White:
		MiningDuration = 0.5f; break;
	case EBlockType::Green:
		MiningDuration = 1.5f; break;
	case EBlockType::Gray:
		MiningDuration = 3.0f; break;
	default:
		return;
	}
}

void ABlockCharacter::HoldMining(const FInputActionValue& Value)
{

	FHitResult HitResult;
	FVector Start = fps_cam->GetComponentLocation();
	FVector End = Start + fps_cam->GetForwardVector() * ray_distance;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, Start, End, ECC_WorldStatic
	);
	
	if (!bHit)
	{
		bIsMining = false;
		MiningProgress = 0.0f;
		return;
	}

	float BlockSize = 256.0f;
	FIntVector Cell(0, 0, 0);
	if (UHierarchicalInstancedStaticMeshComponent* HISM = Cast<UHierarchicalInstancedStaticMeshComponent>(HitResult.Component.Get()))
	{
		const int32 Index = HitResult.Item;
		
		if (Index >= 0 && Index < HISM->GetInstanceCount())
		{
			FTransform T;
			if (HISM->GetInstanceTransform(Index, T, true))
			{
				const FVector P = T.GetLocation();
				Cell=FIntVector(
					FMath::FloorToInt(P.X / BlockSize),
					FMath::FloorToInt(P.Y / BlockSize),
					FMath::FloorToInt(P.Z / BlockSize)
				);
			}
		}
	}

	


	if (Cell != MiningTargetCell)
	{
		MiningProgress = 0.0f;
		MiningTargetCell = Cell;
		return;
	}
	EBlockType* FoundBlockType = WorldBuilderRef->BlockData.Find(MiningTargetCell);
	if (!FoundBlockType)
	{
		return;
	}

	EBlockType BlockType = *FoundBlockType;

	switch (BlockType)
	{
	case EBlockType::Empty:
		break;
	case EBlockType::White:
		MiningDuration = 0.5f; break;
	case EBlockType::Green:
		MiningDuration = 1.5f; break;
	case EBlockType::Gray:
		MiningDuration = 3.0f; break;
	default:
		return;
	}


	if (Cell.Z < WorldBuilderRef->MinZ)
	{
		return;
	}

	MiningProgress += GetWorld()->GetDeltaSeconds();

	if (MiningProgress >= MiningDuration)
	{
		if (WorldBuilderRef)
		{

			
			if (Inventory.Contains(BlockType))
			{
				Inventory[BlockType]++;
			}
			else
			{
				Inventory.Add(BlockType, 1);
			}

			WorldBuilderRef->RemoveBlockAt(Cell);

		}
		//add to inventory data
		bIsMining = false;
		MiningProgress = 0.0f;
		OnInventoryChanged.Broadcast();
	}
}

void ABlockCharacter::StopMining(const FInputActionValue& Value)
{
	bIsMining = false;
	MiningProgress = 0.0f;
}

void ABlockCharacter::SelectBlock1(const FInputActionValue& Value)
{
	HotbarIndex = 0;
	SelectedBlockType = EBlockType::Gray;
	OnInventoryChanged.Broadcast();
}

void ABlockCharacter::SelectBlock2(const FInputActionValue& Value)
{
	HotbarIndex = 1;
	SelectedBlockType = EBlockType::Green;
	OnInventoryChanged.Broadcast();
}

void ABlockCharacter::SelectBlock3(const FInputActionValue& Value)
{
	HotbarIndex = 2;
	SelectedBlockType = EBlockType::White;
	OnInventoryChanged.Broadcast();
}

void ABlockCharacter::ScrollBlock(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis)) return;

	const int32 NumSlots = 3;

	const int32 Delta = (Axis > 0.0f) ? 1 : -1; //favorite operator ? x:y

	HotbarIndex = ((HotbarIndex + Delta) % NumSlots + NumSlots) % NumSlots;

	switch (HotbarIndex)
	{
		case 0: SelectedBlockType = EBlockType::Gray; break;
		case 1: SelectedBlockType = EBlockType::Green; break;
		case 2: SelectedBlockType = EBlockType::White; break;
	default:break;
	}

	OnInventoryChanged.Broadcast();
}

void ABlockCharacter::PlaceBlock(const FInputActionValue& Value)
{
	if (!WorldBuilderRef) return;

	int32* Count = Inventory.Find(SelectedBlockType);
	if (!Count || *Count <= 0) return;

	float BlockSize = 256.0f;
	FHitResult HitResult;
	FVector Start = fps_cam->GetComponentLocation();
	FVector End = Start + fps_cam->GetForwardVector() * ray_distance;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, Start, End, ECC_WorldStatic
	);

	if (!bHit) return;

	FIntVector HitCell(0, 0, 0);
	bool HaveHitCell = false;

	if (UHierarchicalInstancedStaticMeshComponent* HISM = Cast<UHierarchicalInstancedStaticMeshComponent>(HitResult.Component.Get()))
	{
		const int32 Index = HitResult.Item;
		if (Index >= 0 && HISM->GetInstanceCount())
		{
			FTransform T;
			if (HISM->GetInstanceTransform(Index, T, true))
			{
				const FVector P = T.GetLocation();
				HitCell = FIntVector(
					FMath::FloorToInt(P.X / BlockSize),
					FMath::FloorToInt(P.Y / BlockSize),
					FMath::FloorToInt(P.Z / BlockSize));
				HaveHitCell = true;
			}
		}
	}
	FIntVector PlaceCell;

	if (HaveHitCell)
	{
		const FVector N = HitResult.Normal.GetSafeNormal();
		FIntVector Step(0, 0, 0);
		const float ax = FMath::Abs(N.X);
		const float ay = FMath::Abs(N.Y);
		const float az = FMath::Abs(N.Z);

		if (ax >= ay && ax >= az)
		{
			Step.X = (N.X > 0.0f) ? 1 : -1;
		}
		else if (ay >= ax && ay >= az)
		{
			Step.Y = (N.Y > 0.0f) ? 1 : -1;
		}
		else
		{
			Step.Z = (N.Z > 0.0f) ? 1 : -1;
		}
		PlaceCell = HitCell + Step;
	}
	else
	{
		const FVector PlacePosition = HitResult.Location + HitResult.Normal * (BlockSize * 0.5f);
		PlaceCell = FIntVector(
			FMath::FloorToInt(PlacePosition.X / BlockSize),
			FMath::FloorToInt(PlacePosition.Y / BlockSize),
			FMath::FloorToInt(PlacePosition.Z / BlockSize)
		);
	}
	//check if player is in bounds of block to be placed, if so, return.

	

	if (WorldBuilderRef->AddBlockAt(PlaceCell,SelectedBlockType))
	{
		(*Count)--;
		OnInventoryChanged.Broadcast();
	}

}

