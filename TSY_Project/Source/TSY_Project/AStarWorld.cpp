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
	FVector BoxExtent= FVector(WorldExtent.X,WorldExtent.Y,WorldExtent.Z);
	FVector VoxelSize=FVector(VoxelLength.X,VoxelLength.Y,VoxelLength.Z);
	DrawDebugBox(GetWorld(), ActorLocation, BoxExtent, FColor::Green, ShowTime==0?true:false, ShowTime, 0, 4);
	//各个方向的体素格数量
	int32 NumVoxelX=FMath::CeilToInt(WorldExtent.X*2/VoxelLength.X);
	int32 NumVoxelY=FMath::CeilToInt(WorldExtent.Y*2/VoxelLength.Y);
	int32 NumVoxelZ=FMath::CeilToInt(WorldExtent.Z*2/VoxelLength.Z);
	//体素格的起始位置
	FVector StartLocation=ActorLocation-BoxExtent;
	//判断Voxel是否可通过

	// 如果VoxelGrid尚未初始化,重新生成VoxelGrid
	if (VoxelGrid.Num() == 0)
	{
		//判断Voxel是否可通过
		VoxelGrid.SetNum(NumVoxelX*NumVoxelY*NumVoxelZ);
		//体素化
		VoxelProcess(NumVoxelX,NumVoxelY,NumVoxelZ,StartLocation,VoxelSize);
	}
	
	//初始化起点
	int32 StartX=(SearchBegin.X+ActorLocation.X-StartLocation.X)/VoxelLength.X,StartY=(SearchBegin.Y+ActorLocation.Y-StartLocation.Y)/VoxelLength.Y,StartZ=(SearchBegin.Z+ActorLocation.Z-StartLocation.Z)/VoxelLength.Z;
	
	//初始化终点
	int32 EndX = (SearchEnd.X+ActorLocation.X-StartLocation.X)/VoxelLength.X,EndY = (SearchEnd.Y+ActorLocation.Y-StartLocation.Y)/VoxelLength.Y,EndZ = (SearchEnd.Z+ActorLocation.Z-StartLocation.Z)/VoxelLength.Z;
	
	//判读是否非法起点和终点
	if(!(StartX>=0&&StartX<NumVoxelX&&StartY>=0&&StartY<NumVoxelY&&StartZ>=0&&StartZ<NumVoxelZ&&EndX>=0&&EndX<NumVoxelX&&EndY>=0&&EndY<NumVoxelY&&EndZ>=0&&EndZ<NumVoxelZ))
	{
		UE_LOG(LogTemp,Warning,TEXT("Error, Your Start and End points must be within the specified range, otherwise default start and end points will be created."));
		StartX=0,StartY=0,StartZ=0;
		EndX=NumVoxelX-1,EndY=NumVoxelY-1,EndZ=NumVoxelZ-1;
	}
	if(VoxelGrid[StartX*NumVoxelY*NumVoxelZ+StartY*NumVoxelZ+StartZ]==1||VoxelGrid[EndX*NumVoxelY*NumVoxelZ+EndY*NumVoxelZ+EndZ]==1)
	{
		UE_LOG(LogTemp,Warning,TEXT("Error, You cannot choose to start or end at an obstacle."));
		return;
	}
	FVector StartCenter=StartLocation+FVector(StartX*VoxelLength.X+VoxelLength.X/2,StartY*VoxelLength.Y+VoxelLength.Y/2,StartZ*VoxelLength.Z+VoxelLength.Z/2);
	FVector EndCenter = StartLocation + FVector(EndX*VoxelLength.X+VoxelLength.X/2,EndY*VoxelLength.Y+VoxelLength.Y/2,EndZ*VoxelLength.Z+VoxelLength.Z/2);
	DrawDebugBox(GetWorld(), StartCenter, VoxelSize / 2,  FColor::Black, ShowTime==0?true:false, ShowTime, 0, 2);
	DrawDebugBox(GetWorld(), EndCenter, VoxelSize / 2,  FColor::Black, ShowTime==0?true:false, ShowTime, 0, 2);
	
	//初始化启发数组
	std::vector<std::vector<std::vector<float>>>H(NumVoxelX,std::vector<std::vector<float>>(NumVoxelY,std::vector<float>(NumVoxelZ,0)));
	
	for (int32 X = 0; X < NumVoxelX; X++)
	{
		for (int32 Y = 0; Y < NumVoxelY; Y++)
		{
			for (int32 Z = 0; Z < NumVoxelZ; Z++)
			{
				//使用欧式距离作为启发函数
				H[X][Y][Z]=sqrt((X-EndX)*(X-EndX)+(Y-EndY)*(Y-EndY)+(Z-EndZ)*(Z-EndZ));
			}
		}
	}

	// 搜索开始时获取当前时间
	double StartTime = FPlatformTime::Seconds();
	int flag=0;
	//A star
	AStarAlgorithm(NumVoxelX,NumVoxelY,NumVoxelZ,StartX,StartY,StartZ,EndX,EndY,EndZ,flag,StartLocation,H);
	if(flag==0)
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
	//8个BoxExtent
	FVector BoxExtent= FVector(WorldExtent.X,WorldExtent.Y,WorldExtent.Z);
	FVector VoxelSize=FVector(VoxelLength.X,VoxelLength.Y,VoxelLength.Z);
	DrawDebugBox(GetWorld(), ActorLocation, BoxExtent, FColor::Green, ShowTime==0?true:false, ShowTime, 0, 4);
	//各个方向的体素格数量
	int32 NumVoxelX=FMath::CeilToInt(WorldExtent.X*2/VoxelLength.X);
	int32 NumVoxelY=FMath::CeilToInt(WorldExtent.Y*2/VoxelLength.Y);
	int32 NumVoxelZ=FMath::CeilToInt(WorldExtent.Z*2/VoxelLength.Z);
	//体素格的起始位置
	FVector StartLocation=ActorLocation-BoxExtent;
	FVector EndLocation=ActorLocation+BoxExtent;
	
	if (VoxelGrid.Num() == 0)
	{
		//判断Voxel是否可通过
		VoxelGrid.SetNum(NumVoxelX*NumVoxelY*NumVoxelZ);
		//体素化
		VoxelProcess(NumVoxelX,NumVoxelY,NumVoxelZ,StartLocation,VoxelSize);
	}
	
	double TotalStartTime = FPlatformTime::Seconds();
	int32 Test=TestTime;
	while(Test--)
	{
		UE_LOG(LogTemp,Warning,TEXT("TestCase: %d"),TestTime-Test);

		//初始化起点
		int32 StartX=(FMath::FRandRange(StartLocation.X, EndLocation.X)-StartLocation.X)/VoxelLength.X, StartY= (FMath::FRandRange(StartLocation.Y, EndLocation.Y)-StartLocation.Y)/VoxelLength.Y,StartZ = (FMath::FRandRange(StartLocation.Z, EndLocation.Z)-StartLocation.Z)/VoxelLength.Z;
	
		//初始化终点
		int32 EndX = (FMath::FRandRange(StartLocation.X, EndLocation.X)-StartLocation.X)/VoxelLength.X, EndY = (FMath::FRandRange(StartLocation.Y, EndLocation.Y)-StartLocation.Y)/VoxelLength.Y,EndZ = (FMath::FRandRange(StartLocation.Z, EndLocation.Z)-StartLocation.Z)/VoxelLength.Z;
	

		//判读是否非法起点和终点
		if(!(StartX>=0&&StartX<NumVoxelX&&StartY>=0&&StartY<NumVoxelY&&StartZ>=0&&StartZ<NumVoxelZ&&EndX>=0&&EndX<NumVoxelX&&EndY>=0&&EndY<NumVoxelY&&EndZ>=0&&EndZ<NumVoxelZ))
		{
			UE_LOG(LogTemp,Warning,TEXT("Error, Your Start and End points must be within the specified range, otherwise default start and end points will be created."));
			StartX=0,StartY=0,StartZ=0;
			EndX=NumVoxelX-1,EndY=NumVoxelY-1,EndZ=NumVoxelZ-1;
		}
		if(VoxelGrid[StartX*NumVoxelY*NumVoxelZ+StartY*NumVoxelZ+StartZ]==1||VoxelGrid[EndX*NumVoxelY*NumVoxelZ+EndY*NumVoxelZ+EndZ]==1)
		{
			UE_LOG(LogTemp,Warning,TEXT("Error, You cannot choose to start or end at an obstacle."));
			StartX=0,StartY=0,StartZ=0;
			EndX=NumVoxelX-1,EndY=NumVoxelY-1,EndZ=NumVoxelZ-1;
		}
		FVector StartCenter=StartLocation + FVector(StartX*VoxelLength.X+VoxelLength.X/2,StartY*VoxelLength.Y+VoxelLength.Y/2,StartZ*VoxelLength.Z+VoxelLength.Z/2);
		FVector EndCenter = StartLocation + FVector(EndX*VoxelLength.X+VoxelLength.X/2,EndY*VoxelLength.Y+VoxelLength.Y/2,EndZ*VoxelLength.Z+VoxelLength.Z/2);
		//DrawDebugBox(GetWorld(), StartCenter, VoxelSize / 2,  FColor::Black, ShowTime==0?true:false, ShowTime, 0, 2);
		//DrawDebugBox(GetWorld(), EndCenter, VoxelSize / 2,  FColor::Black, ShowTime==0?true:false, ShowTime, 0, 2);
		//初始化启发数组
		std::vector<std::vector<std::vector<float>>>H(NumVoxelX,std::vector<std::vector<float>>(NumVoxelY,std::vector<float>(NumVoxelZ,0)));
	
		for (int32 X = 0; X < NumVoxelX; X++)
		{
			for (int32 Y = 0; Y < NumVoxelY; Y++)
			{
				for (int32 Z = 0; Z < NumVoxelZ; Z++)
				{
					//使用欧式距离作为启发函数
					H[X][Y][Z]=sqrt((X-EndX)*(X-EndX)+(Y-EndY)*(Y-EndY)+(Z-EndZ)*(Z-EndZ));
				}
			}
		}

		// 搜索开始时获取当前时间
		double StartTime = FPlatformTime::Seconds();
		int flag=0;
		//A star
		AStarAlgorithm(NumVoxelX,NumVoxelY,NumVoxelZ,StartX,StartY,StartZ,EndX,EndY,EndZ,flag,StartLocation,H);
		if(flag==0)
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

void AAStarWorld::AStarAlgorithm(int32 &NumVoxelX,int32 &NumVoxelY,int32 &NumVoxelZ,int32 &StartX,int32 &StartY,int32 &StartZ,int32 &EndX,int32 &EndY,int32 &EndZ,int32 &flag,FVector &StartLocation,std::vector<std::vector<std::vector<float>>>&H)
{
	FMapPoint* StartPoint=new FMapPoint(StartX,StartY,StartZ,0);
	std::priority_queue<FMapPoint*, std::vector<FMapPoint*>, CompareLength> Q;
	Q.push(StartPoint);
	
	//防止访问重复起点
	std::vector<std::vector<std::vector<int>>>Book(NumVoxelX,std::vector<std::vector<int>>(NumVoxelY,std::vector<int>(NumVoxelZ,0)));
	Book[StartX][StartY][StartZ]=1;
	while (!Q.empty())
	{
		FMapPoint* NowPoint=Q.top();
		Q.pop();
		if(NowPoint->X==EndX && NowPoint->Y==EndY && NowPoint->Z==EndZ)
		{
			flag=1;
			UE_LOG(LogTemp, Warning, TEXT("Length: %f"), NowPoint->Length*VoxelLength.X);
			if(bDrawLine)
			{
				FMapPoint* prevPoint = NowPoint;
				FVector prevPointCenter = StartLocation + FVector(prevPoint->X * VoxelLength.X + VoxelLength.X / 2, prevPoint->Y * VoxelLength.Y + VoxelLength.Y / 2, prevPoint->Z * VoxelLength.Z + VoxelLength.Z / 2);
				Path.Empty();
				Path.Add(prevPointCenter);
				while (NowPoint->Front != nullptr) {
					NowPoint = NowPoint->Front;
					FVector NowPointCenter = StartLocation + FVector(NowPoint->X * VoxelLength.X + VoxelLength.X / 2, NowPoint->Y * VoxelLength.Y + VoxelLength.Y / 2, NowPoint->Z * VoxelLength.Z + VoxelLength.Z / 2);
					// 绘制连接线
#if WITH_EDITOR
					//DrawDebugLine(GetWorld(), prevPointCenter, NowPointCenter, FColor::Purple, ShowTime==0?true:false, ShowTime, 0, 5);
					DrawDebugDirectionalArrow(GetWorld(), NowPointCenter,prevPointCenter,  20.0f, FColor::Purple, ShowTime==0?true:false, ShowTime, 0, 5);
#endif
					prevPointCenter = NowPointCenter;
					Path.Add(prevPointCenter);
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
					if(0<=NewX&&NewX<NumVoxelX&&0<=NewY&&NewY<NumVoxelY&&0<=NewZ&&NewZ<NumVoxelZ&&Book[NewX][NewY][NewZ]==0&&VoxelGrid[NewX*NumVoxelY*NumVoxelZ+NewY*NumVoxelZ+NewZ]==0)
					{
						Book[NewX][NewY][NewZ]=1;
						FMapPoint* NewPoint=new FMapPoint(NewX,NewY,NewZ,NowPoint->Length+sqrt(DirX*DirX+DirY*DirY+DirZ*DirZ)+H[NewX][NewY][NewZ]);
						NewPoint->Front=NowPoint;
						Q.push(NewPoint);
					}
				}
			}
		}
		
	}
}

void AAStarWorld::VoxelProcess(int32 &NumVoxelX,int32 &NumVoxelY,int32 &NumVoxelZ,FVector &StartLocation,FVector &VoxelSize)
{
	// 忽略自身
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this); 
	//TArray<FHitResult> OutHits;
	for (int32 X = 0; X < NumVoxelX; X++)
	{
		for (int32 Y = 0; Y < NumVoxelY; Y++)
		{
			for (int32 Z = 0; Z < NumVoxelZ; Z++)
			{
				FVector VoxelCenter=StartLocation+FVector(X*VoxelLength.X+VoxelLength.X/2,Y*VoxelLength.Y+VoxelLength.Y/2,Z*VoxelLength.Z+VoxelLength.Z/2);
				bool bIsOverlapping = GetWorld()->OverlapBlockingTestByChannel(
					VoxelCenter,
					FQuat::Identity,
					ECC_WorldStatic,
					FCollisionShape::MakeBox(VoxelSize / 2),
					CollisionParams
				);
				int32 Index=X*NumVoxelY*NumVoxelZ+Y*NumVoxelZ+Z;
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
					DrawDebugBox(GetWorld(), VoxelCenter, VoxelSize / 2, bIsOverlapping ? FColor::Red : FColor::Green, ShowTime==0?true:false, ShowTime, 0, 1);
			}
		}
	}
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
		//8个BoxExtent
		FVector BoxExtent= FVector(WorldExtent.X,WorldExtent.Y,WorldExtent.Z);
		FVector VoxelSize=FVector(VoxelLength.X,VoxelLength.Y,VoxelLength.Z);
		DrawDebugBox(GetWorld(), ActorLocation, BoxExtent, FColor::Green, ShowTime==0?true:false, ShowTime, 0, 4);
		//各个方向的体素格数量
		int32 NumVoxelX=FMath::CeilToInt(WorldExtent.X*2/VoxelLength.X);
		int32 NumVoxelY=FMath::CeilToInt(WorldExtent.Y*2/VoxelLength.Y);
		int32 NumVoxelZ=FMath::CeilToInt(WorldExtent.Z*2/VoxelLength.Z);
		//体素格的起始位置
		FVector StartLocation=ActorLocation-BoxExtent;
		FVector EndLocation=ActorLocation+BoxExtent;
		VoxelGrid.SetNum(NumVoxelX*NumVoxelY*NumVoxelZ);
		VoxelProcess(NumVoxelX,NumVoxelY,NumVoxelZ,StartLocation,VoxelSize);
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

		FVector BoxExtent = FVector(WorldExtent.X, WorldExtent.Y, WorldExtent.Z);
		FVector VoxelSize = FVector(VoxelLength.X, VoxelLength.Y, VoxelLength.Z);
		DrawDebugBox(GetWorld(), ActorLocation, BoxExtent, FColor::Green, ShowTime == 0 ? true : false, ShowTime, 0, 4);

		int32 NumVoxelX = FMath::CeilToInt(WorldExtent.X * 2 / VoxelLength.X);
		int32 NumVoxelY = FMath::CeilToInt(WorldExtent.Y * 2 / VoxelLength.Y);
		int32 NumVoxelZ = FMath::CeilToInt(WorldExtent.Z * 2 / VoxelLength.Z);

		FVector StartLocation = ActorLocation - BoxExtent;
		FVector EndLocation = ActorLocation + BoxExtent;
		VoxelGrid.SetNum(NumVoxelX * NumVoxelY * NumVoxelZ);
		VoxelProcess(NumVoxelX, NumVoxelY, NumVoxelZ, StartLocation, VoxelSize);
		LastActorLocation = CurrentLocation;
	}
}

