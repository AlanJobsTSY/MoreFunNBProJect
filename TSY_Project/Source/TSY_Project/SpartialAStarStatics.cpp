#include "SpartialAStarStatics.h"

FIntVector FSpartialAStarStatics::TransferLocationToCellPosition(const FVector& InPos, const FIntVector& VoxelRange,
	const FVector& WorldExtent, const FVector& VoxelBox)
{
	const FVector Tmp = InPos + WorldExtent;
	
	return {
		FMath::Clamp(static_cast<int32>(Tmp.X / VoxelBox.X), 0, VoxelRange.X),
		FMath::Clamp(static_cast<int32>(Tmp.Y / VoxelBox.Y), 0, VoxelRange.Y),
		FMath::Clamp(static_cast<int32>(Tmp.Z / VoxelBox.Z), 0, VoxelRange.Z)
	};
}

int32 FSpartialAStarStatics::CellPositionToVoxelIndex(const FIntVector& CellPos, const FIntVector& VoxelRange)
{
	return CellPos.X * VoxelRange.Y * VoxelRange.Z + CellPos.Y * VoxelRange.Z + CellPos.Z;
}

FVector FSpartialAStarStatics::TransferCellPositionToLocation(const FIntVector& CellPos, const FVector& BaseLocation, const FVector& VoxelBox)
{
	return BaseLocation + FVector(
		CellPos.X * VoxelBox.X + VoxelBox.X / 2,
		CellPos.Y * VoxelBox.Y + VoxelBox.Y / 2,
		CellPos.Z * VoxelBox.Z + VoxelBox.Z / 2
		);
}
