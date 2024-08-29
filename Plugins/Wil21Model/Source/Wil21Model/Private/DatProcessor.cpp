#include "DatProcessor.h"
#include "HAL/PlatformFilemanager.h"  
#include "Misc/FileHelper.h"
  

double UWil21BlueprintLibrary::DoubleFromHalf(uint16 Half)  
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

void UWil21BlueprintLibrary::ReadRadianceFile(IFileHandle* Handle, double SingleVisibility, FRadianceData& Result)
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

void UWil21BlueprintLibrary::ReadTransmittanceFile(IFileHandle* FileHandle, int Channels, FTransmittanceData& Result)  
{
    if (!FileHandle)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Invalid file handle"));  
        return;  
    }  

    // Read metadata  
    int32 valsRead;  

    valsRead = FileHandle->Read((uint8*)&Result.DDim, sizeof(int32));  
    if (Result.DDim < 1)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Failed to read DDim"));  
        return;  
    }  

    valsRead = FileHandle->Read((uint8*)&Result.ADim, sizeof(int32));  
    if (Result.ADim < 1)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Failed to read ADim"));  
        return;  
    }  

    int32 visibilitiesCount = 0;  
    valsRead = FileHandle->Read((uint8*)&visibilitiesCount, sizeof(int32));  
    if ( visibilitiesCount < 1)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Failed to read visibilitiesCountTrans"));  
        return;  
    }  

    int32 altitudesCount = 0;  
    valsRead = FileHandle->Read((uint8*)&altitudesCount, sizeof(int32));  
    if ( altitudesCount < 1)  
    {  
        UE_LOG(LogTemp, Error, TEXT("Failed to read altitudesCountTrans"));  
        return;  
    }  

    valsRead = FileHandle->Read((uint8*)&Result.RankTrans, sizeof(int32));  
    // if ( Result.RankTrans < 1)  
    // {  
    //     UE_LOG(LogTemp, Error, TEXT("Failed to read RankTrans"));  
    //     return;  
    // }  

    TArray<float> temp;  
    temp.SetNum(FMath::Max(altitudesCount, visibilitiesCount));  

    Result.AltitudesTrans.SetNum(altitudesCount);  
    valsRead = FileHandle->Read((uint8*)temp.GetData(), sizeof(float) * altitudesCount);  
    // if (valsRead != sizeof(float) * altitudesCount)  
    // {  
    //     UE_LOG(LogTemp, Error, TEXT("Failed to read AltitudesTrans"));  
    //     return;  
    // }  
    for (int32 i = 0; i < altitudesCount; i++)  
    {  
        Result.AltitudesTrans[i] = double(temp[i]);  
    }  

    Result.VisibilitiesTrans.SetNum(visibilitiesCount);  
    valsRead = FileHandle->Read((uint8*)temp.GetData(), sizeof(float) * visibilitiesCount);  
    // if (valsRead != sizeof(float) * visibilitiesCount)  
    // {  
    //     UE_LOG(LogTemp, Error, TEXT("Failed to read VisibilitiesTrans"));  
    //     return;  
    // }  
    for (int32 i = 0; i < visibilitiesCount; i++)  
    {  
        Result.VisibilitiesTrans[i] = double(temp[i]);  
    }  

    const int64 totalCoefsU = Result.DDim * Result.ADim * Result.RankTrans * Result.AltitudesTrans.Num();  
    const int64 totalCoefsV = Result.VisibilitiesTrans.Num() * Result.RankTrans * Channels * Result.AltitudesTrans.Num();  

    // Read data  
    Result.DataTransU.SetNum(totalCoefsU);  
    valsRead = FileHandle->Read((uint8*)Result.DataTransU.GetData(), sizeof(float) * totalCoefsU);  
    // if (valsRead != sizeof(float) * totalCoefsU)  
    // {  
    //     UE_LOG(LogTemp, Error, TEXT("Failed to read DataTransU"));  
    //     return;  
    // }  

    Result.DataTransV.SetNum(totalCoefsV);  
    valsRead = FileHandle->Read((uint8*)Result.DataTransV.GetData(), sizeof(float) * totalCoefsV);  
    // if (valsRead != sizeof(float) * totalCoefsV)  
    // {  
    //     UE_LOG(LogTemp, Error, TEXT("Failed to read DataTransV"));  
    //     return;  
    // }  
    
}

FShaderPackedData UWil21BlueprintLibrary::ReadDatFileFromContentFolder(FSkyModelData& SkyModelData, const FString& FileName, double SingleVisibility)  
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
    // ReadTransmittanceFile(Handle, Channels, SkyModelData.TransmittanceData);
    delete Handle;
    FShaderPackedData ShaderPackedData;
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

    ShaderPackedData.DataRad = ConvertDoublesToFUint32s(SkyModelData.RadianceData.DataRad);
    ShaderPackedData.DataRadSize = ShaderPackedData.DataRad.Num();
    
    return ShaderPackedData;
}
TArray<DoublePacked> UWil21BlueprintLibrary::ConvertDoublesToUint32s(const TArray<double>& doubleArray) {  
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
TArray<uint32> UWil21BlueprintLibrary::ConvertDoublesToFUint32s(const TArray<double>& doubleArray) {  
    TArray<uint32> uintArray;  
    uintArray.Reserve(doubleArray.Num()*2); // Reserve space for two uint32s per double  

    for (double value : doubleArray) {  
        uint64_t doubleBits = *reinterpret_cast<const uint64_t*>(&value);
        uintArray.Add(static_cast<uint32_t>(doubleBits & 0xFFFFFFFF));
        uintArray.Add(static_cast<uint32_t>(static_cast<uint32_t>(doubleBits >> 32) & 0xFFFFFFFF));
    }  

    return uintArray;  
}  


