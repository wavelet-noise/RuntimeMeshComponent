// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#include "RuntimeMeshSectionProxy.h"
#include "RuntimeMeshComponentPlugin.h"



bool FRuntimeMeshSectionProxyLODData::CanRender()
{
	if (PositionBuffer.Num() <= 0)
	{
		return false;
	}

	if (PositionBuffer.Num() != TangentsBuffer.Num() || PositionBuffer.Num() != UVsBuffer.Num())
	{
		return false;
	}

	return true;
}

void FRuntimeMeshSectionProxyLODData::BuildVertexDataType(FLocalVertexFactory::FDataType& DataType)
{
	PositionBuffer.Bind(DataType);
	TangentsBuffer.Bind(DataType);
	UVsBuffer.Bind(DataType);
	ColorBuffer.Bind(DataType);
}


void FRuntimeMeshSectionProxyLODData::CreateMeshBatch(FMeshBatch& MeshBatch, bool bCastsShadow, bool bWantsAdjacencyInfo)
{
	MeshBatch.VertexFactory = &VertexFactory;

	MeshBatch.Type = bWantsAdjacencyInfo ? PT_12_ControlPointPatchList : PT_TriangleList;

	MeshBatch.DepthPriorityGroup = SDPG_World;
	MeshBatch.CastShadow = bCastsShadow;

	// Make sure that if the material wants adjacency information, that you supply it
	check(!bWantsAdjacencyInfo || AdjacencyIndexBuffer.Num() > 0);

	FRuntimeMeshIndexBuffer* CurrentIndexBuffer = bWantsAdjacencyInfo ? &AdjacencyIndexBuffer : &IndexBuffer;

	int32 NumIndicesPerTriangle = bWantsAdjacencyInfo ? 12 : 3;
	int32 NumPrimitives = CurrentIndexBuffer->Num() / NumIndicesPerTriangle;

	FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
	BatchElement.IndexBuffer = CurrentIndexBuffer;
	BatchElement.FirstIndex = 0;
	BatchElement.NumPrimitives = NumPrimitives;
	BatchElement.MinVertexIndex = 0;
	BatchElement.MaxVertexIndex = PositionBuffer.Num() - 1;
}


FRuntimeMeshSectionProxy::FRuntimeMeshSectionProxy(ERHIFeatureLevel::Type InFeatureLevel, FRuntimeMeshSectionCreationParamsPtr CreationData)
	: FeatureLevel(InFeatureLevel)
	, UpdateFrequency(CreationData->UpdateFrequency)
	, bIsVisible(CreationData->bIsVisible)
	, bCastsShadow(CreationData->bCastsShadow)
{
	check(IsInRenderingThread());

	LODs.Empty();
	for (int32 Index = 0; Index < CreationData->LODs.Num(); Index++)
	{
		LODs.Emplace(InFeatureLevel, this, CreationData->UpdateFrequency, CreationData->LODs[Index].TangentsVertexBuffer.bUsingHighPrecision,
			CreationData->LODs[Index].UVsVertexBuffer.bUsingHighPrecision, CreationData->LODs[Index].UVsVertexBuffer.NumUVs);

		FRuntimeMeshSectionProxyLODData& LODData = LODs[LODs.Num() - 1];

		LODData.PositionBuffer.Reset(CreationData->LODs[0].PositionVertexBuffer.NumVertices);
		LODData.PositionBuffer.SetData(CreationData->LODs[0].PositionVertexBuffer.Data);

		LODData.TangentsBuffer.Reset(CreationData->LODs[0].TangentsVertexBuffer.NumVertices);
		LODData.TangentsBuffer.SetData(CreationData->LODs[0].TangentsVertexBuffer.Data);

		LODData.UVsBuffer.Reset(CreationData->LODs[0].UVsVertexBuffer.NumVertices);
		LODData.UVsBuffer.SetData(CreationData->LODs[0].UVsVertexBuffer.Data);

		LODData.ColorBuffer.Reset(CreationData->LODs[0].ColorVertexBuffer.NumVertices);
		LODData.ColorBuffer.SetData(CreationData->LODs[0].ColorVertexBuffer.Data);

		LODData.IndexBuffer.Reset(CreationData->LODs[0].IndexBuffer.b32BitIndices ? 4 : 2, CreationData->LODs[0].IndexBuffer.NumIndices, UpdateFrequency);
		LODData.IndexBuffer.SetData(CreationData->LODs[0].IndexBuffer.Data);

		LODData.AdjacencyIndexBuffer.Reset(CreationData->LODs[0].IndexBuffer.b32BitIndices ? 4 : 2, CreationData->LODs[0].AdjacencyIndexBuffer.NumIndices, UpdateFrequency);
		LODData.AdjacencyIndexBuffer.SetData(CreationData->LODs[0].AdjacencyIndexBuffer.Data);

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		if (CanRender())
		{
#endif
			FLocalVertexFactory::FDataType DataType;
			LODData.BuildVertexDataType(DataType);

			LODData.VertexFactory.Init(DataType);
			LODData.VertexFactory.InitResource();
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		}
#endif
	}
}

FRuntimeMeshSectionProxy::~FRuntimeMeshSectionProxy()
{

}

