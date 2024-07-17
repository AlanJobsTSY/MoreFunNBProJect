// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>
#include "AStarWorld.generated.h"
/*
USTRUCT()
struct FVoxelGridIndex
{
	GENERATED_BODY()

	UPROPERTY()
	int32 XIndex;

	UPROPERTY()
	int32 YIndex;

	UPROPERTY()
	int32 ZIndex;
};
*/
UCLASS()
class TSY_PROJECT_API AAStarWorld : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAStarWorld();
	
	struct FMapPoint
	{
		int X;
		int Y;
		int Z;
		float Length;
		FMapPoint* Front=nullptr;
		FMapPoint(int X, int Y, int Z, float Length)
		: X(X), Y(Y), Z(Z), Length(Length) {}
	};
	struct CompareLength {
    bool operator()(const FMapPoint* a, const FMapPoint* b) const {
        return a->Length > b->Length;
    }
};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(CallInEditor)
	void Execute();

	UFUNCTION(CallInEditor)
	void Test();

	UFUNCTION(CallInEditor)
	void DrawPath();

	void AStarAlgorithm(int32 &NumVoxelX,int32 &NumVoxelY,int32 &NumVoxelZ,int32 &StartX,int32 &StartY,int32 &StartZ,int32 &EndX,int32 &EndY,int32 &EndZ,int32 &flag,FVector &StartLocation,std::vector<std::vector<std::vector<float>>>&H);
	
	void VoxelProcess(int32 &NumVoxelX,int32 &NumVoxelY,int32 &NumVoxelZ,FVector &StartLocation,FVector &VoxelSize);

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostEditMove(bool bFinished) override;

	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config", meta=(MakeEditWidget="true"))	
	FVector SearchBegin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config", meta=(MakeEditWidget="true"))
	FVector SearchEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	FVector VoxelLength = FVector(100.f,100.f, 100.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	FVector WorldExtent = FVector(500.f,500.f,500.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	float ShowTime=10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	int32 TestTime=10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	bool bDrawVoxel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Config")
	bool bDrawLine;
	
	UPROPERTY()
	TArray<int32> VoxelGrid;
	
	UPROPERTY()
	TArray<FVector> Path;

private:
	FVector LastActorLocation=FVector::ZeroVector;;
};
