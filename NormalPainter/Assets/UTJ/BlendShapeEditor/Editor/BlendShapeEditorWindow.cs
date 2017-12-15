using UnityEngine;
using UnityEditor;
using System;

namespace UTJ.BlendShapeEditor
{
    [Serializable]
    public class BlendShapeData
    {
        public string name;
        public float[] weights;
        public Mesh[] targets;
    }


    public class BlendShapeEditorWindow : EditorWindow
    {
        #region fields
        public static bool isOpen;

        Vector2 m_scrollPos;
        Mesh m_active;
        BlendShapeData[] m_data;
        #endregion



        #region callbacks

        [MenuItem("Window/Blend Shape Editor")]
        public static void Open()
        {
            var window = EditorWindow.GetWindow<BlendShapeEditorWindow>();
            window.titleContent = new GUIContent("Blend Shape Editor");
            window.Show();
            window.OnSelectionChange();
        }


        private void OnEnable()
        {
            isOpen = true;
        }

        private void OnDisable()
        {
            isOpen = false;
        }

        private void OnGUI()
        {
            var tooltipHeight = 24;
            var windowHeight = position.height;

            EditorGUILayout.BeginVertical(GUILayout.Height(windowHeight - tooltipHeight));
            m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);

            // todo

            EditorGUILayout.EndScrollView();
        }

        private void OnSelectionChange()
        {
            m_active = null;

            var activeGameObject = Selection.activeGameObject;
            if (activeGameObject != null)
            {
                var mf = activeGameObject.GetComponent<MeshFilter>();
                if(mf != null)
                {
                    m_active = mf.sharedMesh;
                }
                else
                {
                    var smr = activeGameObject.GetComponent<SkinnedMeshRenderer>();
                    m_active = smr.sharedMesh;
                }
            }
            else
            {
                var ao = Selection.activeObject as Mesh;
                if (ao != null)
                    m_active = ao;
            }
            Repaint();
        }

        #endregion


        #region impl
        public static Mesh[] ExtractShapes(Mesh target, int shapeIndex)
        {
            var numFrames = target.GetBlendShapeFrameCount(shapeIndex);
            var ret = new Mesh[numFrames];

            var tmpVertices = new Vector3[target.vertexCount];
            var tmpNormals = new Vector3[target.vertexCount];
            var tmpTangents = new Vector4[target.vertexCount];
            var deltaVertices = new Vector3[target.vertexCount];
            var deltaNormals = new Vector3[target.vertexCount];
            var deltaTangents = new Vector3[target.vertexCount];

            var stripped = Instantiate(target);
            stripped.ClearBlendShapes();

            for (int f = 0; f < numFrames; ++f)
            {
                target.GetBlendShapeFrameVertices(shapeIndex, f, deltaVertices, deltaNormals, deltaTangents);
                ApplyDelta(stripped.vertices, deltaVertices, tmpVertices);
                ApplyDelta(stripped.normals, deltaNormals, tmpNormals);
                ApplyDelta(stripped.tangents, deltaTangents, tmpTangents);
                ret[f] = Instantiate(stripped);
            }

            return ret;
        }

        private static void ApplyDelta(Vector3[] from, Vector3[] delta, Vector3[] dst)
        {
            var len = from.Length;
            for (int i = 0; i < len; ++i)
                dst[i] = from[i] + delta[i];
        }

        private static void ApplyDelta(Vector4[] from, Vector3[] delta, Vector4[] dst)
        {
            var len = from.Length;
            for (int i = 0; i < len; ++i)
            {
                dst[i] = new Vector4(
                    from[i].x + delta[i].x,
                    from[i].y + delta[i].y,
                    from[i].z + delta[i].z,
                    from[i].w);
            }
        }


        public static Mesh MergeShapes(Mesh target, BlendShapeData[] shapes)
        {
            var ret = Instantiate(target);
            ret.ClearBlendShapes();

            var deltaVertices = new Vector3[target.vertexCount];
            var deltaNormals = new Vector3[target.vertexCount];
            var deltaTangents = new Vector3[target.vertexCount];

            foreach (var shape in shapes)
            {
                var lastTarget = target;
                var name = shape.name;
                var frames = shape.weights.Length;
                for (int f = 0; f < frames; ++f)
                {
                    var weight = shape.weights[f];
                    var data = shape.targets[f];
                    GenerateDelta(lastTarget.vertices, data.vertices, deltaVertices);
                    GenerateDelta(lastTarget.normals, data.normals, deltaNormals);
                    GenerateDelta(lastTarget.tangents, data.tangents, deltaTangents);
                    ret.AddBlendShapeFrame(name, weight, deltaVertices, deltaNormals, deltaTangents);

                    lastTarget = data;
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
