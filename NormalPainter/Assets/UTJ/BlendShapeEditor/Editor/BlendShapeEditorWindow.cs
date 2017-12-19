using UnityEngine;
using UnityEditor;
using System;
using System.Collections.Generic;

namespace UTJ.BlendShapeEditor
{
    public class BlendShapeEditorWindow : EditorWindow
    {
        #region fields
        public static bool isOpen;

        Vector2 m_scrollPos;
        bool m_foldBlendShapes = true;
        BlendShapeEditorData m_data;

        static readonly int indentSize = 16;
        static readonly string dataPath = "Assets/UTJ/BlendShapeEditor/Data/BlendShapeEditorData.asset";
        #endregion



        #region callbacks

        [MenuItem("Window/Blend Shape Editor")]
        public static void Open()
        {
            var window = EditorWindow.GetWindow<BlendShapeEditorWindow>();
            window.titleContent = new GUIContent("BS Editor");
            window.Show();
        }


        private void OnEnable()
        {
            isOpen = true;

            Undo.undoRedoPerformed += OnUndoRedo;
            var ds = AssetDatabase.LoadAssetAtPath<BlendShapeEditorData>(dataPath);
            if (ds != null)
                m_data = Instantiate(ds);
            if (m_data == null)
                m_data = ScriptableObject.CreateInstance<BlendShapeEditorData>();
        }

        private void OnDisable()
        {
            isOpen = false;

            Undo.undoRedoPerformed -= OnUndoRedo;
            AssetDatabase.DeleteAsset(dataPath);
            AssetDatabase.CreateAsset(Instantiate(m_data), dataPath);
        }

        private void OnGUI()
        {
            m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);
            EditorGUILayout.BeginVertical();
            DrawBlendShapeEditor();
            EditorGUILayout.EndVertical();
            EditorGUILayout.EndScrollView();
        }

        private void OnUndoRedo()
        {
            Repaint();
        }
        #endregion


        #region impl

        public void ModifyBlendShapeData(Action<BlendShapeEditorData> op)
        {
            Undo.RecordObject(m_data, "BlendShapeEditor");
            op(m_data);
        }


        public void DrawBlendShapeEditor()
        {
            EditorGUI.BeginChangeCheck();
            var baseObject = EditorGUILayout.ObjectField("Base Mesh", m_data.baseMesh, typeof(UnityEngine.Object), true);
            if (EditorGUI.EndChangeCheck())
                m_data.baseMesh = baseObject;

            m_foldBlendShapes = EditorGUILayout.Foldout(m_foldBlendShapes, "BlendShapes");
            if (m_foldBlendShapes)
            {
                var bsData = m_data.blendShapeData;
                var evt = Event.current;

                // handle drag & drop
                if (evt.type == EventType.DragUpdated || evt.type == EventType.DragPerform)
                {
                    var dropArea = GUILayoutUtility.GetLastRect();
                    if (dropArea.Contains(evt.mousePosition))
                    {
                        DragAndDrop.visualMode = DragAndDropVisualMode.Copy;

                        if (evt.type == EventType.DragPerform)
                        {
                            DragAndDrop.AcceptDrag();
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            foreach (var obj in DragAndDrop.objectReferences)
                            {
                                var mesh = Utils.ExtractMesh(obj);
                                if (mesh != null)
                                {
                                    var bsd = new BlendShapeData();
                                    bsd.name = mesh.name;
                                    bsd.frames.Add(new BlendShapeFrameData { mesh = obj });
                                    m_data.blendShapeData.Add(bsd);
                                }
                            }
                        }
                        evt.Use();
                    }
                }


                BlendShapeData delBS = null;

                EditorGUI.indentLevel++;
                foreach (var data in bsData)
                {
                    data.fold = EditorGUILayout.Foldout(data.fold, data.name);
                    if (data.fold)
                    {
                        // handle drag & drop
                        if (evt.type == EventType.DragUpdated || evt.type == EventType.DragPerform)
                        {
                            var dropArea = GUILayoutUtility.GetLastRect();
                            if (dropArea.Contains(evt.mousePosition))
                            {
                                DragAndDrop.visualMode = DragAndDropVisualMode.Copy;

                                if (evt.type == EventType.DragPerform)
                                {
                                    DragAndDrop.AcceptDrag();
                                    Undo.RecordObject(m_data, "BlendShapeEditor");
                                    data.ClearInvalidFrames();
                                    foreach (var obj in DragAndDrop.objectReferences)
                                    {
                                        var mesh = Utils.ExtractMesh(obj);
                                        if(mesh != null)
                                            data.frames.Add(new BlendShapeFrameData { mesh = obj });
                                    }
                                    data.NormalizeWeights();
                                }
                                evt.Use();
                            }
                        }

                        EditorGUI.indentLevel++;
                        EditorGUI.BeginChangeCheck();
                        var name = EditorGUILayout.TextField("Name", data.name);
                        if (EditorGUI.EndChangeCheck())
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            data.name = name;
                        }

                        EditorGUILayout.LabelField("Frames (Weight : Mesh)");
                        EditorGUI.indentLevel++;

                        BlendShapeFrameData delFrame = null;
                        foreach (var frame in data.frames)
                        {
                            EditorGUILayout.BeginHorizontal();

                            EditorGUI.BeginChangeCheck();
                            var w = EditorGUILayout.FloatField(frame.weight, GUILayout.Width(100));
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                frame.weight = w;
                            }

                            EditorGUI.BeginChangeCheck();
                            var m = EditorGUILayout.ObjectField(frame.mesh, typeof(UnityEngine.Object), true);
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                frame.mesh = m;
                            }

                            if (GUILayout.Button("-", GUILayout.Width(20)))
                                delFrame = frame;

                            EditorGUILayout.EndHorizontal();
                        }
                        if(delFrame != null)
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            data.frames.Remove(delFrame);
                        }

