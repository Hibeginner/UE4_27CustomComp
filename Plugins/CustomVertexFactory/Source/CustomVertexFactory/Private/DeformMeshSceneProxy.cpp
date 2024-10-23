// Fill out your copyright notice in the Description page of Project Settings.


#include "DeformMeshSceneProxy.h"

/* Helper function that initializes a render resource if it's not initialized, or updates it otherwise*/
static inline void InitOrUpdateResource(FRenderResource* Resource)
{
	if (!Resource->IsInitialized())
	{
		Resource->InitResource();
	}
	else
	{
		Resource->UpdateRHI();
	}
}

/* 
 * Helper function that initializes the vertex buffers of the vertex factory's Data member from the static mesh vertex buffers
 * We're using this so we can initialize only the data that we're interested in.
*/
static void InitVertexFactoryData(FDeformMeshVertexFactory* VertexFactory, FStaticMeshVertexBuffers* VertexBuffers)
{
	ENQUEUE_RENDER_COMMAND(StaticMeshVertexBuffersLegacyInit)(
		[VertexFactory, VertexBuffers](FRHICommandListImmediate& RHICmdList)
		{
			//Initialize or update the RHI vertex buffers
			InitOrUpdateResource(&VertexBuffers->PositionVertexBuffer);
			InitOrUpdateResource(&VertexBuffers->StaticMeshVertexBuffer);

			//Use the RHI vertex buffers to create the needed Vertex stream components in an FDataType instance, and then set it as the data of the vertex factory
			FLocalVertexFactory::FDataType Data;
			VertexBuffers->PositionVertexBuffer.BindPositionVertexBuffer(VertexFactory, Data);
			VertexBuffers->StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(VertexFactory, Data);
			VertexFactory->SetData(Data);

			//Initalize the vertex factory using the data that we just set, this will call the InitRHI() method that we implemented in out vertex factory
			InitOrUpdateResource(VertexFactory);
		});
}

FDeformMeshSceneProxy::FDeformMeshSceneProxy(UDeformMeshComponent* Component): FPrimitiveSceneProxy(Component),
                                                                               MaterialRelevance(
	                                                                               Component->GetMaterialRelevance(
		                                                                               GetScene().GetFeatureLevel()))
{
	// Copy each section
	const uint16 NumSections = Component->DeformMeshSections.Num();

	//Initialize the array of trnasforms and the array of mesh sections proxies
	DeformTransforms.AddZeroed(NumSections);
	Sections.AddZeroed(NumSections);

	for (uint16 SectionIdx = 0; SectionIdx < NumSections; SectionIdx++)
	{
		const FDeformMeshSection& SrcSection = Component->DeformMeshSections[SectionIdx];
		{
			//Create a new mesh section proxy
			FDeformMeshSectionProxy* NewSection = new FDeformMeshSectionProxy(GetScene().GetFeatureLevel());

			//Get the needed data from the static mesh of the mesh section
			//We're assuming that there's only one LOD
			FStaticMeshLODResources& LODResource = SrcSection.StaticMesh->GetRenderData()->LODResources[0];

			FDeformMeshVertexFactory* VertexFactory = &NewSection->VertexFactory;
			//Initialize the vertex factory with the vertex data from the static mesh using the helper function defined above
			InitVertexFactoryData(VertexFactory, &(LODResource.VertexBuffers));

			//Initialize the additional data using setters (Transform Index and pointer to this scene proxy that holds reference to the structured buffer and its SRV
			VertexFactory->SetTransformIndex(SectionIdx);
			VertexFactory->SetSceneProxy(this);

			//Copy the indices from the static mesh index buffer and use it to initialize the mesh section proxy's index buffer
			{
				TArray<uint32> tmp_indices;
				LODResource.IndexBuffer.GetCopy(tmp_indices);
				NewSection->IndexBuffer.AppendIndices(tmp_indices.GetData(), tmp_indices.Num());
				//Initialize the render resource
				BeginInitResource(&NewSection->IndexBuffer);
			}

			//Fill the array of transforms with the transform matrix from each section
			DeformTransforms[SectionIdx] = SrcSection.DeformTransform;

			//Set the max vertex index for this mesh section
			NewSection->MaxVertexIndex = LODResource.VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

			//Get the material of this section
			NewSection->Material = Component->GetMaterial(SectionIdx);

			if (NewSection->Material == NULL)
			{
				NewSection->Material = UMaterial::GetDefaultMaterial(MD_Surface);
			}

			// Copy visibility info
			NewSection->bSectionVisible = SrcSection.bSectionVisible;

			// Save ref to new section
			Sections[SectionIdx] = NewSection;
		}
	}

	//Create the structured buffer only if we have at least one section
	if (NumSections > 0)
	{
		///////////////////////////////////////////////////////////////
		//// CREATING THE STRUCTURED BUFFER FOR THE DEFORM TRANSFORMS OF ALL THE SECTIONS
		//We'll use one structured buffer for all the mesh sections of the component

		//We first create a resource array to use it in the create info for initializing the structured buffer on creation
		TResourceArray<FMatrix>* ResourceArray = new TResourceArray<FMatrix>(true);
		FRHIResourceCreateInfo CreateInfo;
		ResourceArray->Append(DeformTransforms);
		CreateInfo.ResourceArray = ResourceArray;
		//Set the debug name so we can find the resource when debugging in RenderDoc
		CreateInfo.DebugName = TEXT("DeformMesh_TransformsSB");

		DeformTransformsSB = RHICreateStructuredBuffer(sizeof(FMatrix), NumSections * sizeof(FMatrix),
		                                               BUF_ShaderResource, CreateInfo);
		bDeformTransformsDirty = false;
		///////////////////////////////////////////////////////////////
		//// CREATING AN SRV FOR THE STRUCTUED BUFFER SO WA CAN USE IT AS A SHADER RESOURCE PARAMETER AND BIND IT TO THE VERTEX FACTORY
		DeformTransformsSRV = RHICreateShaderResourceView(DeformTransformsSB);

		///////////////////////////////////////////////////////////////
	}
}

