[CmdletBinding(DefaultParameterSetName='Default')]
param(
    [Parameter(ParameterSetName='Install', Mandatory=$false)]
    [switch]$install,
    
    [Parameter(ParameterSetName='Uninstall', Mandatory=$false)]
    [switch]$uninstall,
    
    [Parameter(ParameterSetName='Install', Mandatory=$false)]
    [string]$time = "18:28"
)

# 任务基本信息
$taskName = "下班提醒"
$taskDescription = "每天准时在设定时间提醒您准备下班"

# 获取当前脚本所在目录
$scriptPath = Split-Path -Parent $MyInvocation.MyCommand.Definition

# 设置执行程序的完整路径
$exePath = Join-Path -Path $scriptPath -ChildPath "leaveWork_task.exe"

# 卸载现有任务函数
function Uninstall-Task {
    # 移除已有的同名任务（如果存在）
    if (Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue) {
        Get-ScheduledTask -TaskName $taskName | Unregister-ScheduledTask -Confirm:$false
        Write-Host "计划任务「$taskName」已成功删除！" -ForegroundColor Green
    } else {
        Write-Host "未找到计划任务「$taskName」，无需卸载。" -ForegroundColor Yellow
    }
}

# 安装任务函数
function Install-Task {
    param(
        [string]$runTime
    )
    
    # 检查程序文件是否存在
    if (-not (Test-Path $exePath)) {
        Write-Error "找不到程序文件: $exePath"
        exit 1
    }
    
    # 先卸载可能存在的旧任务
    Uninstall-Task
    
    # 创建任务触发器（每天指定时间运行）
    $triggerTime = New-ScheduledTaskTrigger -Daily -At $runTime
    
    # 创建任务操作（运行程序）
    $action = New-ScheduledTaskAction -Execute $exePath
    
    # 创建主体设置（使用当前用户，无需密码，最高权限）
    $principal = New-ScheduledTaskPrincipal -UserId "$env:USERDOMAIN\$env:USERNAME" -LogonType Interactive -RunLevel Highest
    
    # 创建任务设置
    $settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -StartWhenAvailable
    
    # 注册任务
    Register-ScheduledTask -TaskName $taskName -Description $taskDescription -Trigger $triggerTime -Action $action -Principal $principal -Settings $settings
    
    Write-Host "计划任务「$taskName」已成功创建！" -ForegroundColor Green
    Write-Host "程序将在每天 $runTime 自动启动。" -ForegroundColor Green
    
    # 询问是否要立即测试运行
    $testNow = Read-Host "是否要立即测试运行程序？(Y/N)"
    if ($testNow -eq "Y" -or $testNow -eq "y") {
        Start-Process -FilePath $exePath
        Write-Host "程序已启动进行测试。" -ForegroundColor Yellow
    }
}

# 显示帮助信息
function Show-Help {
    Write-Host "下班提醒任务计划创建脚本" -ForegroundColor Cyan
    Write-Host "用法:" -ForegroundColor Cyan
    Write-Host "  .\create_task.ps1              - 交互式安装计划任务" -ForegroundColor White
    Write-Host "  .\create_task.ps1 -install     - 安装计划任务" -ForegroundColor White
    Write-Host "  .\create_task.ps1 -install -time ""18:30"" - 安装计划任务并指定时间" -ForegroundColor White
    Write-Host "  .\create_task.ps1 -uninstall   - 卸载计划任务" -ForegroundColor White
}

# 主逻辑
if ($install) {
    # 安装模式
    Install-Task -runTime $time
}
elseif ($uninstall) {
    # 卸载模式
    Uninstall-Task
}
else {
    # 无参数时显示帮助并执行交互式安装
    Show-Help
    
    $doInstall = Read-Host "是否安装计划任务？(Y/N)"
    if ($doInstall -eq "Y" -or $doInstall -eq "y") {
        $customTime = Read-Host "输入每天启动的时间(格式如 18:28，直接回车使用默认值)"
        if ([string]::IsNullOrWhiteSpace($customTime)) {
            $customTime = $time
        }
        Install-Task -runTime $customTime
    }
}