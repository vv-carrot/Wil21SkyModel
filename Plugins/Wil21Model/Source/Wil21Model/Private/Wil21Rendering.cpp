#include "Wil21Rendering.h"

#include "FemeerSurfelCacheDefinitions.h"
#include "Engine/TextureRenderTarget2D.h"

#include "PixelShaderUtils.h"

IMPLEMENT_GLOBAL_SHADER(FWil21RDGComputeShader, "/Wil21ModelShaders/Private/Wil21.usf", "Wil21CS1", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FSpectrumToColorRDGCS, "/Wil21ModelShaders/Private/SpectrumToColor.usf", "Wil21CS2", SF_Compute);

void UWil21RenderingBlueprintLibrary::UseRDGComputeWil21(const UObject* WorldContextObject, const FShaderPackedData& ShaderPackedData, const FShaderControlData& ShaderControlData, UTextureRenderTarget2D* OutputRenderTarget)
{

	check(IsInGameThread());
	
	int32 TextureSize = 1024;
	int32 OutputSize = TextureSize * TextureSize / 2;
	FTexture2DRHIRef RenderTargetRHI = OutputRenderTarget->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	ENQUEUE_RENDER_COMMAND(CaptureCommand)
		(
			[ShaderPackedData, ShaderControlData, OutputSize, TextureSize, RenderTargetRHI](FRHICommandListImmediate& RHICmdList) {
				RDGComputeWil21Buffer(RHICmdList, ShaderPackedData, ShaderControlData, OutputSize, TextureSize, RenderTargetRHI);
			});
}
TArray<float> ConvertToFloat(const TArray<double>& DoubleArray)  
{  
	TArray<float> FloatArray;  
	FloatArray.Reserve(DoubleArray.Num());  
	for (const double& Value : DoubleArray)  
	{  
		FloatArray.Add(static_cast<float>(Value));  
	}  
	return FloatArray;  
}  
FRDGBufferRef CreateRawBuffer(FRDGBuilder& GraphBuilder, const TCHAR* Name, const TArray<uint32>& Data)  
{  
	uint32 SizeInBytes = Data.Num() * sizeof(uint32);  
	FRDGBufferDesc Desc = FRDGBufferDesc::CreateBufferDesc(sizeof(uint32), Data.Num());  
	FRDGBufferRef Buffer = GraphBuilder.CreateBuffer(Desc, Name);  

	GraphBuilder.QueueBufferUpload(Buffer, Data.GetData(), SizeInBytes);  

	return Buffer;  
}
void RDGComputeWil21Buffer(FRHICommandListImmediate& RHIImmCmdList, const FShaderPackedData& ShaderPackedData, const FShaderControlData& ShaderControlData, int32 OutputSize, int32 TextureSize, FTexture2DRHIRef RenderTargetRHI)
{
	check(IsInRenderingThread());
	// RDG Begin  
	FRDGBuilder GraphBuilder(RHIImmCmdList);

	TArray<FSpectrum> SpectrumData;
	SpectrumData.SetNum(OutputSize);
	for (int32 i = 0; i < OutputSize; i++)
	{
		for (int32 c = 0; c < 11; c++)
			SpectrumData[i].Values[c] = 0.5f;
	}
	
	FRDGBufferRef SpectrumBuffer = CreateStructuredBuffer(
		GraphBuilder,
		TEXT("SpectrumBuffer"),
		sizeof(FSpectrum),  
		SpectrumData.Num(),  
		SpectrumData.GetData(),  
		sizeof(FSpectrum) * OutputSize
	);

	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
	// Calculate for spectrum buffer and rgb
	{

		// Setup Parameters  
		FWil21RDGComputeShader::FParameters* Parameters = GraphBuilder.AllocParameters<FWil21RDGComputeShader::FParameters>();  
		
		// Set RadianceMetadata parameters  
		Parameters->Rank = ShaderPackedData.Rank;  
		Parameters->SunOffset = ShaderPackedData.SunOffset;  
		Parameters->SunStride = ShaderPackedData.SunStride;  
		Parameters->ZenithOffset = ShaderPackedData.ZenithOffset;  
		Parameters->ZenithStride = ShaderPackedData.ZenithStride;  
		Parameters->EmphOffset = ShaderPackedData.EmphOffset;  
		Parameters->TotalCoefsSingleConfig = ShaderPackedData.TotalCoefsSingleConfig;  
		Parameters->TotalCoefsAllConfigs = ShaderPackedData.TotalCoefsAllConfigs;

		Parameters->ZenithBreaksSize = ShaderPackedData.ZenithBreaks.Num();
		Parameters->SunBreaksSize = ShaderPackedData.SunBreaks.Num();
		Parameters->EmphBreaksSize = ShaderPackedData.EmphBreaks.Num();
		Parameters->VisibilitiesRadSize = ShaderPackedData.VisibilitiesRad.Num();
		Parameters->AlbedosRadSize = ShaderPackedData.AlbedosRad.Num();
		Parameters->AltitudesRadSize = ShaderPackedData.AltitudesRad.Num();
		Parameters->ElevationsRadSize = ShaderPackedData.ElevationsRad.Num();
		Parameters->DataRadSize = ShaderPackedData.DataRadSize;
		
		Parameters->Resolution = ShaderControlData.Resolution;
		Parameters->SolarElevation = ShaderControlData.SolarElevation;
		Parameters->SolarAzimuth = ShaderControlData.SolarAzimuth;
		Parameters->Albedo = ShaderControlData.Albedo;
		Parameters->Visibility = ShaderControlData.Visibility;
		Parameters->Altitude = ShaderControlData.Altitude;

		// vector<>
		// Create structured buffers  
		FRDGBufferRef SunBreaksBuffer = CreateStructuredBuffer(
			GraphBuilder,
			TEXT("SunBreaksBuffer"),
			sizeof(FShaderPackedData),  
			ShaderPackedData.SunBreaks.Num(),  
			ShaderPackedData.SunBreaks.GetData(),  
			sizeof(FShaderPackedData) * ShaderPackedData.SunBreaks.Num()
		);
		FRDGBufferRef ZenithBreaksBuffer = CreateStructuredBuffer(
			GraphBuilder,
			TEXT("ZenithBreaksBuffer"),
			sizeof(FShaderPackedData),  
			ShaderPackedData.ZenithBreaks.Num(),  
			ShaderPackedData.ZenithBreaks.GetData(),  
			sizeof(FShaderPackedData) * ShaderPackedData.ZenithBreaks.Num()
		);
		FRDGBufferRef EmphBreaksBuffer = CreateStructuredBuffer(
			GraphBuilder,
			TEXT("EmphBreaksBuffer"),
			sizeof(FShaderPackedData),  
			ShaderPackedData.EmphBreaks.Num(),  
			ShaderPackedData.EmphBreaks.GetData(),  
			sizeof(FShaderPackedData) * ShaderPackedData.EmphBreaks.Num()
		);
		FRDGBufferRef VisibilitiesRadBuffer = CreateStructuredBuffer(
			GraphBuilder,
			TEXT("VisibilitiesRadBuffer"),
			sizeof(FShaderPackedData),  
			ShaderPackedData.VisibilitiesRad.Num(),  
			ShaderPackedData.VisibilitiesRad.GetData(),  
			sizeof(FShaderPackedData) * ShaderPackedData.VisibilitiesRad.Num()
		);
		FRDGBufferRef AlbedosRadBuffer = CreateStructuredBuffer(
			GraphBuilder,
			TEXT("AlbedosRadBuffer"),
			sizeof(FShaderPackedData),  
			ShaderPackedData.AlbedosRad.Num(),  
			ShaderPackedData.AlbedosRad.GetData(),  
			sizeof(FShaderPackedData) * ShaderPackedData.AlbedosRad.Num()
		);
		FRDGBufferRef AltitudesRadBuffer = CreateStructuredBuffer(
			GraphBuilder,
			TEXT("AltitudesRadBuffer"),
			sizeof(FShaderPackedData),  
			ShaderPackedData.AltitudesRad.Num(),  
			ShaderPackedData.AltitudesRad.GetData(),  
			sizeof(FShaderPackedData) * ShaderPackedData.AltitudesRad.Num()
		);
		FRDGBufferRef ElevationsRadBuffer = CreateStructuredBuffer(
			GraphBuilder,
			TEXT("ElevationsRadBuffer"),
			sizeof(FShaderPackedData),  
			ShaderPackedData.ElevationsRad.Num(),  
			ShaderPackedData.ElevationsRad.GetData(),  
			sizeof(FShaderPackedData) * ShaderPackedData.ElevationsRad.Num()
		);

		FRDGBufferRef DataRadBuffer = CreateRawBuffer(GraphBuilder, TEXT("DataRad"), ShaderPackedData.DataRad);  

		// Set parameters  
		Parameters->SunBreaks = GraphBuilder.CreateSRV(SunBreaksBuffer, PF_Unknown);  
		Parameters->ZenithBreaks = GraphBuilder.CreateSRV(ZenithBreaksBuffer, PF_Unknown);  
		Parameters->EmphBreaks = GraphBuilder.CreateSRV(EmphBreaksBuffer, PF_Unknown);  
		Parameters->VisibilitiesRad = GraphBuilder.CreateSRV(VisibilitiesRadBuffer, PF_Unknown);  
		Parameters->AlbedosRad = GraphBuilder.CreateSRV(AlbedosRadBuffer, PF_Unknown);  
		Parameters->AltitudesRad = GraphBuilder.CreateSRV(AltitudesRadBuffer, PF_Unknown);  
		Parameters->ElevationsRad = GraphBuilder.CreateSRV(ElevationsRadBuffer, PF_Unknown);
		
		Parameters->DataRad = GraphBuilder.CreateSRV(DataRadBuffer, PF_R32_UINT);

		
		// FRDGBufferRef SpectralResponseData = CreateRawBuffer(GraphBuilder, TEXT("SpectralResponse"), ShaderPackedData.SpectralResponse); 
		// Parameters->SpectralResponse = GraphBuilder.CreateSRV(SpectralResponseData, PF_R32_UINT);


	// Set output buffer  
	Parameters->OutputBuffer = GraphBuilder.CreateUAV(SpectrumBuffer, PF_Unknown);

	const FRDGTextureDesc& RenderTargetDesc = FRDGTextureDesc::Create2D(RenderTargetRHI->GetSizeXY(),RenderTargetRHI->GetFormat(), FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef RDGRenderTarget = GraphBuilder.CreateTexture(RenderTargetDesc, TEXT("RDGRenderTarget"));

	FRDGTextureUAVDesc UAVDesc(RDGRenderTarget);
	Parameters->OutTexture = GraphBuilder.CreateUAV(UAVDesc);


	
	// Get ComputeShader From GlobalShaderMap
	const ERHIFeatureLevel::Type FeatureLevel = ERHIFeatureLevel::SM6;
	FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef<FWil21RDGComputeShader> ComputeShader(GlobalShaderMap);

	// Compute Thread Group Count
	FIntVector ThreadGroupCount(
		TextureSize / 32,
		TextureSize / 2 / 32,
		1);
	
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("Wil21RDGCompute"),
		Parameters,
		ERDGPassFlags::Compute,
		[Parameters, ComputeShader, ThreadGroupCount](FRHICommandList& RHICmdList) {
			FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *Parameters, ThreadGroupCount);
		});
		GraphBuilder.QueueTextureExtraction(RDGRenderTarget, &PooledRenderTarget);
	}
	
	GraphBuilder.Execute();
	RHIImmCmdList.CopyTexture(PooledRenderTarget->GetRHI()->GetTexture2D(), RenderTargetRHI->GetTexture2D(), FRHICopyTextureInfo());
}