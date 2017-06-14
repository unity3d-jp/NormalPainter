@echo off

IF NOT EXIST "External\ispc.exe" (
    IF NOT EXIST "External\External.7z" (
        echo "downloading external libararies..."
        powershell.exe -Command "(new-object System.Net.WebClient).DownloadFile('https://github.com/unity3d-jp/MeshSync/releases/download/0.0.2/External.7z', 'External/External.7z')"
    )
    cd External
    7z\7za.exe x -aos External.7z
    cd ..
)
