#include "DataProcessorActor.h"
#include "HAL/PlatformFilemanager.h"  
#include "Misc/FileHelper.h"
  

void ADataProcessor::ReadRadianceFile(IFileHandle* Handle, double SingleVisibility, FRadianceData& Result)
{
    // Read metadata  
    int VisibilityCount = 0;  
    Handle->Read((uint8*)&VisibilityCount, sizeof(int));  

    TArray<double> VisibilitiesRadInFile;  
    VisibilitiesRadInFile.SetNum(VisibilityCount);  
    Handle->Read((uint8*)VisibilitiesRadInFile.GetData(), sizeof(double) * VisibilityCount);  

    // Process visibilities similar to the original function  
    int SkippedVisibilities = 0;  
    if (SingleVisibility <= 0.0 || VisibilityCount <= 1)  
    {  
        Result.VisibilitiesRad = VisibilitiesRadInFile;  
    }  
    else  
    {  
        if (SingleVisibility <= VisibilitiesRadInFile[0])  
        {  
            Result.VisibilitiesRad.Add(VisibilitiesRadInFile[0]);  
        }  
        else if (SingleVisibility >= VisibilitiesRadInFile.Last())  
        {  
            Result.VisibilitiesRad.Add(VisibilitiesRadInFile.Last());  
            SkippedVisibilities = VisibilitiesRadInFile.Num() - 1;  
        }  
        else  
        {  
            int VisIdx = 0;  
            while (SingleVisibility >= VisibilitiesRadInFile[VisIdx])  
            {  
                ++VisIdx;  
            }  
            Result.VisibilitiesRad.Add(VisibilitiesRadInFile[VisIdx - 1]);  
            Result.VisibilitiesRad.Add(VisibilitiesRadInFile[VisIdx]);  
            SkippedVisibilities = VisIdx - 1;  
        } 
    }  

    // Read other metadata  
    int AlbedoCount = 0;  
    Handle->Read((uint8*)&AlbedoCount, sizeof(int));  
    Result.AlbedosRad.SetNum(AlbedoCount);  
    Handle->Read((uint8*)Result.AlbedosRad.GetData(), sizeof(double) * AlbedoCount);

    // Read altitudes, elevations, channels, etc.  
    int AltitudeCount = 0;  
    Handle->Read((uint8*)&AltitudeCount, sizeof(int));  
    if (AltitudeCount < 1)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid altitude count"));

        return;
    }  

    Result.AltitudesRad.SetNum(AltitudeCount);  
    Handle->Read((uint8*)Result.AltitudesRad.GetData(), sizeof(double) * AltitudeCount);  

    int ElevationCount = 0;  
    Handle->Read((uint8*)&ElevationCount, sizeof(int));  
    if (ElevationCount < 1)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid elevation count"));

        return;
    }  

    Result.ElevationsRad.SetNum(ElevationCount);  
    Handle->Read((uint8*)Result.ElevationsRad.GetData(), sizeof(double) * ElevationCount);  

    Handle->Read((uint8*)&Result.Channels, sizeof(int));  
    if (Result.Channels < 1)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid channel count"));

        return;
    }  

    Handle->Read((uint8*)&Result.ChannelStart, sizeof(double));  
    if (Result.ChannelStart < 0)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid channel start"));

        return;
    }  

    Handle->Read((uint8*)&Result.ChannelWidth, sizeof(double));  
    if (Result.ChannelWidth <= 0)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid channel width"));

        return;
    }  
    
    // Calculate totalConfigs, skippedConfigsBegin, skippedConfigsEnd  
    int TotalConfigs = Result.Channels * Result.ElevationsRad.Num() * Result.AltitudesRad.Num() * Result.AlbedosRad.Num() * Result.VisibilitiesRad.Num();  

    int SkippedConfigsBegin = Result.Channels * Result.ElevationsRad.Num() * Result.AltitudesRad.Num() * Result.AlbedosRad.Num() * SkippedVisibilities;  

    int SkippedConfigsEnd = Result.Channels * Result.ElevationsRad.Num() * Result.AltitudesRad.Num() * Result.AlbedosRad.Num() *   
        (VisibilitiesRadInFile.Num() - SkippedVisibilities - Result.VisibilitiesRad.Num());  

    // Read sun, zenith, and emph breaks  
    Handle->Read((uint8*)&Result.MetadataRad.Rank, sizeof(int));  
    if (Result.MetadataRad.Rank < 1)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid rank"));

        return;
    }  

    int SunBreaksCount = 0;  
    Handle->Read((uint8*)&SunBreaksCount, sizeof(int));  
    if (SunBreaksCount < 2)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid sun breaks count"));

        return;
    }  

    Result.MetadataRad.SunBreaks.SetNum(SunBreaksCount);  
    Handle->Read((uint8*)Result.MetadataRad.SunBreaks.GetData(), sizeof(double) * SunBreaksCount);  

    int ZenithBreaksCount = 0;  
    Handle->Read((uint8*)&ZenithBreaksCount, sizeof(int));  
    if (ZenithBreaksCount < 2)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid zenith breaks count"));

        return;
    }  

    Result.MetadataRad.ZenithBreaks.SetNum(ZenithBreaksCount);  
    Handle->Read((uint8*)Result.MetadataRad.ZenithBreaks.GetData(), sizeof(double) * ZenithBreaksCount);  

    int EmphBreaksCount = 0;  
    Handle->Read((uint8*)&EmphBreaksCount, sizeof(int));  
    if (EmphBreaksCount < 2)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid emph breaks count"));

        return;
    }  

    Result.MetadataRad.EmphBreaks.SetNum(EmphBreaksCount);  
    Handle->Read((uint8*)Result.MetadataRad.EmphBreaks.GetData(), sizeof(double) * EmphBreaksCount);  

    // Calculate offsets and strides  
    Result.MetadataRad.SunOffset = 0;  
    Result.MetadataRad.SunStride = Result.MetadataRad.SunBreaks.Num() + Result. MetadataRad.ZenithBreaks.Num();  

    Result.MetadataRad.ZenithOffset = Result.MetadataRad.SunOffset + Result.MetadataRad.SunBreaks.Num();  
    Result.MetadataRad.ZenithStride = Result.MetadataRad.SunStride;  

    Result.MetadataRad.EmphOffset = Result.MetadataRad.SunOffset + Result.MetadataRad.Rank * Result.MetadataRad.SunStride;  

    Result.MetadataRad.TotalCoefsSingleConfig = Result.MetadataRad.EmphOffset + Result.MetadataRad.EmphBreaks.Num();  
    Result.MetadataRad.TotalCoefsAllConfigs = Result.MetadataRad.TotalCoefsSingleConfig * TotalConfigs;

    // Read data  
    int Offset = 0;  
    Result.DataRad.SetNum(Result.MetadataRad.TotalCoefsAllConfigs);  

    TArray<uint16> RadianceTemp;  
    RadianceTemp.SetNum(FMath::Max3(Result.MetadataRad.SunBreaks.Num(), Result.MetadataRad.ZenithBreaks.Num(), Result.MetadataRad.EmphBreaks.Num()));  

    int64 OneConfigByteCount = ((Result.MetadataRad.SunBreaks.Num() + Result.MetadataRad.ZenithBreaks.Num()) * sizeof(uint16) + sizeof(double)) * Result.MetadataRad.Rank + Result.MetadataRad.EmphBreaks.Num() * sizeof(uint16);  

    // Skip configurations if necessary  
    Handle->Seek(Handle->Tell() + OneConfigByteCount * SkippedConfigsBegin);  

    // Read configurations  
    for (int Con = 0; Con < TotalConfigs; ++Con)  
    {  
        for (int R = 0; R < Result.MetadataRad.Rank; ++R)  
        {  
            // Read sun params  
            Handle->Read((uint8*)RadianceTemp.GetData(), sizeof(uint16) * Result.MetadataRad.SunBreaks.Num());  
            for (int I = 0; I < Result.MetadataRad.SunBreaks.Num(); ++I)  
            {  
                Result.DataRad[Offset++] = DoubleFromHalf(RadianceTemp[I]);  
            }  

            // Read zenith scale and params  
            double ZenithScale;  
            Handle->Read((uint8*)&ZenithScale, sizeof(double));  
            Handle->Read((uint8*)RadianceTemp.GetData(), sizeof(uint16) * Result.MetadataRad.ZenithBreaks.Num());  
            for (int I = 0; I < Result.MetadataRad.ZenithBreaks.Num(); ++I)  
            {  
                Result.DataRad[Offset++] = DoubleFromHalf(RadianceTemp[I]) / ZenithScale;  
            }  
        }  

        // Read emphasize params  
        Handle->Read((uint8*)RadianceTemp.GetData(), sizeof(uint16) * Result.MetadataRad.EmphBreaks.Num());  
        for (int I = 0; I < Result.MetadataRad.EmphBreaks.Num(); ++I)  
        {  
            Result.DataRad[Offset++] = DoubleFromHalf(RadianceTemp[I]);  
        }  
    }  

    // Skip remaining configurationsHandle->Seek(Handle->Tell() + OneConfigByteCount * SkippedConfigsEnd);
}

