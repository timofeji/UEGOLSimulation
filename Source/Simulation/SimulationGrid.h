// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulationGrid.generated.h"

struct SIMULATION_API FGrid2D
{

	uint32 SizeY;
	uint32 SizeX;
	TArray<TArray<char>> GridData;

	//Initializes empty values to appropriate grid size
	void Init(uint32 X, uint32 Y )
	{
		SizeX = X; SizeY = Y;
		
		GridData.SetNum(X);
		for(uint32 i  = 0; i < SizeX; i++)
		{
			GridData[i].SetNum(Y);
			for(uint32 j  = 0; j < SizeY; j++)
			{
				GridData[i][j] = 0;
			}
		}
	}

	//Clears grid
	void Clear()
	{
		for(uint32 i  = 0; i < SizeX; i++)
		{
			GridData[i].Init(false, SizeY);
		}
	}
	
	//Clears grid using random values
	void RandClear()
	{
		for(uint32 i  = 0; i < SizeX; i++)
		{
			for(uint32 j  = 0; j < SizeY; j++)
			{
				GridData[i][j] = FMath::RandBool();
			}
		}
	}
	
	//Getter method with index wrapping
	char operator() (uint32 X, uint32 Y)
	{
		return GridData[X %= SizeX][Y %= SizeY];
	}

	//Setter method with index wrapping
	void Set(uint32 X, uint32 Y, char Value)
	{
		GridData[X %= SizeX][Y %= SizeY] = Value;
	}
};

UCLASS()
class SIMULATION_API ASimulationGrid : public AActor
{
	GENERATED_BODY()
	
public:

	ASimulationGrid();
	
	UPROPERTY(EditAnywhere, Category = "Simulation")
	UStaticMesh* GridCellStaticMesh;
	
	UPROPERTY(EditAnywhere, Category = "Simulation")
	UStaticMeshComponent* GridSelectorMesh;

	
	UPROPERTY(EditAnywhere, Category = "Simulation")
	FVector2D GridShape = FVector2D(50,50);
	
	UPROPERTY(EditAnywhere, Category = "Simulation")
	float SimulationStepTime = 1.f;

	//Grid of 'Alive' values
	FGrid2D Grid;
	FVector2D SelectedBlock;
	void ToggleSimulation();

	FVector GridUnitToWorldSpacePosition(const FVector2D& Unit);
	FVector2D WorldSpacePositionToGridUnit(const FVector& Position);

	
protected:
	bool bIsSimulating = false;

	APlayerController* PlayerController;
	void PlaceBlock();
	void RemoveBlock();

	
	void RenderGrid();
	void StepSimulation();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	FTimerHandle SimStepTimeHandle;

	UInstancedStaticMeshComponent* ISMComp;

	TArray<FVector2D> CellsToRender;

	FVector MeshBounds;
	
};

