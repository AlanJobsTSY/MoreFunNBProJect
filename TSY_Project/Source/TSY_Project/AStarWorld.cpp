// Fill out your copyright notice in the Description page of Project Settings.


#include "AStarWorld.h"
#include "DrawDebugHelpers.h"
#include <queue>
#include <vector>
#include <unordered_map>
#include <map>
#include "DiffUtils.h"
#include "Editor.h"
// Sets default values
AAStarWorld::AAStarWorld()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAStarWorld::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAStarWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAStarWorld::Execute()
{
	FlushPersistentDebugLines(GetWorld());
	FVector ActorLocation=GetActorLocation();
	UE_LOG(LogTemp,Warning,TEXT("ActorLocation at (%f, %f, %f)"), ActorLocation.X, ActorLocation.Y, ActorLocation.Z);
	//箱体大小和体素单元格大小
	//float BoxExtentSize=WorldExtent;
	//float VoxelLength=VoxelSize;
	//8个BoxExtent
	DrawDebugBox(GetWorld(), ActorLocation, WorldExtent, FColor::Green, ShowTime==0?true:false, ShowTime, 0, 4);
	//各个方向的体素格数量
	NumVoxel.X=FMath::CeilToInt(WorldExtent.X*2/VoxelLength.X);
	NumVoxel.Y=FMath::CeilToInt(WorldExtent.Y*2/VoxelLength.Y);
	NumVoxel.Z=FMath::CeilToInt(WorldExtent.Z*2/VoxelLength.Z);
	//体素格的起始位置
	FVector StartLocation=ActorLocation-WorldExtent;
	//判断Voxel是否可通过

	// 如果VoxelGrid尚未初始化,重新生成VoxelGrid
	if (VoxelGrid.Num() == 0)
	{
		//判断Voxel是否可通过
		VoxelGrid.SetNum(NumVoxel.X*NumVoxel.Y*NumVoxel.Z);
		//体素化
		VoxelProcess(StartLocation);
	}
	
	//初始化起点
	Start = TransferLocationToCellPosition(SearchBegin);
	
	//初始化终点
	End = TransferLocationToCellPosition(SearchEnd);
	
	//判读是否非法起点和终点
	if(!(Start.X>=0&&Start.X<NumVoxel.X&&Start.Y>=0&&Start.Y<NumVoxel.Y&&Start.Z>=0&&Start.Z<NumVoxel.Z))
	{
		UE_LOG(LogTemp,Warning,TEXT("Error, Your Start points must be within the specified range, otherwise default start and end points will be created."));
		Start.X=0,Start.Y=0,Start.Z=0;
	}
	if(!(End.X>=0&&End.X<NumVoxel.X&&End.Y>=0&&End.Y<NumVoxel.Y&&End.Z>=0&&End.Z<NumVoxel.Z))
	{
		UE_LOG(LogTemp,Warning,TEXT("Error, Your End points must be within the specified range, otherwise default start and end points will be created."));
		End.X=NumVoxel.X-1,End.Y=NumVoxel.Y-1,End.Z=NumVoxel.Z-1;
	}
	if(VoxelGrid[Start.X*NumVoxel.Y*NumVoxel.Z+Start.Y*NumVoxel.Z+Start.Z]==1)
	{
		UE_LOG(LogTemp,Warning,TEXT("Error, You cannot choose to start at an obstacle."));
		Start.X=0,Start.Y=0,Start.Z=0;
	}
	if(VoxelGrid[End.X*NumVoxel.Y*NumVoxel.Z+End.Y*NumVoxel.Z+End.Z]==1)
	{
		UE_LOG(LogTemp,Warning,TEXT("Error, You cannot choose to end at an obstacle."));
		End.X=NumVoxel.X-1,End.Y=NumVoxel.Y-1,End.Z=NumVoxel.Z-1;
	}
	FVector StartCenter=CalcPointCenter(StartLocation,Start.X,Start.Y,Start.Z);
	FVector EndCenter = CalcPointCenter(StartLocation,End.X,End.Y,End.Z);
	DrawDebugBox(GetWorld(), StartCenter, VoxelLength / 2,  FColor::Black, ShowTime==0?true:false, ShowTime, 0, 2);
	DrawDebugBox(GetWorld(), EndCenter, VoxelLength / 2,  FColor::Black, ShowTime==0?true:false, ShowTime, 0, 2);
	
	//初始化启发数组
	TArray<TArray<TArray<float>>> H;
	H.SetNum(NumVoxel.X);
	for (int32 i = 0; i < NumVoxel.X; ++i)
	{
		H[i].SetNum(NumVoxel.Y);
		for (int32 j = 0; j < NumVoxel.Y; ++j)
		{
			H[i][j].SetNum(NumVoxel.Z);
			for (int32 k = 0; k < NumVoxel.Z; ++k)
			{
				H[i][j][k] = 0;
			}
		}
	}
	
	for (int32 X = 0; X < NumVoxel.X; X++)
	{
		for (int32 Y = 0; Y < NumVoxel.Y; Y++)
		{
			for (int32 Z = 0; Z < NumVoxel.Z; Z++)
			{
				//使用欧式距离作为启发函数
				H[X][Y][Z]=FMath::Sqrt(FMath::Square(X-End.X)+FMath::Square(Y-End.Y)+FMath::Square(Z-End.Z));
			}
		}
	}

	// 搜索开始时获取当前时间
	double StartTime = FPlatformTime::Seconds();
	int Flag=0;
	//A star
	AStarAlgorithm(Flag,StartLocation,H);
	if(Flag==0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find a way from start to end."));
		return;
	}
	// 搜索结束时获取当前时间
	double EndTime = FPlatformTime::Seconds();
	double SearchTime = EndTime - StartTime;
	UE_LOG(LogTemp, Warning, TEXT("Search time: %f seconds"), SearchTime);
}

