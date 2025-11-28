# CMake 快速构建脚本 (Windows)
# 使用方法：在 PowerShell 中运行 .\build.ps1

param(
    [string]$BuildType = "Debug",
    [string]$Generator = "Visual Studio 17 2022",
    [string]$QtPath = ""
)

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "LanChat CMake 构建脚本" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# 清理旧的 build 目录
if (Test-Path "build") {
    Write-Host "清理旧的构建目录..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build"
}

# 创建 build 目录
Write-Host "创建构建目录..." -ForegroundColor Green
New-Item -ItemType Directory -Path "build" | Out-Null
Set-Location "build"

# 配置 CMake
Write-Host "配置 CMake 项目..." -ForegroundColor Green
$cmakeArgs = @("..", "-G", $Generator, "-A", "x64")

if ($QtPath -ne "") {
    Write-Host "使用指定的 Qt 路径: $QtPath" -ForegroundColor Yellow
    $cmakeArgs += "-DCMAKE_PREFIX_PATH=$QtPath"
}

& cmake @cmakeArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "CMake 配置失败！" -ForegroundColor Red
    Write-Host "如果找不到 Qt，请使用 -QtPath 参数指定 Qt 安装路径" -ForegroundColor Yellow
    Write-Host "例如: .\build.ps1 -QtPath 'C:\Qt\6.5.3\msvc2019_64'" -ForegroundColor Yellow
    Set-Location ..
    exit 1
}

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "CMake 配置成功！" -ForegroundColor Green
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "下一步操作：" -ForegroundColor Yellow
Write-Host "  1. 编译项目：cmake --build . --config $BuildType" -ForegroundColor White
Write-Host "  2. 或者打开 Visual Studio：start LanChat.sln" -ForegroundColor White
Write-Host "  3. 或者使用此脚本编译：" -ForegroundColor White

# 询问是否立即编译
$compile = Read-Host "是否立即编译项目？(Y/n)"
if ($compile -eq "" -or $compile -eq "Y" -or $compile -eq "y") {
    Write-Host ""
    Write-Host "开始编译..." -ForegroundColor Green
    cmake --build . --config $BuildType
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "=====================================" -ForegroundColor Cyan
        Write-Host "编译成功！" -ForegroundColor Green
        Write-Host "=====================================" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "可执行文件位置: .\bin\LanChat.exe" -ForegroundColor Yellow
    } else {
        Write-Host ""
        Write-Host "编译失败！请检查错误信息。" -ForegroundColor Red
    }
}

Set-Location ..
