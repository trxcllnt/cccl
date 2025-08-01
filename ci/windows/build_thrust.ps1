Param(
    [Parameter(Mandatory = $false)]
    [Alias("std")]
    [ValidateNotNullOrEmpty()]
    [ValidateSet(17, 20)]
    [int]$CXX_STANDARD = 17,
    [Parameter(Mandatory = $false)]
    [Alias("arch")]
    [string]$CUDA_ARCH = ""
)

$ErrorActionPreference = "Stop"

$CURRENT_PATH = Split-Path $pwd -leaf
If($CURRENT_PATH -ne "ci") {
    Write-Host "Moving to ci folder"
    pushd "$PSScriptRoot/.."
}

Import-Module $PSScriptRoot/build_common.psm1 -ArgumentList $CXX_STANDARD, "$CUDA_ARCH"

$PRESET = "thrust-cpp$CXX_STANDARD"
$CMAKE_OPTIONS = ""

if ($CL_VERSION -lt [version]"19.20") {
    $CMAKE_OPTIONS += "-DCCCL_IGNORE_DEPRECATED_COMPILER=ON "
}

configure_and_build_preset "Thrust" "$PRESET" "$CMAKE_OPTIONS"

If($CURRENT_PATH -ne "ci") {
    popd
}
