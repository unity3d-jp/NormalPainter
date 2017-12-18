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
            window.OnSelectionChange();
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

        private void OnSelectionChange()
        {
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
            var baseMesh = EditorGUILayout.ObjectField("Base Mesh", m_data.baseMesh, typeof(UnityEngine.Object), true);
            if (EditorGUI.EndChangeCheck())
                m_data.baseMesh = baseMesh;

            m_foldBlendShapes = EditorGUILayout.Foldout(m_foldBlendShapes, "BlendShapes");
            if (m_foldBlendShapes)
            {
                var bsData = m_data.blendShapeData;
                BlendShapeData delBS = null;

                EditorGUI.indentLevel++;
                foreach (var data in bsData)
                {
                    data.fold = EditorGUILayout.Foldout(data.fold, data.name);
                    if (data.fold)
                    {
                        // handle drag & drop
                        Event evt = Event.current;
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
                                            data.frames.Add(new BlendShapeFrameData { mesh = mesh });
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
                            data.name = name;

                        EditorGUILayout.LabelField("Frames (Weight : Mesh)");
                        EditorGUI.indentLevel++;

                        BlendShapeFrameData delFrame = null;
                        foreach (var frame in data.frames)
                        {
                            EditorGUILayout.BeginHorizontal();

                            EditorGUI.BeginChangeCheck();
                            var w = EditorGUILayout.FloatField(frame.weight, GUILayout.Width(100));
                            if (EditorGUI.EndChangeCheck())
                                frame.weight = w;

                            EditorGUI.BeginChangeCheck();
                            var m = EditorGUILayout.ObjectField(frame.mesh, typeof(UnityEngine.Object), true);
                            if (EditorGUI.EndChangeCheck())
                                frame.mesh = m;

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
                        if (GUILayout.Button("Delete", GUILayout.Width(80)))
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            delBS = data;
                        }
                        if (GUILayout.Button("Normalize Weights", GUILayout.Width(120)))
                        {
                            Undo.RecordObject(m_data, "BlendShapeEditor");
                            data.NormalizeWeights();
                        }
                        GUILayout.EndHorizontal();

                        EditorGUI.indentLevel--;
                    }
                }
                EditorGUI.indentLevel--;

                if (delBS != null)
                    bsData.Remove(delBS);

                GUILayout.Space(6);
                GUILayout.BeginHorizontal();
                GUILayout.Space(indentSize * 1);
                if (GUILayout.Button("Add Blend Shape"))
                {
                    var tmp = new BlendShapeData();
                    tmp.name = "NewBlendShape" + bsData.Count;
                    tmp.frames.Add(new BlendShapeFrameData());
                    bsData.Add(tmp);
                }
                GUILayout.EndHorizontal();
            }

            GUILayout.Space(12);

            if (GUILayout.Button("Export"))
            {
                var result = Generate();
                if (result != null)
                {
                    Utils.MeshToGameObject(result.name, result, Vector3.zero);
                }
            }
        }


        public Mesh Generate()
        {
            var baseMesh = Utils.ExtractMesh(m_data.baseMesh);
            if(baseMesh == null)
            {
                Debug.LogError("BlendShapeEditor: Base mesh is not set");
                return null;
            }

            var ret = Instantiate(baseMesh);

            var baseVertices = baseMesh.vertices;
            var baseNormals = baseMesh.normals;
            var baseTangents = baseMesh.tangents;

            var deltaVertices = new Vector3[baseMesh.vertexCount];
            var deltaNormals = new Vector3[baseMesh.vertexCount];
            var deltaTangents = new Vector3[baseMesh.vertexCount];

            // delete existing blend shapes if needed
            {
                var del = new List<string>();
                int numBS = baseMesh.blendShapeCount;
                for(int si = 0; si < numBS; ++si)
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
                    ret.ClearBlendShapes();
                    for (int si = 0; si < numBS; ++si)
                    {
                        var name = baseMesh.GetBlendShapeName(si);
                        if (!del.Contains(name))
                        {
                            int numFrames = baseMesh.GetBlendShapeFrameCount(si);
                            for (int fi = 0; fi < numFrames; ++fi)
                            {
                                float weight = baseMesh.GetBlendShapeFrameWeight(si, fi);
                                baseMesh.GetBlendShapeFrameVertices(si, fi, deltaVertices, deltaNormals, deltaTangents);
                                ret.AddBlendShapeFrame(name, weight, deltaVertices, deltaNormals, deltaTangents);
                            }
                        }
                    }
                }
            }


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
