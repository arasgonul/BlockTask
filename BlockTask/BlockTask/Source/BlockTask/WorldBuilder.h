// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldBuilder.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;

USTRUCT()
struct FChunkData
{
	GENERATED_BODY()

	UHierarchicalInstancedStaticMeshComponent* GrayHISM = nullptr;
	UHierarchicalInstancedStaticMeshComponent* GreenHISM = nullptr;
	UHierarchicalInstancedStaticMeshComponent* WhiteHISM = nullptr;
	UHierarchicalInstancedStaticMeshComponent* BedrockHISM = nullptr;
	bool bIsLoaded = false;
};

UENUM(BlueprintType)
enum class EBlockType : uint8
{
	Empty,White,Green,Gray
};
UCLASS()
class BLOCKTASK_API AWorldBuilder : public AActor
{
	GENERATED_BODY()
	
public:	

	AWorldBuilder();


	
	//World
	
	UPROPERTY(EditAnywhere, Category = "World")
	int32 ChunkSize = 32;

	UPROPERTY(EditAnywhere, Category = "World")
	int32 LoadRadius = 2;

	TMap<FIntPoint, FChunkData> Chunks;

	TMap<FIntVector, EBlockType> BlockData;

	TArray<FIntPoint> ChunkLoadQueue;



	UFUNCTION(BlueprintCallable)
	FIntPoint GetPlayerChunkCoord();

	UFUNCTION(BlueprintCallable)
	void GenerateChunk(FIntPoint ChunkCoord);

	UFUNCTION(BlueprintCallable)
	void UnloadChunk(FIntPoint ChunkCoord);

	UFUNCTION(BlueprintCallable)
	void UpdateChunks(FIntPoint PlayerChunkCoord);

	UPROPERTY()
	TObjectPtr<APawn> CachedPlayer;

	UPROPERTY(EditAnywhere, Category = "World")
	int32 MinZ = 0;
protected:

	virtual void BeginPlay() override;



	UPROPERTY(EditAnywhere, Category = "World")
	float BlockSize = 256.0f;


	

	UPROPERTY(EditAnywhere, Category = "World")
	int32 MaxZ = 12;

	UPROPERTY(EditAnywhere, Category = "Noise")
	float NoiseScale = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Noise")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TObjectPtr<UStaticMesh> GrayMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TObjectPtr<UStaticMesh> GreenMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TObjectPtr<UStaticMesh> WhiteMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TObjectPtr<UStaticMesh> BedrockMesh;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void RemoveBlockAt(FIntVector Cell);
	bool AddBlockAt(FIntVector Cell,EBlockType SelectedBlockType);
};
