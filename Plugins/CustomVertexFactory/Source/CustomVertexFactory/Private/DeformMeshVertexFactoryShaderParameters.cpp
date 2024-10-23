// Fill out your copyright notice in the Description page of Project Settings.


#include "DeformMeshVertexFactoryShaderParameters.h"
#include "DeformMeshSceneProxy.h"
#include "DeformMeshVertexFactory.h"


FDeformMeshVertexFactoryShaderParameters::FDeformMeshVertexFactoryShaderParameters()
{
}

FDeformMeshVertexFactoryShaderParameters::~FDeformMeshVertexFactoryShaderParameters()
{
}

void FDeformMeshVertexFactoryShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	TransformIndex.Bind(ParameterMap, TEXT("DMTransformIndex"), SPF_Optional);
	TransformsSRV.Bind(ParameterMap, TEXT("DMTransforms"), SPF_Optional);
}

void FDeformMeshVertexFactoryShaderParameters::GetElementShaderBindings(const FSceneInterface* Scene,
                                                                        const FSceneView* View,
                                                                        const FMeshMaterialShader* Shader,
                                                                        const EVertexInputStreamType InputStreamType,
                                                                        ERHIFeatureLevel::Type FeatureLevel,
                                                                        const FVertexFactory* VertexFactory,
                                                                        const FMeshBatchElement& BatchElement,
                                                                        FMeshDrawSingleShaderBindings& ShaderBindings,
                                                                        FVertexInputStreamArray& VertexStreams) const
{
	if (BatchElement.bUserDataIsColorVertexBuffer)
	{
		const auto* LocalVertexFactory = static_cast<const FLocalVertexFactory*>(VertexFactory);
		FColorVertexBuffer* OverrideColorVertexBuffer = (FColorVertexBuffer*)BatchElement.UserData;
		check(OverrideColorVertexBuffer);

		if (!LocalVertexFactory->SupportsManualVertexFetch(FeatureLevel))
		{
			LocalVertexFactory->GetColorOverrideStream(OverrideColorVertexBuffer, VertexStreams);
		}
	}
	const FDeformMeshVertexFactory* DeformMeshVertexFactory = static_cast<const FDeformMeshVertexFactory*>(
		VertexFactory);
	const uint32 Index = DeformMeshVertexFactory->TransformIndex;
	ShaderBindings.Add(TransformIndex, Index);
	ShaderBindings.Add(TransformsSRV, DeformMeshVertexFactory->SceneProxy->GetDeformTransformsSRV());
}

IMPLEMENT_TYPE_LAYOUT(FDeformMeshVertexFactoryShaderParameters);
IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FDeformMeshVertexFactory, SF_Vertex, FDeformMeshVertexFactoryShaderParameters);
