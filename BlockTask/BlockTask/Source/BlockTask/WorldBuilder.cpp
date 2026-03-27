// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldBuilder.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
// Sets default values
AWorldBuilder::AWorldBuilder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
}


FIntPoint AWorldBuilder::GetPlayerChunkCoord()
{
	if (!CachedPlayer)
	{
		return FIntPoint(0, 0);
	}
	FVector WorldPos = CachedPlayer->GetActorLocation();
	int32 ChunkX = FMath::FloorToInt(WorldPos.X / (ChunkSize * BlockSize));
	int32 ChunkY = FMath::FloorToInt(WorldPos.Y / (ChunkSize * BlockSize));
	return FIntPoint(ChunkX, ChunkY);
}

void AWorldBuilder::GenerateChunk(FIntPoint ChunkCoord)
{
	//seed offsets
	const float SeedOffsetX = Seed * 0.00137f;
	const float SeedOffsetY = Seed * 0.00211f;
	//height
	const int32 RangeZ = FMath::Max(1, MaxZ - MinZ);
	const int32 GrayCutZ = MinZ + FMath::RoundToInt(RangeZ * 0.4f);
	const int32 WhiteCutZ = MinZ + FMath::RoundToInt(RangeZ * 0.6f);
	//width+depth
	const int32 StartX = ChunkCoord.X * ChunkSize;
	const int32 StartY = ChunkCoord.Y * ChunkSize;
	//Stone
	UHierarchicalInstancedStaticMeshComponent* GrayHISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
	GrayHISM->SetupAttachment(RootComponent);
	GrayHISM->RegisterComponent();
	GrayHISM->SetStaticMesh(GrayMesh);
	GrayHISM->SetCollisionObjectType(ECC_WorldStatic);
	GrayHISM->SetCollisionResponseToAllChannels(ECR_Ignore);
	GrayHISM->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GrayHISM->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	GrayHISM->SetGenerateOverlapEvents(false);
	//Grass
	UHierarchicalInstancedStaticMeshComponent* GreenHISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);

	GreenHISM->SetupAttachment(RootComponent);
	GreenHISM->RegisterComponent();
	GreenHISM->SetStaticMesh(GreenMesh);
	GreenHISM->SetCollisionObjectType(ECC_WorldStatic);
	GreenHISM->SetCollisionResponseToAllChannels(ECR_Ignore);
	GreenHISM->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GreenHISM->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	GreenHISM->SetGenerateOverlapEvents(false);
	//Snow
	UHierarchicalInstancedStaticMeshComponent* WhiteHISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
	WhiteHISM->SetupAttachment(RootComponent);
	WhiteHISM->RegisterComponent();
	WhiteHISM->SetStaticMesh(WhiteMesh);
	WhiteHISM->SetCollisionObjectType(ECC_WorldStatic);
	WhiteHISM->SetCollisionResponseToAllChannels(ECR_Ignore);
	WhiteHISM->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	WhiteHISM->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	WhiteHISM->SetGenerateOverlapEvents(false);

	UHierarchicalInstancedStaticMeshComponent* BedrockHISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
	BedrockHISM->SetupAttachment(RootComponent);
	BedrockHISM->RegisterComponent();
	BedrockHISM->SetStaticMesh(BedrockMesh);
	BedrockHISM->SetCollisionObjectType(ECC_WorldStatic);
	BedrockHISM->SetCollisionResponseToAllChannels(ECR_Ignore);
	BedrockHISM->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	BedrockHISM->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BedrockHISM->SetGenerateOverlapEvents(false);

	// 2d loop to loop thru columns in chunks
	for (int32 X = StartX; X < StartX + ChunkSize; ++X)
	{
		for (int32 Y = StartY; Y < StartY + ChunkSize; ++Y)
		{
			
			const float Noise = FMath::PerlinNoise2D(
				FVector2D((X + SeedOffsetX) * NoiseScale, (Y + SeedOffsetY) * NoiseScale));

			const float Normalized = (Noise + 1.0f) * 0.5f;

			const int32 ColumnTopZ = FMath::Clamp(MinZ + FMath::RoundToInt(Normalized * (MaxZ - MinZ)), MinZ, MaxZ);


			for (int32 Z = MinZ-1; Z <= ColumnTopZ; ++Z)
			{
				const FVector WorldPos(
					X * BlockSize,
					Y * BlockSize,
					Z * BlockSize
				);


				const FTransform T(FRotator::ZeroRotator, WorldPos, FVector(1.0f));
				
				if (Z == MinZ - 1)
				{
					BedrockHISM->AddInstance(T);
				}

				else if (Z>MinZ-1 &&  Z<= GrayCutZ)
				{
					GrayHISM->AddInstance(T);
					BlockData.Add(FIntVector(X, Y, Z), EBlockType::Gray);
				}
				else if (Z >= WhiteCutZ)
				{
					WhiteHISM->AddInstance(T);
					BlockData.Add(FIntVector(X, Y, Z), EBlockType::White);
				}
				else
				{
					GreenHISM->AddInstance(T);
					BlockData.Add(FIntVector(X, Y, Z), EBlockType::Green);
				}
			}
		}
	}

	FChunkData& Data = Chunks.Add(ChunkCoord);
	Data.GrayHISM = GrayHISM;
	Data.GreenHISM = GreenHISM;
	Data.WhiteHISM = WhiteHISM;
	Data.BedrockHISM = BedrockHISM;
	Data.bIsLoaded = true;
	

}

