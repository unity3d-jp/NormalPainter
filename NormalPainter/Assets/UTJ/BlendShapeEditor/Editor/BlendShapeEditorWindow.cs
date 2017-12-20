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
            if (m_data != null)
            {
                AssetDatabase.DeleteAsset(dataPath);
                AssetDatabase.CreateAsset(Instantiate(m_data), dataPath);
            }
        }

        private void OnGUI()
        {
            if (m_data == null) return;

            m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);
            GUILayout.BeginVertical();
            DrawBlendShapeEditor();
            GUILayout.EndVertical();
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
            if (m_data != null)
            {
                Undo.RecordObject(m_data, "BlendShapeEditor");
                op(m_data);
            }
        }


        public void DrawBlendShapeEditor()
        {
            GUILayout.BeginHorizontal();
            {
                GUILayout.Label("Base Mesh", GUILayout.Width(70));
                var rect = EditorGUILayout.GetControlRect();
                var width = rect.width;
                var pos = rect.position;

                EditorGUI.BeginChangeCheck();
                var baseObject = EditorGUI.ObjectField(new Rect(pos, new Vector2(width , 16)), m_data.baseMesh, typeof(UnityEngine.Object), true);
                if (EditorGUI.EndChangeCheck())
                    m_data.baseMesh = baseObject;
            }
            if (GUILayout.Button("Find Targets", GUILayout.Width(80)))
                FindValidTargets();
            GUILayout.EndHorizontal();


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

                foreach (var data in bsData)
                {
                    GUILayout.BeginHorizontal();
                    GUILayout.Space(indentSize);
                    GUILayout.BeginVertical("Box");
                    GUILayout.BeginHorizontal();
                    data.fold = EditorGUILayout.Foldout(data.fold, data.name);
                    if (GUILayout.Button("-", GUILayout.Width(20)))
                    {
                        Undo.RecordObject(m_data, "BlendShapeEditor");
                        delBS = data;
                    }

                    GUILayout.EndHorizontal();
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

                        GUILayout.BeginHorizontal();
                        GUILayout.Space(indentSize);
                        GUILayout.BeginVertical();

                        {

                            var rect = EditorGUILayout.GetControlRect();
                            var width = rect.width;
                            var pos = rect.position;

                            EditorGUI.BeginChangeCheck();
                            EditorGUI.LabelField(new Rect(pos, new Vector2(width, 16)), "Name");
                            pos.x += 50; width -= 50;
                            var name = EditorGUI.TextField(new Rect(pos, new Vector2(width, 16)), data.name);
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                data.name = name;
                            }
                        }

                        EditorGUILayout.LabelField(new GUIContent("Frames", "Weight Mesh    Vertex Normal Tangent"));

                        GUILayout.BeginHorizontal();
                        GUILayout.Space(indentSize);
                        GUILayout.BeginVertical();

                        BlendShapeFrameData delFrame = null;
                        int numFrames = data.frames.Count;
                        int numV = 0, numN = 0, numT = 0;
                        foreach (var frame in data.frames)
                        {
                            if (frame.vertex) ++numV;
                            if (frame.normal) ++numN;
                            if (frame.tangent) ++numT;

                            GUILayout.BeginHorizontal();

                            var rect = EditorGUILayout.GetControlRect();
                            var width = rect.width;
                            var pos = rect.position;

                            EditorGUI.BeginChangeCheck();
                            var w = EditorGUI.FloatField(new Rect(pos, new Vector2(50, 16)), frame.weight);
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                frame.weight = w;
                            }

                            pos.x += 60;

                            EditorGUI.BeginChangeCheck();
                            var m = EditorGUI.ObjectField(new Rect(pos, new Vector2(width - 65, 16)), frame.mesh, typeof(UnityEngine.Object), true);
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                frame.mesh = m;
                            }

                            EditorGUI.BeginChangeCheck();
                            var v = GUILayout.Toggle(frame.vertex, "V", GUILayout.Width(25));
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                frame.vertex = v;
                            }

                            EditorGUI.BeginChangeCheck();
                            var n = GUILayout.Toggle(frame.normal, "N", GUILayout.Width(25));
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                frame.normal = n;
                            }

                            EditorGUI.BeginChangeCheck();
                            var t = GUILayout.Toggle(frame.tangent, "T", GUILayout.Width(25));
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                frame.tangent = t;
                            }

                            if (GUILayout.Button("-", GUILayout.Width(20)))
                                delFrame = frame;

                            GUILayout.EndHorizontal();
                        }
                        if(delFrame != null)
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            data.frames.Remove(delFrame);
                        }

                        GUILayout.BeginHorizontal();
                        GUILayout.FlexibleSpace();
                        if (numFrames > 0)
                        {
                            bool v = false;
                            EditorGUI.BeginChangeCheck();
                            if (numV == numFrames || numV == 0)
                                v = GUILayout.Toggle(numV == numFrames, "V", GUILayout.Width(25));
                            else
                                v = GUILayout.Toggle(numV == numFrames, "V", "ToggleMixed", GUILayout.Width(25));
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                foreach (var frame in data.frames)
                                    frame.vertex = v;
                            }

                            EditorGUI.BeginChangeCheck();
                            if (numN == numFrames || numN == 0)
                                v = GUILayout.Toggle(numN == numFrames, "N", GUILayout.Width(25));
                            else
                                v = GUILayout.Toggle(numN == numFrames, "N", "ToggleMixed", GUILayout.Width(25));
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                foreach (var frame in data.frames)
                                    frame.normal = v;
                            }

                            EditorGUI.BeginChangeCheck();
                            if (numT == numFrames || numT == 0)
                                v = GUILayout.Toggle(numT == numFrames, "T", GUILayout.Width(25));
                            else
                                v = GUILayout.Toggle(numT == numFrames, "T", "ToggleMixed", GUILayout.Width(25));
                            if (EditorGUI.EndChangeCheck())
                            {
                                Undo.RecordObject(m_data, "BlendShapeEditor");
                                foreach (var frame in data.frames)
                                    frame.tangent = v;
                            }
                        }
                        if (GUILayout.Button("+", GUILayout.Width(20)))
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            data.frames.Add(new BlendShapeFrameData());
                        }
                        GUILayout.EndHorizontal();

                        GUILayout.BeginHorizontal();
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
                        GUILayout.EndHorizontal();

                        GUILayout.EndVertical();
                        GUILayout.EndHorizontal();

                        GUILayout.EndVertical();
                        GUILayout.Space(10);
                        GUILayout.EndHorizontal();
                    }

                    GUILayout.EndVertical();
                    GUILayout.EndHorizontal();
                }

                if (delBS != null)
                    bsData.Remove(delBS);

                GUILayout.BeginHorizontal();
                GUILayout.Space(indentSize);
                GUILayout.BeginHorizontal("Box");
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("+", GUILayout.Width(20)))
                {
                    Undo.RecordObject(m_data, "BlendShapeEditor");
                    var tmp = new BlendShapeData();
                    tmp.name = "NewBlendShape" + bsData.Count;
                    tmp.frames.Add(new BlendShapeFrameData());
                    bsData.Add(tmp);
                }
                GUILayout.EndHorizontal();
                GUILayout.EndHorizontal();
            }

            GUILayout.Space(8);

            GUILayout.BeginHorizontal();
            if (GUILayout.Button("Update Existing Mesh", GUILayout.Width(140)))
            {
                Build(true);
                EditorUtility.SetDirty(m_data.baseMesh);
            }
            {
                EditorGUI.BeginChangeCheck();
                var v = GUILayout.Toggle(m_data.preserveExistingBlendShapes, "Preserve Existing BlendShapes");
                if (EditorGUI.EndChangeCheck())
                {
                    Undo.RecordObject(m_data, "BlendShapeEditor");
                    m_data.preserveExistingBlendShapes = v;
                }
            }
            GUILayout.EndHorizontal();

            GUILayout.Space(6);

            GUILayout.BeginHorizontal();
            if (GUILayout.Button("Generate New Mesh", GUILayout.Width(130)))
            {
                var result = Build();
                if (result != null)
                {
                    var go = Utils.MeshToGameObject(result, Vector3.zero, Utils.ExtractMaterials(m_data.baseMesh));
                    Selection.activeGameObject = go;
                }
            }
            if (GUILayout.Button("Export BaseMesh To .asset", GUILayout.Width(170)))
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
                            Debug.LogError("Invalid path or asset.");
                    }
                }

            }
            GUILayout.EndHorizontal();
        }

        public void FindValidTargets()
        {
            if (m_data.baseMesh == null)
            {
                Debug.LogWarning("Base mesh is not set.");
                return;
            }

            var baseMesh = Utils.ExtractMesh(m_data.baseMesh);
            if (baseMesh == null)
            {
                Debug.LogWarning("Base mesh has no valid mesh.");
                return;
            }

            var vCount = baseMesh.vertexCount;
            var set = new HashSet<UnityEngine.Object>();
            foreach (var cmp in FindObjectsOfType<SkinnedMeshRenderer>())
            {
                var mesh = cmp.sharedMesh;
                if (mesh != null && mesh.vertexCount == vCount)
                    set.Add(cmp.gameObject);
            }
            foreach (var cmp in FindObjectsOfType<MeshFilter>())
            {
                var mesh = cmp.sharedMesh;
                if (mesh != null && mesh.vertexCount == vCount)
                    set.Add(cmp.gameObject);
            }
            set.Remove(m_data.baseMesh);

            if (set.Count == 0)
            {
                Debug.Log("No valid targets in this scene.");
            }
            else
            {
                var sel = new List<UnityEngine.Object>(set);
                sel.Sort((a, b) => { return a.name.CompareTo(b.name); });
                Selection.objects = sel.ToArray();
                Debug.Log(sel.Count + " targets found.");
            }
        }


        public Mesh Build(bool updateExistingMesh = false)
        {
            var baseMesh = Utils.ExtractMesh(m_data.baseMesh);
            if(baseMesh == null)
            {
                Debug.LogError("Base mesh is not set");
                return null;
            }

            Mesh ret = null;
            if (updateExistingMesh)
            {
                ret = baseMesh;
            }
            else
            {
                ret = Instantiate(baseMesh);
                ret.name = baseMesh.name;
            }

            var baseVertices = baseMesh.vertices;
            var baseNormals = baseMesh.normals;
            var baseTangents = baseMesh.tangents;

            var deltaVertices = new Vector3[baseMesh.vertexCount];
            var deltaNormals = new Vector3[baseMesh.vertexCount];
            var deltaTangents = new Vector3[baseMesh.vertexCount];

            // delete existing blend shapes if needed
            if (m_data.preserveExistingBlendShapes)
            {
                var del = new List<string>();
                int numBS = baseMesh.blendShapeCount;
                for (int si = 0; si < numBS; ++si)
                {
                    var name = baseMesh.GetBlendShapeName(si);
                    foreach (var shape in m_data.blendShapeData)
                    {
                        if (shape.name == name)
                        {
                            del.Add(name);
                            break;
                        }
                    }
                }

                if (del.Count > 0)
                {
                    var src = Instantiate(ret);
                    ret.ClearBlendShapes();
                    for (int si = 0; si < numBS; ++si)
                    {
                        var name = src.GetBlendShapeName(si);
                        if (!del.Contains(name))
                        {
                            int numFrames = src.GetBlendShapeFrameCount(si);
                            for (int fi = 0; fi < numFrames; ++fi)
                            {
                                float weight = src.GetBlendShapeFrameWeight(si, fi);
                                src.GetBlendShapeFrameVertices(si, fi, deltaVertices, deltaNormals, deltaTangents);
                                ret.AddBlendShapeFrame(name, weight, deltaVertices, deltaNormals, deltaTangents);
                            }
                        }
                    }
                    DestroyImmediate(src);
                }
            }
            else
            {
                ret.ClearBlendShapes();
            }

            int numAdded = 0;
            // add blend shape data
            foreach (var shape in m_data.blendShapeData)
            {
                var name = shape.name;

                foreach(var frame in shape.frames)
                {
                    var mesh = Utils.ExtractMesh(frame.mesh);
                    if(mesh == null)
                    {
                        Debug.LogError("Invalid target in " + name + " at weight " + frame.weight);
                    }
                    else if (mesh.vertexCount != baseMesh.vertexCount)
                    {
                        Debug.LogError("Invalid target (vertex count doesn't match) in " + name + " at weight " + frame.weight);
                    }
                    else
                    {
                        if (frame.vertex)
                            GenerateDelta(baseVertices, mesh.vertices, deltaVertices);
                        else
                            ZeroClear(deltaVertices);

                        if (frame.normal)
                            GenerateDelta(baseNormals, mesh.normals, deltaNormals);
                        else
                            ZeroClear(deltaNormals);

                        if (frame.tangent)
                            GenerateDelta(baseTangents, mesh.tangents, deltaTangents);
                        else
                            ZeroClear(deltaTangents);

                        ret.AddBlendShapeFrame(name, frame.weight, deltaVertices, deltaNormals, deltaTangents);
                        ++numAdded;
                    }
                }
            }
            Debug.Log("Done: added " + numAdded + " frames.");
            return ret;
        }

        private static void ZeroClear(Vector3[] dst)
        {
            int len = dst.Length;
            var zero = Vector3.zero;
            for (int i = 0; i < len; ++i)
                dst[i] = zero;
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