FDeformMeshSceneProxy::~FDeformMeshSceneProxy()
{
	//For each section , release the render resources
	for (FDeformMeshSectionProxy* Section : Sections)
	{
		if (Section != nullptr)
		{
			Section->IndexBuffer.ReleaseResource();
			Section->VertexFactory.ReleaseResource();
			delete Section;
		}
	}

	//Release the structured buffer and the SRV
	DeformTransformsSB.SafeRelease();
	DeformTransformsSRV.SafeRelease();
}

void FDeformMeshSceneProxy::UpdateDeformTransformsSB_RenderThread()
{
	check(IsInRenderingThread());
	//Update the structured buffer only if it needs update
	if (bDeformTransformsDirty && DeformTransformsSB)
	{
		void* StructuredBufferData = RHILockStructuredBuffer(DeformTransformsSB, 0,
		                                                     DeformTransforms.Num() * sizeof(FMatrix), RLM_WriteOnly);
		FMemory::Memcpy(StructuredBufferData, DeformTransforms.GetData(), DeformTransforms.Num() * sizeof(FMatrix));
		RHIUnlockStructuredBuffer(DeformTransformsSB);
		bDeformTransformsDirty = false;
	}
}

void FDeformMeshSceneProxy::UpdateDeformTransform_RenderThread(int32 SectionIndex, FMatrix Transform)
{
	check(IsInRenderingThread());
	if (SectionIndex < Sections.Num() &&
		Sections[SectionIndex] != nullptr)
	{
		DeformTransforms[SectionIndex] = Transform;
		//Mark as dirty
		bDeformTransformsDirty = true;
	}
}

void FDeformMeshSceneProxy::SetSectionVisibility_RenderThread(int32 SectionIndex, bool bNewVisibility)
{
	check(IsInRenderingThread());

	if (SectionIndex < Sections.Num() &&
		Sections[SectionIndex] != nullptr)
	{
		Sections[SectionIndex]->bSectionVisible = bNewVisibility;
	}
}

void FDeformMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
                                                   const FSceneViewFamily& ViewFamily, uint32 VisibilityMap,
                                                   FMeshElementCollector& Collector) const
{
	// Set up wireframe material (if needed)
	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

	FColoredMaterialRenderProxy* WireframeMaterialInstance = NULL;
	if (bWireframe)
	{
		WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f)
		);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
	}

	// Iterate over sections
	for (const FDeformMeshSectionProxy* Section : Sections)
	{
		if (Section != nullptr && Section->bSectionVisible)
		{
			//Get the section's materil, or the wireframe material if we're rendering in wireframe mode
			FMaterialRenderProxy* MaterialProxy = bWireframe
				                                      ? WireframeMaterialInstance
				                                      : Section->Material->GetRenderProxy();

			// For each view..
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				//Check if our mesh is visible from this view
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					// Allocate a mesh batch and get a ref to the first element
					FMeshBatch& Mesh = Collector.AllocateMesh();
					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					//Fill this batch element with the mesh section's render data
					BatchElement.IndexBuffer = &Section->IndexBuffer;
					Mesh.bWireframe = bWireframe;
					Mesh.VertexFactory = &Section->VertexFactory;
					Mesh.MaterialRenderProxy = MaterialProxy;

					//The LocalVertexFactory uses a uniform buffer to pass primitive data like the local to world transform for this frame and for the previous one
					//Most of this data can be fetched using the helper function below
					bool bHasPrecomputedVolumetricLightmap;
					FMatrix PreviousLocalToWorld;
					int32 SingleCaptureIndex;
					bool bOutputVelocity;
					GetScene().GetPrimitiveUniformShaderParameters_RenderThread(
						GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld,
						SingleCaptureIndex, bOutputVelocity);
					//Allocate a temporary primitive uniform buffer, fill it with the data and set it in the batch element
					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<
						FDynamicPrimitiveUniformBuffer>();
					DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(),
					                                  GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap,
					                                  DrawsVelocity(), bOutputVelocity);
					BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
					BatchElement.PrimitiveIdMode = PrimID_DynamicPrimitiveShaderData;

					//Additional data 
					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = Section->IndexBuffer.GetNumIndices() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = Section->MaxVertexIndex;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = false;

					//Add the batch to the collector
					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}
	}
}

FPrimitiveViewRelevance FDeformMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	Result.bRenderInMainPass = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;
	return Result;
}

bool FDeformMeshSceneProxy::CanBeOccluded() const
{
	return !MaterialRelevance.bDisableDepthTest;
}

uint32 FDeformMeshSceneProxy::GetMemoryFootprint() const
{
	return (sizeof(*this) + GetAllocatedSize());
}

uint32 FDeformMeshSceneProxy::GetAllocatedSize() const
{
	return (FPrimitiveSceneProxy::GetAllocatedSize());
}

FShaderResourceViewRHIRef& FDeformMeshSceneProxy::GetDeformTransformsSRV()
{
	return DeformTransformsSRV;
}

SIZE_T FDeformMeshSceneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}
