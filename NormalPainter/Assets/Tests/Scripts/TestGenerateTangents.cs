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
        string path = Environment.GetEnvironmentVariable("PATH", EnvironmentVariableTarget.Process);
        path = Application.dataPath + "Tests/Plugins/x86_64;" + path;
        Environment.SetEnvironmentVariable("PATH", path, EnvironmentVariableTarget.Process);
        DoTestGenerateTangents_();
    }
    [MethodImpl(MethodImplOptions.NoInlining)]
    public static void DoTestGenerateTangents_()
    {
        TestNormalsAndTangentsEx();
    }
    [DllImport("Test")] static extern void TestNormalsAndTangentsEx();
}
