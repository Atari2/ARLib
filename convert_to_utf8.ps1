foreach($i in Get-ChildItem -filter "*.cpp") {
    $temp = Get-Content $i.fullname
    echo $temp
    # Out-File -filepath $i.fullname -inputobject $temp -encoding utf8 -force
}
foreach($i in Get-ChildItem -filter "*.h") {
    $temp = Get-Content $i.fullname
    echo $temp
    # Out-File -filepath $i.fullname -inputobject $temp -encoding utf8 -force
}