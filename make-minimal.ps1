# Minimal LanChat - Remove all extra files temporarily
# Run this to create a minimal working version

$vcxprojPath = "LanChat.vcxproj"

Write-Host "Creating minimal project configuration..." -ForegroundColor Cyan

# Backup
$backupPath = "$vcxprojPath.full_backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
Copy-Item $vcxprojPath $backupPath -Force
Write-Host "Backup created: $backupPath" -ForegroundColor Green

# Read content
$content = Get-Content $vcxprojPath -Raw -Encoding UTF8

# Remove all the problematic files from compilation
$content = $content -replace '<ClCompile Include="src\\ui\\main_window\\main_window\.cpp" />', '<!-- <ClCompile Include="src\ui\main_window\main_window.cpp" /> -->'
$content = $content -replace '<ClCompile Include="src\\service\\chat_service\.cpp" />', '<!-- <ClCompile Include="src\service\chat_service.cpp" /> -->'
$content = $content -replace '<ClCompile Include="src\\network\\socket_client\.cpp" />', '<!-- <ClCompile Include="src\network\socket_client.cpp" /> -->'
$content = $content -replace '<ClCompile Include="src\\utils\\logger\.cpp" />', '<!-- <ClCompile Include="src\utils\logger.cpp" /> -->'
$content = $content -replace '<ClCompile Include="src\\utils\\config\.cpp" />', '<!-- <ClCompile Include="src\utils\config.cpp" /> -->'

# Remove MOC processing
$content = $content -replace '<QtMoc Include="src\\ui\\main_window\\main_window\.h" />', '<!-- <QtMoc Include="src\ui\main_window\main_window.h" /> -->'
$content = $content -replace '<QtMoc Include="src\\service\\chat_service\.h" />', '<!-- <QtMoc Include="src\service\chat_service.h" /> -->'
$content = $content -replace '<QtMoc Include="src\\network\\socket_client\.h" />', '<!-- <QtMoc Include="src\network\socket_client.h" /> -->'

# Remove UI processing
$content = $content -replace '<QtUic Include="src\\ui\\main_window\\main_window\.ui" />', '<!-- <QtUic Include="src\ui\main_window\main_window.ui" /> -->'

# Remove resource file
$content = $content -replace '<QtRcc Include="src\\ui\\assets\\resources\.qrc" />', '<!-- <QtRcc Include="src\ui\assets\resources.qrc" /> -->'

# Save
$content | Set-Content $vcxprojPath -Encoding UTF8 -NoNewline

Write-Host ""
Write-Host "Minimal configuration created!" -ForegroundColor Green
Write-Host "Only main.cpp will be compiled." -ForegroundColor Yellow
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Open Visual Studio" -ForegroundColor White
Write-Host "  2. Clean Solution" -ForegroundColor White
Write-Host "  3. Rebuild Solution" -ForegroundColor White
Write-Host "  4. Run (F5)" -ForegroundColor White
Write-Host ""
Write-Host "To restore full project later:" -ForegroundColor Yellow
Write-Host "  Copy-Item '$backupPath' '$vcxprojPath' -Force" -ForegroundColor Gray
