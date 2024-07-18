

#pragma once

#include "CoreMinimal.h"

namespace FSpartialAStarStatics
{
	/*
	 * 将3D位置坐标转为体素位置索引坐标
	 * @InPos: 待转换的坐标
	 * @VoxelRange: 体素索引范围
	 * @WorldExtent: 世界区域范围
	 * @VoxelBox: 单个体素尺寸
	 * 
	 * @return: 体素位置索引坐标
	 */
	static FIntVector TransferLocationToCellPosition(const FVector& InPos, const FIntVector& VoxelRange, const FVector& WorldExtent, const FVector& VoxelBox);

	/*
	 * 将体素位置索引坐标转换为体素一维索引
	 * 
	 */
	static int32 CellPositionToVoxelIndex(const FIntVector& CellPos, const FIntVector& VoxelRange);
	
	/*
	 * 将体素索引坐标转为3D位置坐标
	 *
	 * @CellPos: 体素索引坐标
	 * @BaseLocation: 基准位置坐标
	 * @VoxelBox: 单个体素尺寸
	 *
	 * @return: 3D位置坐标
	 */
	static FVector TransferCellPositionToLocation(const FIntVector& CellPos, const FVector& BaseLocation, const FVector& VoxelBox);
}