void FRuntimeMeshSectionProxy::EnsureHasLOD(int32 LODIndex)
{
	check(LODIndex >= 0 && LODIndex < RUNTIMEMESH_MAXLODS);
	while (!LODs.IsValidIndex(LODIndex))
	{
		LODs.Emplace(FeatureLevel, this, UpdateFrequency, LODs[0].TangentsBuffer.IsUsingHighPrecision(), LODs[0].UVsBuffer.IsUsingHighPrecision(), LODs[0].UVsBuffer.GetNumUVs());
	}
}

bool FRuntimeMeshSectionProxy::ShouldRender()
{
	return bIsVisible && CanRender();
}

bool FRuntimeMeshSectionProxy::CanRender()
{
	for (int32 Index = 0; Index < LODs.Num(); Index++)
	{
		if (LODs[Index].CanRender())
		{
			return true;
		}
	}
	return false;
}

bool FRuntimeMeshSectionProxy::WantsToRenderInStaticPath() const
{
	return UpdateFrequency == EUpdateFrequency::Infrequent;
}

bool FRuntimeMeshSectionProxy::CastsShadow() const
{
	return bCastsShadow;
}



void FRuntimeMeshSectionProxy::FinishUpdate_RenderThread(FRuntimeMeshSectionUpdateParamsPtr UpdateData)
{
	check(IsInRenderingThread());

	ERuntimeMeshBuffersToUpdate BuffersToUpdate = UpdateData->BuffersToUpdate;

	while (LODs.Num() <= UpdateData->LODIndex)
	{
		int32 CurrentIndex = LODs.Emplace(FeatureLevel, this, UpdateFrequency, UpdateData->TangentsVertexBuffer.bUsingHighPrecision,
			UpdateData->UVsVertexBuffer.bUsingHighPrecision, UpdateData->UVsVertexBuffer.NumUVs);


#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		if (CanRender())
		{
#endif
			FRuntimeMeshSectionProxyLODData& LODData = LODs[CurrentIndex];
			FLocalVertexFactory::FDataType DataType;
			LODData.BuildVertexDataType(DataType);

			LODData.VertexFactory.Init(DataType);
			LODData.VertexFactory.InitResource();
#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
		}
#endif
	}

	FRuntimeMeshSectionProxyLODData& LODData = LODs[UpdateData->LODIndex];

	// Update position buffer
	if (!!(BuffersToUpdate & ERuntimeMeshBuffersToUpdate::PositionBuffer))
	{
		LODData.PositionBuffer.Reset(UpdateData->PositionVertexBuffer.NumVertices);
		LODData.PositionBuffer.SetData(UpdateData->PositionVertexBuffer.Data);
	}

	// Update tangent buffer
	if (!!(BuffersToUpdate & ERuntimeMeshBuffersToUpdate::TangentBuffer))
	{
		LODData.TangentsBuffer.SetNum(UpdateData->TangentsVertexBuffer.NumVertices);
		LODData.TangentsBuffer.SetData(UpdateData->TangentsVertexBuffer.Data);
	}

	// Update uv buffer
	if (!!(BuffersToUpdate & ERuntimeMeshBuffersToUpdate::UVBuffer))
	{
		LODData.UVsBuffer.SetNum(UpdateData->UVsVertexBuffer.NumVertices);
		LODData.UVsBuffer.SetData(UpdateData->UVsVertexBuffer.Data);
	}

	// Update color buffer
	if (!!(BuffersToUpdate & ERuntimeMeshBuffersToUpdate::ColorBuffer))
	{
		LODData.ColorBuffer.SetNum(UpdateData->ColorVertexBuffer.NumVertices);
		LODData.ColorBuffer.SetData(UpdateData->ColorVertexBuffer.Data);
	}

	// Update index buffer
	if (!!(BuffersToUpdate & ERuntimeMeshBuffersToUpdate::IndexBuffer))
	{
		LODData.IndexBuffer.Reset(UpdateData->IndexBuffer.b32BitIndices ? 4 : 2, UpdateData->IndexBuffer.NumIndices, UpdateFrequency);
		LODData.IndexBuffer.SetData(UpdateData->IndexBuffer.Data);
	}

	// Update index buffer
	if (!!(BuffersToUpdate & ERuntimeMeshBuffersToUpdate::AdjacencyIndexBuffer))
	{
		LODData.AdjacencyIndexBuffer.Reset(UpdateData->AdjacencyIndexBuffer.b32BitIndices ? 4 : 2, UpdateData->AdjacencyIndexBuffer.NumIndices, UpdateFrequency);
		LODData.AdjacencyIndexBuffer.SetData(UpdateData->AdjacencyIndexBuffer.Data);
	}

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 19
	// If this platform uses manual vertex fetch, we need to update the SRVs
	if (RHISupportsManualVertexFetch(GMaxRHIShaderPlatform))
	{
		FLocalVertexFactory::FDataType DataType;
		LODData.BuildVertexDataType(DataType);

		LODData.VertexFactory.ReleaseResource();
		LODData.VertexFactory.Init(DataType);
		LODData.VertexFactory.InitResource();
	}
#endif
}

void FRuntimeMeshSectionProxy::FinishPropertyUpdate_RenderThread(FRuntimeMeshSectionPropertyUpdateParamsPtr UpdateData)
{
	// Copy visibility/shadow
	bIsVisible = UpdateData->bIsVisible;
	bCastsShadow = UpdateData->bCastsShadow;
}
