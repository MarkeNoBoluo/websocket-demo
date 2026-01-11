# 环境检查脚本
Write-Host "=== 环境检查 ===" -ForegroundColor Cyan

# 1. 检查 VS 2022
Write-Host "`n1. 检查 Visual Studio 2022..." -ForegroundColor Yellow
$vsPath = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath
if ($vsPath) {
    Write-Host "  ✓ VS 2022 安装路径: $vsPath" -ForegroundColor Green
} else {
    Write-Host "  ✗ 未找到 VS 2022" -ForegroundColor Red
}

# 2. 检查编译器
Write-Host "`n2. 检查 MSVC 编译器..." -ForegroundColor Yellow
$cl = Get-Command cl -ErrorAction SilentlyContinue
if ($cl) {
    Write-Host "  ✓ 编译器: $($cl.Source)" -ForegroundColor Green
    cl 2>&1 | Select-String "Version" | ForEach-Object { Write-Host "  $_" }
} else {
    Write-Host "  ✗ 未找到 cl.exe (请从 VS 2022 命令行启动)" -ForegroundColor Red
}

# 3. 检查 CMake
Write-Host "`n3. 检查 CMake..." -ForegroundColor Yellow
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
    Write-Host "  ✓ CMake: $($cmake.Source)" -ForegroundColor Green
    cmake --version | Select-Object -First 1 | ForEach-Object { Write-Host "  $_" }
} else {
    Write-Host "  ✗ 未找到 CMake" -ForegroundColor Red
}

# 4. 检查 Ninja
Write-Host "`n4. 检查 Ninja..." -ForegroundColor Yellow
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
if ($ninja) {
    Write-Host "  ✓ Ninja: $($ninja.Source)" -ForegroundColor Green
    ninja --version | ForEach-Object { Write-Host "  版本: $_" }
} else {
    Write-Host "  ✗ 未找到 Ninja (可选，但推荐安装)" -ForegroundColor Yellow
}

# 5. 检查 Boost
Write-Host "`n5. 检查 Boost..." -ForegroundColor Yellow
$boostPath = "D:\LIB\Boost\boost_1_86_0"
if (Test-Path $boostPath) {
    Write-Host "  ✓ Boost 路径存在: $boostPath" -ForegroundColor Green
} else {
    Write-Host "  ✗ Boost 路径不存在: $boostPath" -ForegroundColor Red
}

# 6. 检查 WebSocket++
Write-Host "`n6. 检查 WebSocket++..." -ForegroundColor Yellow
$wsPath = "D:\LIB\websocketpp"
if (Test-Path $wsPath) {
    Write-Host "  ✓ WebSocket++ 路径存在: $wsPath" -ForegroundColor Green
} else {
    Write-Host "  ✗ WebSocket++ 路径不存在: $wsPath" -ForegroundColor Red
}

Write-Host "`n=== 检查完成 ===" -ForegroundColor Cyan
Write-Host "`n建议操作:" -ForegroundColor Yellow
if (-not $cl) {
    Write-Host "  1. 请从 'x64 Native Tools Command Prompt for VS 2022' 启动 VSCode" -ForegroundColor Magenta
}
if (-not $ninja) {
    Write-Host "  2. 安装 Ninja: choco install ninja" -ForegroundColor Magenta
}
Write-Host "  3. 使用 [Unspecified] Kit 或 Ninja 生成器" -ForegroundColor Magenta