ADataProcessor::ADataProcessor()  
{  
    PrimaryActorTick.bCanEverTick = true;
    // OnVariableChangedDelegate.AddDynamic(this, &ADataProcessor::OnVariableChanged);
    // Create render target
    // if(!OutputRenderTarget)
    // {
    //     OutputRenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("OutputRenderTarget"));
    //     OutputRenderTarget->InitCustomFormat(1024, 512, PF_R8G8B8A8, false);
    //     OutputRenderTarget->UpdateResourceImmediate();
    // }

    ReadDatFileFromContentFolder(TEXT("SkyModelDatasetGround.dat"), 0.0);
}

void ADataProcessor::ReadDatFileFromContentFolder(const FString& FileName, double SingleVisibility)  
{  
    FString FilePath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("Wil21Model"), TEXT("Content"), FileName);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();  
    IFileHandle* Handle = PlatformFile.OpenRead(*FilePath);  

    if (!Handle)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Failed to open file: %s"), *FilePath);  
        
    }  
    ReadRadianceFile(Handle, SingleVisibility, SkyModelData.RadianceData);
    int Channels = SkyModelData.RadianceData.Channels;
    delete Handle;

    ShaderPackedData.Rank = SkyModelData.RadianceData.MetadataRad.Rank;
    ShaderPackedData.SunOffset = SkyModelData.RadianceData.MetadataRad.SunOffset;
    ShaderPackedData.SunStride = SkyModelData.RadianceData.MetadataRad.SunStride;
    ShaderPackedData.ZenithOffset = SkyModelData.RadianceData.MetadataRad.ZenithOffset;
    ShaderPackedData.ZenithStride = SkyModelData.RadianceData.MetadataRad.ZenithStride;
    ShaderPackedData.EmphOffset = SkyModelData.RadianceData.MetadataRad.EmphOffset;
    ShaderPackedData.TotalCoefsSingleConfig = SkyModelData.RadianceData.MetadataRad.TotalCoefsSingleConfig;
    ShaderPackedData.TotalCoefsAllConfigs = SkyModelData.RadianceData.MetadataRad.TotalCoefsAllConfigs;
    ShaderPackedData.SunBreaksSize = SkyModelData.RadianceData.MetadataRad.SunBreaks.Num();
    ShaderPackedData.ZenithBreaksSize = SkyModelData.RadianceData.MetadataRad.ZenithBreaks.Num();
    ShaderPackedData.EmphBreaksSize = SkyModelData.RadianceData.MetadataRad.EmphBreaks.Num();
    ShaderPackedData.VisibilitiesRadSize = SkyModelData.RadianceData.VisibilitiesRad.Num();
    ShaderPackedData.AlbedosRadSize = SkyModelData.RadianceData.AlbedosRad.Num();
    ShaderPackedData.AltitudesRadSize = SkyModelData.RadianceData.AltitudesRad.Num();
    ShaderPackedData.ElevationsRadSize = SkyModelData.RadianceData.ElevationsRad.Num();
    

    ShaderPackedData.AlbedosRad = ConvertDoublesToUint32s(SkyModelData.RadianceData.AlbedosRad);
    ShaderPackedData.AltitudesRad = ConvertDoublesToUint32s(SkyModelData.RadianceData.AltitudesRad);
    ShaderPackedData.ElevationsRad = ConvertDoublesToUint32s(SkyModelData.RadianceData.ElevationsRad);
    ShaderPackedData.VisibilitiesRad = ConvertDoublesToUint32s(SkyModelData.RadianceData.VisibilitiesRad);

    
    ShaderPackedData.SunBreaks = ConvertDoublesToUint32s(SkyModelData.RadianceData.MetadataRad.SunBreaks);
    ShaderPackedData.ZenithBreaks = ConvertDoublesToUint32s(SkyModelData.RadianceData.MetadataRad.ZenithBreaks);
    ShaderPackedData.EmphBreaks = ConvertDoublesToUint32s(SkyModelData.RadianceData.MetadataRad.EmphBreaks);

    ShaderPackedData.DataRad = UWil21BlueprintLibrary::ConvertDoublesToFUint32s(SkyModelData.RadianceData.DataRad);
    ShaderPackedData.DataRadSize = ShaderPackedData.DataRad.Num();

 //    TArray<double> SpectralResponseData = {
 //         0.000129900000f, 0.000003917000f, 0.000606100000f,
 //         0.000232100000f, 0.000006965000f, 0.001086000000f,
 //         0.000414900000f, 0.000012390000f, 0.001946000000f,
 //         0.000741600000f, 0.000022020000f, 0.003486000000f,
 //         0.001368000000f, 0.000039000000f, 0.006450001000f,
 //         0.002236000000f, 0.000064000000f, 0.010549990000f,
 //         0.004243000000f, 0.000120000000f, 0.020050010000f,
 //         0.007650000000f, 0.000217000000f, 0.036210000000f,
 //         0.014310000000f, 0.000396000000f, 0.067850010000f,
 //         0.023190000000f, 0.000640000000f, 0.110200000000f,
 //         0.043510000000f, 0.001210000000f, 0.207400000000f,
 //         0.077630000000f, 0.002180000000f, 0.371300000000f,
 //         0.134380000000f, 0.004000000000f, 0.645600000000f,
 //         0.214770000000f, 0.007300000000f, 1.039050100000f,
 //         0.283900000000f, 0.011600000000f, 1.385600000000f,
 //         0.328500000000f, 0.016840000000f, 1.622960000000f,
 //         0.348280000000f, 0.023000000000f, 1.747060000000f,
 //         0.348060000000f, 0.029800000000f, 1.782600000000f,
 //         0.336200000000f, 0.038000000000f, 1.772110000000f,
 //         0.318700000000f, 0.048000000000f, 1.744100000000f,
 //         0.290800000000f, 0.060000000000f, 1.669200000000f,
 //         0.251100000000f, 0.073900000000f, 1.528100000000f,
 //         0.195360000000f, 0.090980000000f, 1.287640000000f,
 //         0.142100000000f, 0.112600000000f, 1.041900000000f,
 //         0.095640000000f, 0.139020000000f, 0.812950100000f,
 //         0.057950010000f, 0.169300000000f, 0.616200000000f,
 //         0.032010000000f, 0.208020000000f, 0.465180000000f,
 //         0.014700000000f, 0.258600000000f, 0.353300000000f,
 //         0.004900000000f, 0.323000000000f, 0.272000000000f,
 //         0.002400000000f, 0.407300000000f, 0.212300000000f,
 //         0.009300000000f, 0.503000000000f, 0.158200000000f,
 //         0.029100000000f, 0.608200000000f, 0.111700000000f,
 //         0.063270000000f, 0.710000000000f, 0.078249990000f,
 //         0.109600000000f, 0.793200000000f, 0.057250010000f,
 //         0.165500000000f, 0.862000000000f, 0.042160000000f,
 //         0.225749900000f, 0.914850100000f, 0.029840000000f,
 //         0.290400000000f, 0.954000000000f, 0.020300000000f,
 //         0.359700000000f, 0.980300000000f, 0.013400000000f,
 //         0.433449900000f, 0.994950100000f, 0.008749999000f,
 //         0.512050100000f, 1.000000000000f, 0.005749999000f,
 //         0.594500000000f, 0.995000000000f, 0.003900000000f,
 //         0.678400000000f, 0.978600000000f, 0.002749999000f,
 //         0.762100000000f, 0.952000000000f, 0.002100000000f,
 //         0.842500000000f, 0.915400000000f, 0.001800000000f,
 //         0.916300000000f, 0.870000000000f, 0.001650001000f,
 //         0.978600000000f, 0.816300000000f, 0.001400000000f,
 //         1.026300000000f, 0.757000000000f, 0.001100000000f,
 //         1.056700000000f, 0.694900000000f, 0.001000000000f,
 //         1.062200000000f, 0.631000000000f, 0.000800000000f,
 //         1.045600000000f, 0.566800000000f, 0.000600000000f,
 //         1.002600000000f, 0.503000000000f, 0.000340000000f,
 //         0.938400000000f, 0.441200000000f, 0.000240000000f,
 //         0.854449900000f, 0.381000000000f, 0.000190000000f,
 //         0.751400000000f, 0.321000000000f, 0.000100000000f,
 //         0.642400000000f, 0.265000000000f, 0.000049999990f,
 //         0.541900000000f, 0.217000000000f, 0.000030000000f,
 //         0.447900000000f, 0.175000000000f, 0.000020000000f,
 //         0.360800000000f, 0.138200000000f, 0.000010000000f,
 //         0.283500000000f, 0.107000000000f, 0.000000000000f,
 //         0.218700000000f, 0.081600000000f, 0.000000000000f,
 //         0.164900000000f, 0.061000000000f, 0.000000000000f,
 //         0.121200000000f, 0.044580000000f, 0.000000000000f,
 //         0.087400000000f, 0.032000000000f, 0.000000000000f,
 //         0.063600000000f, 0.023200000000f, 0.000000000000f,
 //         0.046770000000f, 0.017000000000f, 0.000000000000f,
 //         0.032900000000f, 0.011920000000f, 0.000000000000f,
 //         0.022700000000f, 0.008210000000f, 0.000000000000f,
 //         0.015840000000f, 0.005723000000f, 0.000000000000f,
 //         0.011359160000f, 0.004102000000f, 0.000000000000f,
 //         0.008110916000f, 0.002929000000f, 0.000000000000f,
 //         0.005790346000f, 0.002091000000f, 0.000000000000f,
 //         0.004109457000f, 0.001484000000f, 0.000000000000f,
 //         0.002899327000f, 0.001047000000f, 0.000000000000f,
 //         0.002049190000f, 0.000740000000f, 0.000000000000f,
 //         0.001439971000f, 0.000520000000f, 0.000000000000f,
 //         0.000999949300f, 0.000361100000f, 0.000000000000f,
 //         0.000690078600f, 0.000249200000f, 0.000000000000f,
 //         0.000476021300f, 0.000171900000f, 0.000000000000f,
 //         0.000332301100f, 0.000120000000f, 0.000000000000f,
 //         0.000234826100f, 0.000084800000f, 0.000000000000f,
 //         0.000166150500f, 0.000060000000f, 0.000000000000f,
 //         0.000117413000f, 0.000042400000f, 0.000000000000f,
 //         0.000083075270f, 0.000030000000f, 0.000000000000f,
 //         0.000058706520f, 0.000021200000f, 0.000000000000f,
 //         0.000041509940f, 0.000014990000f, 0.000000000000f,
 //         0.000029353260f, 0.000010600000f, 0.000000000000f,
 //         0.000020673830f, 0.000007465700f, 0.000000000000f,
 //         0.000014559770f, 0.000005257800f, 0.000000000000f,
 //         0.000010253980f, 0.000003702900f, 0.000000000000f,
 //         0.000007221456f, 0.000002607800f, 0.000000000000f,
 //         0.000005085868f, 0.000001836600f, 0.000000000000f,
 //         0.000003581652f, 0.000001293400f, 0.000000000000f,
 //         0.000002522525f, 0.000000910930f, 0.000000000000f,
 //         0.000001776509f, 0.000000641530f, 0.000000000000f,
 //         0.000001251141f, 0.000000451810f, 0.000000000000f
	// };
	// ShaderPackedData.SpectralResponse = UWil21BlueprintLibrary::ConvertDoublesToFUint32s(SpectralResponseData);
}