void AAStarWorld::Test()
{
	FlushPersistentDebugLines(GetWorld());
	FVector ActorLocation=GetActorLocation();
	UE_LOG(LogTemp,Warning,TEXT("ActorLocation at (%f, %f, %f)"), ActorLocation.X, ActorLocation.Y, ActorLocation.Z);
	//箱体大小和体素单元格大小
	//float BoxExtentSize=WorldExtent;
	//float VoxelLength=VoxelSize;

	DrawDebugBox(GetWorld(), ActorLocation, WorldExtent, FColor::Green, ShowTime==0?true:false, ShowTime, 0, 4);
	//各个方向的体素格数量
	NumVoxel.X=FMath::CeilToInt(WorldExtent.X*2/VoxelLength.X);
	NumVoxel.Y=FMath::CeilToInt(WorldExtent.Y*2/VoxelLength.Y);
	NumVoxel.Z=FMath::CeilToInt(WorldExtent.Z*2/VoxelLength.Z);
	//体素格的起始位置
	FVector StartLocation=ActorLocation-WorldExtent;
	FVector EndLocation=ActorLocation+WorldExtent;
	
	if (VoxelGrid.Num() == 0)
	{
		//判断Voxel是否可通过
		VoxelGrid.SetNum(NumVoxel.X*NumVoxel.Y*NumVoxel.Z);
		//体素化
		VoxelProcess(StartLocation);
	}
	
	double TotalStartTime = FPlatformTime::Seconds();
	int32 Test=TestTime;
	while(Test--)
	{
		UE_LOG(LogTemp,Warning,TEXT("TestCase: %d"),TestTime-Test);

		//初始化起点
		Start=GenerateRandomCellPosition(StartLocation,EndLocation);
	
		//初始化终点
		End=GenerateRandomCellPosition(StartLocation,EndLocation);
		
		//判读是否非法起点和终点
		if(!(Start.X>=0&&Start.X<NumVoxel.X&&Start.Y>=0&&Start.Y<NumVoxel.Y&&Start.Z>=0&&Start.Z<NumVoxel.Z))
		{
			UE_LOG(LogTemp,Warning,TEXT("Error, Your Start points must be within the specified range, otherwise default start and end points will be created."));
			Start.X=0,Start.Y=0,Start.Z=0;
		}
		if(!(End.X>=0&&End.X<NumVoxel.X&&End.Y>=0&&End.Y<NumVoxel.Y&&End.Z>=0&&End.Z<NumVoxel.Z))
		{
			UE_LOG(LogTemp,Warning,TEXT("Error, Your End points must be within the specified range, otherwise default start and end points will be created."));
			End.X=NumVoxel.X-1,End.Y=NumVoxel.Y-1,End.Z=NumVoxel.Z-1;
		}
		if(VoxelGrid[Start.X*NumVoxel.Y*NumVoxel.Z+Start.Y*NumVoxel.Z+Start.Z]==1)
		{
			UE_LOG(LogTemp,Warning,TEXT("Error, You cannot choose to start at an obstacle."));
			Start.X=0,Start.Y=0,Start.Z=0;
		}
		if(VoxelGrid[End.X*NumVoxel.Y*NumVoxel.Z+End.Y*NumVoxel.Z+End.Z]==1)
		{
			UE_LOG(LogTemp,Warning,TEXT("Error, You cannot choose to end at an obstacle."));
			End.X=NumVoxel.X-1,End.Y=NumVoxel.Y-1,End.Z=NumVoxel.Z-1;
		}
		//FVector StartCenter=CalcPointCenter(StartLocation,Start.X,Start.Y,Start.Z);
		//FVector EndCenter = CalcPointCenter(StartLocation,End.X,End.Y,End.Z);
		//DrawDebugBox(GetWorld(), StartCenter, VoxelLength / 2,  FColor::Black, ShowTime==0?true:false, ShowTime, 0, 2);
		//DrawDebugBox(GetWorld(), EndCenter, VoxelLength / 2,  FColor::Black, ShowTime==0?true:false, ShowTime, 0, 2);
		//初始化启发数组
		TArray<TArray<TArray<float>>> H;
		H.SetNum(NumVoxel.X);
		for (int32 i = 0; i < NumVoxel.X; ++i)
		{
			H[i].SetNum(NumVoxel.Y);
			for (int32 j = 0; j < NumVoxel.Y; ++j)
			{
				H[i][j].SetNum(NumVoxel.Z);
				for (int32 k = 0; k < NumVoxel.Z; ++k)
				{
					H[i][j][k] = 0;
				}
			}
		}
		
		for (int32 X = 0; X < NumVoxel.X; X++)
		{
			for (int32 Y = 0; Y < NumVoxel.Y; Y++)
			{
				for (int32 Z = 0; Z < NumVoxel.Z; Z++)
				{
					//使用欧式距离作为启发函数
					H[X][Y][Z]=FMath::Sqrt(FMath::Square(X-End.X)+FMath::Square(Y-End.Y)+FMath::Square(Z-End.Z));
				}
			}
		}

		// 搜索开始时获取当前时间
		double StartTime = FPlatformTime::Seconds();
		int Flag=0;
		//A star
		AStarAlgorithm(Flag,StartLocation,H);
		if(Flag==0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot find a way from start to end."));
			return;
		}
		// 搜索结束时获取当前时间
		double EndTime = FPlatformTime::Seconds();
		double SearchTime = EndTime - StartTime;
		UE_LOG(LogTemp, Warning, TEXT("Search time: %f seconds"), SearchTime);
	}
	double TotalEndTime = FPlatformTime::Seconds();
	UE_LOG(LogTemp, Warning, TEXT("\n\nTotal Search time: %f seconds"), TotalEndTime-TotalStartTime);
	
}

