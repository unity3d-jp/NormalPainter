#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;


public class BlendShapeEditorPackaging
{
    [MenuItem("Assets/Make BlendShapeEditor.unitypackage")]
    public static void MakePackage()
    {
        string[] files = new string[]
        {
            "Assets/UTJ/BlendShapeEditor",
        };
        AssetDatabase.ExportPackage(files, "BlendShapeEditor.unitypackage", ExportPackageOptions.Recurse);
    }

}
#endif // UNITY_EDITOR
