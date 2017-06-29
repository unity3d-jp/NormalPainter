
using System.Collections.Generic;
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

        enum Scope
        {
            Selected,
            EntireScene,
        }

        Scope m_scope = Scope.Selected;
        FbxExporter.Format m_format = FbxExporter.Format.FbxBinary;
        FbxExporter.ExportOptions m_opt = FbxExporter.ExportOptions.defaultValue;

        bool DoExport(string path, FbxExporter.Format format, GameObject[] objects)
        {
            var exporter = new FbxExporter(m_opt);
            exporter.CreateScene(System.IO.Path.GetFileName(path));

            foreach (var obj in objects)
                exporter.AddNode(obj);

            var ret = exporter.Write(path, format);
            exporter.Release();
            return ret;
        }

        void OnGUI()
        {
            m_scope = (Scope)EditorGUILayout.EnumPopup("Scope", m_scope);
            m_format = (FbxExporter.Format)EditorGUILayout.EnumPopup("Format", m_format);
            m_opt.flip_handedness = EditorGUILayout.Toggle("Flip Handedness", m_opt.flip_handedness);
            m_opt.flip_faces = EditorGUILayout.Toggle("Flip Faces", m_opt.flip_faces);
            m_opt.scale_factor = EditorGUILayout.FloatField("Scale Factor", m_opt.scale_factor);

            EditorGUILayout.Space();

            if (GUILayout.Button("Export"))
            {
                var objects = new List<GameObject>();

                if (m_scope == Scope.Selected)
                {
                    objects = new List<GameObject>(Selection.gameObjects);
                }
                else
                {
                    foreach (var o in GameObject.FindObjectsOfType<MeshRenderer>())
                        objects.Add(o.gameObject);
                    foreach (var o in GameObject.FindObjectsOfType<SkinnedMeshRenderer>())
                        objects.Add(o.gameObject);
                }

                if (objects.Count == 0)
                {
                    Debug.LogWarning("FbxExporter: Nothing to export");
                }
                else
                {
                    string filename = objects[0].name;
                    var path = EditorUtility.SaveFilePanel("Export .fbx file", "", filename, "fbx");
                    if (path != null && path.Length > 0)
                    {
                        DoExport(path, m_format, objects.ToArray());
                    }
                }
            }
        }
    }
}