void AAStarWorld::DrawPath()
{
	for(int i=1;i<Path.Num();i++)
	{
		DrawDebugDirectionalArrow(GetWorld(), Path[i],Path[i-1],  20.0f, FColor::Orange, ShowTime==0?true:false, ShowTime, 0, 5);
	}
}

void AAStarWorld::AStarAlgorithm(int32& Flag,const FVector& StartLocation,const TArray<TArray<TArray<float>>> &H)
{
	FMapPoint* StartPoint=new FMapPoint(Start.X,Start.Y,Start.Z,0);
	std::priority_queue<FMapPoint*, std::vector<FMapPoint*>, CompareLength> Q;
	Q.push(StartPoint);
	
	//防止访问重复起点
	TArray<TArray<TArray<float>>> Book;
	Book.SetNum(NumVoxel.X);
	for (int32 i = 0; i < NumVoxel.X; ++i)
	{
		Book[i].SetNum(NumVoxel.Y);
		for (int32 j = 0; j < NumVoxel.Y; ++j)
		{
			Book[i][j].SetNum(NumVoxel.Z);
			for (int32 k = 0; k < NumVoxel.Z; ++k)
			{
				Book[i][j][k] = 0;
			}
		}
	}
	Book[Start.X][Start.Y][Start.Z]=1;
	while (!Q.empty())
	{
		FMapPoint* NowPoint=Q.top();
		Q.pop();
		if(NowPoint->X==End.X && NowPoint->Y==End.Y && NowPoint->Z==End.Z)
		{
			Flag=1;
			UE_LOG(LogTemp, Warning, TEXT("Length: %f"), NowPoint->Length*VoxelLength.X);
			if(bDrawLine)
			{
				FMapPoint* PrevPoint = NowPoint;
				FVector PrevPointCenter = CalcPointCenter(StartLocation,PrevPoint->X,PrevPoint->Y,PrevPoint->Z);
				Path.Empty();
				Path.Add(PrevPointCenter);
				while (NowPoint->Front != nullptr) {
					NowPoint = NowPoint->Front;
					FVector NowPointCenter =  CalcPointCenter(StartLocation,NowPoint->X,NowPoint->Y,NowPoint->Z);
					// 绘制连接线
#if WITH_EDITOR
					//DrawDebugLine(GetWorld(), PrevPointCenter, NowPointCenter, FColor::Purple, ShowTime==0?true:false, ShowTime, 0, 5);
					DrawDebugDirectionalArrow(GetWorld(), NowPointCenter,PrevPointCenter,  20.0f, FColor::Purple, ShowTime==0?true:false, ShowTime, 0, 5);
#endif
					PrevPointCenter = NowPointCenter;
					Path.Add(PrevPointCenter);
				}
			}
			break;
		}
		for(int DirX=-1;DirX<=1;DirX++)
		{
			for(int DirY=-1;DirY<=1;DirY++)
			{
				for(int DirZ=-1;DirZ<=1;DirZ++)
				{
					int NewX=NowPoint->X+DirX;
					int NewY=NowPoint->Y+DirY;
					int NewZ=NowPoint->Z+DirZ;
					if(0<=NewX&&NewX<NumVoxel.X&&0<=NewY&&NewY<NumVoxel.Y&&0<=NewZ&&NewZ<NumVoxel.Z&&Book[NewX][NewY][NewZ]==0&&VoxelGrid[NewX*NumVoxel.Y*NumVoxel.Z+NewY*NumVoxel.Z+NewZ]==0)
					{
						Book[NewX][NewY][NewZ]=1;
						FMapPoint* NewPoint=new FMapPoint(NewX,NewY,NewZ,NowPoint->Length+FMath::Square(DirX*DirX+DirY*DirY+DirZ*DirZ)+H[NewX][NewY][NewZ]);
						NewPoint->Front=NowPoint;
						Q.push(NewPoint);
					}
				}
			}
		}
		
	}
}