TArray<DoublePacked> ADataProcessor::ConvertDoublesToUint32s(const TArray<double>& doubleArray) {  
    TArray<DoublePacked> uintArray;  
    uintArray.Reserve(doubleArray.Num()); // Reserve space for two uint32s per double  

    for (double value : doubleArray) {  
        uint64_t doubleBits = *reinterpret_cast<const uint64_t*>(&value);
        DoublePacked tmp;
        tmp.Low = static_cast<uint32_t>(doubleBits & 0xFFFFFFFF);        // Lower 32 bits  
        tmp.High = static_cast<uint32_t>((doubleBits >> 32) & 0xFFFFFFFF); // Upper 32 bits
        uintArray.Add(tmp);
    }  

    return uintArray;  
}


void ADataProcessor::SetVariable(float SolarElevation,float SolarAzimuth, float Albedo, float Visibility)
{
    FShaderControlData NewData;
    NewData.Altitude = 0;
    NewData.Resolution = 1024;
    NewData.Albedo = Albedo;
    NewData.SolarElevation = SolarElevation;
    NewData.SolarAzimuth = SolarAzimuth;
    NewData.Visibility = Visibility;
    if(NewData!=ShaderControlData)
    {
        ShaderControlData = NewData;
        OnVariableChangedDelegate.Broadcast();
    }
    
}

