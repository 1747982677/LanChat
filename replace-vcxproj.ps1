# Force replace vcxproj with minimal version
# Close Visual Studio first!

Write-Host "====================================" -ForegroundColor Cyan
Write-Host "  Creating Minimal LanChat Project" -ForegroundColor Cyan  
Write-Host "====================================" -ForegroundColor Cyan
Write-Host ""

# Check if VS is running
$vsProcess = Get-Process devenv -ErrorAction SilentlyContinue
if ($vsProcess) {
    Write-Host "WARNING: Visual Studio is running!" -ForegroundColor Red
    Write-Host "Please close Visual Studio first, then run this script again." -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Press Enter to exit"
    exit 1
}

$vcxproj = "LanChat.vcxproj"

# Backup
$backup = "$vcxproj.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
Copy-Item $vcxproj $backup -Force
Write-Host "[OK] Backup created: $backup" -ForegroundColor Green

# Create minimal content
$minimalContent = @'
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <ProjectGuid>{B12702AD-ABFB-343A-A199-8E24837244A3}</ProjectGuid>
    <Keyword>QtVS_v304</Keyword>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
    <QtMsBuild Condition="'$(QtMsBuild)'=='' OR !Exists('$(QtMsBuild)\qt.targets')">$(MSBuildProjectDirectory)\QtMsBuild</QtMsBuild>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.Default.props" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)' == 'Debug|x64'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <PlatformToolset>v143</PlatformToolset>
    <UseDebugLibraries>true</UseDebugLibraries>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.props" />
  <ImportGroup Condition="Exists('$(QtMsBuild)\qt_defaults.props')">
    <Import Project="$(QtMsBuild)\qt_defaults.props" />
  </ImportGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)' == 'Debug|x64'" Label="QtSettings">
    <QtInstall>6.5.3_msvc2019_64</QtInstall>
    <QtModules>core;gui;widgets</QtModules>
    <QtBuildConfig>debug</QtBuildConfig>
  </PropertyGroup>
  <ImportGroup Label="ExtensionSettings" />
  <ImportGroup Label="Shared" />
  <ImportGroup Label="PropertySheets" Condition="'$(Configuration)|$(Platform)' == 'Debug|x64'">
    <Import Project="$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
    <Import Project="$(QtMsBuild)\Qt.props" />
  </ImportGroup>
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup Condition="'$(Configuration)|$(Platform)' == 'Debug|x64'">
    <OutDir>.\bin\</OutDir>
  </PropertyGroup>
  <ItemDefinitionGroup Condition="'$(Configuration)|$(Platform)' == 'Debug|x64'">
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <SDLCheck>true</SDLCheck>
    </ClCompile>
    <Link>
      <SubSystem>Windows</SubSystem>
      <GenerateDebugInformation>true</GenerateDebugInformation>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup>
    <ClCompile Include="src\main.cpp" />
  </ItemGroup>
  <Import Project="$(VCTargetsPath)\Microsoft.Cpp.targets" />
  <ImportGroup Condition="Exists('$(QtMsBuild)\qt.targets')">
    <Import Project="$(QtMsBuild)\qt.targets" />
  </ImportGroup>
  <ImportGroup Label="ExtensionTargets" />
</Project>
'@

# Write to file
$minimalContent | Set-Content $vcxproj -Encoding UTF8 -NoNewline

Write-Host "[OK] Minimal project file created" -ForegroundColor Green
Write-Host ""
Write-Host "Project now contains ONLY:" -ForegroundColor Yellow
Write-Host "  - src\main.cpp" -ForegroundColor White
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Open Visual Studio" -ForegroundColor White
Write-Host "  2. Open LanChat.vcxproj" -ForegroundColor White
Write-Host "  3. Build -> Clean Solution" -ForegroundColor White
Write-Host "  4. Build -> Rebuild Solution" -ForegroundColor White
Write-Host "  5. Press F5 to run" -ForegroundColor White
Write-Host ""
Write-Host "Done!" -ForegroundColor Green
Write-Host ""
Read-Host "Press Enter to exit"