void AWorldBuilder::UnloadChunk(FIntPoint ChunkCoord)
{
	FChunkData* Data = Chunks.Find(ChunkCoord);

	if (!Data) return;

	const int32 StartX = ChunkCoord.X * ChunkSize;
	const int32 StartY = ChunkCoord.Y * ChunkSize;

	for (int32 X = StartX; X < StartX + ChunkSize; ++X)
	{
		for (int32 Y = StartY; Y < StartY + ChunkSize; ++Y)
		{
			for (int32 Z = MinZ; Z <= MaxZ; ++Z)
			{
				BlockData.Remove(FIntVector(X, Y, Z));
			}
		}
	}

	if (Data->GrayHISM) Data->GrayHISM->DestroyComponent();
	if (Data->GreenHISM) Data->GreenHISM->DestroyComponent();
	if (Data->WhiteHISM) Data->WhiteHISM->DestroyComponent();
	if (Data->BedrockHISM) Data->BedrockHISM->DestroyComponent();
	Chunks.Remove(ChunkCoord);
	
}





void AWorldBuilder::UpdateChunks(FIntPoint PlayerChunkCoord)
{
	TArray<FIntPoint> ChunksToLoad;

	// Determine which chunks should be in queue 
	for (int32 X = PlayerChunkCoord.X - LoadRadius; X <= PlayerChunkCoord.X + LoadRadius; X++)
	{
		for (int32 Y = PlayerChunkCoord.Y - LoadRadius; Y <= PlayerChunkCoord.Y + LoadRadius; Y++)
		{
			FIntPoint ChunkCoord(X, Y);

			if ((!Chunks.Contains(ChunkCoord) || !Chunks[ChunkCoord].bIsLoaded) && !ChunkLoadQueue.Contains(ChunkCoord))
			{
				ChunkLoadQueue.Add(ChunkCoord);
			}
		}
	}
}

// Called when the game starts or when spawned
void AWorldBuilder::BeginPlay()
{

	Super::BeginPlay();

	CachedPlayer = GetWorld()->GetFirstPlayerController()->GetPawn();

	if (Seed == 0)
	{
		Seed = FMath::Rand();
	}
}

// Called every frame
void AWorldBuilder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	FIntPoint PlayerChunkCoord = GetPlayerChunkCoord();

	UpdateChunks(PlayerChunkCoord);
	
	//load next chunk from queue
	if (ChunkLoadQueue.Num() > 0)
	{
		FIntPoint NextChunk = ChunkLoadQueue[0];
		ChunkLoadQueue.RemoveAt(0);
		GenerateChunk(NextChunk);
	}

	//unload unused chunk
	TArray<FIntPoint> ChunksToUnload;
	for (auto& Pair : Chunks)
	{
		if (FMath::Abs(Pair.Key.X - PlayerChunkCoord.X) > LoadRadius || FMath::Abs(Pair.Key.Y - PlayerChunkCoord.Y) > LoadRadius)
		{
			ChunksToUnload.Add(Pair.Key);
		}
	}
	for (const FIntPoint& Coord : ChunksToUnload)
	{
		UnloadChunk(Coord);
	}
}

void AWorldBuilder::RemoveBlockAt(FIntVector Cell)
{
	EBlockType* Type = BlockData.Find(Cell);

	if (!Type) return;

	FIntPoint ChunkCoord(
		FMath::FloorToInt((float)Cell.X / ChunkSize),
		FMath::FloorToInt((float)Cell.Y / ChunkSize));

	FChunkData* Chunk = Chunks.Find(ChunkCoord);

	if (!Chunk) return;

	UHierarchicalInstancedStaticMeshComponent* TargetHISM = nullptr;

	switch (*Type)
	{
	case EBlockType::Gray: TargetHISM = Chunk->GrayHISM; break;
	case EBlockType::Green: TargetHISM = Chunk->GreenHISM; break;
	case EBlockType::White: TargetHISM = Chunk->WhiteHISM; break;
	default: return;
	}

	FVector WorldPos(
		Cell.X * BlockSize,
		Cell.Y * BlockSize,
		Cell.Z * BlockSize
	);


	for (int32 i = 0; i < TargetHISM->GetInstanceCount(); ++i)
	{
		FTransform InstanceTransform;
		TargetHISM->GetInstanceTransform(i, InstanceTransform, true); 
		// reads the transform of instance number i in target hism and stores it in InstanceTransform ^^
		if (FVector::Dist(InstanceTransform.GetLocation(), WorldPos) < 1.0f)
		{
			TargetHISM->RemoveInstance(i);
			break;
		}
	}

	BlockData.Remove(Cell);
}

bool AWorldBuilder::AddBlockAt(FIntVector Cell, EBlockType SelectedBlockType)
{
	if (BlockData.Contains(Cell)) return false;
	// Ceiling limit
	if (Cell.Z >= MaxZ) return false;
	// Deep limit
	if (Cell.Z < MinZ) return false;

	
	FIntPoint ChunkCoord(
		FMath::FloorToInt((float)Cell.X / ChunkSize),
		FMath::FloorToInt((float)Cell.Y / ChunkSize)
	);

	FChunkData* Chunk = Chunks.Find(ChunkCoord);
	if (!Chunk) return false;

	UHierarchicalInstancedStaticMeshComponent* TargetHISM = nullptr;

	switch (SelectedBlockType)
	{
	case EBlockType::Gray:  TargetHISM = Chunk->GrayHISM;  break;
	case EBlockType::Green: TargetHISM = Chunk->GreenHISM; break;
	case EBlockType::White: TargetHISM = Chunk->WhiteHISM; break;
	default: return false;
	}

	if (!TargetHISM) return false;

	FVector WorldPos(
		Cell.X * BlockSize,
		Cell.Y * BlockSize,
		Cell.Z * BlockSize
	);

	FTransform T(FRotator::ZeroRotator, WorldPos, FVector(1.0f));

	TargetHISM->AddInstance(T);

	BlockData.Add(Cell, SelectedBlockType);

	return true;
}