double ADataProcessor::DoubleFromHalf(uint16 Half)  
{  
    uint32 Hi = (uint32)(Half & 0x8000) << 16;  
    uint16 Abs = Half & 0x7FFF;  
    if (Abs)  
    {  
        Hi |= 0x3F000000 << (uint16)(Abs >= 0x7C00);  
        while (Abs < 0x400)  
        {  
            Abs <<= 1;  
            Hi -= 0x100000;  
        }  
        Hi += (uint32)Abs << 10;  
    }  
    uint64 DBits = (uint64)Hi << 32;  
    double Out;  
    FMemory::Memcpy(&Out, &DBits, sizeof(double));  
    return Out;  
}
void ADataProcessor::OnVariableChanged()
{
    if (!OutputRenderTarget)  
    {  
        UE_LOG(LogTemp, Warning, TEXT("OutputRenderTarget is not initialized"));  
        return;  
    }  
    UWil21RenderingBlueprintLibrary::UseRDGComputeWil21(GetWorld(), ShaderPackedData, ShaderControlData, OutputRenderTarget);
}
#if WITH_EDITOR  
void ADataProcessor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)  
{  
    Super::PostEditChangeProperty(PropertyChangedEvent);  
    FName PropertyName = (PropertyChangedEvent.Property != nullptr)   
        ? PropertyChangedEvent.Property->GetFName()   
        : NAME_None;  

    if (PropertyName == GET_MEMBER_NAME_CHECKED(FShaderControlData, SolarElevation) ||  
        PropertyName == GET_MEMBER_NAME_CHECKED(FShaderControlData, SolarAzimuth) ||  
        PropertyName == GET_MEMBER_NAME_CHECKED(FShaderControlData, Albedo) ||  
        PropertyName == GET_MEMBER_NAME_CHECKED(FShaderControlData, Visibility))  
    {
        if (!bIsSliderChanging)  
        {  
            bIsSliderChanging = true;  
            GetWorld()->GetTimerManager().SetTimer(SliderFinishTimerHandle, this, &ADataProcessor::OnSliderChangeFinished, 0.2f, false);  
        }  
        else  
        {  
            GetWorld()->GetTimerManager().ClearTimer(SliderFinishTimerHandle);  
            GetWorld()->GetTimerManager().SetTimer(SliderFinishTimerHandle, this, &ADataProcessor::OnSliderChangeFinished, 0.2f, false);  
        }  


        // UE_LOG(LogTemp, Warning, TEXT("Re-ComputeWil21"));
        // UWil21RenderingBlueprintLibrary::UseRDGComputeWil21(GetWorld(), ShaderPackedData, ShaderControlData, OutputRenderTarget);
        // OnVariableChanged();  
    }  
}  
#endif
void ADataProcessor::OnSliderUpdate()  
{  
    
    UE_LOG(LogTemp, Log, TEXT("Slider is being updated..."));
    // OnVariableChanged();  
} 
void ADataProcessor::OnSliderChangeFinished()  
{
    GetWorld()->GetTimerManager().ClearTimer(SliderUpdateTimerHandle);
    // 重置标志  
    bIsSliderChanging = false;  
    // 如果 ShaderControlData 的某个属性被修改了，执行相应的逻辑
    UE_LOG(LogTemp, Warning, TEXT("Re-ComputeWil21")); 
    // 在这里进行最终的操作  
    OnVariableChanged();  
}  