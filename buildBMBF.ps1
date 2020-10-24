# Builds a .zip file for loading with BMBF
$NDKPath = Get-Content $PSScriptRoot/ndkpath.txt

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $buildScript += ".cmd"
}

& $buildScript NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk
Compress-Archive -Path "./libs/arm64-v8a/libpracticeplus.so","./bmbfmod.json","./extern/beatsaber-hook/libs/arm64-v8a/libbeatsaber-hook_2019_2_1f1_0_1_1.so","./extern/BeatSaberQuestCustomUI/libs/arm64-v8a/libcustomui.so" -DestinationPath "./practiceplus_v0.2.0.zip" -Update
