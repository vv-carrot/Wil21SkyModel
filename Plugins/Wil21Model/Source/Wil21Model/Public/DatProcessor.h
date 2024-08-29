#pragma once

#include "CoreMinimal.h"
#include "FemeerSurfelCacheDefinitions.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DatProcessor.generated.h"

USTRUCT(BlueprintType, meta = (ScriptName = "Wil21Model"))
struct FRadianceMetadata
{
	GENERATED_USTRUCT_BODY()
	
	int Rank;
	int                 SunOffset;
	int                 SunStride;
	TArray<double> SunBreaks;

	int                 ZenithOffset;
	int                 ZenithStride;
	TArray<double> ZenithBreaks;

	int                 EmphOffset; // not used for polarisation
	TArray<double> EmphBreaks; // not used for polarisation

	int TotalCoefsSingleConfig;
	int TotalCoefsAllConfigs;
	

};

USTRUCT(BlueprintType)  
struct FRadianceData  
{  
	GENERATED_BODY()  

	UPROPERTY(BlueprintReadOnly, Category = "Radiance Data")  
	TArray<double> VisibilitiesRad;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Radiance Data")  
	TArray<double> AlbedosRad;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Radiance Data")  
	TArray<double> AltitudesRad;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Radiance Data")  
	TArray<double> ElevationsRad;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Radiance Data")  
	int32 Channels;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Radiance Data")  
	double ChannelStart;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Radiance Data")  
	double ChannelWidth;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Radiance Data")  
	FRadianceMetadata MetadataRad;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Radiance Data")  
	TArray<double> DataRad;  
}; 

USTRUCT(BlueprintType) 
struct FTransmittanceData  
{  
	GENERATED_BODY()  

	UPROPERTY(BlueprintReadOnly, Category = "Transmittance Data")  
	int32 DDim;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Transmittance Data")  
	int32 ADim;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Transmittance Data")  
	int32 RankTrans;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Transmittance Data")  
	TArray<double> AltitudesTrans;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Transmittance Data")  
	TArray<double> VisibilitiesTrans;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Transmittance Data")  
	TArray<float> DataTransU;  
    
	UPROPERTY(BlueprintReadOnly, Category = "Transmittance Data")  
	TArray<float> DataTransV;  
};   

USTRUCT(BlueprintType) 
struct FSkyModelData  
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model") 
	FRadianceData RadianceData;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model") 
	FTransmittanceData TransmittanceData;
};

struct DoublePacked
{
	uint32 Low;
	uint32 High;
};

USTRUCT(BlueprintType) 
struct FShaderPackedData  
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model") 
	int32 Rank;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 SunOffset;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 SunStride;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 ZenithOffset;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 ZenithStride;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 EmphOffset;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 TotalCoefsSingleConfig;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 TotalCoefsAllConfigs;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 SunBreaksSize;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 ZenithBreaksSize;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 EmphBreaksSize;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 VisibilitiesRadSize;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 AlbedosRadSize;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 AltitudesRadSize;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 ElevationsRadSize;
	UPROPERTY(BlueprintReadOnly, Category = "Sky Model")
	int32 DataRadSize;

	TArray<DoublePacked> SunBreaks;
	TArray<DoublePacked> ZenithBreaks;
	TArray<DoublePacked> EmphBreaks;
	TArray<DoublePacked> VisibilitiesRad;
	TArray<DoublePacked> AlbedosRad;
	TArray<DoublePacked> AltitudesRad;
	TArray<DoublePacked> ElevationsRad;
	TArray<uint32> DataRad;
	// TArray<uint32> SpectralResponse;
};  

USTRUCT(BlueprintType) 
struct FShaderControlData  
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShaderControl") 
	int32 Resolution = 1024;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, interp, Category = "ShaderControl", meta = (ClampMin = "-4.2", ClampMax = "90.0", UIMin = "-4.2", UIMax = "90.0"))
	float SolarElevation = -4.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, interp, Category = "ShaderControl", meta = (ClampMin = "0.0", ClampMax = "360.0", UIMin = "0.0", UIMax = "360.0"))
	float SolarAzimuth = 180.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, interp, Category = "ShaderControl", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Albedo = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, interp, Category = "ShaderControl", meta = (ClampMin = "27.0", ClampMax = "131.8", UIMin = "27.0", UIMax = "131.8"))
	float Visibility = 131.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShaderControl")
	float Altitude = 0.0f;
	bool operator==(const FShaderControlData& Other) const  
	{  
		return Resolution == Other.Resolution && SolarElevation == Other.SolarElevation && SolarAzimuth == Other.SolarAzimuth && Albedo == Other.Albedo && Visibility == Other.Visibility && Altitude == Other.Altitude; 
	}  
	bool operator!=(const FShaderControlData& Other) const  
	{  
		return !(*this == Other);  
	}  
};  

UCLASS(MinimalAPI, meta = (ScriptName = "Wil21Model"))
class UWil21BlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, Category = "Wil21Model")  
	static FShaderPackedData ReadDatFileFromContentFolder(FSkyModelData& SkyModelData, const FString& FileName = "SkyModelDatasetGround.dat", double SingleVisibility =23.8); 
	
	static void ReadRadianceFile(IFileHandle* Handle, double SingleVisibility, FRadianceData& Result);
	static void ReadTransmittanceFile(IFileHandle* Handle, int Channels, FTransmittanceData& Result); 
	// static void ReadRadiance(IFileHandle* Handle, double SingleVisibility, FRadianceData& RadianceData);
	static double DoubleFromHalf(uint16 Half);
	static  TArray<DoublePacked> ConvertDoublesToUint32s(const TArray<double>& doubleArray);
	static  TArray<uint> ConvertDoublesToFUint32s(const TArray<double>& doubleArray);
};