void AAStarWorld::VoxelProcess(const FVector& StartLocation)
{
	// 忽略自身
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this); 
	//TArray<FHitResult> OutHits;
	for (int32 X = 0; X < NumVoxel.X; X++)
	{
		for (int32 Y = 0; Y < NumVoxel.Y; Y++)
		{
			for (int32 Z = 0; Z < NumVoxel.Z; Z++)
			{
				FVector VoxelCenter=CalcPointCenter(StartLocation,X,Y,Z);
				bool bIsOverlapping = GetWorld()->OverlapBlockingTestByChannel(
					VoxelCenter,
					FQuat::Identity,
					ECC_WorldStatic,
					FCollisionShape::MakeBox(VoxelLength / 2),
					CollisionParams
				);
				int32 Index=X*NumVoxel.Y*NumVoxel.Z+Y*NumVoxel.Z+Z;
				if(bIsOverlapping)
				{
					VoxelGrid[Index]=1;
					/*
					for(auto& Hit:OutHits)
					{
						UE_LOG(LogTemp,Warning,TEXT("Voxel at (%d, %d, %d) overlapping with: %s"), X, Y, Z, *Hit.GetActor()->GetName());
					}*/
				}
				else
				{
					VoxelGrid[Index]=0;
				}
				UE_LOG(LogTemp,Warning,TEXT("%d, %d, %d: %d"), X, Y, Z, VoxelGrid[Index]);
				if(bDrawVoxel)
					DrawDebugBox(GetWorld(), VoxelCenter, VoxelLength / 2, bIsOverlapping ? FColor::Red : FColor::Green, ShowTime==0?true:false, ShowTime, 0, 1);
			}
		}
	}
}