                        GUILayout.BeginHorizontal();
                        EditorGUILayout.LabelField(" ");
                        if (GUILayout.Button("+", GUILayout.Width(20)))
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            data.frames.Add(new BlendShapeFrameData());
                        }
                        GUILayout.EndHorizontal();

                        EditorGUI.indentLevel--;

                        GUILayout.BeginHorizontal();
                        GUILayout.Space(indentSize * 2);
                        if (GUILayout.Button("Normalize Weights", GUILayout.Width(120)))
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            data.NormalizeWeights();
                        }
                        if (GUILayout.Button("Sort By Weights", GUILayout.Width(110)))
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            data.SortByWeights();
                        }
                        if (GUILayout.Button("Clear", GUILayout.Width(60)))
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            data.frames.Clear();
                        }
                        if (GUILayout.Button("Delete", GUILayout.Width(60)))
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            delBS = data;
                        }
                        GUILayout.EndHorizontal();

                        EditorGUI.indentLevel--;
                        EditorGUILayout.Space();
                    }
                }
                EditorGUI.indentLevel--;

                if (delBS != null)
                    bsData.Remove(delBS);

                GUILayout.Space(6);
                GUILayout.BeginHorizontal();
                GUILayout.Space(indentSize * 1);
                if (GUILayout.Button("Add BlendShape", GUILayout.Width(120)))
                {
                    Undo.RecordObject(m_data, "BlendShapeEditor");
                    var tmp = new BlendShapeData();
                    tmp.name = "NewBlendShape" + bsData.Count;
                    tmp.frames.Add(new BlendShapeFrameData());
                    bsData.Add(tmp);
                }
                if (GUILayout.Button("Clear BlendShapes", GUILayout.Width(120)))
                {
                    Undo.RecordObject(m_data, "BlendShapeEditor");
                    bsData.Clear();
                }
                GUILayout.EndHorizontal();
            }

            GUILayout.Space(18);

            GUILayout.BeginHorizontal();
            if (GUILayout.Button("Generate New Mesh", GUILayout.Width(140)))
            {
                var result = Compose();
                if (result != null)
                {
                    var go = Utils.MeshToGameObject(result, Vector3.zero, Utils.ExtractMaterials(m_data.baseMesh));
                    Selection.activeGameObject = go;
                }
            }
            if (GUILayout.Button("Update Existing Mesh", GUILayout.Width(140)))
            {
                Compose(true);
                EditorUtility.SetDirty(m_data.baseMesh);
            }
            if (GUILayout.Button("Export BaseMesh To .asset", GUILayout.Width(180)))
            {
                var baseMesh = Utils.ExtractMesh(m_data.baseMesh);
                string path = EditorUtility.SaveFilePanel("Export .asset file", "Assets", baseMesh.name, "asset");
                if (path.Length > 0)
                {
                    var dataPath = Application.dataPath;
                    if (!path.StartsWith(dataPath))
                    {
                        Debug.LogError("Invalid path: Path must be under " + dataPath);
                    }
                    else
                    {
                        path = path.Replace(dataPath, "Assets");
                        AssetDatabase.CreateAsset(Instantiate(baseMesh), path);
                        Debug.Log("Asset exported: " + path);
                    }
                }
            }
            GUILayout.EndHorizontal();

            GUILayout.Space(6);

            GUILayout.BeginHorizontal();
            if (GUILayout.Button("Save Settings", GUILayout.Width(100)))
            {
                string path = EditorUtility.SaveFilePanel("Save settings", "Assets/UTJ/BlendShapeEditor/Data", m_data.baseMesh.name, "asset");
                if (path.Length > 0)
                {
                    var dataPath = Application.dataPath;
                    if (!path.StartsWith(dataPath))
                    {
                        Debug.LogError("Invalid path: Path must be under " + dataPath);
                    }
                    else
                    {
                        path = path.Replace(dataPath, "Assets");
                        AssetDatabase.DeleteAsset(path);
                        AssetDatabase.CreateAsset(Instantiate(m_data), path);
                    }
                }
            }
            if (GUILayout.Button("Load Settings", GUILayout.Width(100)))
            {
                string path = EditorUtility.OpenFilePanel("Load settings", "Assets/UTJ/BlendShapeEditor/Data", "asset");
                if (path.Length > 0)
                {
                    var dataPath = Application.dataPath;
                    if (!path.StartsWith(dataPath))
                    {
                        Debug.LogError("Invalid path: Path must be under " + dataPath);
                    }
                    else
                    {
                        path = path.Replace(dataPath, "Assets");
                        var ds = AssetDatabase.LoadAssetAtPath<BlendShapeEditorData>(path);
                        if (ds != null)
                            m_data = Instantiate(ds);
                        else
                            Debug.LogError("failed.");
                    }
                }

            }
            GUILayout.EndHorizontal();
        }


        public Mesh Compose(bool modifyDixistingMesh = false)
        {
            var baseMesh = Utils.ExtractMesh(m_data.baseMesh);
            if(baseMesh == null)
            {
                Debug.LogError("BlendShapeEditor: Base mesh is not set");
                return null;
            }

            Mesh ret = null;
            if (modifyDixistingMesh)
            {
                ret = baseMesh;
                ret.ClearBlendShapes();
            }
            else
            {
                ret = Instantiate(baseMesh);
                ret.ClearBlendShapes();
                ret.name = baseMesh.name;
            }

            var baseVertices = baseMesh.vertices;
            var baseNormals = baseMesh.normals;
            var baseTangents = baseMesh.tangents;

            var deltaVertices = new Vector3[baseMesh.vertexCount];
            var deltaNormals = new Vector3[baseMesh.vertexCount];
            var deltaTangents = new Vector3[baseMesh.vertexCount];

            // add blend shape data
            foreach (var shape in m_data.blendShapeData)
            {
                var name = shape.name;

                foreach(var frame in shape.frames)
                {
                    var mesh = Utils.ExtractMesh(frame.mesh);
                    if(mesh == null)
                    {
                        Debug.LogError("BlendShapeEditor: Invalid target in " + name + " at weight " + frame.weight);
                    }
                    else if (mesh.vertexCount != baseMesh.vertexCount)
                    {
                        Debug.LogError("BlendShapeEditor: Invalid target (vertex count doesn't match) in " + name + " at weight " + frame.weight);
                    }
                    else
                    {
                        GenerateDelta(baseVertices, mesh.vertices, deltaVertices);
                        GenerateDelta(baseNormals, mesh.normals, deltaNormals);
                        GenerateDelta(baseTangents, mesh.tangents, deltaTangents);
                        ret.AddBlendShapeFrame(name, frame.weight, deltaVertices, deltaNormals, deltaTangents);
                    }
                }
            }
            return ret;
        }

        private static void GenerateDelta(Vector3[] from, Vector3[] to, Vector3[] dst)
        {
            var len = from.Length;
            for (int i = 0; i < len; ++i)
                dst[i] = to[i] - from[i];
        }

        private static void GenerateDelta(Vector4[] from, Vector4[] to, Vector3[] dst)
        {
            var len = from.Length;
            for (int i = 0; i < len; ++i)
                dst[i] = to[i] - from[i];
        }

        #endregion

    }
}
