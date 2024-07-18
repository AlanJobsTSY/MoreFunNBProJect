// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AStarWorld.generated.h"

UCLASS()
class TSY_PROJECT_API AAStarWorld : public AActor {
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAStarWorld();
	// 存放在优先队列中的结构体
	struct FMapPoint {
		int X;
		int Y;
		int Z;
		float Length;
		FMapPoint *Front = nullptr;
		FMapPoint(int X, int Y, int Z, float Length)
			: X(X), Y(Y), Z(Z), Length(Length) {}
	};
	// 比较函数
	struct CompareLength {
		bool operator()(const FMapPoint &a, const FMapPoint &b) const {
			return a.Length < b.Length;
		}
	};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 执行一次
	UFUNCTION(CallInEditor)
	void Execute();

	// 测试TestTime次
	UFUNCTION(CallInEditor)
	void Test();

	// 画出最后一次寻路的结果
	UFUNCTION(CallInEditor)
	void DrawPath();

	// A*
	void AStarAlgorithm(int32 &Flag, const FVector &StartLocation, const TArray< TArray< TArray< float > > > &H);

	// 体素化
	void VoxelProcess(const FVector &StartLocation);

	// 将实际坐标转化为体素化的坐标
	FIntVector TransferLocationToCellPosition(const FVector &InPos) const;

	// 将体素化的坐标转化为实际坐标
	FVector TransferCellPositionToLocation(const FVector &StartLocation, const FIntVector &VoxelCoordinate) const;

	// 生成随机的起始点和终点
	FIntVector GenerateRandomCellPosition(const FVector &StartLocation, const FVector &EndLocation) const;

	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;

	virtual void PostEditMove(bool bFinished) override;



protected:
	// 选定的搜索出发点
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (MakeEditWidget = "true"))
	FVector SearchBegin;

	// 选定的搜索终点
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (MakeEditWidget = "true"))
	FVector SearchEnd;

	// 默认的体素单元大小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector VoxelLength = FVector(100.f, 100.f, 100.f);

	// 默认的搜索世界范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector WorldExtent = FVector(500.f, 500.f, 500.f);

	// 默认的体素单元框线和搜索线路的显示时间，ShowTime为0表示永久显示
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ShowTime = 10.f;

	// 默认的测试次数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 TestTime = 10;

	// 是否画出体素单元框线
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bDrawVoxel = true;

	// 是否画出搜索线路
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bDrawLine = true;

	// 障碍物的存储数组，通过降维得到的，存1为障碍物，存0为可行区域
	UPROPERTY()
	TArray< int32 > VoxelGrid;

	// 最终的搜索路径保存数组
	UPROPERTY()
	TArray< FVector > Path;

	// 上一次Actor的位置
	UPROPERTY()
	FVector LastActorLocation;

private:
	// 体素单元的三轴范围
	FIntVector NumVoxel = FIntVector::ZeroValue;

	// 搜索出发点的体素坐标
	FIntVector Start = FIntVector::ZeroValue;

	// 搜索终点的体素坐标
	FIntVector End = FIntVector::ZeroValue;
};
