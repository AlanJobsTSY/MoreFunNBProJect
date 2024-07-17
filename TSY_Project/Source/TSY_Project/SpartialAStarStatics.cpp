#include "SpartialAStarStatics.h"

FIntVector3 FSpartialAStarStatics::TransferLocationToCellPosition(const FVector& InPos, const FIntVector3& VoxelRange,
	const FVector& WorldExtent, const FVector& VoxelBox)
{
	const FVector Tmp = InPos + WorldExtent;
	
	return {
		FMath::Clamp(static_cast<UE::Math::TIntVector3<int>::IntType>(Tmp.X / VoxelBox.X), 0, VoxelRange.X),
		FMath::Clamp(static_cast<UE::Math::TIntVector3<int>::IntType>(Tmp.Y / VoxelBox.Y), 0, VoxelRange.Y),
		FMath::Clamp(static_cast<UE::Math::TIntVector3<int>::IntType>(Tmp.Z / VoxelBox.Z), 0, VoxelRange.Z)
	};
}

int32 FSpartialAStarStatics::CellPositionToVoxelIndex(const FIntVector3& CellPos, const FIntVector3& VoxelRange)
{
	return CellPos.X * VoxelRange.X * VoxelRange.Z + CellPos.Y * VoxelRange.Z + CellPos.Z;
}

FVector FSpartialAStarStatics::TransferCellPositionToLocation(const FIntVector3& CellPos, const FVector& BaseLocation, const FVector& VoxelBox)
{
	return BaseLocation + FVector(
		CellPos.X * VoxelBox.X + VoxelBox.X / 2,
		CellPos.Y * VoxelBox.Y + VoxelBox.Y / 2,
		CellPos.Z * VoxelBox.Z + VoxelBox.Z / 2
		);
}
