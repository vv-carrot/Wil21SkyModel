#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"  
#include "DatProcessor.h"
#include "Wil21Rendering.h"

#include "DataProcessorActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVariableChangedDelegate); 
UCLASS()
class ADataProcessor : public AActor
{
	GENERATED_BODY()
public:
	ADataProcessor();
	UFUNCTION(BlueprintCallable, Category = "Wil21Model")  
	void ReadDatFileFromContentFolder(const FString& FileName = "SkyModelDatasetGround.dat", double SingleVisibility =23.8); 
	UPROPERTY(BlueprintAssignable, Category="Events")  
	FOnVariableChangedDelegate OnVariableChangedDelegate;
	UFUNCTION()  
	void SetVariable(float SolarElevation,float SolarAzimuth, float Albedo, float Visibility);
	UFUNCTION()
	void OnVariableChanged();
private:
	void ReadRadianceFile(IFileHandle* Handle, double SingleVisibility, FRadianceData& Result);
	
	void OnSliderChangeFinished();
	void OnSliderUpdate();
	// Data processing functions
	double DoubleFromHalf(uint16 Half);
	TArray<DoublePacked> ConvertDoublesToUint32s(const TArray<double>& doubleArray);
	// For computing parameters 
	FShaderPackedData ShaderPackedData;
	FSkyModelData SkyModelData;

	// For updating slider values
	FTimerHandle SliderUpdateTimerHandle;  
	FTimerHandle SliderFinishTimerHandle;  
	bool bIsSliderChanging = false;  

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShaderControl")  
	FShaderControlData ShaderControlData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShaderControl")  
	UTextureRenderTarget2D* OutputRenderTarget;
protected:  
#if WITH_EDITOR  
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;  
#endif  
};