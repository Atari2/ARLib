function ConvertTo-UTF8 {
    param(
        [string]$path = $pwd, 
        [bool]$recurse = $false
    )
    if ($recurse) {
        foreach($i in Get-ChildItem -Recurse -Path $path -Filter "*.cpp") {
            $temp = Get-Content $i.fullname
            Out-File -filepath $i.fullname -inputobject $temp -encoding utf8 -force
        }
        foreach($i in Get-ChildItem -Recurse -Path $path -Filter "*.h") {
            $temp = Get-Content $i.fullname
            Out-File -filepath $i.fullname -inputobject $temp -encoding utf8 -force
        }
    } else {
        foreach($i in Get-ChildItem -Path $path -Filter "*.cpp") {
            $temp = Get-Content $i.fullname
            Out-File -filepath $i.fullname -inputobject $temp -encoding utf8 -force
        }
        foreach($i in Get-ChildItem -Path $path -Filter "*.h") {
            $temp = Get-Content $i.fullname
            Out-File -filepath $i.fullname -inputobject $temp -encoding utf8 -force
        }
    }
}
ConvertTo-UTF8
ConvertTo-UTF8 "ASM_GCC"
ConvertTo-UTF8 "ASM_MSVC"
ConvertTo-UTF8 "PlayGround"
ConvertTo-UTF8 "Linux" $true
ConvertTo-UTF8 "Windows" $true
ConvertTo-UTF8 "XNative" $true
ConvertTo-UTF8 "Tests"
ConvertTo-UTF8 "Benchmark"