#pragma once

#include "RenderGraph.h"
#include "RHI.h"
#include "CoreMinimal.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RHIStaticStates.h"
#include "RenderResource.h"
#include "Engine/TextureRenderTarget2D.h"
//
// #include "PipelineStateCache.h"
//
// #include "GlobalShader.h"
// #include "RenderGraphUtils.h"
// #include "RenderTargetPool.h"
// #include "RHIStaticStates.h"
// #include "ShaderParameterUtils.h"
// #include "PixelShaderUtils.h"


#include  "DatProcessor.h"
#include "Wil21Rendering.generated.h"
// 


class FRHICommandListImmediate;
struct IPooledRenderTarget;

#define SPECTRUM_SIZE 11

struct FSpectrum  
{  
	double Values[SPECTRUM_SIZE];  
};




UCLASS(MinimalAPI, meta = (ScriptName = "Wil21Rendering"))
class UWil21RenderingBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Wil21Rendering", meta = (WorldContext = "WorldContextObject"))
	static void UseRDGComputeWil21(const UObject* WorldContextObject, const FShaderPackedData& ShaderPackedData, const FShaderControlData& ShaderControlData, UTextureRenderTarget2D* OutputRenderTarget);

};


class FWil21RDGComputeShader : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FWil21RDGComputeShader);
	SHADER_USE_PARAMETER_STRUCT(FWil21RDGComputeShader, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		// Shader control data
		SHADER_PARAMETER(int32, Resolution)
		SHADER_PARAMETER(float, SolarElevation)
		SHADER_PARAMETER(float, SolarAzimuth)
		SHADER_PARAMETER(float, Albedo)
		SHADER_PARAMETER(float, Visibility)
		SHADER_PARAMETER(float, Altitude)
	
		// RadianceMetadata  
		 SHADER_PARAMETER(int32, Rank)  
		 SHADER_PARAMETER(int32, SunOffset)  
		 SHADER_PARAMETER(int32, SunStride)  
		 SHADER_PARAMETER(int32, ZenithOffset)  
		 SHADER_PARAMETER(int32, ZenithStride)  
		 SHADER_PARAMETER(int32, EmphOffset)  
		 SHADER_PARAMETER(int32, TotalCoefsSingleConfig)  
		 SHADER_PARAMETER(int32, TotalCoefsAllConfigs)  
		 SHADER_PARAMETER(int32, SunBreaksSize)  
		 SHADER_PARAMETER(int32, ZenithBreaksSize)  
		 SHADER_PARAMETER(int32, EmphBreaksSize)  

		 // RadianceData  
		 SHADER_PARAMETER(int32, VisibilitiesRadSize)  
		 SHADER_PARAMETER(int32, AlbedosRadSize)  
		 SHADER_PARAMETER(int32, AltitudesRadSize)  
		 SHADER_PARAMETER(int32, ElevationsRadSize)
		 SHADER_PARAMETER(int32, DataRadSize)  

		 // Meta data buffers  
		 SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<DoublePacked>, SunBreaks)  
		 SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<DoublePacked>, ZenithBreaks)  
		 SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<DoublePacked>, EmphBreaks)  

		 // Radiance data buffers  
		 SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<DoublePacked>, VisibilitiesRad)  
		 SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<DoublePacked>, AlbedosRad)  
		 SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<DoublePacked>, AltitudesRad)  
		 SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<DoublePacked>, ElevationsRad)  
		 SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<uint32>, DataRad)  

		 // SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<uint32>, SpectralResponse) 
		 // Output buffer  
		 SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<FSpectrum>, OutputBuffer)
	     SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutTexture)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		// return RHISupportsComputeShaders(Parameters.Platform);
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);; // 
	}
};

class FSpectrumToColorRDGCS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSpectrumToColorRDGCS);
	SHADER_USE_PARAMETER_STRUCT(FSpectrumToColorRDGCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		// Input data buffer
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FSpectrum>, InputBuffer)
		SHADER_PARAMETER_RDG_BUFFER_SRV(Buffer<uint32>, SpectralResponseBuffer) 
		// Output texure
	    SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutTexture)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		// return RHISupportsComputeShaders(Parameters.Platform);
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM6);; // 
	}
};



void RDGComputeWil21Buffer(FRHICommandListImmediate& RHIImmCmdList, const FShaderPackedData& ShaderPackedData, const FShaderControlData& ShaderControlData, int32 OutputSize, int32 TextureSize, TRefCountPtr<FRDGPooledBuffer> DataRadPooledBuffer, FTexture2DRHIRef RenderTargetRHI);
////////////////////// Util functions //////////////////////
TArray<float> ConvertToFloat(const TArray<double>& DoubleArray);
FRDGBufferRef CreateRawBuffer(FRDGBuilder& GraphBuilder, const TCHAR* Name, const TArray<float>& Data);

