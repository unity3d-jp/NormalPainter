using System;
using UnityEngine;
using UnityEditor;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public static class TestGenerateTangents
{
    [MenuItem("Test/TestGenerateTangents")]
    public static void DoTestGenerateTangents()
    {
        SetLibraryPath();
        DoTestGenerateTangents_();
    }

    static void SetLibraryPath()
    {
        var libraryPath = Application.dataPath + "Tests/Plugins/x86_64";
        var path = Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.Process);
        if(!path.Contains(libraryPath))
        {
            path = libraryPath + ";" + path;
            Environment.SetEnvironmentVariable("PATH", path, EnvironmentVariableTarget.Process);
        }
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    public static void DoTestGenerateTangents_()
    {
        RunTest("TestNormalsAndTangents");
        Debug.Log(Marshal.PtrToStringAnsi(GetLogMessage()));
    }

    [DllImport("Test")] static extern IntPtr GetLogMessage();
    [DllImport("Test")] static extern void RunTest(string name);
    [DllImport("Test")] static extern void RunAllTests();
}
