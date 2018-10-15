// Copyright 2016-2018 Chris Conway (Koderz). All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Components/MeshComponent.h"
#include "RuntimeMeshRendering.h"
#include "RuntimeMeshUpdateCommands.h"


struct FRuntimeMeshSectionNullBufferElement
{
	FPackedNormal Normal;
	FPackedNormal Tangent;
	FColor Color;
	FVector2DHalf UV0;

	FRuntimeMeshSectionNullBufferElement()
		: Normal(FVector(0.0f, 0.0f, 1.0f))
		, Tangent(FVector(1.0f, 0.0f, 0.0f))
		, Color(FColor::Transparent)
		, UV0(FVector2D::ZeroVector)
	{ }
};

class FRuntimeMeshSectionProxyLODData
{
public:

	/** Vertex factory for this section */
	FRuntimeMeshVertexFactory VertexFactory;

	/** Vertex buffer containing the positions for this section */
	FRuntimeMeshPositionVertexBuffer PositionBuffer;

	/** Vertex buffer containing the tangents for this section */
	FRuntimeMeshTangentsVertexBuffer TangentsBuffer;

	/** Vertex buffer containing the UVs for this section */
	FRuntimeMeshUVsVertexBuffer UVsBuffer;

	/** Vertex buffer containing the colors for this section */
	FRuntimeMeshColorVertexBuffer ColorBuffer;

	/** Index buffer for this section */
	FRuntimeMeshIndexBuffer IndexBuffer;

	/** Index buffer for this section */
	FRuntimeMeshIndexBuffer AdjacencyIndexBuffer;

	FRuntimeMeshSectionProxyLODData(ERHIFeatureLevel::Type InFeatureLevel, FRuntimeMeshSectionProxy* InSectionParent, EUpdateFrequency UpdateFrequency, bool bUseHighPrecisionTangents, bool bUseHighPrecisionUVs, int32 NumUVs)
		: VertexFactory(InFeatureLevel, InSectionParent)
		, PositionBuffer(UpdateFrequency)
		, TangentsBuffer(UpdateFrequency, bUseHighPrecisionTangents)
		, UVsBuffer(UpdateFrequency, bUseHighPrecisionUVs, NumUVs)
		, ColorBuffer(UpdateFrequency)
		, IndexBuffer(UpdateFrequency, false)
		, AdjacencyIndexBuffer(UpdateFrequency, false)
	{

	}

	~FRuntimeMeshSectionProxyLODData()
	{
		check(IsInRenderingThread());

		PositionBuffer.ReleaseResource();
		TangentsBuffer.ReleaseResource();
		UVsBuffer.ReleaseResource();
		ColorBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		AdjacencyIndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}


	bool CanRender();
	FRuntimeMeshVertexFactory* GetVertexFactory() { return &VertexFactory; }
	void BuildVertexDataType(FLocalVertexFactory::FDataType& DataType);



	void CreateMeshBatch(FMeshBatch& MeshBatch, bool bCastsShadow, bool bWantsAdjacencyInfo);
};


class FRuntimeMeshSectionProxy : public TSharedFromThis<FRuntimeMeshSectionProxy, ESPMode::NotThreadSafe>
{
	ERHIFeatureLevel::Type FeatureLevel;

	/** Update frequency of this section */
	EUpdateFrequency UpdateFrequency;

	TArray<FRuntimeMeshSectionProxyLODData, TInlineAllocator<RUNTIMEMESH_MAXLODS>> LODs;

	/** Whether this section is currently visible */
	bool bIsVisible;

	/** Should this section cast a shadow */
	bool bCastsShadow;

public:
	FRuntimeMeshSectionProxy(ERHIFeatureLevel::Type InFeatureLevel, FRuntimeMeshSectionCreationParamsPtr CreationData);

	~FRuntimeMeshSectionProxy();

	void EnsureHasLOD(int32 LODIndex);
	int32 NumLODs() const { return LODs.Num(); }

	FRuntimeMeshSectionProxyLODData* GetLOD(int32 LODIndex) { return &LODs[LODIndex]; }

	bool ShouldRender();
	bool CanRender();

	bool WantsToRenderInStaticPath() const;

	bool CastsShadow() const;



	void FinishUpdate_RenderThread(FRuntimeMeshSectionUpdateParamsPtr UpdateData);

	void FinishPropertyUpdate_RenderThread(FRuntimeMeshSectionPropertyUpdateParamsPtr UpdateData);
};