FIntVector AAStarWorld::TransferLocationToCellPosition(const FVector& InPos) const
{
	FVector Tmp = InPos + WorldExtent;
	return {
		static_cast<int32>(Tmp.X / VoxelLength.X),
		static_cast<int32>(Tmp.Y / VoxelLength.Y),
		static_cast<int32>(Tmp.Z / VoxelLength.Z)
	};
}

FVector AAStarWorld::CalcPointCenter(const FVector& StartLocation, int32 X, int32 Y, int32 Z) const
{
	return StartLocation+FVector(X*VoxelLength.X+VoxelLength.X/2,Y*VoxelLength.Y+VoxelLength.Y/2,Z*VoxelLength.Z+VoxelLength.Z/2);
}

FIntVector AAStarWorld::GenerateRandomCellPosition(const FVector& StartLocation, const FVector& EndLocation) const
{
	int32 RandomX = (FMath::FRandRange(StartLocation.X, EndLocation.X) - StartLocation.X) / VoxelLength.X;
	int32 RandomY = (FMath::FRandRange(StartLocation.Y, EndLocation.Y) - StartLocation.Y) / VoxelLength.Y;
	int32 RandomZ = (FMath::FRandRange(StartLocation.Z, EndLocation.Z) - StartLocation.Z) / VoxelLength.Z;

	return FIntVector(RandomX, RandomY, RandomZ);
}

void AAStarWorld::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName ChangedPropertyName = PropertyChangedEvent.MemberProperty->GetFName();
	
	//UE_LOG(LogTemp, Warning, TEXT("TSYTEST1 %s"), *ChangedPropertyName.ToString());
	//UE_LOG(LogTemp, Warning, TEXT("TSYTEST2 %s"), *GET_MEMBER_NAME_CHECKED(AAStarWorld, VoxelLength).ToString());
	// 检查更改的属性是否是SearchBegin
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AAStarWorld, VoxelLength) ||ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AAStarWorld, WorldExtent))
	{
		// 如果SearchBegin发生更改，执行VoxelProcess()函数
		FlushPersistentDebugLines(GetWorld());
		FVector ActorLocation=GetActorLocation();
		UE_LOG(LogTemp,Warning,TEXT("ActorLocation at (%f, %f, %f)"), ActorLocation.X, ActorLocation.Y, ActorLocation.Z);
		//箱体大小和体素单元格大小
		DrawDebugBox(GetWorld(), ActorLocation, WorldExtent, FColor::Green, ShowTime==0?true:false, ShowTime, 0, 4);
		//各个方向的体素格数量
		NumVoxel.X=FMath::CeilToInt(WorldExtent.X*2/VoxelLength.X);
		NumVoxel.Y=FMath::CeilToInt(WorldExtent.Y*2/VoxelLength.Y);
		NumVoxel.Z=FMath::CeilToInt(WorldExtent.Z*2/VoxelLength.Z);
		//体素格的起始位置
		FVector StartLocation=ActorLocation-WorldExtent;
		VoxelGrid.SetNum(NumVoxel.X*NumVoxel.Y*NumVoxel.Z);
		VoxelProcess(StartLocation);
	}
}

void AAStarWorld::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	FVector CurrentLocation = GetActorLocation();
	// 检查鼠标拖动操作是否已结束

	if (bFinished&&CurrentLocation!=LastActorLocation){
		FlushPersistentDebugLines(GetWorld());
		FVector ActorLocation = GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("ActorLocation at (%f, %f, %f)"), ActorLocation.X, ActorLocation.Y, ActorLocation.Z);
		
		DrawDebugBox(GetWorld(), ActorLocation, WorldExtent, FColor::Green, ShowTime == 0 ? true : false, ShowTime, 0, 4);

		NumVoxel.X = FMath::CeilToInt(WorldExtent.X * 2 / VoxelLength.X);
		NumVoxel.Y = FMath::CeilToInt(WorldExtent.Y * 2 / VoxelLength.Y);
		NumVoxel.Z = FMath::CeilToInt(WorldExtent.Z * 2 / VoxelLength.Z);

		FVector StartLocation = ActorLocation - WorldExtent;

		VoxelGrid.SetNum(NumVoxel.X * NumVoxel.Y * NumVoxel.Z);
		VoxelProcess(StartLocation);
		LastActorLocation = CurrentLocation;
	}
}

