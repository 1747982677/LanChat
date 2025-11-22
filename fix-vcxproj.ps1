# 修复 LanChat.vcxproj - PowerShell 脚本
# 使用方法：关闭 Visual Studio 后，在项目根目录运行此脚本

$vcxprojPath = "LanChat.vcxproj"

Write-Host "================================" -ForegroundColor Cyan
Write-Host "LanChat 项目修复脚本" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

# 检查文件是否存在
if (-not (Test-Path $vcxprojPath)) {
    Write-Host "错误：找不到 LanChat.vcxproj 文件" -ForegroundColor Red
    Write-Host "请确保您在项目根目录" -ForegroundColor Yellow
    exit 1
}

# 备份原文件
Write-Host "步骤 1：备份原文件..." -ForegroundColor Yellow
$backupPath = "$vcxprojPath.backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
Copy-Item $vcxprojPath $backupPath -Force
Write-Host "  备份已创建：$backupPath" -ForegroundColor Green

# 读取文件内容
Write-Host "步骤 2：读取项目文件..." -ForegroundColor Yellow
$content = Get-Content $vcxprojPath -Raw -Encoding UTF8

# 检查当前状态
$hasNetwork = $content -match '<QtModules>.*network.*</QtModules>'
$hasSocketMoc = $content -match '<QtMoc Include="src\\network\\socket_client\.h" />'

Write-Host "  当前状态：" -ForegroundColor Cyan
Write-Host "    网络模块：$(if ($hasNetwork) { '已找到' } else { '缺失' })" -ForegroundColor $(if ($hasNetwork) { 'Green' } else { 'Yellow' })
Write-Host "    SocketClient MOC：$(if ($hasSocketMoc) { '已找到' } else { '缺失' })" -ForegroundColor $(if ($hasSocketMoc) { 'Green' } else { 'Yellow' })

# 修复 1：添加网络模块
Write-Host "步骤 3：添加网络模块..." -ForegroundColor Yellow
$originalContent = $content
$content = $content -replace '<QtModules>core;gui;widgets</QtModules>', '<QtModules>core;gui;widgets;network</QtModules>'

if ($content -ne $originalContent) {
    Write-Host "  网络模块添加成功" -ForegroundColor Green
} else {
    if ($hasNetwork) {
        Write-Host "  网络模块已存在" -ForegroundColor Green
    } else {
        Write-Host "  警告：无法添加网络模块" -ForegroundColor Red
    }
}

# 修复 2：将 socket_client.h 添加到 QtMoc
Write-Host "步骤 4：将 socket_client.h 添加到 QtMoc..." -ForegroundColor Yellow
if ($content -notmatch '<QtMoc Include="src\\network\\socket_client\.h" />') {
    $content = $content -replace '(<QtMoc Include="src\\service\\chat_service\.h" />)', "`$1`r`n    <QtMoc Include=`"src\network\socket_client.h`" />"
    Write-Host "  socket_client.h 已添加到 QtMoc" -ForegroundColor Green
} else {
    Write-Host "  socket_client.h 已在 QtMoc 中" -ForegroundColor Green
}

# 修复 3：移除重复的 ClInclude
Write-Host "步骤 5：移除重复的头文件引用..." -ForegroundColor Yellow
$beforeCleanup = $content
$content = $content -replace '\s*<ClInclude Include="src\\network\\socket_client\.h" />\r?\n?', ''

if ($content -ne $beforeCleanup) {
    Write-Host "  重复引用已删除" -ForegroundColor Green
} else {
    Write-Host "  未发现重复引用" -ForegroundColor Green
}

# 保存修改后的文件
Write-Host "步骤 6：保存更改..." -ForegroundColor Yellow
$content | Set-Content $vcxprojPath -Encoding UTF8 -NoNewline
Write-Host "  文件保存成功" -ForegroundColor Green

# 总结
Write-Host ""
Write-Host "================================" -ForegroundColor Cyan
Write-Host "修复完成！" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "接下来：" -ForegroundColor Cyan
Write-Host "  1. 打开 Visual Studio" -ForegroundColor White
Write-Host "  2. 右键单击解决方案 -> 清理解决方案" -ForegroundColor White
Write-Host "  3. 右键单击解决方案 -> 重新生成解决方案" -ForegroundColor White
Write-Host "  4. 按 F5 运行" -ForegroundColor White
Write-Host ""
Write-Host "如果您需要还原原文件：" -ForegroundColor Yellow
Write-Host "  Copy-Item '$backupPath' '$vcxprojPath' -Force" -ForegroundColor Gray
Write-Host ""
