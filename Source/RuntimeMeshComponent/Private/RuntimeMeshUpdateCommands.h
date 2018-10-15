// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Components/MeshComponent.h"
#include "RuntimeMeshCore.h"



struct FRuntimeMeshSectionVertexBufferParams
{
	TArray<uint8> Data;
	int32 NumVertices;
};
struct FRuntimeMeshSectionTangentVertexBufferParams : public FRuntimeMeshSectionVertexBufferParams
{
	bool bUsingHighPrecision;
};
struct FRuntimeMeshSectionUVVertexBufferParams : public FRuntimeMeshSectionVertexBufferParams
{
	bool bUsingHighPrecision;
	int32 NumUVs;
};

struct FRuntimeMeshSectionIndexBufferParams
{
	bool b32BitIndices;
	TArray<uint8> Data;
	int32 NumIndices;
};

struct FRuntimeMeshSectionLODUpdateParams
{
	FRuntimeMeshSectionVertexBufferParams PositionVertexBuffer;
	FRuntimeMeshSectionTangentVertexBufferParams TangentsVertexBuffer;
	FRuntimeMeshSectionUVVertexBufferParams UVsVertexBuffer;
	FRuntimeMeshSectionVertexBufferParams ColorVertexBuffer;

	FRuntimeMeshSectionIndexBufferParams IndexBuffer;
	FRuntimeMeshSectionIndexBufferParams AdjacencyIndexBuffer;
};


struct FRuntimeMeshSectionCreationParams
{
	EUpdateFrequency UpdateFrequency;

	TArray<FRuntimeMeshSectionLODUpdateParams, TInlineAllocator<RUNTIMEMESH_MAXLODS>> LODs;

	bool bIsVisible;
	bool bCastsShadow;
};
using FRuntimeMeshSectionCreationParamsPtr = TSharedPtr<FRuntimeMeshSectionCreationParams, ESPMode::NotThreadSafe>;

struct FRuntimeMeshSectionUpdateParams
{
	int32 LODIndex;
	ERuntimeMeshBuffersToUpdate BuffersToUpdate;

	FRuntimeMeshSectionVertexBufferParams PositionVertexBuffer;
	FRuntimeMeshSectionTangentVertexBufferParams TangentsVertexBuffer;
	FRuntimeMeshSectionUVVertexBufferParams UVsVertexBuffer;
	FRuntimeMeshSectionVertexBufferParams ColorVertexBuffer;

	FRuntimeMeshSectionIndexBufferParams IndexBuffer;
	FRuntimeMeshSectionIndexBufferParams AdjacencyIndexBuffer;
};
using FRuntimeMeshSectionUpdateParamsPtr = TSharedPtr<FRuntimeMeshSectionUpdateParams, ESPMode::NotThreadSafe>;

struct FRuntimeMeshSectionPropertyUpdateParams
{
	bool bIsVisible;
	bool bCastsShadow;
};
using FRuntimeMeshSectionPropertyUpdateParamsPtr = TSharedPtr<FRuntimeMeshSectionPropertyUpdateParams, ESPMode::NotThreadSafe>;


struct FRuntimeMeshLODDataUpdateParams
{
	TArray<float, TInlineAllocator<8>> ScreenSizes;
};
using FRuntimeMeshLODDataUpdateParamsPtr = TSharedPtr<FRuntimeMeshLODDataUpdateParams, ESPMode::NotThreadSafe>;
