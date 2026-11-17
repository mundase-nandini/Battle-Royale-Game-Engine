$ErrorActionPreference = 'Stop'

$gccBin = 'C:\Users\Neha\Downloads\gcc-13.2.0-no-debug\bin'
$gcc = Join-Path $gccBin 'gcc.exe'
$gxx = Join-Path $gccBin 'g++.exe'

if (!(Test-Path $gcc)) { throw "GCC not found: $gcc" }
if (!(Test-Path $gxx)) { throw "G++ not found: $gxx" }

$ninjaCommand = Get-Command ninja -ErrorAction SilentlyContinue
if (!$ninjaCommand) { throw "Ninja not found. Add Ninja to PATH." }

$env:PATH = "$gccBin;$env:PATH"

Write-Host "Using GCC: $gcc"
Write-Host "Using G++: $gxx"
Write-Host "Configuring RBG Engine..." -ForegroundColor Cyan

if (Test-Path build) {
    Remove-Item build -Recurse -Force
}

& cmake -S . -B build -G Ninja `
    "-DCMAKE_C_COMPILER=$gcc" `
    "-DCMAKE_CXX_COMPILER=$gxx"

if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed. Build stopped."
}

Write-Host "Compiling RBG Engine..." -ForegroundColor Cyan

& cmake --build build --parallel

if ($LASTEXITCODE -ne 0) {
    throw "Compilation failed. Build stopped."
}

if (!(Test-Path '.\build\RBGEngine.exe')) {
    throw "Build reported success but RBGEngine.exe was not found."
}

Write-Host "RBG ENGINE BUILD SUCCESSFUL!" -ForegroundColor Green
Write-Host "Run: .\build\RBGEngine.exe" -ForegroundColor Cyan
