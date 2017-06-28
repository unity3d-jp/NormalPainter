
using UnityEngine;
using UnityEditor;


namespace UTJ.FbxExporter
{
    public class FbxExporterWindow : EditorWindow
    {
        [MenuItem("Window/Fbx Exporter")]
        public static void Open()
        {
            var window = EditorWindow.GetWindow<FbxExporterWindow>();
            window.titleContent = new GUIContent("Fbx Exporter");
            window.Show();
        }


        public bool ExportObjects(string path, FbxExporter.Format format, GameObject[] objects)
        {
            var exporter = new FbxExporter();
            exporter.CreateScene(System.IO.Path.GetFileName(path));

            foreach (var obj in objects)
                exporter.AddNode(obj);

            var ret = exporter.Write(path, format);
            exporter.Release();
            return ret;
        }

        void OnGUI()
        {
            if (GUILayout.Button("Export Selection"))
            {
                var go = Selection.activeGameObject;
                if (go != null)
                {
                    var path = EditorUtility.SaveFilePanel("Export .fbx file", "", go.name, "fbx");
                    if (path != null && path.Length > 0)
                    {
                        ExportObjects(path, FbxExporter.Format.FbxAscii, Selection.gameObjects);
                    }
                }
            }
        }
    }
}

